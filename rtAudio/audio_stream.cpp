/***************************************************************************
                          audio_stream.cpp  -  joining to rtAudio audio routines
                             -------------------
    begin                : 2006
    copyright            : (C) 2006-2006 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Please read LICENSE.txt for details.
 ***************************************************************************/
#include "audio_stream.h"
#include "gdata.h"

AudioStream::AudioStream()
{
  buffer_size = 1024;
  num_buffers = 0;
  audio = NULL;
  buffer = NULL;
  flowBuffer.setAutoGrow(true);
  inDevice = outDevice = 0;
}

AudioStream::~AudioStream()
{
  close();
  if (audio) delete audio;
}

void AudioStream::close()
{
  if (audio) {
    audio->stopStream();
    audio->closeStream();
  }
}

int AudioStream::open(int mode_, int freq_, int channels_, int /*bits_*/, int buffer_size_)
{
  mode = mode_;
  freq = freq_;
  channels = channels_;
  bits = 32; //bits_; //ignored, just use floats and let rtAudio do the conversion
  buffer_size = buffer_size_;
  num_buffers = 4;

  QStringList inNames = getInputDeviceNames();
  QString audioInputQString = gdata->qsettings->value("Sound/soundInput", "Default").toString();
  QByteArray audioInputQBA = audioInputQString.toUtf8();
  const char* audioInput = audioInputQBA.constData();
  inDevice = getDeviceNumber(audioInput);

  QStringList outNames = getOutputDeviceNames();
  QString audioOutputQString = gdata->qsettings->value("Sound/soundOutput", "Default").toString();
  QByteArray audioOutputQBA = audioOutputQString.toUtf8();
  const char* audioOutput = audioOutputQBA.constData();
  outDevice = getDeviceNumber(audioOutput);

  fprintf(stderr, "Input Device %d: %s\n", inDevice, audioInput);
  fprintf(stderr, "Output Device %d: %s\n", outDevice, audioOutput);

  try {
    if (mode == F_READ) {
      audio = new RtAudio(0, 0, inDevice, channels, RTAUDIO_FLOAT32, freq, &buffer_size, num_buffers);
    } else if (mode == F_WRITE) {
      audio = new RtAudio(outDevice, channels, 0, 0, RTAUDIO_FLOAT32, freq, &buffer_size, num_buffers);
    } else if (mode == F_RDWR) {
      audio = new RtAudio(outDevice, channels, inDevice, channels, RTAUDIO_FLOAT32, freq, &buffer_size, num_buffers);
    } else {
      fprintf(stderr, "No mode selected\n");
      return -1;
    }
  } catch (RtError& error) {
    error.printMessage();
    audio = NULL;
    return -1;
  }

  if (buffer_size != buffer_size_) {
    fprintf(stderr, "Warning: Got buffer_size of %d instead of %d\n", buffer_size, buffer_size_);
  }

  try {
    buffer = (float*)audio->getStreamBuffer(); // Get a pointer to the stream buffer
    audio->startStream();
  } catch (RtError& error) {
    error.printMessage();
    delete audio;
    return -1;
  }

  return 0;
}

size_t AudioStream::writeFloats(float** channelData, size_t length, int ch)
{
  float* bufPtr = buffer;
  int c;
  size_t j;
  if (length == buffer_size) {
    for (j = 0; j < length; j++) {
      for (c = 0; c < ch; c++) {
        *bufPtr++ = channelData[c][j];
      }
    }

    // Trigger the output of the data buffer
    try {
      audio->tickStream();
    } catch (RtError& error) {
      error.printMessage();
      return 0;
    }
  } else {
    for (j = 0; j < length; j++) {
      for (c = 0; c < ch; c++) {
        flowBuffer.put(channelData[c][j]);
      }
    }
    while (flowBuffer.size() >= buffer_size * ch) {
      size_t recieved = flowBuffer.get(buffer, buffer_size * ch);
      if (recieved != buffer_size * ch) fprintf(stderr, "AudioStream::writeFloats: Error recieved != buffer_size*ch\n");
      // Trigger the output of the data buffer
      try {
        audio->tickStream();
      } catch (RtError& error) {
        error.printMessage();
        return 0;
      }
    }
  }
  return length;
}

size_t AudioStream::readFloats(float** channelData, size_t length, int ch)
{
  float* bufPtr = buffer;
  int c;
  size_t j;
  if (length == buffer_size) {

    // Trigger the input of the data buffer
    try {
      audio->tickStream();
    } catch (RtError& error) {
      error.printMessage();
      return 0;
    }

    for (j = 0; j < length; j++) {
      for (c = 0; c < ch; c++) {
        channelData[c][j] = *bufPtr++;
      }
    }
  } else {
    fprintf(stderr, "Error reading floats\n");
  }
  return length;
}

