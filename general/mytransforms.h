/***************************************************************************
                          mytransforms.h  -  description
                             -------------------
    begin                : Sat Dec 11 2004
    copyright            : (C) 2004 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#ifndef MYTRANSFORMS_H
#define MYTRANSFORMS_H

#include <fftw3.h>
#include "analysisdata.h"
#include "fast_smooth.h"

class Channel;
class Filter;

class MyTransforms
{
  int n, k, size; //n = size of data, k = size of padding for autocorrelation, size = n+k
  fftwf_plan planDataTime2FFT, planDataFFT2Time;
  fftwf_plan planAutocorrTime2FFT, planAutocorrFFT2Time;
  float *dataTemp;
  float *dataTime, *dataFFT;
  float *autocorrTime, *autocorrFFT;
  float *hanningCoeff, hanningScalar;
  float *harmonicsAmpLeft, *harmonicsPhaseLeft;
  float *harmonicsAmpCenter, *harmonicsPhaseCenter;
  float *harmonicsAmpRight, *harmonicsPhaseRight;
  bool beenInit;
  double freqPerBin;
  //float threshold;
  double rate;
  int numHarmonics;
  fast_smooth *fastSmooth;
  
 public:
  bool equalLoudness;
  
  MyTransforms();
  virtual ~MyTransforms();

  void init(int n_, int k_, double rate_, /*float threshold_=0.9, */bool equalLoudness_=false, int numHarmonics_=40);
  void uninit();
  double autocorr(float *input, float *output);
  double autoLogCorr(float *input, float *output);
  double asdf(float *input, float *output);
  double nsdf(float *input, float *output);
  static size_t findNSDFMaxima(float *input, size_t len, std::vector<size_t> &maxPositions);
  static size_t findNSDFsubMaximum(float *input, size_t len, float threshold);
  void calculateAnalysisData(int chunk, Channel *ch/*, float threshold*/);

  void doChannelDataFFT(Channel *ch, float *curInput, int chunk);
  void calcHarmonicAmpPhase(float *harmonicAmp, float *harmonicPhase, int binsPerHarmonic);
  void doHarmonicAnalysis(float *input, AnalysisData &analysisData, double period);

  float get_fine_clarity_measure(double period);
  double get_max_note_change(float *input, double period);
  void applyHanningWindow(float *d);
  static double calcFreqCentroid(float *buffer, size_t len);
  static double calcFreqCentroidFromLogMagnitudes(float *buffer, size_t len);
};
  
#endif
