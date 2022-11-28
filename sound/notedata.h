/***************************************************************************
                          notedata.h  -  description
                             -------------------
    begin                : Mon May 23 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#ifndef NOTEDATA_H
#define NOTEDATA_H

#include "analysisdata.h"
#include "SmartPtr.h"
#include "array1d.h"
#include <vector>


class Channel;

/**
@author Philip McLeod
*/
class NoteData
{
  Channel *channel = NULL;
  int _startChunk = 0; //the chunk at which this note starts on
  int _endChunk = 0; //the chunk after the last one in the note
  float maxLogRMS = 0.0f; //The maximum RMS volume during the note
  float maxIntensityDB = 0.0f; //The maximum intensity volume during the note
  float maxCorrelation = 0.0f;
  float maxPurity = 0.0f;
  float _numPeriods = 0.0f;
  float _periodOctaveEstimate = 0.0f; /*< This is in terms of the periodRatio at the beginning of the note */
  float _volume = 0.0f; //A normalised volume between 0 and 1
  float _avgPitch = 0.0f;
  int loopStep = 0;  // In the vibrato-analysis, search pitchLookupSmoothed with steps of the size of loopStep
  int loopStart = 0;  // Remembers where the previous vibrato-analysis broke off
  int prevExtremumTime = 0;
  float prevExtremumPitch = 0.0f;
  enum PrevExtremum {NONE, FIRST_MAXIMUM, FIRST_MINIMUM, MAXIMUM, MINIMUM};
  PrevExtremum prevExtremum = NONE;

public:
  Array1d<float> nsdfAggregateData;
  Array1d<float> nsdfAggregateDataScaled;
  double nsdfAggregateRoof = 0.0; //keeps the sum of scalers. i.e. The highest possible aggregate value
  float firstNsdfPeriod = 0.0f;
  float currentNsdfPeriod = 0.0f;

  NoteData() { }
  NoteData(Channel *channel_);
  //NoteData(int startChunk_, int endChunk_, float logRMS_, float intensityDB_, float correlation_, float purity_);
  NoteData(Channel *channel_, int startChunk_, AnalysisData *analysisData);
  ~NoteData();

  SmartPtr<Array1d<int> > maxima;
  SmartPtr<Array1d<int> > minima;

  void    resetData();
  //int   size() { return _endChunk - _startChunk; }
  bool    isValid() { return (numChunks() > 2); }
  void    setStartChunk(int startChunk_) { _startChunk = startChunk_; }
  void    setEndChunk(int endChunk_) { _endChunk = endChunk_; }
  int     startChunk() { return _startChunk; }
  int     endChunk() { return _endChunk; }
  //void  addValues(float logRMS_, float intensityDB_, float correlation_, float purity_);
  void    addData(AnalysisData *analysisData, float periods);
  int     numChunks() { return _endChunk - _startChunk; }
  double  noteLength(); /**< in seconds */
  float   numPeriods() { return _numPeriods; }
  double  avgFreq() { return numPeriods() / noteLength(); } /**< in Hertz */
  double  avgPitch(); /**< in semi-tones */
  void    setPeriodOctaveEstimate(float periodOctaveEstimate_) { _periodOctaveEstimate = periodOctaveEstimate_; }
  float   periodOctaveEstimate() { return _periodOctaveEstimate; }
  void    addVibratoData(int chunk);
  float   volume() { return _volume; }
  void    recalcAvgPitch();
};

#endif
