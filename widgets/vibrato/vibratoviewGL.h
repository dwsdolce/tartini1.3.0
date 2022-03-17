/***************************************************************************
                          vibratoviewGL.h  -  description
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
#ifndef VIBRATOVIEWGL_H
#define VIBRATOVIEWGL_H

#include "viewwidget.h"

class VibratoSpeedWidgetGL;
class VibratoCircleWidgetGL;
class VibratoPeriodWidgetGL;
class VibratoTimeAxis;
class VibratoWidgetGL;
class LEDIndicator;

class VibratoViewGL : public ViewWidget {
  Q_OBJECT

  public:
    VibratoViewGL(int viewID_, QWidget *parent = 0);
    virtual ~VibratoViewGL();

    QSize sizeHint() const { return QSize(500, 300); }

    QVector<LEDIndicator*> leds;

  private:
    VibratoSpeedWidgetGL *vibratoSpeedWidgetGL;
    VibratoCircleWidgetGL* vibratoCircleWidgetGL;
    VibratoPeriodWidgetGL* vibratoPeriodWidgetGL;
    VibratoTimeAxis* vibratoTimeAxis;
    VibratoWidgetGL *vibratoWidgetGL;
};


#endif
