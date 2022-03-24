/***************************************************************************
                          vibratospeedwidgetGL.h  -  description
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
#ifndef VIBRATOSPEEDWIDGETGL_H
#define VIBRATOSPEEDWIDGETGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class VibratoSpeedWidgetGL : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    VibratoSpeedWidgetGL(QWidget *parent);
    virtual ~VibratoSpeedWidgetGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QSize minimumSizeHint() const { return QSize(100, 75); }

  private:
    //data goes here
    float speedValueToDraw, widthValueToDraw;
    float prevVibratoSpeed, prevVibratoWidth;

    QFont speedWidthFont;

    int widthLimit;
    int currentNoteNumber;
    int prevNoteNumber;

    float hzLabelX, hzLabelY;
    float centsLabelX, centsLabelY;

    struct labelStruct {
      QString label;
      float x;
      float y;
    };

    int speedLabelCounter;
    labelStruct speedLabels[100];

    int widthLabelCounter;
    labelStruct widthLabels[100];

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_program_line;

    // Different parts of the dial.
    const int VBO_DIAL = 0;
    const int VBO_ARC = 1;
    const int VBO_MARKERS = 2;

    QOpenGLBuffer m_vbo_dial_speed[3];
    int m_dial_count_speed[3];
    QOpenGLBuffer m_vbo_needle_speed;
    int m_needle_count_speed;
    bool m_draw_needle_speed = false;

    QOpenGLVertexArrayObject m_vao_dial_speed[3];
    QOpenGLVertexArrayObject m_vao_needle_speed;

    QOpenGLBuffer m_vbo_dial_width[3];
    int m_dial_count_width[3];
    QOpenGLBuffer m_vbo_needle_width;
    int m_needle_count_width;
    bool m_draw_needle_width = false;

    QOpenGLVertexArrayObject m_vao_dial_width[3];
    QOpenGLVertexArrayObject m_vao_needle_width;

    int msgCount;

  public slots:
    void doUpdate();
};

#endif
