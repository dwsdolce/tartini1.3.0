/***************************************************************************
                          vibratowidgetGL.h  -  description
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
#ifndef VIBRATOWIDGETGL_H
#define VIBRATOWIDGETGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class VibratoWidgetGL : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    VibratoWidgetGL(QWidget *parent, int nls);
    virtual ~VibratoWidgetGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QSize sizeHint() const { return QSize(300, 100); }

  private:
    int noteLabelOffset; // The horizontal space in pixels a note label requires
    double zoomFactorX;
    double zoomFactorY;
    int offsetY;

    QFont vibratoFont;

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_program_line;

    QOpenGLBuffer m_vbo_verticalPeriodBarsShading1;
    int m_verticalPeriodBarsShading1Count;
    QOpenGLVertexArrayObject m_vao_verticalPeriodBarsShading1;
    QColor m_verticalPeriodBarsShading1Color;

    QOpenGLBuffer m_vbo_verticalPeriodBarsShading2;
    int m_verticalPeriodBarsShading2Count;
    QOpenGLVertexArrayObject m_vao_verticalPeriodBarsShading2;
    QColor m_verticalPeriodBarsShading2Color;

    QOpenGLBuffer m_vbo_verticalSeparatorLines;
    int m_verticalSeparatorLinesCount;
    QOpenGLVertexArrayObject m_vao_verticalSeparatorLines;
    QColor m_verticalSeparatorLinesColor;

    QVector<QLineF>m_referenceLines;
    QColor m_referenceLinesColor;

    QOpenGLBuffer m_vbo_pronyWidthBand;
    int m_pronyWidthBandCount;
    QOpenGLVertexArrayObject m_vao_pronyWidthBand;
    QColor m_pronyWidthBandColor;

    QOpenGLBuffer m_vbo_pronyAveragePitch;
    int m_pronyAveragePitchCount;
    QOpenGLVertexArrayObject m_vao_pronyAveragePitch;
    QColor m_pronyAveragePitchColor;

    QOpenGLBuffer m_vbo_vibratoPolyline;
    int m_vibratoPolylineCount;
    QOpenGLVertexArrayObject m_vao_vibratoPolyline;
    QColor m_vibratoPolylineColor;

    QOpenGLBuffer m_vbo_currentWindowBand;
    int m_currentWindowBandCount;
    QOpenGLVertexArrayObject m_vao_currentWindowBand;
    QColor m_currentWindowBandColor;

    QOpenGLBuffer m_vbo_currentTimeLine;
    int m_currentTimeLineCount;
    QOpenGLVertexArrayObject m_vao_currentTimeLine;
    QColor m_currentTimeLineColor;

    QOpenGLBuffer m_vbo_maximaPoints;
    int m_maximaPointsCount;
    QOpenGLVertexArrayObject m_vao_maximaPoints;
    QColor m_maximaPointsColor;

    QOpenGLBuffer m_vbo_minimaPoints;
    int m_minimaPointsCount;
    QOpenGLVertexArrayObject m_vao_minimaPoints;
    QColor m_minimaPointsColor;

    int noteLabelCounter;
    struct noteLabelStruct {
      QString label;
      float y;
    };
    noteLabelStruct noteLabels[100];

  public slots:
    void doUpdate();
    void setZoomFactorX(double x);
    void setZoomFactorY(double y);
    void setOffsetY(int value);
};

#endif
