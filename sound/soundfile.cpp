 /***************************************************************************
                           soundfile.cpp  -  description
                              -------------------
     begin                : Sat Jul 10 2004
     copyright            : (C) 2004-2005 by Philip McLeod
     email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
  ***************************************************************************/

#include "myassert.h"
#include "soundfile.h"
#include "mystring.h"
#include "array1d.h"
#include "useful.h"
#include <algorithm>
#include "channel.h"

#include <QProgressBar>
#include "mainwindow.h"
#include <qstatusbar.h>
#include <qlabel.h>
#include <qdir.h>

#include "audio_stream.h"
#include "wave_stream.h"
#ifdef USE_SOX
#include "sox_stream.h"
#else
#if USE_OGG
#include "ogg_stream.h"
#endif
#endif

#include "QMessageBox"

//const double v8=0x7F, v16=0x7FFF, v32=0x7FFFFFFF;

typedef unsigned char byte;

SoundFile::SoundFile()
{
  filename.clear();
  filteredFilename.clear();
  stream = NULL;
  filteredStream = NULL;
  _framesPerChunk = 0;
  tempWindowBuffer = NULL;
  //tempWindowBufferDouble = NULL;
  tempWindowBufferFiltered = NULL;
  //tempWindowBufferFilteredDouble = NULL;
  _startTime = 0.0;
  _chunkNum = 0;
  _offset = 0; //Number of frame to read into file to get to time 0 (half buffer size).
  _saved = true;
  mutex = new QMutex(QMutex::Recursive);
  firstTimeThrough = true;
  _doingDetailedPitch = false;
}

SoundFile::~SoundFile()
{
  uninit();
  delete mutex;
}

//free up all the memory of everything used
void SoundFile::uninit()
{
  if(gdata->audioThread.playSoundFile() == this || gdata->audioThread.recSoundFile() == this) {
    gdata->stop();
  }

  if(stream) { delete stream; stream = NULL; }
  if(filteredStream) {
    delete filteredStream; filteredStream = NULL;
    //Delete the temporary filtered file from disk!
    if (!QFile::remove(filteredFilename)) {
        fprintf(stderr, "Error removing file %s\n", filteredFilename.toUtf8().constData());
    }
  }
  setFilename(QString::null);
  setFilteredFilename(QString::null);
  for(int j=0; j<numChannels(); j++) {
    delete channels(j);
    delete[] (tempWindowBuffer[j]-16);
    //delete[] (tempWindowBufferDouble[j]-16);
    delete[] (tempWindowBufferFiltered[j]-16);
    //delete[] (tempWindowBufferFilteredDouble[j]-16);
  }
  channels.resize(0);
  delete[] tempWindowBuffer; tempWindowBuffer = NULL;
  //delete[] tempWindowBufferDouble; tempWindowBufferDouble = NULL;
  delete[] tempWindowBufferFiltered; tempWindowBufferFiltered = NULL;
  //delete[] tempWindowBufferFilteredDouble; tempWindowBufferFilteredDouble = NULL;

  setFramesPerChunk(0);
  _startTime = 0.0;
  _chunkNum = 0;
  _offset = 0;
}

void SoundFile::setFilename(const QString& filename_)
{
  filename = filename_;
}

void SoundFile::setFilteredFilename(const QString& filteredFilename_)
{
  filteredFilename = filteredFilename_;

}

QString SoundFile::getNextTempFilename()
{
  QString tempFileFolder = gdata->qsettings->value("General/tempFilesFolder", QDir::toNativeSeparators(QDir::currentPath())).toString();
  QDir dir = QDir(tempFileFolder);
  QFileInfo fileInfo;
  QString fileName;
  bool fileExists;
  int index = 1;
  do {
    fileExists = false;
    fileName.sprintf("temp%03d.wav", index);
    fileInfo.setFile(dir, fileName);
    if(fileInfo.exists()) {
      fileExists = true;
      index++;
    }
  } while(fileExists);
  return fileInfo.absoluteFilePath();
}

