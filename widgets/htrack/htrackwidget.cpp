/***************************************************************************
                          htrackwidget.cpp  -  description
                             -------------------
    begin                : Aug 2002
    copyright            : (C) 2002-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Please read LICENSE.txt for details.
 ***************************************************************************/

#include <QMouseEvent>
#include <QWheelEvent>

#include "htrackwidget.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include "shader.h"
#include "mygl.h"

#include "gdata.h"
#include "useful.h"
#include "channel.h"
#include "array1d.h"
#include "piano3d.h"
#include "musicnotes.h"

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

HTrackWidget::HTrackWidget(QWidget* parent, const char* name)
  : QOpenGLWidget(parent)
{
  setObjectName(name);
}

HTrackWidget::~HTrackWidget()
{
  delete piano3d;
}

void HTrackWidget::initializeGL()
{
  initializeOpenGLFunctions();

   // build and compile our shader program
  // ------------------------------------
  try {
    Shader ourShader(&m_program_camera, ":/camera.vs.glsl", ":/shader.fs.glsl");
  } catch (...) {
    close();
  }

  try {
    Shader ourShader(&m_program_lighting, ":/shaderMaterial.vs.glsl", ":/shaderMaterial.fs.glsl");
  } catch (...) {
    close();
  }

  // Create the vao/vbo items

  // Set some appropriate values vefore creating the piano.

  setPeakThreshold(0.05f);
  setDistanceAway(1500.0);
  setViewAngleVertical(-35.0);
  setViewAngleHorizontal(20.0);
  translateX = 0.0;
  translateY = -60.0;

  piano3d = new Piano3d();

  glEnable(GL_NORMALIZE);
  glEnable(GL_DEPTH_TEST);

  m_model.setToIdentity();

#ifdef DWS
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  setLightSpecular(0.5, 0.5, 0.5);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
#endif
}

void HTrackWidget::resizeGL(int w, int h)
{
  // setup viewport, projection etc.:
  glViewport(0, 0, w, h);
}

