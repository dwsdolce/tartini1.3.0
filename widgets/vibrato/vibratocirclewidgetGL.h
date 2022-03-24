/***************************************************************************
                          vibratocirclewidgetGL.h  -  description
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
#ifndef VIBRATOCIRCLEWIDGETGL_H
#define VIBRATOCIRCLEWIDGETGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class VibratoCircleWidgetGL : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    VibratoCircleWidgetGL(QWidget *parent);
    virtual ~VibratoCircleWidgetGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QSize minimumSizeHint() const { return QSize(100, 75); }
    int getType() { return type; }

  private:
    float accuracy;
    int type;
    int lastPeriodToDraw;

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_program_line;

    // Different parts of the referenceCircle.
    const int VBO_CIRCLE = 0;
    const int VBO_LINES = 1;
    QOpenGLBuffer m_vbo_referenceCircle[2];
    int m_referenceCircleCount[2];
    QOpenGLVertexArrayObject m_vao_referenceCircle[2];

    QOpenGLBuffer m_vbo_currentPeriod;
    int m_currentPeriodCount;
    QOpenGLVertexArrayObject m_vao_currentPeriod;
    int m_CurrentPeriodAlpha;

    QOpenGLBuffer m_vbo_prevPeriods[6];
    int m_prevPeriodsCount[6];
    QOpenGLVertexArrayObject m_vao_prevPeriods[6];
    int m_prevPeriodsAlpha[6];

  public slots:
    void doUpdate();
    void setAccuracy(int value);
    void setType(int value);

};

#endif
