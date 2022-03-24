/***************************************************************************
                          vibratoperiodwidgetGL.h  -  description
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
#ifndef VIBRATOPERIODWIDGETGL_H
#define VIBRATOPERIODWIDGETGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class VibratoPeriodWidgetGL : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    VibratoPeriodWidgetGL(QWidget *parent);
    virtual ~VibratoPeriodWidgetGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QSize minimumSizeHint() const { return QSize(100, 75); }

  private:
    int prevLeftMinimumTime;
    bool smoothedPeriods;
    bool drawSineReference;
    bool sineStyle;
    bool drawPrevPeriods;
    bool periodScaling;
    bool drawComparison;

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_program_line;

    QOpenGLBuffer m_vbo_sineReference;
    int m_sineReferenceCount;
    QOpenGLVertexArrayObject m_vao_sineReference;

    QOpenGLBuffer m_vbo_previousPoly[5];
    int m_previousPolyCount[5];
    QOpenGLVertexArrayObject m_vao_previousPoly[5];
    int m_previousPolyAlpha[5];

    QOpenGLBuffer m_vbo_currentPeriod;
    int m_currentPeriodCount;
    QOpenGLVertexArrayObject m_vao_currentPeriod;

    QOpenGLBuffer m_vbo_comparisonPoly;
    int m_comparisonPolyCount;
    QOpenGLVertexArrayObject m_vao_comparisonPoly;

    QOpenGLBuffer m_vbo_comparisonReference;
    int m_comparisonReferenceCount;
    QOpenGLVertexArrayObject m_vao_comparisonReference;

    int lastPeriodToDraw;

  public slots:
    void doUpdate();
    void setSmoothedPeriods(bool value);
    void setDrawSineReference(bool value);
    void setSineStyle(bool value);
    void setDrawPrevPeriods(bool value);
    void setPeriodScaling(bool value);
    void setDrawComparison(bool value);
};

#endif
