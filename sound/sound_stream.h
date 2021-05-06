/***************************************************************************
                          sound_stream.h  -  description
                             -------------------
    begin                : Sat Nov 27 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef SOUND_STREAM_H
#define SOUND_STREAM_H
#include <QString>

#define F_NONE         0
#define F_READ         1
#define F_WRITE        2
#define F_RDWR         3

//class FBuffer;
class QStringList;

class SoundStream
{
 public:
  int freq;
  int channels;
  int bits;
  int mode;

  SoundStream() { mode=F_NONE; }
  virtual ~SoundStream() {}

  int sample_size() { return (bits+7) / 8; }
  int frame_size() { return sample_size() * channels; }

  virtual size_t read_bytes(void *data, size_t length) = 0;
  virtual size_t read_frames(void *data, size_t length) = 0;
  virtual size_t write_bytes(void *data, size_t length) = 0;
  virtual size_t write_frames(void *data, size_t length) = 0;

  virtual size_t writeFloats(float **channelData, size_t length, int ch);
  virtual size_t readFloats(float **channelData, size_t length, int ch);
  virtual size_t writeReadFloats(float **outChannelData, int outCh, float **inChannelData, int inCh, size_t length);
};

#endif
