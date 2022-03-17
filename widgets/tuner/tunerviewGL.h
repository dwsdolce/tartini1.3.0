/***************************************************************************
                          tunerviewGL.h  -  description
                             -------------------
    begin                : Mon Jan 10 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef TUNERVIEWGL_H
#define TUNERVIEWGL_H

#include <vector>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>

#include "viewwidget.h"
#include "tunerwidget.h"

class QPixmap;
class TunerWidgetGL;
class LEDIndicator;
class QwtSlider;
class Channel;

class TunerViewGL : public ViewWidget {
  Q_OBJECT

  public:
    TunerViewGL(int viewID_, QWidget *parent = 0);
    virtual ~TunerViewGL();

    QSize sizeHint() const { return QSize(200, 200); }

  public slots:
    void setLed(int index, bool value);
    void doUpdate();

  private:
    void resetLeds();
    
    TunerWidgetGL *tunerWidgetGL;
    std::vector<LEDIndicator*> leds;
    QwtSlider *slider;
};


#endif
