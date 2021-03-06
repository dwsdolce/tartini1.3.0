/***************************************************************************
                          useful.h  -  Generic useful functions
                             -------------------
    begin                : 2002
    copyright            : (C) 2002-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef MUSICNOTES_H
#define MUSICNOTES_H

#include "useful.h"
#include "array1d.h"
#include "gdata.h"

/** Converts the frequencies freq (in hertz) into their note number on the midi scale
    i.e. the number of semi-tones above C0
    Note: The note's pitch will contain its fractional part
    Reference = http://www.borg.com/~jglatt/tutr/notenum.htm
  @param freq The frequency in Hz
  @return The pitch in fractional part semitones from the midi scale.
*/
inline double freq2pitch(double freq)
{
#ifdef log2
	return -36.3763165622959152488 + 12.0*log2(freq);
#else
	return -36.3763165622959152488 + 39.8631371386483481*log10(freq);
#endif
}

/** Does the opposite of the function above
*/
inline double pitch2freq(double note)
{
	double result = pow10((note + 36.3763165622959152488) / 39.8631371386483481);
	return result;
}

const char* noteName(int pitch);
inline const char* noteName(double pitch) { return noteName(toInt(pitch)); }

int noteOctave(int pitch);
inline int noteOctave(double pitch) { return noteOctave(toInt(pitch)); }

int noteValue(int pitch);
inline int noteValue(double pitch) { return noteValue(toInt(pitch)); }

bool isBlackNote(int pitch);
inline bool isBlackNote(double pitch) { return isBlackNote(toInt(pitch)); }

class MusicScale
{
  Array1d<int> pNotes;
  std::vector<bool> pSemitoneLookup;
  char *pName;
  int _semitoneOffset;

 public:
  enum MusicScale_t { Chromatic, Major, NaturalMinor, HarmonicMinor, MelodicMinor };

          MusicScale() { pName = NULL; _semitoneOffset = 0; }
          ~MusicScale();
  void    addScale(const char *theName, const int *theNotes, int length, int semitoneOffset_);
  size_t     size() { return pNotes.size(); }
  int     note(int j) { return pNotes[j]; }
  bool    hasSemitone(int j) { return pSemitoneLookup[j]; }
  const char * name() { return pName; }
  int semitoneOffset() { return _semitoneOffset; }
};

/** This defines the notes relative to the root for 1 octave of the scale
*/
class MusicKey
{
  Array1d<double> noteOffsets; //ordered midi values of the notes in 1 octave
  Array1d<int> noteTypes;
  char *pName;

 public:
          MusicKey() { pName = NULL; }
          ~MusicKey();

  void    setScaleMidi(double *theNoteOffsets, int *types, int n);
  void    setScaleCents(double *theNoteOffsets, int *types, int n);
  void    setScaleRatios(double *theNoteOffsets, int *types, int n);
  void    setName(const char *theName);
  const char * name() { return pName; }
  size_t     size() const { return noteOffsets.size(); }
  double  noteOffset(int j) const { return noteOffsets[j]; }
  int     noteType(int j) const { return noteTypes[j]; }
  int     nearestNoteIndex(double x);
  double  nearestNote(double x);
  double  nearestNoteDistance(double x);
};

void initMusicStuff();

#define NUM_MUSIC_KEYS 12
extern std::vector<MusicKey> gMusicKeys;
extern std::vector<MusicScale> gMusicScales;

extern const char *gMusicKeyName[NUM_MUSIC_KEYS];
extern int gMusicKeyRoot[NUM_MUSIC_KEYS];
extern int gMusicKey;

class TuningMode
{
 public:
  enum ModeName_t { Chromatic, Major, NaturalMinor, HarmonicMinor, MelodicMinor };
  enum TuningName_t { EvenTempered, JustIntonation, PythagoreanTuning, MeantoneTemperament };
  static TuningMode tuningMode[5][4];

  std::vector<double> pitchOffset;
};

class NoteInfo
{
  TuningMode::ModeName_t modeName;
  TuningMode::TuningName_t tuningName;
  int rootNote; //range 0 - 11 starting on C
  double rootPitch;
};

#endif
