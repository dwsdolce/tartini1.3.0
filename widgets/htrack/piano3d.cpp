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
  // These will be GL_TRIANGLE_STRIP for the key top
  QVector3D normal(0.0f, 1.0f, 0.0f);

  key << QVector3D(WHITE_KEY_WIDTH, 0.0f, 0.0f);
  key << QVector3D(normal);
  key << QVector3D(0.0f,            0.0f, 0.0f);
  key << QVector3D(normal);
  key << QVector3D(WHITE_KEY_WIDTH, 0.0f, WHITE_KEY_LENGTH);
  key << QVector3D(normal);
  key << QVector3D(0.0f,            0.0f, WHITE_KEY_LENGTH);
  key << QVector3D(normal);

  // These will be GL_LINES for the outline of the key top
  keyOutline << QVector3D(0.0f,            0.0f, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, 0.0f, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, 0.0f, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, 0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, 0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            0.0f, 0.0f);
  keyOutline << QVector3D(normal);

  //front surface
  // Beveled front edge of key.
  QVector3D pt1(0.0f, 0.0f, WHITE_KEY_LENGTH);
  QVector3D pt2(WHITE_KEY_WIDTH, 0.0f, WHITE_KEY_LENGTH);
  QVector3D pt3(0.0f, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  QVector3D side1 = pt1 - pt2;
  QVector3D side2 = pt3 - pt2;
  normal = QVector3D::crossProduct(side1, side2);
  normal.normalize();

  key << QVector3D(WHITE_KEY_WIDTH, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(normal);
  key << QVector3D(0.0f,            -2.0f, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(normal);

  keyOutline << QVector3D(0.0f,             0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH,  0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -2.0f, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,             0.0f, WHITE_KEY_LENGTH);
  keyOutline << QVector3D(normal);

  // Front of the key.
  normal = QVector3D(0.0f, 0.0f, 1.0f);

  key << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(normal);
  key << QVector3D(0.0f, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  key << QVector3D(normal);

  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f,             WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -2.0f,             WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -2.0f,             WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(WHITE_KEY_WIDTH, -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -WHITE_KEY_HEIGHT, WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0.0f,            -2.0f,             WHITE_KEY_LENGTH_INSIDE);
  keyOutline << QVector3D(normal);

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
  QVector3D normal(-1.0f, 0.0f, 0.0f);

  key << QVector3D( 0.0f, 0.0f,             0.0f);
  key << QVector3D(normal);
  key << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  key << QVector3D(normal);
  key << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  key << QVector3D(normal);
  key << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  key << QVector3D(normal);

  keyOutline << QVector3D( 0.0f, 0.0f,             0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D( 0.0f, 0.0f,             0.0f);
  keyOutline << QVector3D(normal);

  // Top of the key
  normal = QVector3D(0.0f, 1.0f, 0.0f);

  key << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  key << QVector3D(normal);
  key << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  key << QVector3D(normal);

  keyOutline << QVector3D(0.0f,            BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(0,               BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);

  // Right side of the key
  normal = QVector3D(1.0f, 0.0f, 0.0f);

  key << QVector3D(BLACK_KEY_WIDTH, 0.0f, 0.0f);
  key << QVector3D(normal);
  key << QVector3D(BLACK_KEY_WIDTH, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  key << QVector3D(normal);

  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0.0f);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f,             0.0f);
  keyOutline << QVector3D(normal);

  m_vao_aBlackKey.bind();
  m_vbo_aBlackKey.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_aBlackKey.bind();
  m_vbo_aBlackKey.allocate(key.constData(), key.count() * 3 * sizeof(float));
  m_aBlackKeyCount = key.count()/2;

  // Front of the key
  QVector3D pt1(0.0f, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  QVector3D pt2(BLACK_KEY_WIDTH, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  QVector3D pt3(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  QVector3D side1 = pt2 - pt1;
  QVector3D side2 = pt3 - pt1;
  normal = QVector3D::crossProduct(side1, side2);
  normal.normalize();

  keyFront << QVector3D(0.0f,            0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyFront << QVector3D(normal);
  keyFront << QVector3D(BLACK_KEY_WIDTH, 0.0f,             BLACK_KEY_LENGTH_BOTTOM);
  keyFront << QVector3D(normal);
  keyFront << QVector3D(0.0f,            BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyFront << QVector3D(normal);
  keyFront << QVector3D(BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, BLACK_KEY_LENGTH_TOP);
  keyFront << QVector3D(normal);

  keyOutline << QVector3D(0.0f,            0.0f, BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);
  keyOutline << QVector3D(BLACK_KEY_WIDTH, 0.0f, BLACK_KEY_LENGTH_BOTTOM);
  keyOutline << QVector3D(normal);

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

  program.bind();
  program.setUniformValue("material.ambient", QVector3D(0.9f, 0.9f, 0.9f));
  program.setUniformValue("material.diffuse", QVector3D(0.9f, 0.9f, 0.9f));
  program.setUniformValue("material.specular", QVector3D(0.2f, 0.2f, 0.2f));
  program.setUniformValue("material.shininess", 50.0f);
  program.release();

  QMatrix4x4 pianoKeyModel = pianoModel;
  QMatrix4x4 pianoKeyPressedModel;

  for(j=0; j<_numKeys; j++) {
    curKey = j+_firstKey;
    if(!isBlackNote(curKey)) {
      program.bind();
      program.setUniformValue("material.ambient", QVector3D(0.9f, 0.9f, 0.9f));
      program.setUniformValue("material.diffuse", QVector3D(0.9f, 0.9f, 0.9f));
      program.release();
      keyColor = QColor(230, 230, 230);

      if (keyStates[j]) {
        pianoKeyPressedModel = pianoKeyModel;
        pianoKeyPressedModel.rotate(5.0, 1.0, 0.0, 0.0);

        keyColor = QColor(230, 230, 0);
        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.9f, 0.9f, 0.0f));
        program.setUniformValue("material.diffuse", QVector3D(0.9f, 0.9f, 0.0f));
        program.setUniformValue("model", pianoKeyPressedModel);
        program.release();

        MyGL::DrawShape(program, m_vao_aWhiteKey, m_vbo_aWhiteKey, m_aWhiteKeyCount, GL_TRIANGLE_STRIP);

        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.0f, 0.0f, 0.0f));
        program.setUniformValue("material.diffuse", QVector3D(0.0f, 0.0f, 0.0f));
        program.release();

        MyGL::DrawShape(program, m_vao_aWhiteKeyOutline, m_vbo_aWhiteKeyOutline, m_aWhiteKeyOutlineCount, GL_LINES);
      } else {
        program.bind();
        program.setUniformValue("model", pianoKeyModel);
        program.release();

        MyGL::DrawShape(program, m_vao_aWhiteKey, m_vbo_aWhiteKey, m_aWhiteKeyCount, GL_TRIANGLE_STRIP);

        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.0f, 0.0f, 0.0f));
        program.setUniformValue("material.diffuse", QVector3D(0.0f, 0.0f, 0.0f));
        program.release();
        MyGL::DrawShape(program, m_vao_aWhiteKeyOutline, m_vbo_aWhiteKeyOutline, m_aWhiteKeyOutlineCount, GL_LINES);
      }
      pianoKeyModel.translate(WHITE_KEY_WIDTH, 0.0, 0.0);
    }
  }
  
  //draw black keys filled
  for(j=0; j<_numKeys; j++) {
    curKey = j+_firstKey;
    if(isBlackNote(curKey)) {
      QColor keyColor(38, 38, 38);
      program.bind();
      program.setUniformValue("material.ambient", QVector3D(0.15f, 0.15f, 0.15f));
      program.setUniformValue("material.diffuse", QVector3D(0.15f, 0.15f, 0.15f));
      program.release();

      pianoKeyModel = pianoModel;
      pianoKeyModel.translate(keyOffsets[j], 0.0, 0.0);
      if(keyStates[j]) {
        pianoKeyPressedModel = pianoKeyModel;
        pianoKeyPressedModel.rotate(5.0, 1.0, 0.0, 0.0);

        keyColor = QColor(128, 128, 0);
        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.5, 0.5, 0.0));
        program.setUniformValue("material.diffuse", QVector3D(0.5, 0.5, 0.0));
        program.setUniformValue("model", pianoKeyPressedModel);
        program.release();

        MyGL::DrawShape(program, m_vao_aBlackKey, m_vbo_aBlackKey, m_aBlackKeyCount, GL_TRIANGLE_STRIP);
        MyGL::DrawShape(program, m_vao_aBlackKeyFront, m_vbo_aBlackKeyFront, m_aBlackKeyFrontCount, GL_TRIANGLE_STRIP);

        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.35f, 0.35f, 0.35f));
        program.setUniformValue("material.diffuse", QVector3D(0.35f, 0.35f, 0.35f));
        program.release();
        MyGL::DrawShape(program, m_vao_aBlackKeyOutline, m_vbo_aBlackKeyOutline, m_aBlackKeyOutlineCount, GL_LINES);
      } else {
        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.35f, 0.35f, 0.35f));
        program.setUniformValue("material.diffuse", QVector3D(0.35f, 0.35f, 0.35f));
        program.setUniformValue("model", pianoKeyModel);
        program.release();

        MyGL::DrawShape(program, m_vao_aBlackKey, m_vbo_aBlackKey, m_aBlackKeyCount, GL_TRIANGLE_STRIP);
        MyGL::DrawShape(program, m_vao_aBlackKeyFront, m_vbo_aBlackKeyFront, m_aBlackKeyFrontCount, GL_TRIANGLE_STRIP);

        program.bind();
        program.setUniformValue("material.ambient", QVector3D(0.35f, 0.35f, 0.35f));
        program.setUniformValue("material.diffuse", QVector3D(0.35f, 0.35f, 0.35f));
        program.release();
        MyGL::DrawShape(program, m_vao_aBlackKeyOutline, m_vbo_aBlackKeyOutline, m_aBlackKeyOutlineCount, GL_LINES);
      }
    }
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
}
