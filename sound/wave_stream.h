/***************************************************************************
                          wave_stream.h  -  description
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
#ifndef WAVE_STREAM_H
#define WAVE_STREAM_H

#include "sound_file_stream.h"
#include <stdio.h>

class QStringList;

class WaveStream : public SoundFileStream
{
  int header_length;
 public:
  FILE *file;

  WaveStream();
  virtual ~WaveStream();

  int open_read(const QString& filename);
  int read_header();
  size_t read_bytes(void *data, size_t length);
  size_t read_frames(void *data, size_t length);

  int open_write(const QString& filename, int freq_=44100, int channels_=2, int bits_=16);
  void write_header();
  size_t write_bytes(void *data, size_t length);
  size_t write_frames(void *data, size_t length);

  QStringList getOutputDeviceNames();

  void close();

  void jump_to_frame(size_t frame);
  void jump_back(size_t frames);
  void jump_forward(size_t frames);
};

#endif
