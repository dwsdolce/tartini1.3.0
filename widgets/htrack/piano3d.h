/***************************************************************************
                          piano3d.h  -  description
                             -------------------
    begin                : 31 March 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#ifndef PIANO3D_H
#define PIANO3D_H

#include <vector>
#include <stdio.h>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QMatrix4x4>

#include "myassert.h"

#define WHITE_KEY_WIDTH            23.5f
#define WHITE_KEY_LENGTH           148.0f
#define WHITE_KEY_LENGTH_INSIDE    142.0f
#define WHITE_KEY_HEIGHT           16.0f
#define BLACK_KEY_WIDTH            11.0f
#define BLACK_KEY_LENGTH_TOP       94.0f
#define BLACK_KEY_LENGTH_BOTTOM    100.0f
#define BLACK_KEY_HEIGHT           10.0f
#define OCTAVE_WIDTH               164.5f

class Piano3d
{
public:
  Piano3d(int numKeys_ = 85, int firstKey_ = 21);
  ~Piano3d();

  void drawWhiteKey();
  void drawBlackKey();

  void draw(QOpenGLShaderProgram &program, QMatrix4x4 &pianoModel);
  
  void init(int numKeys_=85, int firstKey_=21);
    
  double offsetAtKey(int keyNum);
  bool keyState(int keyNum) { return (keyNum >= 0 && keyNum < _numKeys) ? keyStates[keyNum] : false; }
  bool midiKeyState(int keyNum) { return keyState(keyNum-_firstKey); }
  void setKeyState(int keyNum, bool state) { if(keyNum >= 0 && keyNum < _numKeys) keyStates[keyNum] = state; }
  void setMidiKeyState(int keyNum, bool state) { setKeyState(keyNum-_firstKey, state); }
  void setAllKeyStatesOff();

  double pianoWidth();
  int numKeys() { return _numKeys; }
  int firstKey() { return _firstKey; }
  double firstKeyOffset;
  
private:
  std::vector<bool> keyStates;
  std::vector<float> keyOffsets;
  int _numKeys;
  int _firstKey; //on the midi scale (ie middle C = 60)

  QOpenGLVertexArrayObject m_vao_aWhiteKey;
  QOpenGLBuffer m_vbo_aWhiteKey;
  int m_aWhiteKeyCount;

  QOpenGLVertexArrayObject m_vao_aWhiteKeyOutline;
  QOpenGLBuffer m_vbo_aWhiteKeyOutline;
  int m_aWhiteKeyOutlineCount;


  QOpenGLVertexArrayObject m_vao_aBlackKey;
  QOpenGLBuffer m_vbo_aBlackKey;
  int m_aBlackKeyCount;

  QOpenGLVertexArrayObject m_vao_aBlackKeyFront;
  QOpenGLBuffer m_vbo_aBlackKeyFront;
  int m_aBlackKeyFrontCount;

  QOpenGLVertexArrayObject m_vao_aBlackKeyOutline;
  QOpenGLBuffer m_vbo_aBlackKeyOutline;
  int m_aBlackKeyOutlineCount;
};

#endif
