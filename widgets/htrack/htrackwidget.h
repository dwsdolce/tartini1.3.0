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

class Piano3d;

class HTrackWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT        // must include this if you use Qt signals/slots

public:
  HTrackWidget(QWidget* parent, const char* name = NULL);
  ~HTrackWidget();

  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

  // For non-camera use
  void translate(float x, float y);

  void mouseMoveEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void mouseDoubleClickEvent(QMouseEvent* e) override;
  void wheelEvent(QWheelEvent* e) override;

  float peakThreshold()
  {
    return m_peakThreshold;
  }
  double viewAngleHorizontal()
  {
    return m_viewAngleHorizontal;
  }
  double viewAngleVertical()
  {
    return m_viewAngleVertical;
  }

public slots:
  void setPeakThreshold(float peakThreshold)
  {
    m_peakThreshold = peakThreshold;
  }

  // This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
  // Mouse Motion.
  void setViewAngleHorizontalQWTWheel(double angle)
  {
    if (angle != m_viewAngleHorizontal) {
      m_viewAngleHorizontal = angle;
    }
  }

  void setViewAngleHorizontal(double angle)
  {
    if (angle != m_viewAngleHorizontal) {
      m_viewAngleHorizontal = angle; emit viewAngleHorizontalChanged(angle);
    }
  }

  // This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
  // Mouse Motion.
  void setViewAngleVerticalQWTWheel(double angle)
  {
    if (angle != m_viewAngleVertical) {
      m_viewAngleVertical = angle;
    }
  }

  void setViewAngleVertical(double angle)
  {
    if (angle != m_viewAngleVertical) {
      m_viewAngleVertical = angle; emit viewAngleVerticalChanged(angle);
    }
  }

  // This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
  // Mouse Motion.
  void setDistanceAwayQWTWheel(double distance)
  {
    if (distance != m_distanceAway) {
      m_distanceAway = distance;
    }
  }

  void setDistanceAway(double distance)
  {
    if (distance != m_distanceAway) {
      m_distanceAway = distance; emit distanceAwayChanged(distance);
    }
  }
  void home(void);

signals:
  void distanceAwayChanged(double);
  void viewAngleHorizontalChanged(double);
  void viewAngleVerticalChanged(double);

private:
  bool m_useLighting;
  
  float m_peakThreshold;

  double m_distanceAway;
  double m_viewAngleHorizontal;
  double m_viewAngleVertical;
  double translateX, translateY;

  Piano3d* piano3d;

  // State for the mouse handling
  bool mouseDown;
  int mouseX, mouseY;

  QMatrix4x4 m_model;

  QOpenGLShaderProgram m_program_camera;
  QOpenGLShaderProgram m_program_lighting;
};

#endif