bool SoundFile::openRead(const QString& filename_)
{
  uninit();
  setSaved(true);

  setFilename(filename_);
  setFilteredFilename(getNextTempFilename());
  fprintf(stderr, "Opening file: %s\n(FilteredFilename: %s)\n", filename.toUtf8().constData(), filteredFilename.toUtf8().constData());
  QFileInfo fi(filename);
  if(fi.suffix() == "wav") {
	stream = new WaveStream;
    filteredStream = new WaveStream;
  }
  else {
    fprintf(stderr, "Cannot open file of this type. %s\n", filename.toUtf8().constData());
    return false;
  }

  if(stream->open_read(filename)) {
    fprintf(stderr, "Error opening %s\n", filename.toUtf8().constData());
    return false;
  }
  
  if(filteredStream->open_write(filteredFilename, stream->freq, stream->channels, stream->bits)) {
    fprintf(stderr, "Error opening %s\n", filteredFilename.toUtf8().constData());
    delete stream; stream = NULL;
    QString s = QString("Error opening ") + filteredFilename + QString(" for writing.\nPossible cause: temp folder is read-only or disk is out of space.\nPlease select a writable Temp Folder");
    QMessageBox::warning(mainWindow, "Error", s, QMessageBox::Ok, QMessageBox::NoButton);
    mainWindow->menuPreferences();
    return false;
  }

  channels.resize(stream->channels);
  int windowSize_ = gdata->getAnalysisBufferSize(stream->freq);
  fprintf(stderr, "channels = %zu\n", numChannels());

  int stepSize_ = gdata->getAnalysisStepSize(stream->freq);
  _offset = windowSize_ / 2;
  setFramesPerChunk(stepSize_); // The user needs to be able to set this

  for(int j=0; j<numChannels(); j++) {
    channels(j) = new Channel(this, windowSize_);
    fprintf(stderr, "channel size = %zu\n", channels(j)->size());
    channels(j)->setColor(gdata->getNextColor());
  }
  myTransforms.init(windowSize_, 0, stream->freq, gdata->doingEqualLoudness());

  fprintf(stderr, "----------Opening------------\n");
  fprintf(stderr, "filename = %s\n", filename_.toUtf8().constData());
  fprintf(stderr, "rate = %d\n", rate());
  fprintf(stderr, "channels = %zu\n", numChannels());
  fprintf(stderr, "bits = %d\n", bits());
  fprintf(stderr, "windowSize = %d\n", windowSize_);
  fprintf(stderr, "stepSize = %d\n", stepSize_);
  fprintf(stderr, "-----------------------------\n");

  //setup the tempChunkBuffers
  tempWindowBuffer = new float *[numChannels()];
  //tempWindowBufferDouble = new double *[numChannels()];
  tempWindowBufferFiltered = new float *[numChannels()];
  //tempWindowBufferFilteredDouble = new double *[numChannels()];
  for(int c=0; c<numChannels(); c++) {
    tempWindowBuffer[c] = (new float[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    //tempWindowBufferDouble[c] = (new double[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    tempWindowBufferFiltered[c] = (new float[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    //tempWindowBufferFilteredDouble[c] = (new double[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
  }

  _doingDetailedPitch = gdata->doingDetailedPitch();

  return true;
}

bool SoundFile::openWrite(const QString& filename_, int rate_, int channels_, int bits_, int windowSize_, int stepSize_)
{
  uninit();
  setSaved(false);

  _offset = windowSize_ / 2;
  fprintf(stderr, "--------Recording------------\n");
  fprintf(stderr, "filename = %s\n", filename_.toUtf8().constData());
  fprintf(stderr, "rate = %d\n", rate_);
  fprintf(stderr, "channels = %d\n", channels_);
  fprintf(stderr, "bits = %d\n", bits_);
  fprintf(stderr, "windowSize = %d\n", windowSize_);
  fprintf(stderr, "stepSize = %d\n", stepSize_);
  fprintf(stderr, "-----------------------------\n");

  setFramesPerChunk(stepSize_);

  setFilename(filename_);
  setFilteredFilename(getNextTempFilename());
  
  stream = new WaveStream;
  filteredStream = new WaveStream;
  if(stream->open_write(filename, rate_, channels_, bits_)) {
    fprintf(stderr, "Error opening %s for writting\n", filename.toUtf8().constData());
    delete stream; stream = NULL;
    delete filteredStream; filteredStream = NULL;
    QString s = QString("Error opening ") + QString(filename) + QString(" for writing.\nPossible cause: temp folder is read-only or disk is out of space.\nPlease select a writable Temp Folder");
    QMessageBox::warning(mainWindow, "Error", s, QMessageBox::Ok, QMessageBox::NoButton);
    mainWindow->menuPreferences();
    return false;
  }
  if(filteredStream->open_write(filteredFilename.toUtf8(), rate_, channels_, bits_)) {
    fprintf(stderr, "Error opening %s for writting\n", filteredFilename.toUtf8().constData());
    delete stream; stream = NULL;
    delete filteredStream; filteredStream = NULL;
    QString s = QString("Error opening ") + QString(filteredFilename.toUtf8()) + QString(" for writing.\nPossible cause: temp folder is read-only or disk is out of space.\nPlease select a writable Temp Folder");
    QMessageBox::warning(mainWindow, "Error", s, QMessageBox::Ok, QMessageBox::NoButton);
    mainWindow->menuPreferences();
    return false;
  }

  channels.resize(stream->channels);
  fprintf(stderr, "channels = %zu\n", numChannels());
  for(int j=0; j<numChannels(); j++) {
    channels(j) = new Channel(this, windowSize_);
    fprintf(stderr, "channel size = %zu\n", channels(j)->size());
    channels(j)->setColor(gdata->getNextColor());
  }
  myTransforms.init(windowSize_, 0, stream->freq, gdata->doingEqualLoudness());

  //setup the tempChunkBuffers
  tempWindowBuffer = new float*[numChannels()];
  //tempWindowBufferDouble = new double*[numChannels()];
  tempWindowBufferFiltered = new float*[numChannels()];
  //tempWindowBufferFilteredDouble = new double*[numChannels()];
  for(int c=0; c<numChannels(); c++) {
    tempWindowBuffer[c] = (new float[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    //tempWindowBufferDouble[c] = (new double[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    tempWindowBufferFiltered[c] = (new float[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
    //tempWindowBufferFilteredDouble[c] = (new double[bufferSize()+16]) + 16; //array ranges from -16 to bufferSize()
  }

  _doingDetailedPitch = gdata->doingDetailedPitch();

  return true;
}

void SoundFile::rec2play()
{
  stream->close();
  stream->open_read(filename);
  filteredStream->close();
  filteredStream->open_read(filteredFilename);
  jumpToChunk(totalChunks());
  fprintf(stderr, "filteredFilename = %s\n", filteredFilename.toUtf8().constData());
  fprintf(stderr, "totalChunks = %d\n", totalChunks());
}

void SoundFile::close()
{
  uninit();
}
  
/** Lock the mutex's of all the channels */
void SoundFile::lock()
{
  mutex->lock();
  for(int j=0; j<numChannels(); j++)
    channels(j)->lock();
}

/** Unlock the mutex's of all the channels */
void SoundFile::unlock()
{
  for(int j=0; j<numChannels(); j++)
    channels(j)->unlock();
  mutex->unlock();
}

/** Plays one chunk of sound to the associated stream
  @return true on success. false for end of file or error
*/
bool SoundFile::playChunk()
{
  if(!setupPlayChunk()) return false;
  size_t ret = blockingWrite(gdata->audio_stream, tempWindowBuffer, framesPerChunk());
  if(ret < framesPerChunk()) {
    fprintf(stderr, "Error writing to audio device\n");
  }
  return true;
}

void SoundFile::applyEqualLoudnessFilter(size_t n)
{
  int c, j;
  for(c=0; c<numChannels(); c++) {
    channels(c)->highPassFilter->filter(tempWindowBuffer[c], tempWindowBufferFiltered[c], n);
    for(j=0; j<n; j++) tempWindowBufferFiltered[c][j] = bound(tempWindowBufferFiltered[c][j], -1.0f, 1.0f);
  }
}

//void SoundFile::initRecordingChunk()
void SoundFile::recordChunk(size_t n)
{
  size_t ret = blockingRead(gdata->audio_stream, tempWindowBuffer, n);
  if(ret < n) {
    fprintf(stderr, "Data lost in reading from audio device\n");
  }
  
  finishRecordChunk(n);
}

void SoundFile::finishRecordChunk(size_t n)
{
  if(equalLoudness()) applyEqualLoudnessFilter(n);

  FilterState filterState;

  lock(); 
  for(int c=0; c<numChannels(); c++) {
    channels(c)->shift_left(n);
    toChannelBuffer(c, n);
    channels(c)->processNewChunk(&filterState);
  }
  unlock();

  size_t ret = blockingWrite(stream, tempWindowBuffer, n);
  if(ret < n) fprintf(stderr, "Error writing to disk\n");
  if(equalLoudness()) ret = blockingWrite(filteredStream, tempWindowBufferFiltered, n);
  if(ret < n) fprintf(stderr, "Error writing to disk\n");

  setCurrentChunk(currentStreamChunk());
}

bool SoundFile::setupPlayChunk() {
  int c;
  int n = framesPerChunk();
  size_t ret = blockingRead(stream, tempWindowBuffer, n);
  if(equalLoudness()) ret = blockingRead(filteredStream, tempWindowBufferFiltered, n);
  if(ret < n) return false;

  lock();
  for(c=0; c<numChannels(); c++) {
    channels(c)->shift_left(n);
    toChannelBuffer(c, n);
    if(gdata->doingActive() && channels(c) == gdata->getActiveChannel()) {
      channels(c)->processChunk(currentChunk()+1);
    }
  }
  setCurrentChunk(currentStreamChunk());
  unlock();
  return true;
}

/** Plays one chunk of sound to the associated stream
  @return true on success. false for end of file or error
*/
//Note: static
bool SoundFile::playRecordChunk(SoundFile *play, SoundFile *rec)
{
  int n = rec->framesPerChunk();
  myassert(n == play->framesPerChunk());
  myassert(rec->numChannels() == play->numChannels());

  play->setupPlayChunk();

  size_t ret = blockingWriteRead(gdata->audio_stream, play->tempWindowBuffer, int(play->numChannels()), rec->tempWindowBuffer, int(rec->numChannels()), n);
  if(ret < n) {
    fprintf(stderr, "Error writing/reading to audio device\n");
  }

  rec->finishRecordChunk(n);
  return true;
}

/** Reads framesPerChunk frames from the SoundStream s
    into the end of the channel buffers. I.e losing framesPerChunk
    frames from the beginning of the channel buffers.
    If s is NULL (the defult) the the file stream is used
  @param s The SoundStream to read from or NULL (the default) the current file stream is used    
  @return The number of frames actually read ( <= framesPerChunk() )
*/
size_t SoundFile::readChunk(size_t n)
{
  int c;
  
  size_t ret = blockingRead(stream, tempWindowBuffer, n);
  if(equalLoudness()) {
    applyEqualLoudnessFilter(n);
    ret = blockingWrite(filteredStream, tempWindowBufferFiltered, ret);
  }
  if(ret < n) return ret;
  
  FilterState filterState;
  
  lock();

  for(c=0; c<numChannels(); c++) {
    channels(c)->shift_left(n);
    toChannelBuffer(c, n);
    channels(c)->processNewChunk(&filterState);
  }
  setCurrentChunk(currentStreamChunk());
  unlock();
  return ret;
}

/** Waits until there is n frames of data to read from s,
  Then puts the data in buffer.
  @param s The SoundStream to read from
  @param buffer The chunk buffer to put the data into. (s->channels of at least n length)
    This is padded with zeros (up to n) if there is less than n frames read.
  @param n The number of frames to read
  @return The number of frames read.
*/
size_t SoundFile::blockingRead(SoundStream *s, float **buffer, size_t n)
{ 
  int c;
  myassert(s);
  myassert(n >= 0 && n <= bufferSize());
  size_t ret = s->readFloats(buffer, n, int(numChannels()));
  if(ret < n) { //if not all bytes are read
    for(c=0; c<numChannels(); c++) { //set remaining bytes to zeros
      std::fill(buffer[c]+ret, buffer[c]+n, 0.0f);
    }
  }
  return ret;
}

/** Writes n frames of data to s from buffer.
  @param s The SoundStream to write to
  @param buffer The chunk buffer data to use. (s->channels of at least n length)
  @param n The number of frames to write
  @return The number of frames written.
*/
size_t SoundFile::blockingWrite(SoundStream *s, float **buffer, size_t n)
{
  if(s == NULL) return n;
  return s->writeFloats(buffer, n, int(numChannels()));
}

/** Writes n frames of data to s and reads n frames of data from s
If less than n frams are read, the remaining buffer is filled with zeros
@param s The SoundStream to write to then read from
@param writeBuffer The data that is to be written to the stream
@param readBuffer The data that is read back from the stream
@param n The amount of data in each of the buffers. Note: They must be the same size
@param ch The number of channels
*/
//Note: static
size_t SoundFile::blockingWriteRead(SoundStream *s, float **writeBuffer, int writeCh, float **readBuffer, int readCh, size_t n)
{
  int c;
  myassert(s);
  //myassert(n >= 0 && n <= bufferSize());
  size_t ret = s->writeReadFloats(writeBuffer, writeCh, readBuffer, readCh, n);
  if(ret < n) { //if not all bytes are read
    for(c=0; c<readCh; c++) { //set remaining bytes to zeros
      std::fill(readBuffer[c]+ret, readBuffer[c]+n, 0.0f);
    }
  }
  return ret;
}

/**
  @param c Channel number
  @param n Number of frames to copy from tempWindowBuffer into channel
*/
void SoundFile::toChannelBuffer(int c, size_t n)
{
  std::copy(tempWindowBuffer[c], tempWindowBuffer[c]+n, channels(c)->end() - n);
  if(equalLoudness()) {
    std::copy(tempWindowBufferFiltered[c], tempWindowBufferFiltered[c]+n, channels(c)->filteredInput.end() - n);
  }
}

void SoundFile::toChannelBuffers(size_t n)
{
  int c;
  lock();
  myassert(n >= 0 && n <= bufferSize());
  for(c=0; c<numChannels(); c++) {
    channels(c)->shift_left(n);
    toChannelBuffer(c, n);
  }
  unlock();
}

size_t SoundFile::readN(size_t n)
{
  myassert(n >= 0 && n <= bufferSize());
  size_t ret = blockingRead(stream, tempWindowBuffer, n);
  if(equalLoudness()) ret = blockingRead(filteredStream, tempWindowBufferFiltered, n);
  toChannelBuffers(n);
  return ret;
}

/** Process the chunks for all the channels
   Increment the chunkNum.
*/
void SoundFile::processNewChunk()
{
  FilterState filterState;
  for(int j=0; j<numChannels(); j++) {
    channels(j)->lock();
    channels(j)->processNewChunk(&filterState);
    channels(j)->unlock();
  }
}


/** Preprocess the whole sound file,
   by looping through and processing every chunk in the file.
   A progress bar is displayed in the toolbar, because this can
   be time consuming.
*/
void SoundFile::preProcess()
{
  gdata->setDoingActiveAnalysis(true);
  myassert(firstTimeThrough == true);
  readChunk(bufferSize() - offset());

  // Create a progress bar in the status bar to tell the user we're preprocessing
  MainWindow* theMainWindow = mainWindow;
  QStatusBar *theStatusBar = theMainWindow->statusBar();
  QLabel *message = new QLabel("Preprocessing data:", theStatusBar);
  message->setObjectName("message");

  QProgressBar *progress = new QProgressBar(theStatusBar);
  progress->setObjectName("progress bar");
  progress->setMaximum(int(stream->totalFrames() / framesPerChunk()));
  progress->setValue(0);
  progress->setMaximumHeight(16);


  theStatusBar->addWidget(message);
  theStatusBar->addWidget(progress);

  message->show();
  progress->show();

  int frameCount = 1;
  int updateInterval = MAX(1, progress->maximum() / 50); // We'll update 50 times only

  while(readChunk(framesPerChunk()) == framesPerChunk()) { // put data in channels
    //printf("pos = %d\n", stream->pos);
    frameCount++;

    if (frameCount % updateInterval == 0) {
         progress->setValue(progress->value() + updateInterval);
         qApp->processEvents();
         frameCount = 1;
    }
  }
  //printf("totalChunks=%d\n", totalChunks());
  //printf("currentChunks=%d\n", currentChunk());
  filteredStream->close();
  filteredStream->open_read(filteredFilename);
  jumpToChunk(0);


  progress->setValue(progress->maximum());
  theStatusBar->removeWidget(progress);
  theStatusBar->removeWidget(message);
  delete progress;
  delete message;

  gdata->setDoingActiveAnalysis(false);
  firstTimeThrough = false;
  //printf("freqLookup.size()=%d\n", channels(0)->freqLookup.size());
}

//shift all the channels data left by n frames
void SoundFile::shift_left(int n)
{
  for(int j=0; j<numChannels(); j++) {
    channels(j)->shift_left(n);
  }
}

void SoundFile::jumpToChunk(int chunk)
{
  int c;
  lock();
  if(chunk * framesPerChunk() < offset()) {
    size_t pos = offset() - chunk * framesPerChunk();
    stream->jump_to_frame(0);
    if(equalLoudness()) filteredStream->jump_to_frame(0);
    for(c=0; c<numChannels(); c++)
      channels(c)->reset();
    readN(bufferSize() - pos);
  } else {
    size_t pos = chunk * framesPerChunk() - offset();
    stream->jump_to_frame(pos);
    if(equalLoudness()) filteredStream->jump_to_frame(pos);
    readN(bufferSize());
  }
  setCurrentChunk(chunk);
  
  unlock();
}

size_t SoundFile::bufferSize() {
  myassert(!channels.isEmpty());
  return channels(0)->size();
}

int SoundFile::totalChunks() {
  myassert(!channels.isEmpty());
  return channels(0)->totalChunks();
}

bool SoundFile::inFile()
{
  int c = currentChunk();
  return (c >= 0 && c < totalChunks());
}