/**
This requires that inChannelData and outChannelData have the same number of channels
 and the same length, and this length is the same size as the AudioStream buffer
@param outChannelData The data to write out to the sound card
@param inChannelData The data read back from the sound card
@param length The amount of data per channel. This has to be the same for in and out data
@param ch The number of channels. This has to be the same of in and out data
*/
size_t AudioStream::writeReadFloats(float** outChannelData, int outCh, float** inChannelData, int inCh, size_t length)
{
  float* bufPtr = buffer;
  int c;
  size_t j;

  //put the outChannelData into the Audio buffer to be written out
  if (length == buffer_size) {
    for (j = 0; j < length; j++) {
      for (c = 0; c < outCh; c++) {
        *bufPtr++ = outChannelData[c][j];
      }
    }

    // Trigger the input/output of the data buffer
    try {
      audio->tickStream();
    } catch (RtError& error) {
      error.printMessage();
      return 0;
    }

    //get the new data from the Audio buffer and put it in inChannelData
    bufPtr = buffer;
    for (j = 0; j < length; j++) {
      for (c = 0; c < inCh; c++) {
        inChannelData[c][j] = *bufPtr++;
      }
    }
  } else {
    fprintf(stderr, "Error: audio buffer length is a different size than the data chunk buffer\n");
    fprintf(stderr, "Not supported on Read/Write yet. Try changing the buffer size in preferences\n");
  }
  //printf("done\n"); fflush(stdout);
  return length;
}

QStringList AudioStream::getInputDeviceNames()
{
  QStringList toReturn;
  toReturn << "Default";

  RtAudio* tempAudio = NULL;
  try {
    tempAudio = new RtAudio();
  } catch (RtError& error) {
    error.printMessage();
    return toReturn;
  }

  int numDevices = tempAudio->getDeviceCount(); // Determine the number of devices available

  // Scan through devices for various capabilities
  RtAudioDeviceInfo info;
  for (int i = 1; i <= numDevices; i++) {

    try {
      info = tempAudio->getDeviceInfo(i);
    } catch (RtError& error) {
      error.printMessage();
      break;
    }

    if (info.inputChannels > 0) toReturn << info.name.c_str();
  }

  delete tempAudio; // Clean up
  //toReturn += gdata->qsettings->value("Sound/soundInput", "/dev/dsp").toString();
  return toReturn;
}

QStringList AudioStream::getOutputDeviceNames()
{
  QStringList toReturn;
  toReturn << "Default";

  RtAudio* tempAudio = NULL;
  try {
    tempAudio = new RtAudio();
  } catch (RtError& error) {
    error.printMessage();
    return toReturn;
  }

  int numDevices = tempAudio->getDeviceCount(); // Determine the number of devices available

  // Scan through devices for various capabilities
  RtAudioDeviceInfo info;
  for (int i = 1; i <= numDevices; i++) {

    try {
      info = tempAudio->getDeviceInfo(i);
    } catch (RtError& error) {
      error.printMessage();
      break;
    }

    if (info.outputChannels > 0) toReturn << info.name.c_str();
  }

  delete tempAudio; // Clean up
  //toReturn += gdata->qsettings->value("Sound/soundOutput", "/dev/dsp").toString();
  return toReturn;
}

/** @param deviceName The name of a device as given from get*DeviceNames
  @return The device number that matches the name, or -1 if the device name is not found
*/
int AudioStream::getDeviceNumber(const char* deviceName)
{
  int deviceNumber = -1;
  if (strcmp("Default", deviceName) == 0) return 0;

  RtAudio* tempAudio = NULL;
  try {
    tempAudio = new RtAudio();
  } catch (RtError& error) {
    error.printMessage();
    return -1;
  }

  int numDevices = tempAudio->getDeviceCount(); // Determine the number of devices available

  // Scan through devices for various capabilities
  RtAudioDeviceInfo info;
  for (int i = 1; i <= numDevices; i++) {

    try {
      info = tempAudio->getDeviceInfo(i);
    } catch (RtError& error) {
      error.printMessage();
      break;
    }

    if (strcmp(info.name.c_str(), deviceName) == 0) {
      deviceNumber = i; break;
    }
  }

  delete tempAudio; // Clean up
  //toReturn += gdata->qsettings->value("Sound/soundOutput", "/dev/dsp").toString();
  return deviceNumber;
}
