/***************************************************************************
                          htrackwidget.h  -  description
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

#ifndef HTRACK_H
#define HTRACK_H

#ifdef DWS
#include <QGLWidget>
#endif

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QTime>
#include <QDebug>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix4x4>
#include <vector>

#include "camera.h"

class Piano3d;
#ifdef DWS
class HTrackWidget : public QGLWidget
#endif
  class HTrackWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT        // must include this if you use Qt signals/slots

public:
  HTrackWidget(QWidget* parent, const char* name = NULL);
  ~HTrackWidget();

  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

  void translate(float x, float y, float z);
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void wheelEvent(QWheelEvent* e);

  float peakThreshold()
  {
    return _peakThreshold;
  }
  double viewAngleHorizontal()
  {
    return _viewAngleHorizontal;
  }
  double viewAngleVertical()
  {
    return _viewAngleVertical;
  }

public slots:
  void setPeakThreshold(float peakThreshold)
  {
    _peakThreshold = peakThreshold;
  }
// This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
// Mouse Motion.
  void setViewAngleHorizontalQWTWheel(double angle)
  {
    if (angle != _viewAngleHorizontal) {
      _viewAngleHorizontal = angle;
    }
  }
  void setViewAngleHorizontal(double angle)
  {
    if (angle != _viewAngleHorizontal) {
      _viewAngleHorizontal = angle; emit viewAngleHorizontalChanged(angle);
    }
  }
  // This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
  // Mouse Motion.
  void setViewAngleVerticalQWTWheel(double angle)
  {
    if (angle != _viewAngleVertical) {
      _viewAngleVertical = angle;
    }
  }
  void setViewAngleVertical(double angle)
  {
    if (angle != _viewAngleVertical) {
      _viewAngleVertical = angle; emit viewAngleVerticalChanged(angle);
    }
  }
  // This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
  // Mouse Motion.
  void setDistanceAwayQWTWheel(double distance)
  {
    if (distance != _distanceAway) {
      _distanceAway = distance;
    }
  }
  void setDistanceAway(double distance)
  {
    if (distance != _distanceAway) {
      _distanceAway = distance; emit distanceAwayChanged(distance);
    }
  }
  void home();

signals:
  void distanceAwayChanged(double);
  void viewAngleHorizontalChanged(double);
  void viewAngleVerticalChanged(double);

private:
  float _peakThreshold;

  double _distanceAway;
  double _viewAngleHorizontal;
  double _viewAngleVertical;
  Piano3d* piano3d;
  double translateX, translateY;

  // Define the camera to use
  Camera m_camera;

  // State for the mouse handling
  bool mouseDown;
  int mouseX, mouseY;
  bool firstMouse = true;
  bool trackMouse = false;
  float lastX;
  float lastY;

  // timing for use in mouse handling
  float getTime()
  {
    return QTime::currentTime().second() + QTime::currentTime().msec() / 1000.0f;
  }

  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame

  QMatrix4x4 m_model;
  QMatrix4x4 m_view;
  QMatrix4x4 m_projection;

  QOpenGLShaderProgram m_program_camera;
  QOpenGLShaderProgram m_program_lighting;
};

#endif