void HTrackWidget::paintGL()
{
  Channel* active = gdata->getActiveChannel();

  QColor bg = gdata->backgroundColor();
  glClearColor(double(bg.red()) / 256.0, double(bg.green()) / 256.0, double(bg.blue()) / 256.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Pass Projection Matrix to shader
  // Setup model, view, and projection matrices.
  m_projection.setToIdentity();

  double nearPlane = 100.0;
  double w2 = double(width()) / 2.0;
  double h2 = double(height()) / 2.0;
  double ratio = nearPlane / double(width());
  m_projection.frustum((-w2 - translateX) * ratio,
    (w2 - translateX) * ratio,
    (-h2 - translateY) * ratio,
    (h2 - translateY) * ratio,
    nearPlane, 10000.0);


  m_model.setToIdentity();
  m_view.setToIdentity();

  double center_x = 0.0, center_z = 0.0;
  m_view.lookAt(QVector3D(0.0, 0.0/*_distanceAway*/, _distanceAway), QVector3D(center_x, 0.0, center_z), QVector3D(0.0, 1.0, 0.0));

  m_view.rotate(-viewAngleVertical(), 1.0, 0.0, 0.0);
  m_view.rotate(viewAngleHorizontal(), 0.0, 1.0, 0.0);

  m_program_camera.bind();
  m_program_camera.setUniformValue("projection", m_projection);
  m_program_camera.setUniformValue("view", m_view);
  m_program_camera.release();

#ifdef DWS
  // Setup lighting
  setLightPosition(0.0, 2000.0, -1600.0);
  setLightAmbient(0.4f, 0.4f, 0.4f);
  setLightDiffuse(0.9f, 0.9f, 0.9f);
#endif

  // Draw the piano keyboard.
  double pianoWidth = piano3d->pianoWidth();
  QMatrix4x4 pianoModel = m_model;
  pianoModel.translate(-pianoWidth / 2.0, 0.0, 0.0);//set center of keyboard at origin

  piano3d->setAllKeyStatesOff();
  if (active) {
    AnalysisData* data = active->dataAtCurrentChunk();
    if (data && active->isVisibleChunk(data)) {
      piano3d->setMidiKeyState(toInt(data->pitch), true);
    }
  }
#ifdef DWS
  glLineWidth(0.01f);
#endif
  piano3d->draw(m_program_camera, pianoModel);

#ifdef DWS
  // Change the lighting
  setLightDirection(-1.0, 0.0, 0.0);
  setLightAmbient(0.2f, 0.2f, 0.2f);
  setLightDiffuse(0.9f, 0.9f, 0.9f);
#endif

  // Change the scaling
  QMatrix4x4 harmonicModel = pianoModel;
  harmonicModel.translate(-piano3d->firstKeyOffset, 0.0f, 0.0f);
  harmonicModel.scale(OCTAVE_WIDTH / 12.0f, 200.0f, 5.0f); //set a scale of 1 semitime = 1 unit
  m_program_camera.bind();
  m_program_camera.setUniformValue("model", harmonicModel);
  m_program_camera.release();

#ifdef DWS
  glTranslatef(-piano3d->firstKeyOffset, 0.0, 0.0);
  glScaled(OCTAVE_WIDTH / 12.0, 200.0, 5.0); //set a scale of 1 semitime = 1 unit

  glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
  glLineWidth(1.0);
#endif

  if (active) {
    active->lock();
    double pos;
    int j;
    uint numHarmonics = 40;
    int visibleChunks = 512; //128;
    Array2d<float> pitches(numHarmonics, visibleChunks);
    Array2d<float> amps(numHarmonics, visibleChunks);

    AnalysisData* data;
    int finishChunk = active->currentChunk();
    int startChunk = finishChunk - visibleChunks;
    uint harmonic;
    int chunkOffset;

    //draw the time ref lines
    {
      QVector<QVector3D> refLinePoints;
#ifdef DWS
      glBegin(GL_LINES);
#endif
      for (j = roundUp(startChunk, 16); j < finishChunk; j += 16) {
        if (active->isValidChunk(j)) {
          refLinePoints << QVector3D(piano3d->firstKey(), 0.0, double(j - finishChunk));
          refLinePoints << QVector3D(piano3d->firstKey() + piano3d->numKeys(), 0.0, double(j - finishChunk));
#ifdef DWS
          glVertex3f(piano3d->firstKey(), 0.0, double(j - finishChunk));
          glVertex3f(piano3d->firstKey() + piano3d->numKeys(), 0.0, double(j - finishChunk));
#endif
        }
      }
#ifdef DWS
      glEnd();
#endif
      QOpenGLVertexArrayObject vao_refLines;
      QOpenGLBuffer vbo_refLines;
      vao_refLines.create();
      vbo_refLines.create();
      vao_refLines.bind();
      vbo_refLines.setUsagePattern(QOpenGLBuffer::DynamicDraw);
      vbo_refLines.bind();
      vbo_refLines.allocate(refLinePoints.constData(), refLinePoints.count() * 3 * sizeof(float));
      MyGL::DrawShape(m_program_camera, vao_refLines, vbo_refLines, refLinePoints.count(), GL_LINES, QColor(0.3f, 0.3f, 0.3f, 1.0f));
      vao_refLines.destroy();
      vbo_refLines.destroy();
    }

      //build a table of frequencies and amplitudes for faster drawing
    for (chunkOffset = 0; chunkOffset < visibleChunks; chunkOffset++) {
      data = active->dataAtChunk(startChunk + chunkOffset);
      if (data && data->harmonicFreq.size() > 0) {
        for (harmonic = 0; harmonic < numHarmonics; harmonic++) {
          pitches(harmonic, chunkOffset) = freq2pitch(data->harmonicFreq[harmonic]);
          amps(harmonic, chunkOffset) = data->harmonicAmp[harmonic];
        }
      } else {
        for (harmonic = 0; harmonic < numHarmonics; harmonic++) {
          pitches(harmonic, chunkOffset) = 0.0;
          amps(harmonic, chunkOffset) = 0.0;
        }
      }
    }

    bool insideLine;
    float curPitch = 0.0, curAmp = 0.0, prevPitch, diffNote;

    //draw the outlines
    {
#ifdef DWS
      setMaterialSpecular(0.0, 0.0, 0.0, 0.0);
      setMaterialColor(0.0f, 0.0f, 0.0f);
      glLineWidth(2.0);
#endif

      //printf("_peakThreshold = %f\n", _peakThreshold);
      for (harmonic = 0; harmonic < numHarmonics; harmonic++) {
        QVector<QVector3D> harmonicOutline;

        insideLine = false;
        pos = -double(visibleChunks - 1);
        for (chunkOffset = 0; chunkOffset < visibleChunks; chunkOffset++, pos++) {
          curAmp = amps(harmonic, chunkOffset) - _peakThreshold;
          //printf("curAmp = %f\n", curAmp);
          if (curAmp > 0.0) {
            prevPitch = curPitch;
            curPitch = pitches(harmonic, chunkOffset);
            diffNote = prevPitch - curPitch;
            if (fabs(diffNote) < 1.0) {
              if (!insideLine) {
                harmonicOutline.clear();
                harmonicOutline << QVector3D(curPitch, 0, pos);
#ifdef DWS

                glBegin(GL_LINE_STRIP);
                glVertex3f(curPitch, 0, pos);
#endif
                insideLine = true;
              }
              harmonicOutline << QVector3D(curPitch, curAmp, pos);
#ifdef DWS

              glVertex3f(curPitch, curAmp, pos);
#endif
            } else {
              QOpenGLVertexArrayObject vao_harmonicOutline;
              QOpenGLBuffer vbo_harmonicOutline;
              vao_harmonicOutline.create();
              vbo_harmonicOutline.create();
              vao_harmonicOutline.bind();
              vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
              vbo_harmonicOutline.bind();

              vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
              MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(0, 0, 0, 1));
              vao_harmonicOutline.destroy();
              vbo_harmonicOutline.destroy();
#ifdef DWS
              glEnd();
#endif
              insideLine = false;
            }
          } else {
            if (insideLine) {
              harmonicOutline << QVector3D(curPitch, 0, pos - 1);

              QOpenGLVertexArrayObject vao_harmonicOutline;
              QOpenGLBuffer vbo_harmonicOutline;
              vao_harmonicOutline.create();
              vbo_harmonicOutline.create();
              vao_harmonicOutline.bind();
              vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
              vbo_harmonicOutline.bind();

              vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
              MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(0, 0, 0, 1));
              vao_harmonicOutline.destroy();
              vbo_harmonicOutline.destroy();
#ifdef DWS
              glVertex3f(curPitch, 0, pos - 1);
              glEnd();
#endif
              insideLine = false;
            }
          }
        }
        if (insideLine) {
          harmonicOutline << QVector3D(curPitch, 0, pos - 1);

          QOpenGLVertexArrayObject vao_harmonicOutline;
          QOpenGLBuffer vbo_harmonicOutline;
          vao_harmonicOutline.create();
          vbo_harmonicOutline.create();
          vao_harmonicOutline.bind();
          vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          vbo_harmonicOutline.bind();

          vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
          MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(0, 0, 0, 1));
          vao_harmonicOutline.destroy();
          vbo_harmonicOutline.destroy();
#ifdef DWS
          glVertex3f(curPitch, 0, pos - 1);
          glEnd();
#endif
          insideLine = false;
        }
      }
    }

    bool isEven;
    //draw the faces
    glShadeModel(GL_FLAT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (harmonic = 0; harmonic < numHarmonics; harmonic++) {
      QColor harmonicColor;
      QVector<QVector3D> harmonicPolygon;

      if (harmonic % 2 == 0) {
        isEven = true;
        harmonicColor = QColor(128, 128, 230);
#ifdef DWS
        setMaterialColor(0.5f, 0.5f, 0.9f);
#endif
      } else {
        harmonicColor = QColor(128, 230, 128);

#ifdef DWS
        setMaterialColor(0.5f, 0.9f, 0.5f);
#endif
        isEven = false;
      }
      insideLine = false;
      pos = -double(visibleChunks - 1);
      // Create polygon of the harnmonics
      for (chunkOffset = 0; chunkOffset < visibleChunks; chunkOffset++, pos++) {

        if (amps(harmonic, chunkOffset) > _peakThreshold) {
          if (!insideLine) {
            // Polygon
#ifdef DWS
            glBegin(GL_QUAD_STRIP);
#endif
            insideLine = true;
            curPitch = pitches(harmonic, chunkOffset);
            harmonicPolygon.clear();
            harmonicPolygon << QVector3D(curPitch, 0, pos);
            harmonicPolygon << QVector3D(curPitch, (amps(harmonic, chunkOffset) - _peakThreshold), pos);

#ifdef DWS
            glVertex3f(curPitch, (amps(harmonic, chunkOffset) - _peakThreshold), pos);
            glVertex3f(curPitch, 0, pos);
#endif
          } else {
            prevPitch = curPitch;
            curPitch = pitches(harmonic, chunkOffset);
            diffNote = prevPitch - curPitch;
            if (fabs(diffNote) < 1.0) {
              harmonicPolygon << QVector3D(curPitch, 0, pos);
              harmonicPolygon << QVector3D(curPitch, (amps(harmonic, chunkOffset) - _peakThreshold), pos);
#ifdef DWS
              glNormal3f(diffNote, 0.0, 1.0);
              glVertex3f(curPitch, (amps(harmonic, chunkOffset) - _peakThreshold), pos);
              glVertex3f(curPitch, 0, pos);
#endif
            } else {
              QOpenGLVertexArrayObject vao_harmonicPolygon;
              QOpenGLBuffer vbo_harmonicPolygon;
              vao_harmonicPolygon.create();
              vbo_harmonicPolygon.create();
              vao_harmonicPolygon.bind();
              vbo_harmonicPolygon.setUsagePattern(QOpenGLBuffer::DynamicDraw);
              vbo_harmonicPolygon.bind();

              vbo_harmonicPolygon.allocate(harmonicPolygon.constData(), harmonicPolygon.count() * 3 * sizeof(float));
              MyGL::DrawShape(m_program_camera, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count(), GL_TRIANGLE_STRIP, harmonicColor);
              vao_harmonicPolygon.destroy();
              vbo_harmonicPolygon.destroy();
#ifdef DWS
              glEnd();
#endif
              insideLine = false;
            }
          }
        } else {
          if (insideLine) {
            QOpenGLVertexArrayObject vao_harmonicPolygon;
            QOpenGLBuffer vbo_harmonicPolygon;
            vao_harmonicPolygon.create();
            vbo_harmonicPolygon.create();
            vao_harmonicPolygon.bind();
            vbo_harmonicPolygon.setUsagePattern(QOpenGLBuffer::DynamicDraw);
            vbo_harmonicPolygon.bind();

            vbo_harmonicPolygon.allocate(harmonicPolygon.constData(), harmonicPolygon.count() * 3 * sizeof(float));
            MyGL::DrawShape(m_program_camera, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count(), GL_TRIANGLE_STRIP, harmonicColor);
            vao_harmonicPolygon.destroy();
            vbo_harmonicPolygon.destroy();
#ifdef DWS
            glEnd();
#endif
            insideLine = false;
          }
        }
      }
      if (insideLine) {
        QOpenGLVertexArrayObject vao_harmonicPolygon;
        QOpenGLBuffer vbo_harmonicPolygon;
        vao_harmonicPolygon.create();
        vbo_harmonicPolygon.create();
        vao_harmonicPolygon.bind();
        vbo_harmonicPolygon.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        vbo_harmonicPolygon.bind();

        vbo_harmonicPolygon.allocate(harmonicPolygon.constData(), harmonicPolygon.count() * 3 * sizeof(float));
        MyGL::DrawShape(m_program_camera, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count(), GL_TRIANGLE_STRIP, harmonicColor);
        vao_harmonicPolygon.destroy();
        vbo_harmonicPolygon.destroy();
#ifdef DWS
        glEnd();
#endif
        insideLine = false;
      }
    }
    active->unlock();
  }
}

void HTrackWidget::home()
{
  setPeakThreshold(0.05f);
  setDistanceAway(1500.0);
  setViewAngleVertical(-35.0);
  setViewAngleHorizontal(20.0);
  translateX = 0.0;
  translateY = -60.0;
}

void HTrackWidget::translate(float x, float y, float z)
{
  translateX += x;
  translateY += y;
}

void HTrackWidget::mousePressEvent(QMouseEvent* e)
{
  mouseDown = true;
  mouseX = e->x();
  mouseY = e->y();
}

void HTrackWidget::mouseMoveEvent(QMouseEvent* e)
{
  if (mouseDown) {
    translate(float(e->x() - mouseX), -float(e->y() - mouseY), 0.0);

    mouseX = e->x();
    mouseY = e->y();
    update();
  }
}

void HTrackWidget::mouseReleaseEvent(QMouseEvent*)
{
  mouseDown = false;
}

void HTrackWidget::wheelEvent(QWheelEvent* e)
{
  setDistanceAway(_distanceAway * pow(2.0, -(double(e->delta()) / double(WHEEL_DELTA)) / 20.0));
  update();
  e->accept();
}

