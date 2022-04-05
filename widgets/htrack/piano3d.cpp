/***************************************************************************
                          piano3d.cpp  -  description
                             -------------------
    begin                : 31 Mar 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#include <algorithm>

#include "piano3d.h"
#include "useful.h"
#include "musicnotes.h"
#include "mygl.h"

double keyOffsetTable[12] = { 0.0, 15.5, 23.5, 43.5, 47.0, 70.5, 85.5, 94.0, 112.5, 117.5, 139.0, 141.0 };

Piano3d::Piano3d(int numKeys_, int firstKey_)
{
  init(numKeys_, firstKey_);
}

Piano3d::~Piano3d()
{
  m_vao_aWhiteKey.destroy();
  m_vbo_aWhiteKey.destroy();
  m_vao_aWhiteKeyOutline.destroy();
  m_vbo_aWhiteKeyOutline.destroy();

  m_vao_aBlackKey.destroy();
  m_vbo_aBlackKey.destroy();
  m_vao_aBlackKeyOutline.destroy();
  m_vbo_aBlackKeyOutline.destroy();
}

/**
@return The width of the current piano keyboard in mm
*/
double Piano3d::pianoWidth()
{
  return (isBlackNote(_numKeys-1)+_firstKey) ? keyOffsets[_numKeys-1]+BLACK_KEY_WIDTH : keyOffsets[_numKeys-1]+WHITE_KEY_WIDTH;
}

/**
@return The distance the in mm away from where the 0 midi note would be
*/
double Piano3d::offsetAtKey(int keyNum)
{
  return (noteOctave(keyNum)+1)*OCTAVE_WIDTH + keyOffsetTable[cycle(keyNum, 12)] - firstKeyOffset;
}

/** Initialises the piano.
This can be called again to reinitialise the piano to a different size
@param numKeys_ The number of keys the piano is to have
@param firstKey_ The midi note number that will be the lowest key on the piano
*/
void Piano3d::init(int numKeys_, int firstKey_)
{
  _numKeys = numKeys_;
  _firstKey = firstKey_;
  keyStates.resize(_numKeys, false);
  keyOffsets.resize(_numKeys);
  firstKeyOffset = (noteOctave(_firstKey)+1)*OCTAVE_WIDTH + keyOffsetTable[cycle(_firstKey, 12)];
  int curKey;
  for(int j=0; j<_numKeys; j++) {
    curKey = j+_firstKey;
    keyOffsets[j] = offsetAtKey(curKey);
  }
  
  //build display lists for a white and a black key (for fast rendering later)
  m_vao_aWhiteKey.create();
  m_vbo_aWhiteKey.create();
  m_vao_aWhiteKeyOutline.create();
  m_vbo_aWhiteKeyOutline.create();

  m_vao_aBlackKey.create();
  m_vbo_aBlackKey.create();
  m_vao_aBlackKeyFront.create();
  m_vbo_aBlackKeyFront.create();
  m_vao_aBlackKeyOutline.create();
  m_vbo_aBlackKeyOutline.create();

  drawWhiteKey();
  drawBlackKey();
}

void Piano3d::setAllKeyStatesOff()
{
  std::fill(keyStates.begin(), keyStates.end(), false);
}

