/***************************************************************************
                          tunerwidget.h  -  description
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
#ifndef TUNERWIDGET_H
#define TUNERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class TunerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

  public:
    using QOpenGLWidget::QOpenGLWidget;
    TunerWidget(QWidget *parent);
    virtual ~TunerWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    QSize minimumSizeHint() const { return QSize(100, 75); }

  private:
    //data goes here
    float needleValueToDraw;
    float prevNeedleValue;
    int prevClosePitch;
    double curPitch;

    QFont tunerFont;

    float centsLabelX, centsLabelY;

    int tunerLabelCounter;
    struct tunerLabelStruct {
      QString label;
      float x;
      float y;
    };
    tunerLabelStruct tunerLabels[100];

    void resetLeds();

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_program_line;

    // Different parts of the dial.
    const int VBO_DIAL = 0;
    const int VBO_ARC = 1;
    const int VBO_MARKERS = 2;
    QOpenGLBuffer m_vbo_dial[3];
    int m_dial_count[3];
    QOpenGLBuffer m_vbo_needle;
    int m_needle_count;
    bool m_draw_needle = false;

    QOpenGLVertexArrayObject m_vao_dial[3];
    QOpenGLVertexArrayObject m_vao_needle;

  signals:
    void ledSet(int index, bool value);

  public slots:
    void doUpdate(double thePitch);

};


#endif
