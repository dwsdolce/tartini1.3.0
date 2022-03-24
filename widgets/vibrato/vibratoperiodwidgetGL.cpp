/***************************************************************************
                          vibratoperiodwidgetGL.cpp  -  description
                             -------------------
    begin                : May 18 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Please read LICENSE.txt for details.
 ***************************************************************************/
#include "vibratoperiodwidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include "shader.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoPeriodWidgetGL::VibratoPeriodWidgetGL(QWidget* parent)
  : QOpenGLWidget(parent)
{
  prevLeftMinimumTime = -1;
  lastPeriodToDraw = -1;

  smoothedPeriods = true;
  drawSineReference = false;
  sineStyle = false;
  drawPrevPeriods = false;
  periodScaling = false;
  drawComparison = false;

  m_sineReferenceCount = 0;
  for (int i = 0; i < 5; i++) {
    m_previousPolyCount[i] = 0;
  }
  m_currentPeriodCount = 0;
  m_comparisonPolyCount = 0;
}

VibratoPeriodWidgetGL::~VibratoPeriodWidgetGL()
{
  // Remove display lists
  QOpenGLContext* c = QOpenGLContext::currentContext();
  if (c) {
    makeCurrent();
  }

  m_vao_sineReference.destroy();
  m_vbo_sineReference.destroy();

  for (int i = 0; i < 5; i++) {
    m_vao_previousPoly[i].destroy();
    m_vbo_previousPoly[i].destroy();
  }

  m_vao_currentPeriod.destroy();
  m_vbo_currentPeriod.destroy();

  m_vao_comparisonPoly.destroy();
  m_vbo_comparisonPoly.destroy();
  
  m_vao_comparisonReference.destroy();
  m_vbo_comparisonReference.destroy();

  m_program.deleteLater();
  m_program_line.deleteLater();
  doneCurrent();
}

void VibratoPeriodWidgetGL::initializeGL()
{
  initializeOpenGLFunctions();

// build and compile our shader program
// ------------------------------------
  try {
    Shader ourShader(&m_program, ":/shader.vs.glsl", ":/shader.fs.glsl");
  } catch (...) {
    close();
  }

  try {
    Shader ourShader(&m_program_line, ":/shader.vs.glsl", ":/shader.fs.glsl", ":/shader.gs.glsl");
  } catch (...) {
    close();
  }

  m_vao_sineReference.create();
  m_vbo_sineReference.create();

  for (int i = 0; i < 5; i++) {
    m_vao_previousPoly[i].create();
    m_vbo_previousPoly[i].create();
  }

  m_vao_currentPeriod.create();
  m_vbo_currentPeriod.create();

  m_vao_comparisonPoly.create();
  m_vbo_comparisonPoly.create();

  m_vao_comparisonReference.create();
  m_vbo_comparisonReference.create();
}

