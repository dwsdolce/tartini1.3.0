/***************************************************************************
                          ledindicator.cpp  -  description
                             -------------------
    begin                : Tue Jan 11 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#include "ledindicator.h"

#include <qpixmap.h>
#include <QPaintEvent>

LEDIndicator::LEDIndicator(QWidget *parent, const char *name, const QColor &on, const QColor &off)
: QWidget(parent)
{
  setObjectName(name);
  setMinimumSize(sizeHint());
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->on = on;
  this->off = off;
  QSize s = size();

  active = false;

  // Stop QT from erasing the background all the time
  setAttribute(Qt::WA_OpaquePaintEvent);
}

LEDIndicator::~LEDIndicator()
{
}

void LEDIndicator::setOn(bool on) {
	if (active != on) {
		active = on;
		update();
	}
}

void LEDIndicator::toggle() {
  active = !active;
}

bool LEDIndicator::lit() {
  return active;
}

void LEDIndicator::paintEvent(QPaintEvent *)
{

  QPainter p(this);
  p.setPen(palette().color(QPalette::Foreground));
  p.setBackground(palette().color(QPalette::Window));

  p.fillRect(0, 0, QWidget::width(), QWidget::height(), (active) ? on : off);

  p.setPen(palette().color(QPalette::BrightText));

  QFontMetrics fm = p.fontMetrics();
  int fontHeight = fm.height() / 4;
  int fontWidth = fm.width(objectName()) / 2;
  
  p.drawText(QWidget::width()/2 - fontWidth, QWidget::height()/2 + fontHeight, objectName());
}
