/***************************************************************************
                          tunerwidget.cpp  -  description
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
#include <QPixmap>
#include <QPainter>
#include <QPolygon>
#include <QPaintEvent>
#include <QDebug>

#include "tunerwidget.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "musicnotes.h"
#include "useful.h"

TunerWidget::TunerWidget(QWidget* parent)
  : DrawWidget(parent)
{
  curPitch = 0;
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);
}

TunerWidget::~TunerWidget()
{}

void TunerWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();

  /*
    The tuner is basically just a sector. It has a radius of 1.2 * height() or
  1.2 * width() when appropriate, and we draw the largest sector we can fit on screen.

    To calculate this sector, we have to work out the starting angle, and work out how
    much we should draw. From basic trig, we can calculate the starting angle:

    <--- w ---->
    ___________
    \    |    /             sin t = Opposite / Hypotenuse
     \   |   /                    = 0.5 width() / radius
      \  |  / r = 1.2h (or 1.2w)  = width() / 2 * radius
       \ |t/
        \|/

    So the starting angle is 90 - t. The finishing angle is just the starting angle + 2t.

    To draw the markers, we can just use the equation of a circle to get x and y points
    along it.

    cos(i) = x / r   ->  x = r * cos(i)
    sin(i) = y / r   ->  y = r * sin(i)

    These points have to be adjusted to fit in the widget, so the x co-ordinates get
    radius added to them, and the y co-ordinates are taken away from radius.

    From there, we just step along the edge, drawing a line at each point we want. The
    lines would eventually meet up with the radius of the circle (width()/2, radius).

  */
  QPen pen(palette().color(QPalette::WindowText), 2);
  p.setPen(pen);  // Border

  double halfWidth = double(width()) / 2.0;
  double radius = 1.8 * MAX(height() / 2, halfWidth);
  QPointF center(halfWidth, radius);
  double theta = asin(double(width()) / (2 * radius));
  double thetaDeg = theta * 180.0 / PI;
  double rho = (PI / 2.0) - theta;

  {// Draw the semicircle
    p.setBrush(Qt::white);  // Fill colour
    // Draw Outer pie shape including the outline
    p.drawPie(QRectF(halfWidth - radius, 0, 2.0 * radius, 2.0 * radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
    // Draw a dividing arc within the pie.
    p.drawArc(QRectF(halfWidth - (radius / 2.0), radius / 2.0, radius, radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
  }

  p.setPen(palette().color(QPalette::WindowText));
  p.setBrush(palette().color(QPalette::WindowText));

  double step = (2 * theta) / 12.0;
  double stop = rho + (2 * theta) - (step / 2);
  {// Draw line markings
    for (double i = rho + step; i < stop; i += step) {
      qreal x = halfWidth + radius * cos(i);
      qreal y = radius - radius * sin(i);
      QPointF start(x, y);
      double t = 0.05; //0.025;
      p.drawLine(start, start + t * (center - start));
    }
  }

  {//Draw the text labels
    p.setPen(palette().color(QPalette::WindowText));

    const char* theNames[11] = { "+50", "+40", "+30", "+20", "+10", "0", "-10", "-20", "-30", "-40", "-50" };
    QFontMetrics fm = p.fontMetrics();
    int halfFontHeight = fm.height() / 2;
    int halfFontWidth;

    for (int j = 0; j < 11;) {
      double i = rho + step * (j + 1);
      qreal x = halfWidth + radius * cos(i);
      qreal y = radius - radius * sin(i);
      QPointF start(x, y);
      double t = 0.08; //0.025;
      QPointF pt = start + t * (center - start);
      halfFontWidth = fm.horizontalAdvance(theNames[j]) / 2;

      p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, theNames[j]);
      if (radius < 300) j += 2; else j++;
    }
    halfFontWidth = fm.horizontalAdvance("Cents") / 2;
    p.drawText(center.x() - halfFontWidth, center.y() * 0.2 + halfFontHeight, "Cents");
  }

  { //draw needle
    qreal closePitch = toInt(curPitch);
    double needleValue = 100 * (curPitch - float(closePitch));

    resetLeds();
    if (closePitch == 0) {
        // No pitch, don't draw the needle this update
      prevNeedleValue = -999;
      prevClosePitch = 0;
      needleValueToDraw = -999;

      resetLeds();
    } else {
      int VTLEDLetterLookup[12] = { 2, 2, 3, 3, 4, 5, 5, 6, 6, 0, 0, 1 };
      emit(ledSet(VTLEDLetterLookup[noteValue(closePitch)], true));
      if (isBlackNote(closePitch)) {
        emit(ledSet(7, true));
      }

      prevNeedleValue = needleValue;
      prevClosePitch = closePitch;
      needleValueToDraw = needleValue;

      if ((needleValueToDraw > -60.0) && (needleValueToDraw < 60.0)) {

        double centAngle = (2 * theta) / 120;
        double note = rho + (fabs(needleValueToDraw - 60) * centAngle);
        qreal halfKnobWidth = MAX(radius * 0.02, 1);

        p.setPen(Qt::darkRed);
        QPointF noteX(halfWidth + radius * cos(note), radius - radius * sin(note));
        QPointF knobNote(halfWidth + halfKnobWidth * cos(note), height() - halfKnobWidth * sin(note));
        QPointF knobLeft = center - QPointF(halfKnobWidth, 0);
        QPointF knobRight = center + QPointF(halfKnobWidth, 0);

        p.setBrush(Qt::red);
        QPolygonF points;
        points << noteX << knobRight<< knobLeft;
        p.drawPolygon(points);
      }
    }
  }
  endDrawing();
}

void TunerWidget::doUpdate(double thePitch)
{
  curPitch = thePitch;
  qreal closePitch = thePitch;
  double needleValue = 100 * (thePitch - float(closePitch));

  if ((fabs(prevNeedleValue - needleValue) < 0.05) && (prevClosePitch == closePitch)) {
    // Pitch hasn't changed (much), no update needed
    return;
  }
  update();
}

void TunerWidget::resetLeds()
{
  for (int i = 0; i < 8; i++) {
    emit(ledSet(i, false));
  }
}
