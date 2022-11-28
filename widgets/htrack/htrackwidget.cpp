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
#include "htrackwidget.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QPainter>
#include <QVector4D>
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
  m_pianoLightPosition = QVector3D(0.0f, 2000.0f, -1600.0f);
  m_harmonicLightPosition = QVector3D(-1000.0, 0.0, 0.0);

  setFocusPolicy(Qt::StrongFocus);
}

HTrackWidget::~HTrackWidget()
{
  makeCurrent();
  delete piano3d;
  m_vao_light.destroy();
  m_vbo_light.destroy();
  m_program_camera.deleteLater();
  m_program_lighting.deleteLater();
  doneCurrent();
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

  m_program_lighting.bind();
  m_program_lighting.setUniformValue("light.specular", QVector3D(0.5, 0.5, 0.5));
  m_program_lighting.release();

  // Define the light cube that uses the light.position and the specified color
  QVector<QVector3D> lightVector;
  float lightOffset = 20.0f;
  // Bottom
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);

  // Top
  lightVector << QVector3D(-lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset,  lightOffset);

  // Left
  lightVector << QVector3D(-lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset,  lightOffset);

  // Right
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);

  // Front
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset, -lightOffset, -lightOffset);

  // Back
  lightVector << QVector3D(-lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset, -lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D( lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset,  lightOffset);
  lightVector << QVector3D(-lightOffset,  lightOffset, -lightOffset);

  m_vao_light.create();
  m_vbo_light.create();

  m_program_camera.bind();
  m_vao_light.bind();

  m_vbo_light.setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_vbo_light.bind();
  m_vbo_light.allocate(lightVector.constData(), lightVector.count() * 3 * sizeof(float));
  m_count_light = lightVector.count();
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
  QMatrix4x4 projection;
  projection.setToIdentity();

  double nearPlane = 100.0;

  double w2 = double(width()) / 2.0;
  double h2 = double(height()) / 2.0;
  double ratio = nearPlane / double(width());
  projection.frustum((-w2 - translateX) * ratio,
                     ( w2 - translateX) * ratio,
                     (-h2 - translateY) * ratio,
                     ( h2 - translateY) * ratio,
                     nearPlane, 10000.0);

  m_model.setToIdentity();

  QMatrix4x4 view;
  view.setToIdentity();
  view.lookAt(QVector3D(0.0, 0.0, m_distanceAway), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));

  view.rotate(-viewAngleVertical(), 1.0, 0.0, 0.0);
  view.rotate(viewAngleHorizontal(), 0.0, 1.0, 0.0);

  // Use m_program_camera for the ref lines and the outlines of the harmonics
  m_program_camera.bind();
  m_program_camera.setUniformValue("projection", projection);
  m_program_camera.setUniformValue("view", view);
  m_program_camera.release();

  // use m_program_lighting for the piano and the faces of the harmonics
  m_program_lighting.bind();
  m_program_lighting.setUniformValue("projection", projection);
  m_program_lighting.setUniformValue("view", view);

  // Setup lighting
  // I think this is the same as used in the lookAt function.
  m_program_lighting.setUniformValue("viewPos", QVector3D(0.0, 0.0, m_distanceAway));

  // Light is positioned behind the keyboard and above it.
  m_program_lighting.setUniformValue("light.position", QVector4D(m_pianoLightPosition, 1.0f));
  m_program_lighting.setUniformValue("light.ambient", QVector3D(0.4f, 0.4f, 0.4f));
  m_program_lighting.setUniformValue("light.diffuse", QVector3D(0.9f, 0.9f, 0.9f));

  m_program_lighting.release();

  // Draw the light for the piano
  QMatrix4x4 lightModel = m_model;
  lightModel.translate(m_pianoLightPosition);
  m_program_camera.bind();
  m_program_camera.setUniformValue("model", lightModel);
  m_program_camera.release();
  MyGL::DrawShape(m_program_camera, m_vao_light, m_vbo_light, m_count_light, GL_TRIANGLES, QColor(255.0f, 255.0f, 255.0f, 1.0f));

  // Draw the piano keyboard.
  double pianoWidth = piano3d->pianoWidth();
  QMatrix4x4 pianoModel = m_model;
  pianoModel.translate(-pianoWidth / 2.0, 0.0, 0.0); //set center of keyboard at origin

  piano3d->setAllKeyStatesOff();
  if (active) {
    AnalysisData* data = active->dataAtCurrentChunk();
    if (data && active->isVisibleChunk(data)) {
      piano3d->setMidiKeyState(toInt(data->pitch), true);
    }
  }

  piano3d->draw(m_program_lighting, pianoModel);


  // Draw the harmonic lightlight
  lightModel = m_model;
  lightModel.translate(m_harmonicLightPosition);
  m_program_camera.bind();
  m_program_camera.setUniformValue("model", lightModel);
  m_program_camera.release();
  MyGL::DrawShape(m_program_camera, m_vao_light, m_vbo_light, m_count_light, GL_TRIANGLES, QColor(255.0f, 255.0f, 255.0f, 1.0f));

  // Change the scaling for the harmonic display
  QMatrix4x4 harmonicModel = pianoModel;
  harmonicModel.translate(-piano3d->firstKeyOffset, 0.0f, 0.0f);
  harmonicModel.scale(OCTAVE_WIDTH / 12.0f, 200.0f, 5.0f); //set a scale of 1 semitime = 1 unit

  m_program_camera.bind();
  m_program_camera.setUniformValue("model", harmonicModel);
  m_program_camera.release();

  m_program_lighting.bind();
 // m_program_lighting.setUniformValue("light.position", QVector4D(-1.0, 0.0, 0.0, 0.0));
  m_program_lighting.setUniformValue("light.position", QVector4D(m_harmonicLightPosition, 1.0));
  m_program_lighting.setUniformValue("light.ambient", QVector3D(0.2f, 0.2f, 0.2f));
  m_program_lighting.setUniformValue("light.diffuse", QVector3D(0.9f, 0.9f, 0.9f));

  m_program_lighting.setUniformValue("model", harmonicModel);
  m_program_lighting.setUniformValue("normalMatrix", harmonicModel.normalMatrix());
  m_program_lighting.release();

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
      for (j = roundUp(startChunk, 16); j < finishChunk; j += 16) {
        if (active->isValidChunk(j)) {
          refLinePoints << QVector3D(piano3d->firstKey(), 0.0, double(j - finishChunk));
          refLinePoints << QVector3D(piano3d->firstKey() + piano3d->numKeys(), 0.0, double(j - finishChunk));
        }
      }
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
    bool isEven;
    //draw the faces and outlines
    glShadeModel(GL_FLAT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (harmonic = 0; harmonic < numHarmonics; harmonic++) {
      QVector<QVector3D> harmonicOutline;
      QVector<QVector3D> harmonicPolygon;
      // Change the face color on alternatic harmonics
      if (harmonic % 2 == 0) {
        isEven = true;
        m_program_lighting.bind();
        m_program_lighting.setUniformValue("material.ambient", QVector3D(0.5f, 0.5f, 0.9f));
        m_program_lighting.setUniformValue("material.diffuse", QVector3D(0.5f, 0.5f, 0.9f));
        m_program_lighting.release();
      } else {
        m_program_lighting.bind();
        m_program_lighting.setUniformValue("material.ambient", QVector3D(0.5f, 0.9f, 0.5f));
        m_program_lighting.setUniformValue("material.diffuse", QVector3D(0.5f, 0.9f, 0.5f));
        m_program_lighting.release();
        isEven = false;
      }
      insideLine = false;
      pos = -double(visibleChunks - 1);
      // Create polygon of the harnmonics
      QVector3D pt1;
      QVector3D pt2;
      QVector3D pt3;
      QVector3D pt4;
      bool firstPt = true;
      for (chunkOffset = 0; chunkOffset < visibleChunks; chunkOffset++, pos++) {
        curAmp = amps(harmonic, chunkOffset) - m_peakThreshold;
        if (curAmp > 0.0) { 
          if (!insideLine) {
            // Polygon
            insideLine = true;
            curPitch = pitches(harmonic, chunkOffset);
            harmonicPolygon.clear();
            pt1 = QVector3D(curPitch, curAmp, pos);
            pt2 = QVector3D(curPitch, 0, pos);
            firstPt = true;
          } else {
            prevPitch = curPitch;
            curPitch = pitches(harmonic, chunkOffset);
            diffNote = prevPitch - curPitch;
            if (fabs(diffNote) < 1.0) {
              pt3 = QVector3D(curPitch, (amps(harmonic, chunkOffset) - m_peakThreshold), pos);
              pt4 = QVector3D(curPitch, 0, pos);

              if (firstPt) {
                harmonicOutline << pt2 << pt1;
                firstPt = false;
              }
              harmonicOutline << pt3;

              // Calculate normal to this pair of triangles
              // Normal points to negative x
              // Positive x is the pitch
              // Positive y is the amplitude aboce the threashold
              // Negative z is the chunk number (corresponding to time)
              QVector3D side1 = pt3 - pt2;
              QVector3D side2 = pt1 - pt2;
              QVector3D normal = QVector3D::crossProduct(side1, side2);
              normal.normalize();
              harmonicPolygon << pt1;
              harmonicPolygon << normal;
              harmonicPolygon << pt2;
              harmonicPolygon << normal;
              harmonicPolygon << pt3;
              harmonicPolygon << normal;

              harmonicPolygon << pt3;
              harmonicPolygon << normal;
              harmonicPolygon << pt2;
              harmonicPolygon << normal;
              harmonicPolygon << pt4;
              harmonicPolygon << normal;
              // Save for the next normal
              pt1 = pt3;
              pt2 = pt4;
            } else {
              QOpenGLVertexArrayObject vao_harmonicPolygon;
              QOpenGLBuffer vbo_harmonicPolygon;
              vao_harmonicPolygon.create();
              vbo_harmonicPolygon.create();
              vao_harmonicPolygon.bind();
              vbo_harmonicPolygon.setUsagePattern(QOpenGLBuffer::DynamicDraw);
              vbo_harmonicPolygon.bind();

              vbo_harmonicPolygon.allocate(harmonicPolygon.constData(), harmonicPolygon.count() * 3 * sizeof(float));
              MyGL::DrawShape(m_program_lighting, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count() / 2, GL_TRIANGLES);
              vao_harmonicPolygon.destroy();
              vbo_harmonicPolygon.destroy();

              if (!firstPt) {
                harmonicOutline << pt2; // this was pt4 while creating polygons
                QOpenGLVertexArrayObject vao_harmonicOutline;
                QOpenGLBuffer vbo_harmonicOutline;
                vao_harmonicOutline.create();
                vbo_harmonicOutline.create();
                vao_harmonicOutline.bind();
                vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
                vbo_harmonicOutline.bind();

                vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
                MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(Qt::black));
                vao_harmonicOutline.destroy();
                vbo_harmonicOutline.destroy();
              }

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
            MyGL::DrawShape(m_program_lighting, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count() / 2, GL_TRIANGLES);
            vao_harmonicPolygon.destroy();
            vbo_harmonicPolygon.destroy();
            if (!firstPt) {
              harmonicOutline << pt2; // this was pt4 while creating polygons
              QOpenGLVertexArrayObject vao_harmonicOutline;
              QOpenGLBuffer vbo_harmonicOutline;
              vao_harmonicOutline.create();
              vbo_harmonicOutline.create();
              vao_harmonicOutline.bind();
              vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
              vbo_harmonicOutline.bind();

              vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
              MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(Qt::black));
              vao_harmonicOutline.destroy();
              vbo_harmonicOutline.destroy();
            }
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
        MyGL::DrawShape(m_program_lighting, vao_harmonicPolygon, vbo_harmonicPolygon, harmonicPolygon.count() / 2, GL_TRIANGLES);
        vao_harmonicPolygon.destroy();
        vbo_harmonicPolygon.destroy();
        if (!firstPt) {
          harmonicOutline << pt2; // this was pt4 while creating polygons
          QOpenGLVertexArrayObject vao_harmonicOutline;
          QOpenGLBuffer vbo_harmonicOutline;
          vao_harmonicOutline.create();
          vbo_harmonicOutline.create();
          vao_harmonicOutline.bind();
          vbo_harmonicOutline.setUsagePattern(QOpenGLBuffer::DynamicDraw);
          vbo_harmonicOutline.bind();

          vbo_harmonicOutline.allocate(harmonicOutline.constData(), harmonicOutline.count() * 3 * sizeof(float));
          MyGL::DrawShape(m_program_camera, vao_harmonicOutline, vbo_harmonicOutline, harmonicOutline.count(), GL_LINE_STRIP, QColor(Qt::black));
          vao_harmonicOutline.destroy();
          vbo_harmonicOutline.destroy();
        }
        insideLine = false;
      }
    }

    active->unlock();
  }
}