void VibratoPeriodWidgetGL::resizeGL(int w, int h)
{
  glViewport(0, 0, (GLint)w, (GLint)h);

   // Create model transformation matrix to go from:
  //		x: 0 to width
  //		y: 0 to height
  //	to:
  //		x: -1.0f to 1.0f
  //		y: -1.0f to 1.0f
  QMatrix4x4 model;
  model.setToIdentity();
  model.translate(QVector3D(-1.0f, -1.0f, 0.0f));
  model.scale(2.0f / width(), 2.0f / height(), 1.0f);

  m_program.bind();
  m_program.setUniformValue("model", model);
  m_program.release();

  m_program_line.bind();
  m_program_line.setUniformValue("model", model);
  m_program_line.setUniformValue("screen_size", QVector2D(w, h));
  m_program_line.release();

  // Calculate the horizontal reference line
  const float halfHeight = 0.5 * height();

  QVector<QVector3D> comparisonReference;
  comparisonReference << QVector3D(0, halfHeight, 0.0f) << QVector3D(w, halfHeight, 0.0f);

  m_vao_comparisonReference.bind();
  m_vbo_comparisonReference.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_comparisonReference.bind();
  m_vbo_comparisonReference.allocate(comparisonReference.constData(), comparisonReference.count() * 3 * sizeof(float));
  m_vao_comparisonReference.release();
  m_vbo_comparisonReference.release();
  m_comparisonReferenceCount = comparisonReference.count();

  // Do forced update on resize
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::paintGL()
{
  QColor bg = gdata->backgroundColor();
  glClearColor(double(bg.red()) / 256.0, double(bg.green()) / 256.0, double(bg.blue()) / 256.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Draw the horizontal reference line
  MyGL::DrawLine(m_program_line, m_vao_comparisonReference, m_vbo_comparisonReference, m_comparisonReferenceCount, GL_LINES, 1.5, QColor(0, 0, 0, 64));

  // Draw the sinewave
  if (m_sineReferenceCount > 0) {
    MyGL::DrawLine(m_program_line, m_vao_sineReference, m_vbo_sineReference, m_sineReferenceCount, GL_LINE_STRIP, 2.0, QColor(255, 255, 0, 255));
  }

  // Draw the comparison
  if (m_comparisonPolyCount > 0) {
    MyGL::DrawLine(m_program_line, m_vao_comparisonPoly, m_vbo_comparisonPoly, m_comparisonPolyCount, GL_LINE_STRIP, 2.0, QColor(0, 255, 0, 255));
  }

  // Draw the previous periods
  for (int i = 4; i >= 0; i--) {
    if (lastPeriodToDraw >= i) {
      if (m_previousPolyCount[i] > 0) {
        MyGL::DrawLine(m_program_line, m_vao_previousPoly[i], m_vbo_previousPoly[i], m_previousPolyCount[i], GL_LINE_STRIP, 2.0, QColor(127, 0, 0, m_previousPolyAlpha[i]));
      }
    }
  }

  // Draw the current period
  if (m_currentPeriodCount > 0) {
    MyGL::DrawLine(m_program_line, m_vao_currentPeriod, m_vbo_currentPeriod, m_currentPeriodCount, GL_LINE_STRIP, 2.0, QColor(127, 0, 0, 255));
  }
}

void VibratoPeriodWidgetGL::doUpdate()
{
  Channel* active = gdata->getActiveChannel();

  int leftMinimumTime = -1;
  int maximumTime = -1;
  int rightMinimumTime = -1;
  int leftMinimumAt = -1;

  if ((active) && (active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
    AnalysisData* data = active->dataAtCurrentChunk();
    if ((data != nullptr) && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      int smoothDelay = int(active->pitchBigSmoothingFilter->delay());
      int currentTime = active->chunkAtCurrentTime() * active->framesPerChunk() + smoothDelay;
      size_t maximaSize = note->maxima->size();
      size_t minimaSize = note->minima->size();

      // Determine which period to show: the 2 rightmost minima + the maximum in between
      if ((maximaSize >= 1) && (minimaSize == 2)) {
        // Only 2 minima
        if (currentTime >= note->minima->at(1)) {
          leftMinimumTime = note->minima->at(0);
          rightMinimumTime = note->minima->at(1);
          leftMinimumAt = 0;
        }
      } else if ((maximaSize >= 1) && (minimaSize > 2)) {
        // More than 2 minima
        for (int i = 2; i < minimaSize; i++) {
          if ((currentTime >= note->minima->at(i - 1)) && (currentTime <= note->minima->at(i))) {
            leftMinimumTime = note->minima->at(i - 2);
            rightMinimumTime = note->minima->at(i - 1);
            leftMinimumAt = i - 2;
            break;
          }
        }
        if (currentTime > note->minima->at(minimaSize - 1)) {
          leftMinimumTime = note->minima->at(minimaSize - 2);
          rightMinimumTime = note->minima->at(minimaSize - 1);
          leftMinimumAt = int(minimaSize - 2);
        }
      }
      // The maximum in between
      for (int i = 0; i < maximaSize; i++) {
        if ((note->maxima->at(i) >= leftMinimumTime) && (note->maxima->at(i) <= rightMinimumTime)) {
          maximumTime = note->maxima->at(i);
          break;
        }
      }
    }
  }

  if (prevLeftMinimumTime == leftMinimumTime) {
    // Same period as previous, don't change the polys & no update needed

  } else {
    // Period has changed

    makeCurrent();

    if (leftMinimumTime == -1) {
      // No period to draw, erase polys & update
      m_sineReferenceCount = 0;
      for (int i = 0; i < 5; i++) {
        m_previousPolyCount[i] = 0;
      }
      m_currentPeriodCount = 0;
      m_comparisonPolyCount = 0;
      prevLeftMinimumTime = -1;

      update();

    } else {
      // New period, calculate new polys & update
      m_sineReferenceCount = 0;
      for (int i = 0; i < 5; i++) {
        m_previousPolyCount[i] = 0;
      }
      m_currentPeriodCount = 0;
      m_comparisonPolyCount = 0;

      AnalysisData* data = active->dataAtCurrentChunk();
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      large_vector<float> thePitchLookup;
      int theDelay;
      if (smoothedPeriods) {
        thePitchLookup = active->pitchLookupSmoothed;
        theDelay = 0;
      } else {
        thePitchLookup = active->pitchLookup;
        theDelay = int(active->pitchBigSmoothingFilter->delay() - active->pitchSmallSmoothingFilter->delay());
      }

      int theLeftMinimumTime = leftMinimumTime - theDelay;
      int theRightMinimumTime = rightMinimumTime - theDelay;
      int theMaximumTime = maximumTime - theDelay;
      int periodDuration = theRightMinimumTime - theLeftMinimumTime;

      float minimumPitch = (thePitchLookup.at(theLeftMinimumTime) >
        thePitchLookup.at(theRightMinimumTime))
        ? thePitchLookup.at(theRightMinimumTime)
        : thePitchLookup.at(theLeftMinimumTime);
      float maximumPitch = thePitchLookup.at(theMaximumTime);
      float periodWidth = maximumPitch - minimumPitch;

      const float halfHeight = 0.5 * height();

      // Display either sine- or cosine-style periods
      int theSineDelay = 0;
      if (sineStyle) {
        theSineDelay = toInt((rightMinimumTime - leftMinimumTime) * 0.25);
      }

      // Calculate the sinewave, currentPeriod, and comparison
      if ((width() > 0) && (height() > 0)) {
        QVector<QVector3D> currentPeriodVertices;
        QVector<QVector3D> sineVertices;
        QVector<QVector3D> comparisonVertices;
        double xValue;
        double ySineValue, yCurrentValue;
        for (float xx = 0; xx < width(); xx++) {
          xValue = 0.05 * width() + 0.9 * xx;
          if (drawSineReference) {
            if (sineStyle) {
              ySineValue = halfHeight + halfHeight * 0.9 * sin((xx / width()) * 2 * PI);
            } else {
              ySineValue = halfHeight + halfHeight * -0.9 * cos((xx / width()) * 2 * PI);
            }
            sineVertices << QVector3D(xValue, ySineValue, 0.0f);
          }
          yCurrentValue = 0.05 * height() + 0.9 * ((thePitchLookup.at(toInt((xx / width()) * periodDuration + theLeftMinimumTime + theSineDelay)) - minimumPitch) / periodWidth) * height();
          currentPeriodVertices << QVector3D(xValue, yCurrentValue, 0.0f);
          if (drawComparison && drawSineReference) {
            comparisonVertices << QVector3D(xValue, halfHeight + yCurrentValue - ySineValue, 0.0f);
          }
        }
        if (drawSineReference) {
          m_vao_sineReference.bind();
          m_vbo_sineReference.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          m_vbo_sineReference.bind();
          m_vbo_sineReference.allocate(sineVertices.constData(), sineVertices.count() * 3 * sizeof(float));
          m_vao_sineReference.release();
          m_vbo_sineReference.release();
          m_sineReferenceCount = sineVertices.count();
        }
        m_vao_currentPeriod.bind();
        m_vbo_currentPeriod.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_currentPeriod.bind();
        m_vbo_currentPeriod.allocate(currentPeriodVertices.constData(), currentPeriodVertices.count() * 3 * sizeof(float));
        m_vao_currentPeriod.release();
        m_vbo_currentPeriod.release();
        m_currentPeriodCount = currentPeriodVertices.count();
        if (drawComparison && drawSineReference) {
          m_vao_comparisonPoly.bind();
          m_vbo_comparisonPoly.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          m_vbo_comparisonPoly.bind();
          m_vbo_comparisonPoly.allocate(comparisonVertices.constData(), comparisonVertices.count() * 3 * sizeof(float));
          m_vao_comparisonPoly.release();
          m_vbo_comparisonPoly.release();
          m_comparisonPolyCount = comparisonVertices.count();
        }
      }

      // Calculate the previous period(s)
      if (drawPrevPeriods && (width() > 0) && (height() > 0)) {
        for (int i = 0; i < 5; i++) {
          QVector<QVector3D> vertices;
          if (leftMinimumAt - (i + 1) < 0) {
            lastPeriodToDraw = i - 1;
            break;
          }

          int thisPrevLeftMinimumTime = note->minima->at(leftMinimumAt - (i + 1)) - theDelay;
          int thisPrevRightMinimumTime = note->minima->at(leftMinimumAt - i) - theDelay;
          int thisPrevDuration = thisPrevRightMinimumTime - thisPrevLeftMinimumTime;
          int thisPrevMaximumTime = 0;
          for (int j = 0; j < note->maxima->size(); j++) {
            if ((note->maxima->at(j) >= thisPrevLeftMinimumTime) && (note->maxima->at(j) <= thisPrevRightMinimumTime)) {
              thisPrevMaximumTime = note->maxima->at(j) - theDelay;
              break;
            }
          }

          float thisPrevMinimumPitch = (thePitchLookup.at(thisPrevLeftMinimumTime) >
            thePitchLookup.at(thisPrevRightMinimumTime))
            ? thePitchLookup.at(thisPrevRightMinimumTime)
            : thePitchLookup.at(thisPrevLeftMinimumTime);
          float thisPrevMaximumPitch = thePitchLookup.at(thisPrevMaximumTime);
          float thisPrevWidth = thisPrevMaximumPitch - thisPrevMinimumPitch;

          if (periodScaling) {
            for (float xx = 0; xx < width(); xx++) {
              int offset = toInt((xx / width()) * thisPrevDuration + thisPrevLeftMinimumTime + theSineDelay);
              vertices << QVector3D(0.05 * width() + 0.9 * xx, 0.05 * height() + 0.9 * ((thePitchLookup.at(offset) - thisPrevMinimumPitch) / thisPrevWidth) * height(), 0.0f);
              m_previousPolyAlpha[i] = toInt(float(1 / pow(2, i + 1)) * 255);
            }
            m_vao_previousPoly[i].bind();
            m_vbo_previousPoly[i].setUsagePattern(QOpenGLBuffer::DynamicDraw);
            m_vbo_previousPoly[i].bind();
            m_vbo_previousPoly[i].allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
            m_vao_previousPoly[i].release();
            m_vbo_previousPoly[i].release();
            m_previousPolyCount[i] = vertices.count();
          } else {
            for (float xx = 0; xx < width(); xx++) {
              int offset = toInt((xx / width()) * thisPrevDuration + thisPrevLeftMinimumTime + theSineDelay);
              float xxx = xx * (float(thisPrevDuration) / periodDuration);
              xxx = xxx + ((0.5 * (periodDuration - thisPrevDuration) / periodDuration) * width());
              vertices << QVector3D(0.05 * width() + 0.9 * xxx, 0.05 * height() + 0.9 * ((thePitchLookup.at(offset) - minimumPitch) / periodWidth) * height(), 0.0f);
              m_previousPolyAlpha[i] = toInt(float(1 / pow(2, i + 1)) * 255);
            }


            m_vao_previousPoly[i].bind();
            m_vbo_previousPoly[i].setUsagePattern(QOpenGLBuffer::DynamicDraw);
            m_vbo_previousPoly[i].bind();
            m_vbo_previousPoly[i].allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
            m_vao_previousPoly[i].release();
            m_vbo_previousPoly[i].release();
            m_previousPolyCount[i] = vertices.count();
          }
          lastPeriodToDraw = i;
        }
      }
      prevLeftMinimumTime = leftMinimumTime;

      update();
    }
  }
}

void VibratoPeriodWidgetGL::setSmoothedPeriods(bool value)
{
  smoothedPeriods = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::setDrawSineReference(bool value)
{
  drawSineReference = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::setSineStyle(bool value)
{
  sineStyle = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::setDrawPrevPeriods(bool value)
{
  drawPrevPeriods = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::setPeriodScaling(bool value)
{
  periodScaling = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidgetGL::setDrawComparison(bool value)
{
  drawComparison = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}
