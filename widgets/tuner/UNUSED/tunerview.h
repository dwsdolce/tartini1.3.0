/***************************************************************************
                          tunerview.h  -  description
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
#ifndef TUNERVIEW_H
#define TUNERVIEW_H

#include <vector>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>

#include "viewwidget.h"
#include "tunerwidget.h"

class QPixmap;
class TunerWidget;
class LEDIndicator;
class QwtSlider;
class Channel;

class TunerView : public ViewWidget {
  Q_OBJECT

  public:
    TunerView(int viewID_, QWidget *parent = 0);
    virtual ~TunerView();

    QSize sizeHint() const { return QSize(200, 200); }

  public slots:
    void setLed(int index, bool value);
    void doUpdate();

  private:
    void resetLeds();
    
    TunerWidget *tunerWidget;
    std::vector<LEDIndicator*> leds;
    QwtSlider *slider;
};


#endif
