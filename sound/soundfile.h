/***************************************************************************
                          soundfile.h  -  description
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

#ifndef SOUND_FILE
#define SOUND_FILE

#include "array1d.h"
#include "sound_file_stream.h"
#include "gdata.h"
#include "mytransforms.h"
#include "mainwindow.h"

class Channel;

extern const double v8, v16, v32;

class SoundFile
{
//private:
protected:
  int _chunkNum;
  int _framesPerChunk; /**< The number of samples to move every chunk */
  float **tempWindowBuffer; //array is indexed from -16 !!
  float **tempWindowBufferFiltered; //array is indexed from -16 !!
  double _startTime;
  size_t _offset;
  bool _saved;
  QMutex *mutex;
  bool _doingDetailedPitch;

  size_t blockingRead(SoundStream *s, float **buffer, size_t n); //low level
  size_t blockingWrite(SoundStream *s, float **buffer, size_t n); //low level
  static size_t blockingWriteRead(SoundStream *s, float **writeBuffer, int writeCh, float **readBuffer, int readCh, size_t n);
  void toChannelBuffers(size_t n); //low level
  void toChannelBuffer(int c, size_t n);
public:
  QString filename;
  QString filteredFilename;
  SoundFileStream *stream; /**< Pointer to the file's SoundFileStream */
  SoundFileStream *filteredStream; /**< Pointer to the file's filtered SoundFileStream */
  Array1d<Channel*> channels; /**< The actual sound data is stored seperately for each channel */
  MyTransforms myTransforms;
  bool firstTimeThrough;
  
  SoundFile();
  ~SoundFile();
  void uninit();
  void setFilename(const QString &filename_);
  void setFilteredFilename(const QString& filteredFilename_);
  QString getNextTempFilename();
  size_t numChannels() { return channels.size(); }
  bool openRead(const QString& filename_);
  bool openWrite(const QString& ilename_, int rate_, int channels_, int bits_, int windowSize_, int stepSize_);
  void preProcess();
  void rec2play();
  void close();

  void lock();
  void unlock();
  
  size_t readN(size_t n);
  bool playChunk();
  bool setupPlayChunk();
  void recordChunk(size_t n);
  void finishRecordChunk(size_t n);
  static bool playRecordChunk(SoundFile *playSoundFile, SoundFile *recSoundFile);
  void applyEqualLoudnessFilter(size_t n);
  size_t readChunk(size_t n);
  void processNewChunk();
  double startTime() { return _startTime; }
  void setStartTime(double t) { _startTime = t; }
  int currentStreamChunk() { return int((stream->pos() - offset()) / framesPerChunk()); }
  int currentRawChunk() { return _chunkNum; }
  int currentChunk() { return _chunkNum; }
  void setCurrentChunk(int x) { _chunkNum = x; }
  void incrementChunkNum() { _chunkNum++; }

  size_t offset() { return _offset; }
  double timePerChunk() { return double(framesPerChunk()) / double(rate()); }
  int chunkAtTime(double t) { return toInt(chunkFractionAtTime(t)); } //this is not bounded
  double chunkFractionAtTime(double t) { return t / timePerChunk(); } //this is not bounded
  double timeAtChunk(int chunk) { return double(chunk) * timePerChunk(); }
  double timeAtCurrentChunk() { return timeAtChunk(currentChunk()); }
  int chunkAtCurrentTime() { return chunkAtTime(gdata->view->currentTime()); }
  void shift_left(int n);
  void jumpToChunk(int chunk);
  void jumpToTime(double t) { jumpToChunk(chunkAtTime(t)); }
  
  int rate() { myassert(stream != NULL); return stream->freq; }
  int bits() { myassert(stream != NULL); return stream->bits; }
  int framesPerChunk() { return _framesPerChunk; }
  void setFramesPerChunk(int stepSize) { _framesPerChunk = stepSize; }
  size_t bufferSize();
  int totalChunks();
  bool inFile(); /**< Returns false if past end of file */

  bool saved() { return _saved; }
  void setSaved(bool newState) { _saved = newState; }
  bool equalLoudness() { return myTransforms.equalLoudness; }
  bool doingDetailedPitch() { return _doingDetailedPitch; }
};

#endif