void Piano3d::drawWhiteKey()
{
  QVector<QVector3D> key, keyOutline;

  //top surface
#ifdef DWS
  This is for lighting
  glNormal3f(0.0, 1.0, 0.0);
#endif
  // These will be GL_TRIANGLE_STRIP for the key top
  key << QVector3D(0.0f,             0.0f, 0.0f);
  key << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  key << QVector3D(WHITE_KEY_WIDTH,  0.0f, 0.0f);
  key << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  key << QVector3D(0.0f,             0.0f, WHITE_KEY_LENGTH);
  key << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  key << QVector3D(WHITE_KEY_WIDTH,  0.0f, WHITE_KEY_LENGTH);
  key << QVector3D(0.0f,             1.0f, 0.0f); // Normal

  // These will be GL_LINES for the outline of the key top
  keyOutline << QVector3D(0.0f,             0.0f, 0.0f);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, 0.0f);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, 0.0f);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(0.0f,             0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(0.0f,             0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal
  keyOutline << QVector3D(0.0f,             0.0f, 0.0f);
  keyOutline << QVector3D(0.0f,             1.0f, 0.0f); // Normal

  //front surface
#ifdef DWS
  THis is for lighting
  glNormal3f(0.0f, -1.0f, 0.2f); 
#endif
  // Beveled front edge of key.
  //todo: fix normal
  key << QVector3D(0.0f,                    -2.0f, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(0.0f,                    -1.0f, 0.2f); // Normal
  key << QVector3D(WHITE_KEY_WIDTH,         -2.0f, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(0.0f,                    -1.0f, 0.2f); // Normal

  keyOutline << QVector3D(0.0f,              0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,   0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,   0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH, - 2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(0.0f,             -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(0.0f,             -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal
  keyOutline << QVector3D(0.0f,              0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(0.0f,             -1.0f, 0.2f); // Normal

    // Front of the key.
  #ifdef DWS
  glNormal3f(0.0f, 0.0f, 1.0f);
#endif

  key << QVector3D(0.0f,             -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(0.0f,             0.0f,              1.0f); // Normal
  key << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(0.0f,             0.0f,              1.0f); // Normal

  keyOutline << QVector3D(0.0f,            -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(0.0f,            -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(0.0f,            -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal
  keyOutline << QVector3D(0.0f,            -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(0.0f,             0.0f, 1.0f); // Normal

  m_vao_aWhiteKey.bind();
  m_vbo_aWhiteKey.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aWhiteKey.bind();
  m_vbo_aWhiteKey.allocate(key.constData(), key.count() * 3 * sizeof(float));
  m_aWhiteKeyCount = key.count()/2; // Take into account both vertices and normals.

  m_vao_aWhiteKeyOutline.bind();
  m_vbo_aWhiteKeyOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aWhiteKeyOutline.bind();
  m_vbo_aWhiteKeyOutline.allocate(keyOutline.constData(), keyOutline.count() * 3 * sizeof(float));
  m_aWhiteKeyOutlineCount = keyOutline.count()/2; // Take into account both vertices and normals.
}

void Piano3d::drawBlackKey()
{
  QVector<QVector3D> key, keyFront, keyOutline;

  // Left side of the key
#ifdef DWS
  glNormal3f(-1.0f, 0.0f, 0.0f);
#endif
  key << QVector3D( 0.0f, 0.0f,             0.0f);
  key << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  key << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  key << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  key << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  key << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  key << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  key << QVector3D(-1.0f, 0.0f,             0.0f); // Normal

  keyOutline << QVector3D( 0.0f, 0.0f,             0.0f);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal
  keyOutline << QVector3D( 0.0f, 0.0f,             0.0f);
  keyOutline << QVector3D(-1.0f, 0.0f,             0.0f); // Normal

  // Top of the key
#ifdef DWS
  glNormal3f(0.0f, 1.0f, 0.0f);
#endif
  key << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  key << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  key << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  key << QVector3D(0.0f,            1.0f,             0.0f); // Normal

  keyOutline << QVector3D(0.0f,            BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal
  keyOutline << QVector3D(0,               BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(0.0f,            1.0f,             0.0f); // Normal

  // Right side of the key
#ifdef DWS
  glNormal3f(1.0f, 0.0f, 0.0f);
#endif
  key << QVector3D(BLACK_KEY_WIDTH, 0.0f, 0.0f);
  key << QVector3D(1.0f,            0.0f, 0.0f); // Normal
  key << QVector3D(BLACK_KEY_WIDTH, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  key << QVector3D(1.0f,            0.0f, 0.0f); // Normal

  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             0.0f);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             0.0f);
  keyOutline << QVector3D(1.0f,            0.0f,             0.0f); // Normal

  m_vao_aBlackKey.bind();
  m_vbo_aBlackKey.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aBlackKey.bind();
  m_vbo_aBlackKey.allocate(key.constData(), key.count() * 3 * sizeof(float));
  m_aBlackKeyCount = key.count()/2;

  // Front of the key
#ifdef DWS
  glNormal3f(0.0f, 3.0f, 1.0f); //todo: fix normal
#endif
  keyFront << QVector3D(0.0f,            0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyFront << QVector3D(0.0f,            3.0f,             1.0f); // Normal
  keyFront << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyFront << QVector3D(0.0f,            3.0f,             1.0f); // Normal
  keyFront << QVector3D(0.0f,            BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyFront << QVector3D(0.0f,            3.0f,             1.0f); // Normal
  keyFront << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyFront << QVector3D(0.0f,            3.0f,             1.0f); // Normal

  keyOutline << QVector3D(0.0f,            0.0f, BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(0.0f,            3.0f, 1.0f); // Normal
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(0.0f,            3.0f, 1.0f); // Normal

  m_vao_aBlackKeyFront.bind();
  m_vbo_aBlackKeyFront.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aBlackKeyFront.bind();
  m_vbo_aBlackKeyFront.allocate(keyFront.constData(), keyFront.count() * 3 * sizeof(float));
  m_aBlackKeyFrontCount = keyFront.count()/2;

  m_vao_aBlackKeyOutline.bind();
  m_vbo_aBlackKeyOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aBlackKeyOutline.bind();
  m_vbo_aBlackKeyOutline.allocate(keyOutline.constData(), keyOutline.count() * 3 * sizeof(float));
  m_aBlackKeyOutlineCount = keyOutline.count()/2;
}

void Piano3d::setMaterialWhiteKey()
{
  GLfloat pianoKeyDiffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pianoKeyDiffuse);
}

void Piano3d::setMaterialBlackKey()
{
  GLfloat pianoKeyDiffuse[] = { 0.3f, 0.3f, 0.3f, 1.0f };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pianoKeyDiffuse);
}

void Piano3d::draw(QOpenGLShaderProgram& program, QMatrix4x4& pianoModel)
{
  int curKey;
  int j;

  glShadeModel(GL_FLAT);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);
  glPolygonMode(GL_FRONT, GL_FILL);
    
  //draw white keys filled
  QColor keyColor(230, 230, 230);
#ifdef DWS
  setMaterialSpecular( 0.2f, 0.2f, 0.2f, 50.0f);
  setMaterialColor(0.9f, 0.9f, 0.9f);
#endif
  QMatrix4x4 pianoKeyModel = pianoModel;
  QMatrix4x4 pianoKeyPressedModel;

  for(j=0; j<_numKeys; j++) {
    curKey = j+_firstKey;
    if(!isBlackNote(curKey)) {
      keyColor = QColor(230, 230, 230);
#ifdef DWS
      setMaterialColor(0.9f, 0.9f, 0.9f);
#endif

      if(keyStates[j]) {
        keyColor = QColor(230, 230, 0);

#ifdef DWS
        setMaterialColor(0.9f, 0.9f, 0.0f);
#endif
        pianoKeyPressedModel = pianoKeyModel;
        pianoKeyPressedModel.rotate(5.0, 1.0, 0.0, 0.0);

        program.bind();
        program.setUniformValue("model", pianoKeyPressedModel);
        program.release();
        MyGL::DrawShapeNormal(program, m_vao_aWhiteKey, m_vbo_aWhiteKey, m_aWhiteKeyCount, GL_TRIANGLE_STRIP, keyColor);
        keyColor = QColor(0, 0, 0);
#ifdef DWS
        setMaterialColor(0.0, 0.0, 0.0);
#endif
        MyGL::DrawShapeNormal(program, m_vao_aWhiteKeyOutline, m_vbo_aWhiteKeyOutline, m_aWhiteKeyOutlineCount, GL_LINES, keyColor);
#ifdef DWS
#endif
      } else {
        program.bind();
        program.setUniformValue("model", pianoKeyModel);
        program.release();
        MyGL::DrawShapeNormal(program, m_vao_aWhiteKey, m_vbo_aWhiteKey, m_aWhiteKeyCount, GL_TRIANGLE_STRIP, keyColor);
        keyColor = QColor(0, 0, 0);
#ifdef DWS
        setMaterialColor(0.0, 0.0, 0.0);
#endif
        MyGL::DrawShapeNormal(program, m_vao_aWhiteKeyOutline, m_vbo_aWhiteKeyOutline, m_aWhiteKeyOutlineCount, GL_LINES, keyColor);
      }
      pianoKeyModel.translate(WHITE_KEY_WIDTH, 0.0, 0.0);
    }
  }
  
  //draw black keys filled
  for(j=0; j<_numKeys; j++) {
    curKey = j+_firstKey;
    if(isBlackNote(curKey)) {
      QColor keyColor(38, 38, 38);
#ifdef DWS
      setMaterialColor(0.15f, 0.15f, 0.15f);
#endif
      pianoKeyModel = pianoModel;
      pianoKeyModel.translate(keyOffsets[j], 0.0, 0.0);
      if(keyStates[j]) {
        pianoKeyPressedModel = pianoKeyModel;
        pianoKeyPressedModel.rotate(5.0, 1.0, 0.0, 0.0);

        keyColor = QColor(128, 128, 0);
#ifdef DWS
        setMaterialColor(0.5, 0.5, 0.0);
#endif
        program.bind();
        program.setUniformValue("model", pianoKeyPressedModel);
        program.release();
        MyGL::DrawShapeNormal(program, m_vao_aBlackKey, m_vbo_aBlackKey, m_aBlackKeyCount, GL_TRIANGLE_STRIP, keyColor);
        MyGL::DrawShapeNormal(program, m_vao_aBlackKeyFront, m_vbo_aBlackKeyFront, m_aBlackKeyFrontCount, GL_TRIANGLE_STRIP, keyColor);

        keyColor = QColor(90, 90, 90);
#ifdef DWS
        setMaterialColor(0.35f, 0.35f, 0.35f);
#endif
        MyGL::DrawShape(program, m_vao_aBlackKeyOutline, m_vbo_aBlackKeyOutline, m_aBlackKeyOutlineCount, GL_LINES, keyColor);
      } else {
        program.bind();
        program.setUniformValue("model", pianoKeyModel);
        program.release();
        MyGL::DrawShapeNormal(program, m_vao_aBlackKey, m_vbo_aBlackKey, m_aBlackKeyCount, GL_TRIANGLE_STRIP, keyColor);
        MyGL::DrawShapeNormal(program, m_vao_aBlackKeyFront, m_vbo_aBlackKeyFront, m_aBlackKeyFrontCount, GL_TRIANGLE_STRIP, keyColor);

        keyColor = QColor(90, 90, 90);
#ifdef DWS
        setMaterialColor(0.35f, 0.35f, 0.35f);
#endif
        MyGL::DrawShapeNormal(program, m_vao_aBlackKeyOutline, m_vbo_aBlackKeyOutline, m_aBlackKeyOutlineCount, GL_LINES, keyColor);
      }
    }
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
}
