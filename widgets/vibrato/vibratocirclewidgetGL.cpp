/***************************************************************************
                          vibratocirclewidgetGL.cpp  -  description
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
#include "vibratocirclewidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include "shader.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoCircleWidgetGL::VibratoCircleWidgetGL(QWidget* parent)
  : QOpenGLWidget(parent)
{
  accuracy = 1.0;
  type = 5;
  lastPeriodToDraw = -1;
  m_CurrentPeriodAlpha = 200;
  m_currentPeriodCount = 0;
}

VibratoCircleWidgetGL::~VibratoCircleWidgetGL()
{
  // Remove display lists
  QOpenGLContext* c = QOpenGLContext::currentContext();
  if (c) {
    makeCurrent();
  }
  for (int i = 0; i < 2; i++) {
    m_vao_referenceCircle[i].destroy();
    m_vbo_referenceCircle[i].destroy();
  }

  m_vao_currentPeriod.destroy();
  m_vbo_currentPeriod.destroy();

  for (int i = 0; i < 6; i++) {
    m_vao_prevPeriods[i].destroy();
    m_vbo_prevPeriods[i].destroy();
  }

  m_program.deleteLater();
  m_program_line.deleteLater();
  doneCurrent();
}

void VibratoCircleWidgetGL::initializeGL()
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

  for (int i = 0; i < 2; i++) {
    m_vao_referenceCircle[i].create();
    m_vbo_referenceCircle[i].create();
  }

  m_vao_currentPeriod.create();
  m_vbo_currentPeriod.create();

  for (int i = 0; i < 6; i++) {
    m_vao_prevPeriods[i].create();
    m_vbo_prevPeriods[i].create();
  }
}

void VibratoCircleWidgetGL::resizeGL(int w, int h)
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

  // Calculate reference circle + lines
  // The lines
  const float halfWidth = 0.5 * width();
  const float halfHeight = 0.5 * height();
  QVector<QVector3D> lines;
  // The horizontal line
  lines << QVector3D(0, halfHeight, 0.0f) << QVector3D(w, halfHeight, 0.0f);

  // The vertical line
  lines << QVector3D(halfWidth, 0, 0.0f) << QVector3D(halfWidth, h, 0.0f);

  m_vao_referenceCircle[VBO_LINES].bind();
  m_vbo_referenceCircle[VBO_LINES].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_referenceCircle[VBO_LINES].bind();
  m_vbo_referenceCircle[VBO_LINES].allocate(lines.constData(), static_cast<int>(lines.count() * 3 * sizeof(float)));
  m_vao_referenceCircle[VBO_LINES].release();
  m_vbo_referenceCircle[VBO_LINES].release();
  m_referenceCircleCount[VBO_LINES] = lines.count();

  // The circle
  const float DEG2RAD = float(PI / 180);
  QVector<QVector3D> circle;
  for (int i = 0; i < 360; i = i + 1) {
    float degInRad = (float)i * DEG2RAD;
    circle << QVector3D(halfWidth + 0.8 * halfWidth * cos(degInRad), halfHeight + 0.8 * halfHeight * sin(degInRad), 0.0f);
  }
  //circle << QVector3D(halfWidth, 0, 0.0f) << QVector3D(halfWidth, h, 0.0f);
  m_vao_referenceCircle[VBO_CIRCLE].bind();
  m_vbo_referenceCircle[VBO_CIRCLE].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_referenceCircle[VBO_CIRCLE].bind();
  m_vbo_referenceCircle[VBO_CIRCLE].allocate(circle.constData(), static_cast<int>(circle.count() * 3 * sizeof(float)));
  m_vao_referenceCircle[VBO_CIRCLE].release();
  m_vbo_referenceCircle[VBO_CIRCLE].release();
  m_referenceCircleCount[VBO_CIRCLE] = circle.count();

// Do forced update on resize
  doUpdate();
}

void VibratoCircleWidgetGL::paintGL()
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

  // Draw the reference circle
  MyGL::DrawLine(m_program_line, m_vao_referenceCircle[VBO_LINES], m_vbo_referenceCircle[VBO_LINES], m_referenceCircleCount[VBO_LINES], GL_LINES, 1.5, QColor(0, 0, 0, 64));
  MyGL::DrawLine(m_program_line, m_vao_referenceCircle[VBO_CIRCLE], m_vbo_referenceCircle[VBO_CIRCLE], m_referenceCircleCount[VBO_CIRCLE], GL_LINE_LOOP, 1.5, QColor(Qt::black));

  if ((type == 1) || (type == 2)) {
    // Draw a line using the current period
    if (m_currentPeriodCount > 0) {
      MyGL::DrawLine(m_program_line, m_vao_currentPeriod, m_vbo_currentPeriod, m_currentPeriodCount, GL_LINES, 2.0, QColor(255, 0, 0, 255));
    }
  } else if ((type == 4) || (type == 5)) {
    // Draw a Triangle Fan using the current period
    if (m_currentPeriodCount > 0) {
      MyGL::DrawShape(m_program, m_vao_currentPeriod, m_vbo_currentPeriod, m_currentPeriodCount, GL_TRIANGLE_FAN, QColor(255, 0, 0, m_CurrentPeriodAlpha));
    }
  } else if (type == 3) {
    // Draw a line using the previous periods
    for (int i = 5; i >= 0; i--) {
      if (lastPeriodToDraw >= i) {
        MyGL::DrawLine(m_program_line, m_vao_prevPeriods[i], m_vbo_prevPeriods[i], m_prevPeriodsCount[i], GL_LINES, 2.0, QColor(255, 0, 0, m_prevPeriodsAlpha[i]));
      }
    }
  }
}

void VibratoCircleWidgetGL::doUpdate()
{
  Channel* active = gdata->getActiveChannel();

  int leftMinimumTime = -1;
  int maximumTime = -1;
  int rightMinimumTime = -1;
  int leftMinimumAt = -1;
  int maximumAt = -1;
  m_currentPeriodCount = 0;

  
  if ((active) && (active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
    AnalysisData* data = active->dataAtCurrentChunk();
    if (data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      // Determine which delay to use
      size_t smoothDelay = active->pitchBigSmoothingFilter->delay();
      large_vector<float> pitchLookupUsed = active->pitchLookupSmoothed;

      int currentTime = int(active->chunkAtCurrentTime() * active->framesPerChunk() + smoothDelay);
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
          maximumAt = i;
          break;
        }
      }

      if (maximumTime > 0) {
        // There is at least one full period before this one
        const float halfWidth = 0.5 * width();
        const float halfHeight = 0.5 * height();
        QVector<QVector3D> vertices;

        if ((type == 1) || (type == 2)) {
          const int stepSize = active->rate() / 1000;  // Draw element for every 0.001s
          const int prevPeriodDuration = rightMinimumTime - leftMinimumTime;
          const int currentChunk = active->chunkAtCurrentTime();

          float prevMinimumPitch = (pitchLookupUsed.at(leftMinimumTime) >
            pitchLookupUsed.at(rightMinimumTime))
            ? pitchLookupUsed.at(rightMinimumTime)
            : pitchLookupUsed.at(leftMinimumTime);  // Minimum pitch of previous period
          float prevMaximumPitch = pitchLookupUsed.at(maximumTime);       // Maximum pitch of previous period
          float prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
          float prevWidth = prevMaximumPitch - prevMinimumPitch;

          size_t end_i = std::min((currentChunk + 1) * active->framesPerChunk() + smoothDelay, pitchLookupUsed.size());
          for (int i = rightMinimumTime; i < end_i; i += stepSize) {

            if (i > (int)pitchLookupUsed.size()) {
              // Break out of loop when end of smoothed pitchlookup is reached
              break;
            }

            if ((i - rightMinimumTime) > prevPeriodDuration) {
              // Break out of loop when current period is longer than previous period, 'circle' is complete then
              break;
            }

            float currentPitch = pitchLookupUsed.at(i);
            float prevPitch = pitchLookupUsed.at(i - prevPeriodDuration);

            switch (type) {
              case 1:
              {
                float phase = float(i - rightMinimumTime) / prevPeriodDuration;
                float cosPhase = cos(phase * twoPI);
                float sinPhase = sin(phase * twoPI);
                float comparison = (currentPitch - prevPitch) / prevWidth;

                vertices << QVector3D(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase), halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase), 0.0f);
                break;
              }
              case 2:
              {
                float phase = float(i - rightMinimumTime) / prevPeriodDuration;
                float cosPhase = cos(phase * twoPI);
                float sinPhase = sin(phase * twoPI);
                float comparison = (currentPitch - (prevAvgPitch + (-cosPhase * prevWidth * 0.5))) / prevWidth;

                vertices << QVector3D(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase), halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase), 0.0f);
                break;
              }
              default:
              {
                break;
              }
            }
          }

          m_vao_currentPeriod.bind();
          m_vbo_currentPeriod.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          m_vbo_currentPeriod.bind();
          m_vbo_currentPeriod.allocate(vertices.constData(), static_cast<int>(vertices.count() * 3 * sizeof(float)));
          m_vao_currentPeriod.release();
          m_vbo_currentPeriod.release();
          m_currentPeriodCount = vertices.count();
        } else if (type == 3) {
          const int stepSize = active->rate() / 1000;  // Draw element for every 0.001s
         
          lastPeriodToDraw = -1;
          for (int p = 0; p < 6; p++) {
            if (leftMinimumAt - p < 0) {
              lastPeriodToDraw = p - 1;
              break;
            }

            leftMinimumTime = note->minima->at(leftMinimumAt - p);
            rightMinimumTime = note->minima->at(leftMinimumAt - (p - 1));
            for (int j = 0; j < note->maxima->size(); j++) {
              if ((note->maxima->at(j) >= leftMinimumTime) && (note->maxima->at(j) <= rightMinimumTime)) {
                maximumTime = note->maxima->at(j);
                break;
              }
            }

            float prevMinimumPitch = (pitchLookupUsed.at(leftMinimumTime) >
              pitchLookupUsed.at(rightMinimumTime))
              ? pitchLookupUsed.at(rightMinimumTime)
              : pitchLookupUsed.at(leftMinimumTime);
            float prevMaximumPitch = pitchLookupUsed.at(maximumTime);
            float prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
            float prevWidth = prevMaximumPitch - prevMinimumPitch;
            for (int i = leftMinimumTime; i <= rightMinimumTime; i += stepSize) {

              float pitch = pitchLookupUsed.at(i);

              float phase = float(i - leftMinimumTime) / float(rightMinimumTime - leftMinimumTime);
              float cosPhase = cos(phase * twoPI);
              float sinPhase = sin(phase * twoPI);
              float comparison = (pitch - (prevAvgPitch + (-cosPhase * prevWidth * 0.5))) / prevWidth;

              vertices << QVector3D(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase), halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase), 0.0f);
              m_prevPeriodsAlpha[p] = toInt(float(1 / pow(2, p)) * 255);
            }
            lastPeriodToDraw = p;
          }
        } else if (type == 4) {

          const int periodDuration = rightMinimumTime - leftMinimumTime;
          m_CurrentPeriodAlpha = 200;

          vertices << QVector3D(halfWidth, halfHeight, 0.0f);
          for (int j = 0; j < 360; j++) {
            int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
            if (i > rightMinimumTime) {
              i = rightMinimumTime;
            }

            float pitch = pitchLookupUsed.at(i);

            float phase = float(i - leftMinimumTime) / float(periodDuration);
            float cosPhase = cos(phase * twoPI);
            float sinPhase = sin(phase * twoPI);
            float minStep = float(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
            float thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
            float thisMaximumPitch = pitchLookupUsed.at(maximumTime);
            float thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
            float thisWidth = thisMaximumPitch - thisMinimumPitch;
            float comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;

            vertices << QVector3D(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase), halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase), 0.0f);
          }

          vertices << QVector3D(1.8 * halfWidth, halfHeight, 0.0f);

          m_vao_currentPeriod.bind();
          m_vbo_currentPeriod.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          m_vbo_currentPeriod.bind();
          m_vbo_currentPeriod.allocate(vertices.constData(), static_cast<int>(vertices.count() * 3 * sizeof(float)));
          m_vao_currentPeriod.release();
          m_vbo_currentPeriod.release();
          m_currentPeriodCount = vertices.count();

        } else if (type == 5) {
          if (leftMinimumAt == 0) {  // Fade in first period
            const int periodDuration = rightMinimumTime - leftMinimumTime;
            const int fadeAlphaInt = 200 * (currentTime - rightMinimumTime) / periodDuration;
            m_CurrentPeriodAlpha = (fadeAlphaInt > 255) ? 255 : fadeAlphaInt;

            vertices << QVector3D(halfWidth, halfHeight, 0.0f);
            for (int j = 0; j < 360; j++) {
              int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
              if (i > rightMinimumTime) {
                i = rightMinimumTime;
              }

              float pitch = pitchLookupUsed.at(i);

              float phase = float(i - leftMinimumTime) / float(periodDuration);
              float cosPhase = cos(phase * twoPI);
              float sinPhase = sin(phase * twoPI);
              float minStep = float(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
              float thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
              float thisMaximumPitch = pitchLookupUsed.at(maximumTime);
              float thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
              float thisWidth = thisMaximumPitch - thisMinimumPitch;
              float comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;

              vertices << QVector3D(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase), halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase), 0.0f);
            }

            vertices << QVector3D(1.8 * halfWidth, halfHeight, 0.0f);
          } else if (leftMinimumAt > 0) {  // Morph from previous period to current period

            const int prevLeftMinimumTime = note->minima->at(leftMinimumAt - 1);
            const int prevRightMinimumTime = leftMinimumTime;
            const int prevMaximumTime = note->maxima->at(maximumAt - 1);
            const int prevPeriodDuration = prevRightMinimumTime - prevLeftMinimumTime;
            float prevMaximumPitch = pitchLookupUsed.at(prevMaximumTime);
            const int periodDuration = rightMinimumTime - leftMinimumTime;
            float thisMaximumPitch = pitchLookupUsed.at(maximumTime);
            float distanceRatio = (currentTime - rightMinimumTime) / float(periodDuration);
            if (distanceRatio > 1) {
              distanceRatio = 1;
            }

            m_CurrentPeriodAlpha = 200;

            vertices << QVector3D(halfWidth, halfHeight, 0.0f);
            for (int j = 0; j < 360; j++) {
              int prevI = prevLeftMinimumTime + toInt(floor((j / float(360)) * prevPeriodDuration));
              if (prevI > prevRightMinimumTime) {
                prevI = prevRightMinimumTime;
              }
              int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
              if (i > rightMinimumTime) {
                i = rightMinimumTime;
              }

              float prevPitch = pitchLookupUsed.at(prevI);
              float prevPhase = float(prevI - prevLeftMinimumTime) / float(prevPeriodDuration);
              float prevCosPhase = cos(prevPhase * twoPI);
              float prevSinPhase = sin(prevPhase * twoPI);
              float prevMinStep = float(prevI - prevLeftMinimumTime) * ((pitchLookupUsed.at(prevRightMinimumTime) - pitchLookupUsed.at(prevLeftMinimumTime)) / prevPeriodDuration);
              float prevMinimumPitch = pitchLookupUsed.at(prevLeftMinimumTime) + prevMinStep;
              float prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
              float prevWidth = prevMaximumPitch - prevMinimumPitch;
              float prevComparison = (prevPitch - (prevAvgPitch + (-prevCosPhase * prevWidth * 0.5))) / prevWidth;
              float prevX = halfWidth + halfWidth * (0.8 * prevCosPhase + accuracy * prevComparison * prevCosPhase);
              float prevY = halfHeight + halfHeight * (0.8 * prevSinPhase + accuracy * prevComparison * prevSinPhase);

              float pitch = pitchLookupUsed.at(i);
              float phase = float(i - leftMinimumTime) / float(periodDuration);
              float cosPhase = cos(phase * twoPI);
              float sinPhase = sin(phase * twoPI);
              float minStep = float(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
              float thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
              float thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
              float thisWidth = thisMaximumPitch - thisMinimumPitch;
              float comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;
              float thisX = halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase);
              float thisY = halfHeight + halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase);

              vertices << QVector3D(prevX + distanceRatio * (thisX - prevX), prevY + distanceRatio * (thisY - prevY), 0.0f);
            }
            vertices << QVector3D(1.8 * halfWidth, halfHeight, 0.0f);
          }
          m_vao_currentPeriod.bind();
          m_vbo_currentPeriod.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          m_vbo_currentPeriod.bind();
          m_vbo_currentPeriod.allocate(vertices.constData(), static_cast<int>(vertices.count() * 3 * sizeof(float)));
          m_vao_currentPeriod.release();
          m_vbo_currentPeriod.release();
          m_currentPeriodCount = vertices.count();
        }
      }
    }
  }
  update();
}

void VibratoCircleWidgetGL::setAccuracy(int value)
{
  accuracy = value / 10.0;
  doUpdate();
}

void VibratoCircleWidgetGL::setType(int value)
{
  type = value;
  doUpdate();
}
