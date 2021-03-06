/***************************************************************************
                          audio_stream.h  -  linux audio routines
                             -------------------
    begin                : 2003
    copyright            : (C) 2003-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <stdio.h>
#include "RtAudio.h"
#include "sound_stream.h"
#include "RingBuffer.h"

class AudioStream : public SoundStream
{
  int buffer_size; //in frames
  int num_buffers;
  RtAudio *audio;
  float *buffer;
  RingBuffer<float> flowBuffer;
  int inDevice, outDevice;
 public:

  AudioStream();
  virtual ~AudioStream();

  int bufferSize() { return buffer_size; }
  int numBuffers() { return num_buffers; }
  int totalBufferFrames() { return buffer_size * num_buffers; }
  int inTotalBufferFrames() { return totalBufferFrames(); }
  int outTotalBufferFrames() { return totalBufferFrames(); }

  void close();

  int open(int mode_, int freq_=44100, int channels_=2, int bits_=16, int buffer_size_=2048);

  //note: use the writeFloats or readFloats instead
  // These are not abstract.
  virtual size_t read_bytes(void * /*data*/, size_t /*length*/) { return 0; } //not implemented
  virtual size_t read_frames(void * /*data*/, size_t /*length*/) { return 0; } //not implemented
  virtual size_t write_bytes(void * /*data*/, size_t /*length*/) { return 0; } //not implemented
  virtual size_t write_frames(void * /*data*/, size_t /*length*/) { return 0; } //not implemented

  size_t writeFloats(float **channelData, size_t length, int ch);
  size_t readFloats(float **channelData, size_t length, int ch);
  size_t writeReadFloats(float **outChannelData, int outCh, float **inChannelData, int inCh, size_t length);
  bool isSameInOutDevice() { return inDevice == outDevice; }

  static QStringList getInputDeviceNames();
  static QStringList getOutputDeviceNames();
  static int getDeviceNumber(const char *deviceName);

};

#endif
