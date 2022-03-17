/***************************************************************************
                          freqviewGL.h  -  description
                             -------------------
    begin                : Fri Dec 10 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#ifndef FREQVIEWGL_H
#define FREQVIEWGL_H

#include "viewwidget.h"

class FreqWidgetGL;
class AmplitudeWidgetGL;
class MyScrollBar;
class TimeAxis;
class QwtWheel;

class FreqViewGL: public ViewWidget {
  Q_OBJECT

  public:
    FreqViewGL(int viewID_, QWidget *parent = 0);
    virtual ~FreqViewGL();

    QSize sizeHint() const { return QSize(600, 560); }

  public slots:
    void zoomIn();
    void zoomOut();
    void setAmplitudeZoom(double);
  
  private:
    FreqWidgetGL *freqWidgetGL;
    AmplitudeWidgetGL *amplitudeWidgetGL;
    MyScrollBar *freqScrollBar;
    MyScrollBar *amplitudeScrollBar;
    QwtWheel *freqWheelY;
    QwtWheel *amplitudeWheelY;
    QwtWheel *freqWheelX;
    
    TimeAxis *timeAxis;

};

#endif
