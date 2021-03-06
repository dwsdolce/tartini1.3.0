/***************************************************************************
                          freqdrawwidget.h  -  description
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

#ifndef FREQDRAWWIDGET_H
#define FREQDRAWWIDGET_H

#include "drawwidget.h"
#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QPaintDevice>

class FreqDrawWidget : public DrawWidget {

Q_OBJECT

public:
  enum DragModes {
    DragNone = 0,
    DragChannel = 1,
    DragBackground = 2,
    DragTimeBar = 3
  };


  FreqDrawWidget(QWidget *parent, const char* name = 0);
  virtual ~FreqDrawWidget();

  static void drawReferenceLines(QPaintDevice &pd, QPainter &p, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType);
  void paintEvent( QPaintEvent * );
  QSize sizeHint() const { return QSize(400, 350); }

private:
  int dragMode;
  int mouseX, mouseY;
  double downTime, downNote;
  
  void keyPressEvent( QKeyEvent *k );
  void keyReleaseEvent( QKeyEvent *k);
  void leaveEvent ( QEvent * e);
  
  void mousePressEvent( QMouseEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void wheelEvent(QWheelEvent * e);
  void resizeEvent (QResizeEvent *q);
  
  double mouseTime(int x);
  double mousePitch(int y);
  Channel *channelAtPixel(int x, int y);
  
  QPixmap *buffer;
  
};

#endif
