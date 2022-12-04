/***************************************************************************
                          vibratowidgetGL.cpp  -  description
                             -------------------
    begin                : May 18 2005
    copyright            : (C) 2005-2007 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <QPainter>
#include <QOpenGLContext>

#include "vibratowidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include "shader.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "musicnotes.h"

VibratoWidgetGL::VibratoWidgetGL(QWidget *parent, int nls)
  : QOpenGLWidget(parent)
{
  noteLabelOffset = nls; // The horizontal space in pixels a note label requires
  zoomFactorX = 2.0;
  zoomFactorY = 1.0;
  offsetY = 0;
  noteLabelCounter = 0;
  for (int i = 0; i < 100; i++) {
    noteLabels[i].label = QString(8,' ');
    noteLabels[i].y = 0.0f;
  }
  vibratoFont = QFont();
  vibratoFont.setPointSize(9);

  // Set the colors for all of the items with static color
  m_verticalSeparatorLinesColor = QColor(131, 144, 159);
  m_referenceLinesColor = QColor(144, 156, 170);
  m_pronyWidthBandColor = QColor(0, 0, 0, 64);
  m_pronyAveragePitchColor = QColor(0,0,0,127);
  m_vibratoPolylineColor = QColor(127, 0, 0);
  m_currentWindowBandColor = QColor(palette().color(QPalette::WindowText).red(), palette().color(QPalette::WindowText).green(), palette().color(QPalette::WindowText).blue(), 64);

  m_currentTimeLineColor = QColor(Qt::black);
  m_maximaPointsColor = QColor(255, 255, 0);
  m_minimaPointsColor = QColor(0, 255, 0);
}

VibratoWidgetGL::~VibratoWidgetGL()
{
  // Remove display lists
    QOpenGLContext* c = QOpenGLContext::currentContext();
    if (c) {
        makeCurrent();
    }

    m_vao_verticalPeriodBarsShading1.destroy();
    m_vbo_verticalPeriodBarsShading1.destroy();

    m_vao_verticalPeriodBarsShading2.destroy();
    m_vbo_verticalPeriodBarsShading2.destroy();

    m_vao_verticalSeparatorLines.destroy();
    m_vbo_verticalSeparatorLines.destroy();

    m_referenceLines.clear();

    m_vao_pronyWidthBand.destroy();
    m_vbo_pronyWidthBand.destroy();

    m_vao_pronyAveragePitch.destroy();
    m_vbo_pronyAveragePitch.destroy();

    m_vao_vibratoPolyline.destroy();
    m_vbo_vibratoPolyline.destroy();

    m_vao_currentWindowBand.destroy();
    m_vbo_currentWindowBand.destroy();

    m_vao_currentTimeLine.destroy();
    m_vbo_currentTimeLine.destroy();

    m_vao_maximaPoints.destroy();
    m_vbo_maximaPoints.destroy();

    m_vao_minimaPoints.destroy();
    m_vbo_minimaPoints.destroy();
}

void VibratoWidgetGL::initializeGL()
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

  m_vao_verticalPeriodBarsShading1.create();
  m_vbo_verticalPeriodBarsShading1.create();

  m_vao_verticalPeriodBarsShading2.create();
  m_vbo_verticalPeriodBarsShading2.create();

  m_vao_verticalSeparatorLines.create();
  m_vbo_verticalSeparatorLines.create();

  m_vao_pronyWidthBand.create();
  m_vbo_pronyWidthBand.create();

  m_vao_pronyAveragePitch.create();
  m_vbo_pronyAveragePitch.create();

  m_vao_vibratoPolyline.create();
  m_vbo_vibratoPolyline.create();

  m_vao_currentWindowBand.create();
  m_vbo_currentWindowBand.create();

  m_vao_currentTimeLine.create();
  m_vbo_currentTimeLine.create();

  m_vao_maximaPoints.create();
  m_vbo_maximaPoints.create();

  m_vao_minimaPoints.create();
  m_vbo_minimaPoints.create();
}

void VibratoWidgetGL::resizeGL(int w, int h)
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

  update();
}

void VibratoWidgetGL::paintGL()
{
    doUpdate();

  // Clear background
  QColor bg = gdata->backgroundColor();
  glClearColor(double(bg.red()) / 256.0, double(bg.green()) / 256.0, double(bg.blue()) / 256.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  QPainter p;
  p.begin(this);

  p.beginNativePainting();

  // Draw the vertical bars that indicate the vibrato periods
  MyGL::DrawShape(m_program, m_vao_verticalPeriodBarsShading1, m_vbo_verticalPeriodBarsShading1, m_verticalPeriodBarsShading1Count, GL_TRIANGLES, m_verticalPeriodBarsShading1Color);
  MyGL::DrawShape(m_program, m_vao_verticalPeriodBarsShading2, m_vbo_verticalPeriodBarsShading2, m_verticalPeriodBarsShading2Count, GL_TRIANGLES, m_verticalPeriodBarsShading2Color);

  // Draw the vertical separator lines through the extrema
    MyGL::DrawLine(m_program_line, m_vao_verticalSeparatorLines, m_vbo_verticalSeparatorLines, m_verticalSeparatorLinesCount, GL_LINES, 1.0, m_verticalSeparatorLinesColor);

  p.endNativePainting();

  // Draw the horizontal reference lines
  QPen stipplePen(m_referenceLinesColor);
  stipplePen.setStyle(Qt::DashDotDotLine);
  p.setPen(stipplePen);
  p.drawLines(m_referenceLines);

  p.beginNativePainting();

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Draw the light grey band indicating the vibratowidth according to the Prony-algorithm
  MyGL::DrawShape(m_program, m_vao_pronyWidthBand, m_vbo_pronyWidthBand, m_pronyWidthBandCount, GL_TRIANGLES, m_pronyWidthBandColor);

  // Draw the average pitch according to the Prony-algorithm
  MyGL::DrawLine(m_program_line, m_vao_pronyAveragePitch, m_vbo_pronyAveragePitch, m_pronyAveragePitchCount, GL_LINE_STRIP, 1.0, m_pronyAveragePitchColor);

  // Draw the vibrato-polyline
  MyGL::DrawLine(m_program_line, m_vao_vibratoPolyline, m_vbo_vibratoPolyline, m_vibratoPolylineCount, GL_LINE_STRIP, 2.0, m_vibratoPolylineColor);

  // Draw the light grey band indicating which time is being used in the current window
  MyGL::DrawShape(m_program, m_vao_currentWindowBand, m_vbo_currentWindowBand, m_currentWindowBandCount, GL_TRIANGLES, m_currentWindowBandColor);

  // Draw the current timeline
  glDisable(GL_LINE_SMOOTH);
  MyGL::DrawLine(m_program_line, m_vao_currentTimeLine, m_vbo_currentTimeLine, m_currentTimeLineCount, GL_LINE_STRIP, 1.0, m_currentTimeLineColor);

  // Draw the maxima & minima
  glEnable(GL_POINT_SMOOTH);
  glPointSize(3.0);
  MyGL::DrawShape(m_program, m_vao_maximaPoints, m_vbo_maximaPoints, m_maximaPointsCount, GL_POINTS, m_maximaPointsColor);
  MyGL::DrawShape(m_program, m_vao_minimaPoints, m_vbo_minimaPoints, m_minimaPointsCount, GL_POINTS, m_minimaPointsColor);

  p.endNativePainting();

  // Draw the labels
  QFontMetrics fm = QFontMetrics(vibratoFont);
  setFont(vibratoFont);

  // Draw the note labels
  for (int i = 0; i < noteLabelCounter; i++) {
    p.drawText(3, height() - noteLabels[i].y + 4, noteLabels[i].label);
    p.drawText(width() - noteLabelOffset + 3, height() - noteLabels[i].y + 4, noteLabels[i].label);
  }
}

void VibratoWidgetGL::doUpdate()
{
  noteLabelCounter = 0;

  makeCurrent();

  Channel *active = gdata->getActiveChannel();

  if (active) {
    ChannelLocker channelLocker(active);
    AnalysisData *data = active->dataAtCurrentChunk();
    if(data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData *note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      const int myStartChunk = note->startChunk();
      const int myEndChunk = note->endChunk();
      const int myCurrentChunk = active->chunkAtCurrentTime();
      const float halfHeight = 0.5 * height();
      const int maximaSize = int(note->maxima->size());
      const int minimaSize = int(note->minima->size());
      const float avgPitch = note->avgPitch();
      const int framesPerChunk = active->framesPerChunk();
      const float zoomFactorYx100 = zoomFactorY * 100;

      float windowOffset;
      large_vector<float> pitchLookupUsed = active->pitchLookupSmoothed;
      int smoothDelay = int(active->pitchBigSmoothingFilter->delay());

      QVector<QVector3D> vertices;

      if ((myEndChunk - myStartChunk) * zoomFactorX > width() - 2 * noteLabelOffset) {
        // The vibrato-polyline doesn't fit in the window
        if ((myCurrentChunk - myStartChunk) * zoomFactorX < (width() - 2 * noteLabelOffset)/2) {
          // We're at the left side of the vibrato-polyline
          windowOffset = 0 - noteLabelOffset;
        } else if ((myEndChunk - myCurrentChunk) * zoomFactorX < (width() - 2 * noteLabelOffset)/2) {
          // We're at the right side of the vibrato-polyline
          windowOffset = (myEndChunk - myStartChunk) * zoomFactorX - width() + noteLabelOffset + 1;
        } else {  // We're somewhere in the middle of the vibrato-polyline
          windowOffset = (myCurrentChunk - myStartChunk) * zoomFactorX - width()/2;
        }
      } else {  // The vibrato-polyline does fit in the window
        windowOffset = 0 - noteLabelOffset;
      }

      // Calculate the alternating vertical bars that indicate the vibrato periods
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        int color1Bars = 0;  // No. of bars with the left side at a maximum
        int color2Bars = 0;  // No. of bars with the left side at a minimum
        bool maximumFirst = true;   // The first extremum is a maximum?

        if (maximaSize + minimaSize >= 2) {  // There is at least one bar to calculate
          if (maximaSize == minimaSize) {
            if (note->maxima->at(0) < note->minima->at(0)) {
              color1Bars = maximaSize;
              color2Bars = minimaSize - 1;
              maximumFirst = true;
            } else {
              color1Bars = maximaSize - 1;
              color2Bars = minimaSize;
              maximumFirst = false;
            }
          } else {
            color1Bars = maximaSize > minimaSize ? minimaSize : maximaSize;
            color2Bars = maximaSize > minimaSize ? minimaSize : maximaSize;
            maximumFirst = maximaSize > minimaSize;
          }
        }

        float x1, x2;

        // Calculate the bars with the left side at a maximum
        vertices.clear();
        for (int i = 0; i < color1Bars; i++) {
          x1 = ((((float)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
          if (x1 > width() - noteLabelOffset) { break; }
          if (maximumFirst) {
            x2 = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          } else {
            x2 = ((((float)note->minima->at(i+1) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          }
          if (x2 < noteLabelOffset) { continue; }
          if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

          vertices << QVector3D(x1, 0, 0.0f);
          vertices << QVector3D(x2, 0, 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
          vertices << QVector3D(x2, height(), 0.0f);
          vertices << QVector3D(x2, 0, 0.0f);
        }
        m_verticalPeriodBarsShading1Color = gdata->shading1Color();

        m_vao_verticalPeriodBarsShading1.bind();
        m_vbo_verticalPeriodBarsShading1.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_verticalPeriodBarsShading1.bind();
        m_vbo_verticalPeriodBarsShading1.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
        m_vao_verticalPeriodBarsShading1.release();
        m_vbo_verticalPeriodBarsShading1.release();
        m_verticalPeriodBarsShading1Count = vertices.count();

        // Calculate the bars with the left side at a minimum
        vertices.clear();
        for (int i = 0; i < color2Bars; i++) {
          x1 = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
          if (x1 > width() - noteLabelOffset) { break; }
          if (maximumFirst) {
            x2 = ((((float)note->maxima->at(i+1) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          } else {
            x2 = ((((float)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          }
          if (x2 < noteLabelOffset) { continue; }
          if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

          vertices << QVector3D(x1, 0, 0.0f);
          vertices << QVector3D(x2, 0, 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
          vertices << QVector3D(x2, height(), 0.0f);
          vertices << QVector3D(x2, 0, 0.0f);
        }
        m_verticalPeriodBarsShading2Color = gdata->shading2Color();

        m_vao_verticalPeriodBarsShading2.bind();
        m_vbo_verticalPeriodBarsShading2.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_verticalPeriodBarsShading2.bind();
        m_vbo_verticalPeriodBarsShading2.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
        m_vao_verticalPeriodBarsShading2.release();
        m_vbo_verticalPeriodBarsShading2.release();
        m_verticalPeriodBarsShading2Count = vertices.count();

        // Calculate the vertical separator lines through the maxima
        vertices.clear();
        for (int i = 0; i < maximaSize; i++) {
          x1 = ((((float)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { continue; }
          if (x1 > width() - noteLabelOffset) { break; }
          vertices << QVector3D(x1, 0, 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
        }

        // Calculate the vertical separator lines through the minima
        for (int i = 0; i < minimaSize; i++) {
          x1 = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { continue; }
          if (x1 > width() - noteLabelOffset) { break; }
          vertices << QVector3D(x1, 0, 0.0f);
          vertices << QVector3D(x1, height(), 0.0f);
        }

        m_vao_verticalSeparatorLines.bind();
        m_vbo_verticalSeparatorLines.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_verticalSeparatorLines.bind();
        m_vbo_verticalSeparatorLines.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
        m_vao_verticalSeparatorLines.release();
        m_vbo_verticalSeparatorLines.release();
        m_verticalSeparatorLinesCount = vertices.count();
      }

      // Calculate the horizontal reference lines + note labels
      m_referenceLines.clear();
      const int nearestNote = toInt(avgPitch);
      QString noteLabel;
      const qreal referenceLineX1 = noteLabelOffset;
      const qreal referenceLineX2 = width() - noteLabelOffset;
      qreal referenceLineY;

      // Calculate the nearest reference line + note label
      referenceLineY = halfHeight + ((nearestNote - avgPitch) * zoomFactorYx100) + offsetY;
      m_referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

      if ((noteOctave(nearestNote) >= 0) && (noteOctave(nearestNote) <= 9)) {
        noteLabel = noteLabel.asprintf("%s%d", noteName(nearestNote), noteOctave(nearestNote));
      } else {
        noteLabel = noteLabel.asprintf("%s ", noteName(nearestNote));
      }
      noteLabels[noteLabelCounter].label = noteLabel;
      noteLabels[noteLabelCounter].y = referenceLineY;
      noteLabelCounter++;

      // Calculate as many reference lines + note labels above the note as can be seen
      for (int i = 1; ; i++) {
        referenceLineY = halfHeight + ((nearestNote + i - avgPitch) * zoomFactorYx100) + offsetY;
        if (referenceLineY > height()) { break; }
        m_referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

        if ((noteOctave(nearestNote + i) >= 0) && (noteOctave(nearestNote + i) <= 9)) {
          noteLabel = noteLabel.asprintf("%s%d", noteName(nearestNote + i), noteOctave(nearestNote + i));
        } else {
          noteLabel = noteLabel.asprintf("%s ", noteName(nearestNote + i));
        }
        noteLabels[noteLabelCounter].label = noteLabel;
        noteLabels[noteLabelCounter].y = referenceLineY;
        noteLabelCounter++;
      }

      // Calculate as many reference lines + note labels below the note as can be seen
      for (int i = -1; ; i--) {
        referenceLineY = halfHeight + ((nearestNote + i - avgPitch) * zoomFactorYx100) + offsetY;
        if (referenceLineY < 0) { break; }
        m_referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

        if ((noteOctave(nearestNote + i) >= 0) && (noteOctave(nearestNote + i) <= 9)) {
          noteLabel = noteLabel.asprintf("%s%d", noteName(nearestNote + i), noteOctave(nearestNote + i));
        } else {
          noteLabel = noteLabel.asprintf("%s ", noteName(nearestNote + i));
        }
        noteLabels[noteLabelCounter].label = noteLabel;
        noteLabels[noteLabelCounter].y = referenceLineY;
        noteLabelCounter++;
      }

      // Calculate the light grey band indicating the vibratowidth according to the Prony-algorithm
      vertices.clear();
      for (int chunk = myStartChunk; chunk < myEndChunk - 1; chunk++) {
        float x1 = (chunk - myStartChunk) * zoomFactorX - windowOffset;
        if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
        if (x1 > width() - noteLabelOffset) { break; }
        float x2 = (chunk + 1 - myStartChunk) * zoomFactorX - windowOffset;
        if (x2 < noteLabelOffset) { continue; }
        if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

        float vibratoPitch1 = active->dataAtChunk(chunk)->vibratoPitch;
        float vibratoWidth1 = active->dataAtChunk(chunk)->vibratoWidth;
        float vibratoPitch2 = active->dataAtChunk(chunk + 1)->vibratoPitch;
        float vibratoWidth2 = active->dataAtChunk(chunk + 1)->vibratoWidth;

        float y1 = halfHeight + ((vibratoPitch1 + vibratoWidth1 - avgPitch) * zoomFactorYx100) + offsetY;
        float y2 = halfHeight + ((vibratoPitch1 - vibratoWidth1 - avgPitch) * zoomFactorYx100) + offsetY;
        float y3 = halfHeight + ((vibratoPitch2 - vibratoWidth2 - avgPitch) * zoomFactorYx100) + offsetY;
        float y4 = halfHeight + ((vibratoPitch2 + vibratoWidth2 - avgPitch) * zoomFactorYx100) + offsetY;

        vertices << QVector3D(x1, y1, 0.0f);
        vertices << QVector3D(x2, y4, 0.0f);
        vertices << QVector3D(x1, y2, 0.0f);
        vertices << QVector3D(x1, y2, 0.0f);
        vertices << QVector3D(x2, y3, 0.0f);
        vertices << QVector3D(x2, y4, 0.0f);
      }
      m_vao_pronyWidthBand.bind();
      m_vbo_pronyWidthBand.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_pronyWidthBand.bind();
      m_vbo_pronyWidthBand.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_pronyWidthBand.release();
      m_vbo_pronyWidthBand.release();
      m_pronyWidthBandCount = vertices.count();

      // Calculate the average pitch according to the Prony-algorithm
      vertices.clear();
      for (int chunk = myStartChunk; chunk < myEndChunk; chunk++) {
        float x = (chunk - myStartChunk) * zoomFactorX - windowOffset;
        if (x < noteLabelOffset) { continue; }
        if (x > width() - noteLabelOffset) { break; }
        float y = halfHeight + ((active->dataAtChunk(chunk)->vibratoPitch - avgPitch) * zoomFactorYx100) + offsetY;
        vertices << QVector3D(x, y, 0.0f);
      }
      m_vao_pronyAveragePitch.bind();
      m_vbo_pronyAveragePitch.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_pronyAveragePitch.bind();
      m_vbo_pronyAveragePitch.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_pronyAveragePitch.release();
      m_vbo_pronyAveragePitch.release();
      m_pronyAveragePitchCount = vertices.count();

      // Calculate the vibrato-polyline
      vertices.clear();
      if ((active->doingDetailedPitch()) && (pitchLookupUsed.size() > 0)) {
        // Detailed pitch information available, calculate polyline using this info
        const int pitchLookupUsedSizeLimit = int(pitchLookupUsed.size() - 1);
        const int beginningOfNote = myStartChunk * framesPerChunk;
        const int endOfNote = myEndChunk * framesPerChunk - 1;
        float chunk;
        float y;
        int offset;
        for (int x = noteLabelOffset; x < width() - noteLabelOffset; x++) {
          chunk = ((x + windowOffset) / zoomFactorX + myStartChunk);
          if ((chunk >= myStartChunk) && (chunk <= myEndChunk)) {
            offset = toInt(chunk * framesPerChunk + smoothDelay);
            if(offset > endOfNote) offset = endOfNote;
            if (offset < beginningOfNote + 3*smoothDelay) {
              int timeAtZero = beginningOfNote + smoothDelay;
              float scaleX = (offset - timeAtZero) / float(2*smoothDelay);
              float pitchAtZero = active->dataAtChunk(myStartChunk)->pitch;
              //float pitchAt2SmoothDelay = pitchLookupUsed.at(beginningOfNote + 3*smoothDelay);
              int smoothDelayPos3 = std::min(beginningOfNote + 3*smoothDelay, (int)pitchLookupUsed.size()-1);
              if(smoothDelayPos3 >= 0) {
                float pitchAt2SmoothDelay = pitchLookupUsed.at(smoothDelayPos3);
                y = halfHeight + (pitchAtZero + scaleX * (pitchAt2SmoothDelay - pitchAtZero) - avgPitch) * zoomFactorYx100;
              } else { y = 0; }
            } else {
              offset = std::min(offset, pitchLookupUsedSizeLimit);
              y = halfHeight + (pitchLookupUsed.at(offset) - avgPitch) * zoomFactorYx100;
            }

            y += offsetY;  // Vertical scrollbar offset
            vertices << QVector3D(x, y, 0.0f);
          }
        }
      } else {  // No detailed pitch information available, calculate polyline using the chunkdata
        float x, y;
        for (int chunk = myStartChunk; chunk < myEndChunk; chunk++) {
          x = (chunk - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((active->dataAtChunk(chunk)->pitch - avgPitch) * zoomFactorYx100) + offsetY;
          vertices << QVector3D(x, y, 0.0f);
        }
      }
      m_vao_vibratoPolyline.bind();
      m_vbo_vibratoPolyline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_vibratoPolyline.bind();
      m_vbo_vibratoPolyline.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_vibratoPolyline.release();
      m_vbo_vibratoPolyline.release();
      m_vibratoPolylineCount = vertices.count();

      // Calculate a light grey band indicating which time is being used in the current window
      vertices.clear();
      const double halfWindowTime = (double)active->size() / (double)(active->rate() * 2);
      int pixelLeft = toInt((active->chunkAtTime(gdata->view->currentTime() - halfWindowTime) - myStartChunk) * zoomFactorX - windowOffset);
      int pixelRight = toInt((active->chunkAtTime(gdata->view->currentTime() + halfWindowTime) - myStartChunk) * zoomFactorX - windowOffset);

      vertices << QVector3D(pixelLeft, 0, 0.0f);
      vertices << QVector3D(pixelRight, 0, 0.0f);
      vertices << QVector3D(pixelLeft, height(), 0.0f);
      vertices << QVector3D(pixelLeft, height(), 0.0f);
      vertices << QVector3D(pixelRight, height(), 0.0f);
      vertices << QVector3D(pixelRight, 0, 0.0f);

      m_vao_currentWindowBand.bind();
      m_vbo_currentWindowBand.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_currentWindowBand.bind();
      m_vbo_currentWindowBand.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_currentWindowBand.release();
      m_vbo_currentWindowBand.release();
      m_currentWindowBandCount = vertices.count();

      // Calculate the current timeline
      vertices.clear();
      const float timeLineX = toInt((myCurrentChunk - myStartChunk) * zoomFactorX - windowOffset);
      vertices << QVector3D(timeLineX, 0, 0.0f);
      vertices << QVector3D(timeLineX, height(), 0.0f);

      m_vao_currentTimeLine.bind();
      m_vbo_currentTimeLine.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_currentTimeLine.bind();
      m_vbo_currentTimeLine.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_currentTimeLine.release();
      m_vbo_currentTimeLine.release();
      m_currentTimeLineCount = vertices.count();

      // Calculate the points of maxima and minima
      vertices.clear();
      // Calculate the maxima
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        float x, y;
        for (int i = 0; i < maximaSize; i++) {
          x = ((((float)note->maxima->at(i) - smoothDelay)/ framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((pitchLookupUsed.at(note->maxima->at(i)) - avgPitch) * zoomFactorYx100) + offsetY;
          vertices << QVector3D(x, y, 0.0f);
        }
      }
      m_vao_maximaPoints.bind();
      m_vbo_maximaPoints.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_maximaPoints.bind();
      m_vbo_maximaPoints.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_maximaPoints.release();
      m_vbo_maximaPoints.release();
      m_maximaPointsCount = vertices.count();

      // Calculate the minima
      vertices.clear();
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        float x, y;
        for (int i = 0; i < minimaSize; i++) {
          x = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((pitchLookupUsed.at(note->minima->at(i)) - avgPitch) * zoomFactorYx100) + offsetY;
          vertices << QVector3D(x, y, 0.0f);
        }
      }
      m_vao_minimaPoints.bind();
      m_vbo_minimaPoints.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vbo_minimaPoints.bind();
      m_vbo_minimaPoints.allocate(vertices.constData(), vertices.count() * 3 * sizeof(float));
      m_vao_minimaPoints.release();
      m_vbo_minimaPoints.release();
      m_minimaPointsCount = vertices.count();
    }
  }
}

void VibratoWidgetGL::setZoomFactorX(double x)
{
  zoomFactorX = 2*pow10(log2(x/25));
  update();
}

void VibratoWidgetGL::setZoomFactorY(double y)
{
  zoomFactorY = y;
  update();
}

void VibratoWidgetGL::setOffsetY(int value)
{
  offsetY = value;
  update();
}
