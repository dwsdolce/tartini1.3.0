/***************************************************************************
                          vibratospeedwidget.h  -  description
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
#ifndef VIBRATOSPEEDWIDGET_H
#define VIBRATOSPEEDWIDGET_H

#include "drawwidget.h"
#include <QPixmap>
#include <QPaintEvent>

class VibratoSpeedWidget : public DrawWidget {
  Q_OBJECT

  public:
    VibratoSpeedWidget(QWidget *parent);
    virtual ~VibratoSpeedWidget();

    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent*);

    QSize minimumSizeHint() const { return QSize(100, 75); }

  private:
    //data goes here
    double speedValueToDraw, widthValueToDraw;
    double prevVibratoSpeed, prevVibratoWidth;

    QFont speedWidthFont;

    int widthLimit;
    int currentNoteNumber;
    int prevNoteNumber;

    struct labelStruct {
      QString label;
      float x;
      float y;
    };

    int speedLabelCounter;
    labelStruct speedLabels[100];

    int widthLabelCounter;
    labelStruct widthLabels[100];

  public slots:
    void doUpdate();
};

#endif
