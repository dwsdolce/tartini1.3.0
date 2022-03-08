/***************************************************************************
                          tunerwidget.h  -  description
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
#ifndef TUNERWIDGET_H
#define TUNERWIDGET_H

#include "drawwidget.h"
#include <QPixmap>
#include <QPaintEvent>

// Forward declarations of classes the h file doesn't need to know specifics about
class QPixmap;

class TunerWidget : public DrawWidget {
  Q_OBJECT

  public:
    TunerWidget(QWidget* parent);
    virtual ~TunerWidget();

    void paintEvent( QPaintEvent * );

    QSize minimumSizeHint() const { return QSize(100, 75); }

  private:
    float value_;

    void resetLeds();

  signals:
    void ledSet(int index, bool value);

  public slots:
    void doUpdate(double thePitch);
};


#endif