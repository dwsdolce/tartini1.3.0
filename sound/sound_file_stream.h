/***************************************************************************
                          sound_file_stream.h  -  description
                             -------------------
    begin                : Sat Jul 10 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@pmcleod.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef SOUND_FILE_STREAM_H
#define SOUND_FILE_STREAM_H

#include "sound_stream.h"
#include <stdio.h>

class SoundFileStream : public SoundStream
{
protected:
  size_t _total_frames;
  size_t _pos; //in frames

  void setPos(size_t newPos) { _pos = newPos; }
public:

  SoundFileStream() { _total_frames=-1; _pos=0; };
  virtual ~SoundFileStream() {};

  size_t data_length() { return totalFrames() * frame_size(); }
  size_t totalFrames() { return _total_frames; }
  size_t pos() { return _pos; }
  
  virtual int open_read(const QString&/*filename*/) = 0;

  virtual size_t read_bytes(void* /*data*/, size_t /*length*/) = 0;
  virtual size_t read_frames(void* /*data*/, size_t /*length*/) = 0;

  virtual int open_write(const QString& /*filename*/, int /*freq_*/ = 44100, int /*channels_*/ = 2, int /*bits_*/ = 16) = 0;

  virtual size_t write_bytes(void* /*data*/, size_t /*length*/) = 0;
  virtual size_t write_frames(void* /*data*/, size_t /*length*/) = 0;

  virtual void close() {};

  virtual void jump_to_frame(size_t /*frame*/) {};
  virtual void jump_back(size_t /*frames*/) {};
  virtual void jump_forward(size_t /*frames*/) {};
};

#endif