void HTrackWidget::home(void)
{
  setPeakThreshold(0.05f);

  // Reset the camera view
  setDistanceAway(1500.0);
  setViewAngleVertical(-35.0);
  setViewAngleHorizontal(20.0);
  translateX = 0.0;
  translateY = -60.0;

  // Reset the light position
  m_pianoLightPosition = QVector3D(0.0f, 2000.0f, -1600.0f);
  m_harmonicLightPosition = QVector3D(-2000.0, 2000.0, 2000.0);

  update();
}

void HTrackWidget::translate(float x, float y)
{
  translateX += x;
  translateY += y;
  update();
}

void HTrackWidget::mouseDoubleClickEvent(QMouseEvent*)
{
  home();
  update();
}

void HTrackWidget::mousePressEvent(QMouseEvent*e)
{
  // Without camera use
  mouseDown = true;
  mouseX = e->x();
  mouseY = e->y();
}

void HTrackWidget::mouseMoveEvent(QMouseEvent* e)
{
  if (mouseDown) {
    translate(float(e->x() - mouseX), -float(e->y() - mouseY));

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
  setDistanceAway(m_distanceAway * pow(2.0, -(double(e->angleDelta().y()) / double(WHEEL_DELTA)) / 20.0));
  update();
  e->accept();
}

void HTrackWidget::keyPressEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_W) {
    m_pianoLightPosition.setY(m_pianoLightPosition.y() + 100);
  }
  if (e->key() == Qt::Key_S) {
    m_pianoLightPosition.setY(m_pianoLightPosition.y() - 100);
  }
  if (e->key() == Qt::Key_A) {
    m_pianoLightPosition.setX(m_pianoLightPosition.x() - 100);
  }
  if (e->key() == Qt::Key_D) {
    m_pianoLightPosition.setX(m_pianoLightPosition.x() + 100);
  }
  if (e->key() == Qt::Key_Q) {
    m_pianoLightPosition.setZ(m_pianoLightPosition.z() - 100);
  }
  if (e->key() == Qt::Key_E) {
    m_pianoLightPosition.setZ(m_pianoLightPosition.z() + 100);
  }

  if (e->key() == Qt::Key_I) {
    m_harmonicLightPosition.setY(m_harmonicLightPosition.y() + 100);
  }
  if (e->key() == Qt::Key_K) {
    m_harmonicLightPosition.setY(m_harmonicLightPosition.y() - 100);
  }
  if (e->key() == Qt::Key_J) {
    m_harmonicLightPosition.setX(m_harmonicLightPosition.x() - 100);
  }
  if (e->key() == Qt::Key_L) {
    m_harmonicLightPosition.setX(m_harmonicLightPosition.x() + 100);
  }
  if (e->key() == Qt::Key_U) {
    m_harmonicLightPosition.setZ(m_harmonicLightPosition.z() - 100);
  }
  if (e->key() == Qt::Key_O) {
    m_harmonicLightPosition.setZ(m_harmonicLightPosition.z() + 100);
  }
  update();
}