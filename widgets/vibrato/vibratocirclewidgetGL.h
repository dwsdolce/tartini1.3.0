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

class VibratoCircleWidgetGL : public QOpenGLWidget {
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

    GLuint referenceCircle;
    GLuint currentPeriod;
    GLuint prevPeriods[6];

  public slots:
    void doUpdate();
    void setAccuracy(int value);
    void setType(int value);

};

#endif
