/***************************************************************************
                          vibratospeedwidgetGL.cpp  -  description
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
#include "vibratospeedwidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include "shader.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoSpeedWidgetGL::VibratoSpeedWidgetGL(QWidget* parent)
  : QOpenGLWidget(parent)
{
  speedValueToDraw = 0;
  widthValueToDraw = 0;
  prevVibratoSpeed = 0;
  prevVibratoWidth = 0;
  widthLimit = 50;
  prevNoteNumber = -1;
  speedLabelCounter = 0;
  widthLabelCounter = 0;
  for (int i = 0; i < 100; i++) {
    speedLabels[i].label = QString(8, ' ');
    speedLabels[i].x = 0.0f;
    speedLabels[i].y = 0.0f;
    widthLabels[i].label = QString(8, ' ');
    widthLabels[i].x = 0.0f;
    widthLabels[i].y = 0.0f;
  }
  speedWidthFont = QFont();
  speedWidthFont.setPointSize(9);

  msgCount = 1;
}

VibratoSpeedWidgetGL::~VibratoSpeedWidgetGL()
{
  // Remove display lists
  QOpenGLContext* c = QOpenGLContext::currentContext();
  if (c) {
    makeCurrent();
  }

  m_vao_dial_speed[VBO_DIAL].destroy();
  m_vao_dial_speed[VBO_ARC].destroy();
  m_vao_dial_speed[VBO_MARKERS].destroy();
  m_vao_needle_speed.destroy();
  m_vbo_dial_speed[VBO_DIAL].destroy();
  m_vbo_dial_speed[VBO_ARC].destroy();
  m_vbo_needle_speed.destroy();

  m_vao_dial_width[VBO_DIAL].destroy();
  m_vao_dial_width[VBO_ARC].destroy();
  m_vao_dial_width[VBO_MARKERS].destroy();
  m_vao_needle_width.destroy();
  m_vbo_dial_width[VBO_DIAL].destroy();
  m_vbo_dial_width[VBO_ARC].destroy();
  m_vbo_needle_width.destroy();

  m_program.deleteLater();
  m_program_line.deleteLater();
  doneCurrent();
}

void VibratoSpeedWidgetGL::initializeGL()
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
  m_vao_needle_speed.create();
  m_vbo_needle_speed.create();
  m_vao_dial_speed[VBO_DIAL].create();
  m_vao_dial_speed[VBO_ARC].create();
  m_vao_dial_speed[VBO_MARKERS].create();
  m_vbo_dial_speed[VBO_DIAL].create();
  m_vbo_dial_speed[VBO_ARC].create();
  m_vbo_dial_speed[VBO_MARKERS].create();

  m_vao_needle_width.create();
  m_vbo_needle_width.create();
  m_vao_dial_width[VBO_DIAL].create();
  m_vao_dial_width[VBO_ARC].create();
  m_vao_dial_width[VBO_MARKERS].create();
  m_vbo_dial_width[VBO_DIAL].create();
  m_vbo_dial_width[VBO_ARC].create();
  m_vbo_dial_width[VBO_MARKERS].create();
}

void VibratoSpeedWidgetGL::resizeGL(int w, int h)
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

  // Calculate the dials with the new width & height

  // The following calculates in 0 to width and 0 to height in X and Y
  const float halfWidth = 0.5 * width();
  const float halfHeight = 0.5 * height();
  const float radius = 1.8 * MAX(halfHeight, halfWidth);
  const float theta = asin(float(width()) / (2 * radius));
  const float rho = (PI / 2.0) - theta;

    // Calculate the speed dial with the new width & height
  const float speedCenterX = halfWidth;
  const float speedCenterY = height() - radius;

  // Calculate the white semicircle of the dial and the black outline of the dial
  float speedUpperArcTheta = -theta;
  float speedUpperArcThetaStep = (2 * theta) / width();
  QVector<QVector3D> speedUpperArc;
  speedUpperArc << QVector3D((float)speedCenterX, (float)speedCenterY, 0.0);
  for (int x = 0; x < width(); x++) {
    speedUpperArc << QVector3D((float)x, float(speedCenterY + (sin(0.5 * PI - speedUpperArcTheta) * radius)), 0.0f);
    speedUpperArcTheta += speedUpperArcThetaStep;
  }

  m_vao_dial_speed[VBO_DIAL].bind();
  m_vbo_dial_speed[VBO_DIAL].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_speed[VBO_DIAL].bind();
  m_vbo_dial_speed[VBO_DIAL].allocate(speedUpperArc.constData(), speedUpperArc.count() * 3 * sizeof(float));
  m_vao_dial_speed[VBO_DIAL].release();
  m_vbo_dial_speed[VBO_DIAL].release();
  m_dial_count_speed[VBO_DIAL] = speedUpperArc.count();

  // Calculate the dial arc
  float speedLowerArcTheta = -theta;
  float speedLowerArcThetaStep = (2 * theta) / halfWidth;
  QVector<QVector3D> speedLowerArc;
  for (float x = 0.25 * width(); x < 0.75 * width(); x++) {
    speedLowerArc << QVector3D(x, speedCenterY + (0.5 * sin(0.5 * PI - speedLowerArcTheta) * radius), 0.0f);
    speedLowerArcTheta += speedLowerArcThetaStep;
  }
  m_vao_dial_speed[VBO_ARC].bind();
  m_vbo_dial_speed[VBO_ARC].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_speed[VBO_ARC].bind();
  m_vbo_dial_speed[VBO_ARC].allocate(speedLowerArc.constData(), speedLowerArc.count() * 3 * sizeof(float));
  m_vao_dial_speed[VBO_ARC].release();
  m_vbo_dial_speed[VBO_ARC].release();
  m_dial_count_speed[VBO_ARC] = speedLowerArc.count();

  // Calculate line markings & text labels
  float speedStep = (2 * theta) / 14.0;
  speedLabelCounter = 0;
  const char* speedLabelLookup[13] = { "12   ", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0" };

  QVector<QVector3D> speedLineMarking;
  for (int j = 0; j < 13; j++) {
    float i = rho + speedStep * (j + 1);
    float startX = speedCenterX + radius * cos(i);
    float startY = speedCenterY + radius * sin(i);
    if (width() < 175) {  // Small dial
      if ((j % 4) == 0) {
        // Bigger marking + text label
        speedLineMarking << QVector3D(startX, startY, 0.0f);
        speedLineMarking << QVector3D(startX + 0.05 * (speedCenterX - startX), startY + 0.05 * (speedCenterY - startY), 0.0f);

        speedLabels[speedLabelCounter].x = startX + 0.08 * (speedCenterX - startX);
        speedLabels[speedLabelCounter].y = startY + 0.08 * (speedCenterY - startY);
        speedLabels[speedLabelCounter++].label = speedLabelLookup[j];
      } else {
        // Smaller marking, no text label
        speedLineMarking << QVector3D(startX, startY, 0.0f);
        speedLineMarking << QVector3D(startX + 0.03 * (speedCenterX - startX), startY + 0.03 * (speedCenterY - startY), 0.0f);
      }
    } else if (width() < 300) {  // Bigger dial
      if ((j % 2) == 0) {
        // Bigger marking + text label
        speedLabelLookup[0] = "12";

        speedLineMarking << QVector3D(startX, startY, 0.0f);
        speedLineMarking << QVector3D(startX + 0.05 * (speedCenterX - startX), startY + 0.05 * (speedCenterY - startY), 0.0f);

        speedLabels[speedLabelCounter].x = startX + 0.08 * (speedCenterX - startX);
        speedLabels[speedLabelCounter].y = startY + 0.08 * (speedCenterY - startY);
        speedLabels[speedLabelCounter++].label = speedLabelLookup[j];
      } else {
        // Smaller marking, no text label
        speedLineMarking << QVector3D(startX, startY, 0.0f);
        speedLineMarking << QVector3D(startX + 0.03 * (speedCenterX - startX), startY + 0.03 * (speedCenterY - startY), 0.0f);
      }
    } else {  // Big dial
      // Bigger marking + text label
      speedLabelLookup[0] = "12";

      speedLineMarking << QVector3D(startX, startY, 0.0f);
      speedLineMarking << QVector3D(startX + 0.05 * (speedCenterX - startX), startY + 0.05 * (speedCenterY - startY), 0.0f);

      speedLabels[speedLabelCounter].x = startX + 0.08 * (speedCenterX - startX);
      speedLabels[speedLabelCounter].y = startY + 0.08 * (speedCenterY - startY);
      speedLabels[speedLabelCounter++].label = speedLabelLookup[j];
    }
  }
  m_vao_dial_speed[VBO_MARKERS].bind();
  m_vbo_dial_speed[VBO_MARKERS].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_speed[VBO_MARKERS].bind();
  m_vbo_dial_speed[VBO_MARKERS].allocate(speedLineMarking.constData(), speedLineMarking.count() * 3 * sizeof(float));
  m_vao_dial_speed[VBO_MARKERS].release();
  m_vbo_dial_speed[VBO_MARKERS].release();
  m_dial_count_speed[VBO_MARKERS] = speedLineMarking.count();

  // Calculate the position of the "Hz" label
  hzLabelX = speedCenterX;
  hzLabelY = speedCenterY + 0.7 * radius;

  // Calculate the width dial with the new width & height
  const float widthCenterX = halfWidth;
  const float widthCenterY = halfHeight - radius;

  // Calculate the white semicircle of the dial and the black outline of the dial
  float widthUpperArcTheta = -theta;
  float widthUpperArcThetaStep = (2 * theta) / width();
  QVector<QVector3D> widthUpperArc;
  widthUpperArc << QVector3D((float)widthCenterX, (float)widthCenterY, 0.0);
  for (int x = 0; x < width(); x++) {
    widthUpperArc << QVector3D((float)x, (float)(widthCenterY + (sin(0.5 * PI - widthUpperArcTheta) * radius)), 0.0f);
    widthUpperArcTheta += widthUpperArcThetaStep;
  }
  m_vao_dial_width[VBO_DIAL].bind();
  m_vbo_dial_width[VBO_DIAL].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_width[VBO_DIAL].bind();
  m_vbo_dial_width[VBO_DIAL].allocate(widthUpperArc.constData(), widthUpperArc.count() * 3 * sizeof(float));
  m_vao_dial_width[VBO_DIAL].release();
  m_vbo_dial_width[VBO_DIAL].release();
  m_dial_count_width[VBO_DIAL] = widthUpperArc.count();

  // Calculate the black arc inside the dial
  float widthLowerArcTheta = -theta;
  float widthLowerArcThetaStep = (2 * theta) / halfWidth;
  QVector<QVector3D> widthLowerArc;
  for (float x = 0.25 * width(); x < 0.75 * width(); x++) {
    widthLowerArc << QVector3D(x, widthCenterY + (0.5 * sin(0.5 * PI - widthLowerArcTheta) * radius), 0.0f);
    widthLowerArcTheta += widthLowerArcThetaStep;
  }
  m_vao_dial_width[VBO_ARC].bind();
  m_vbo_dial_width[VBO_ARC].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_width[VBO_ARC].bind();
  m_vbo_dial_width[VBO_ARC].allocate(widthLowerArc.constData(), widthLowerArc.count() * 3 * sizeof(float));
  m_vao_dial_width[VBO_ARC].release();
  m_vbo_dial_width[VBO_ARC].release();
  m_dial_count_width[VBO_ARC] = widthLowerArc.count();

  // Calculate line markings & text labels
  float widthStep = (2 * theta) / 12.0;
  widthLabelCounter = 0;
  char widthLabel[5];

  QVector<QVector3D> widthLineMarking;
  for (int j = 0; j < 11; j++) {
    float i = rho + widthStep * (j + 1);
    float startX = widthCenterX + radius * cos(i);
    float startY = widthCenterY + radius * sin(i);
    if (width() < 250) {
      if ((j % 2) == 0) {
        // Bigger marking + text label
        widthLineMarking << QVector3D(startX, startY, 0.0f);
        widthLineMarking << QVector3D(startX + 0.05 * (widthCenterX - startX), startY + 0.05 * (widthCenterY - startY), 0.0f);

        widthLabels[widthLabelCounter].x = startX + 0.08 * (widthCenterX - startX);
        widthLabels[widthLabelCounter].y = startY + 0.08 * (widthCenterY - startY);
        sprintf(widthLabel, "%d", widthLimit - ((widthLimit / 10) * j));
        widthLabels[widthLabelCounter++].label = widthLabel;
      } else {
        // Smaller marking, no text label
        widthLineMarking << QVector3D(startX, startY, 0.0f);
        widthLineMarking << QVector3D(startX + 0.03 * (widthCenterX - startX), startY + 0.03 * (widthCenterY - startY), 0.0f);
      }
    } else {
      // Bigger marking + text label
      widthLineMarking << QVector3D(startX, startY, 0.0f);
      widthLineMarking << QVector3D(startX + 0.05 * (widthCenterX - startX), startY + 0.05 * (widthCenterY - startY), 0.0f);

      widthLabels[widthLabelCounter].x = startX + 0.08 * (widthCenterX - startX);
      widthLabels[widthLabelCounter].y = startY + 0.08 * (widthCenterY - startY);
      sprintf(widthLabel, "%d", widthLimit - ((widthLimit / 10) * j));
      widthLabels[widthLabelCounter++].label = widthLabel;
    }
  }
  m_vao_dial_width[VBO_MARKERS].bind();
  m_vbo_dial_width[VBO_MARKERS].setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_vbo_dial_width[VBO_MARKERS].bind();
  m_vbo_dial_width[VBO_MARKERS].allocate(widthLineMarking.constData(), widthLineMarking.count() * 3 * sizeof(float));
  m_vao_dial_width[VBO_MARKERS].release();
  m_vbo_dial_width[VBO_MARKERS].release();
  m_dial_count_width[VBO_MARKERS] = widthLineMarking.count();

  // Calculate the "Cents" label
  centsLabelX = widthCenterX;
  centsLabelY = widthCenterY + 0.7 * radius;

  glEndList();

  // Do forced update on resize
  prevVibratoSpeed = -999;
  prevVibratoWidth = -999;

  doUpdate();
}

void VibratoSpeedWidgetGL::paintGL()
{
  QColor bg = gdata->backgroundColor();
  glClearColor(double(bg.red()) / 255.0, double(bg.green()) / 255.0, double(bg.blue()) / 255.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  QPainter p;
  p.begin(this);

  QFontMetrics fm = QFontMetrics(speedWidthFont);
  setFont(speedWidthFont);
  int halfFontHeight = fm.height() / 2;
  int halfFontWidth;

  p.beginNativePainting();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Draw each of the dial VBOs.
  // Dial Face
  // Draw the speed dial
  MyGL::DrawShape(m_program, m_vao_dial_speed[VBO_DIAL], m_vbo_dial_speed[VBO_DIAL], m_dial_count_speed[VBO_DIAL], GL_TRIANGLE_FAN, QColor(Qt::white));

  // Draw the Dial Outline using a line width
  MyGL::DrawLine(m_program_line, m_vao_dial_speed[VBO_DIAL], m_vbo_dial_speed[VBO_DIAL], m_dial_count_speed[VBO_DIAL], GL_LINE_LOOP, 3.0, QColor(Qt::black));

  // Lower Arc with line width
  MyGL::DrawLine(m_program_line, m_vao_dial_speed[VBO_ARC], m_vbo_dial_speed[VBO_ARC], m_dial_count_speed[VBO_ARC], GL_LINE_STRIP, 3.0, QColor(Qt::black));

  // Line Markings with line width
  MyGL::DrawLine(m_program_line, m_vao_dial_speed[VBO_MARKERS], m_vbo_dial_speed[VBO_MARKERS], m_dial_count_speed[VBO_MARKERS], GL_LINES, 3.0, QColor(Qt::black));

  p.endNativePainting();

  // Draw the labels
  // Take into account that QPainter origin is upper left while OpenGL is lower left.
  p.drawText(hzLabelX - (0.5 * fm.horizontalAdvance("Hz")), height() - hzLabelY, "Hz");
  for (int i = 0; i < speedLabelCounter; i++) {
    halfFontWidth = fm.horizontalAdvance(speedLabels[i].label)/2;
    p.drawText(speedLabels[i].x - halfFontWidth, height() - (speedLabels[i].y - halfFontHeight), speedLabels[i].label);
  }

  p.beginNativePainting();
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Draw the speed needle
  if (speedValueToDraw != 0) {
    // Draw the needle inside
    MyGL::DrawShape(m_program, m_vao_needle_speed, m_vbo_needle_speed, m_needle_count_speed, GL_TRIANGLES, QColor(Qt::red));
    // Draw the needle outside with a line width
    MyGL::DrawLine(m_program_line, m_vao_needle_speed, m_vbo_needle_speed, m_needle_count_speed, GL_LINE_LOOP, 3.0, QColor(Qt::darkRed));
  }
  //======================================================

  // Draw the width dial
  MyGL::DrawShape(m_program, m_vao_dial_width[VBO_DIAL], m_vbo_dial_width[VBO_DIAL], m_dial_count_width[VBO_DIAL], GL_TRIANGLE_FAN, QColor(Qt::white));

  // Draw the Dial Outline using a line width
  MyGL::DrawLine(m_program_line, m_vao_dial_width[VBO_DIAL], m_vbo_dial_width[VBO_DIAL], m_dial_count_width[VBO_DIAL], GL_LINE_LOOP, 2.0, QColor(Qt::black));

  // Lower Arc with line width
  MyGL::DrawLine(m_program_line, m_vao_dial_width[VBO_ARC], m_vbo_dial_width[VBO_ARC], m_dial_count_width[VBO_ARC], GL_LINE_STRIP, 2.0, QColor(Qt::black));

  // Line Markings with line width
  MyGL::DrawLine(m_program_line, m_vao_dial_width[VBO_MARKERS], m_vbo_dial_width[VBO_MARKERS], m_dial_count_width[VBO_MARKERS], GL_LINES, 2.0, QColor(Qt::black));

  p.endNativePainting();

  // Draw the labels
  // Take into account that QPainter origin is upper left while OpenGL is lower left.
  p.drawText(centsLabelX - (0.5 * fm.horizontalAdvance("Cents")), height() - centsLabelY, "Cents");
  for (int i = 0; i < widthLabelCounter; i++) {
    halfFontWidth = fm.horizontalAdvance(widthLabels[i].label) / 2;
    p.drawText(widthLabels[i].x - halfFontWidth, height() - (widthLabels[i].y - halfFontHeight), widthLabels[i].label);
  }

  p.beginNativePainting();

  // Draw the width needle
  if (widthValueToDraw != 0) {
    // Draw the needle inside
    MyGL::DrawShape(m_program, m_vao_needle_width, m_vbo_needle_width, m_needle_count_width, GL_TRIANGLES, QColor(Qt::red));
    // Draw the needle outside with a line width
    MyGL::DrawLine(m_program_line, m_vao_needle_width, m_vbo_needle_width, m_needle_count_width, GL_LINE_LOOP, 3.0, QColor(Qt::darkRed));
  }

  p.endNativePainting();

  p.end();
}

void VibratoSpeedWidgetGL::doUpdate()
{
  Channel* active = gdata->getActiveChannel();

  float vibratoSpeed = 0;
  float vibratoWidth = 0;
  int currentNoteNumber = -1;

  if (active) {
    AnalysisData* data;
    if (gdata->soundMode & SOUND_REC) data = active->dataAtChunk(active->chunkAtCurrentTime() - active->pronyDelay());
    else data = active->dataAtCurrentChunk();
    if (data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      currentNoteNumber = data->noteIndex;
      vibratoSpeed = data->vibratoSpeed;
      vibratoWidth = 200 * data->vibratoWidth;
    }
  }
  msgCount++;

 if ((fabs(prevVibratoSpeed - vibratoSpeed) < 0.05) && (fabs(prevVibratoWidth - vibratoWidth) < 0.05)) {
    // Needle values haven't changed (much) , no update needed
  return;

 } else {
    // Needle values have changed
    QVector<QVector3D> needleVectorsSpeed;
    QVector<QVector3D> needleVectorsWidth;

    if ((vibratoSpeed == 0) && (vibratoWidth == 0)) {
      // No needle values, don't draw the needles this update

      prevVibratoSpeed = 0;
      prevVibratoWidth = 0;

      speedValueToDraw = 0;
      widthValueToDraw = 0;
      prevNoteNumber = -1;
      update();
    } else {
      // Needle values, draw the needles this update

      prevVibratoSpeed = vibratoSpeed;
      prevVibratoWidth = vibratoWidth;

      speedValueToDraw = vibratoSpeed;
      widthValueToDraw = vibratoWidth;

      const float halfWidth = 0.5 * width();
      const float halfHeight = 0.5 * height();
      const float radius = 1.8 * MAX(0.5 * height(), halfWidth);
      const float theta = asin(float(width()) / (2 * radius));
      const float rho = (PI * 0.5) - theta;
      const int halfKnobWidth = MAX(toInt(radius * 0.02), 1);

      // Calculate the speed needle, if speedValueToDraw is set to sensible value
      if ((speedValueToDraw > 0) && (speedValueToDraw <= 12)) {
        const float centerX = halfWidth;
        const float centerY = height() - radius;

        float hzAngle = (2 * theta) / 14.0;
        float note = PI - rho - (speedValueToDraw * hzAngle) - hzAngle;

        float noteX = centerX + radius * cos(note);
        float noteY = centerY + radius * sin(note);
        float knobLeftX = centerX - halfKnobWidth;
        float knobLeftY = centerY;
        float knobRightX = centerX + halfKnobWidth;
        float knobRightY = centerY;

        // Inside and outside of the needle
        needleVectorsSpeed << QVector3D(noteX, noteY, 0.0f);
        needleVectorsSpeed << QVector3D(knobLeftX, knobLeftY, 0.0f);
        needleVectorsSpeed << QVector3D(knobRightX, knobRightY, 0.0f);

        m_vao_needle_speed.bind();
        m_vbo_needle_speed.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_needle_speed.bind();
        m_vbo_needle_speed.allocate(needleVectorsSpeed.constData(), needleVectorsSpeed.count() * 3 * sizeof(float));
        m_vao_needle_speed.release();
        m_vbo_needle_speed.release();
        m_needle_count_speed = needleVectorsSpeed.count();
      } else {
        speedValueToDraw = 0;
      }

      // Determine whether the scale needs adjusting
      int oldWidthLimit = widthLimit;
      if ((widthValueToDraw > 0) && (currentNoteNumber == prevNoteNumber)) {
        if (widthValueToDraw > 50) {
          widthLimit = std::max(widthLimit, 100);
        }
        if (widthValueToDraw > 100) {
          widthLimit = std::max(widthLimit, 200);
        }
        if (widthValueToDraw > 200) {
          widthLimit = std::max(widthLimit, 300);
        }
      }
      if (prevNoteNumber != currentNoteNumber) {
        widthLimit = 50;
      }
      if (widthLimit != oldWidthLimit) {
        // Do resize and redraw the dial if the scale is adjusted
        resizeGL(width(), height());
      }
      prevNoteNumber = currentNoteNumber;

      // Calculate the width needle, if widthValueToDraw is set to sensible value
      if ((widthValueToDraw > 0) && (widthValueToDraw <= widthLimit)) {
        const float centerX = halfWidth;
        const float centerY = halfHeight - radius;

        float centAngle = (2 * theta) / (widthLimit + 2 * (widthLimit / 10));
        float note = PI - rho - (widthValueToDraw * centAngle) - (widthLimit / 10) * centAngle;
        int halfKnobWidth = MAX(toInt(radius * 0.02), 1);

        float noteX = centerX + radius * cos(note);
        float noteY = centerY + radius * sin(note);
        float knobLeftX = centerX - halfKnobWidth;
        float knobLeftY = centerY;
        float knobRightX = centerX + halfKnobWidth;
        float knobRightY = centerY;

        // Inside and outside of the needle
        needleVectorsWidth << QVector3D(noteX, noteY, 0.0f);
        needleVectorsWidth << QVector3D(knobLeftX, knobLeftY, 0.0f);
        needleVectorsWidth << QVector3D(knobRightX, knobRightY, 0.0f);

        m_vao_needle_width.bind();
        m_vbo_needle_width.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_vbo_needle_width.bind();
        m_vbo_needle_width.allocate(needleVectorsWidth.constData(), needleVectorsWidth.count() * 3 * sizeof(float));
        m_vao_needle_width.release();
        m_vbo_needle_width.release();
        m_needle_count_width = needleVectorsWidth.count();
      } else {
        widthValueToDraw = 0;
      }
      update();
    }
  }
}
