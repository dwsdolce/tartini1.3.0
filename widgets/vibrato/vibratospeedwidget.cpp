/***************************************************************************
                          vibratospeedwidget.cpp  -  description
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
#include "vibratospeedwidget.h"
#include <QPainter>
#include <QDebug>

#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoSpeedWidget::VibratoSpeedWidget(QWidget* parent)
  : DrawWidget(parent)
{
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);

  speedValueToDraw = 0;
  widthValueToDraw = 0;
  prevVibratoSpeed = 0;
  prevVibratoWidth = 0;
  currentNoteNumber = -1;
  widthLimit = 50;
  prevNoteNumber = -1;
  speedLabelCounter = 0;
  widthLabelCounter = 0;
  for (int i = 0; i < 100; i++) {
    speedLabels[i].label = QString(8, ' ');
    speedLabels[i].x = 0.0f;
    speedLabels[i].y = 0.0f;
    widthLabels[i].label = QString(8, ' ');
    widthLabels[i].x = 0.0f;
    widthLabels[i].y = 0.0f;
  }
  speedWidthFont = QFont();
  speedWidthFont.setPointSize(9);
}

VibratoSpeedWidget::~VibratoSpeedWidget()
{}

void VibratoSpeedWidget::resizeEvent(QResizeEvent*)
{
  // Do forced update on resize
  prevVibratoSpeed = -999;
  prevVibratoWidth = -999;

  // Paint event is called immediately on exit of this routine.
}

void VibratoSpeedWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();
  QPen pen(palette().color(QPalette::Foreground), 2);
  p.setPen(pen);  // Border

  QFontMetrics fm = QFontMetrics(speedWidthFont);
  setFont(speedWidthFont);
  int halfFontHeight = fm.height() / 2;
  char widthLabel[5];
  int halfFontWidth;

  const double halfWidth = double(width()) / 2.0;
  const double halfHeight = double(height()) / 2.0;
  const double radius = 1.8 * MAX(halfHeight, halfWidth);
  QPointF speedCenter(halfWidth, radius);
  QPointF widthCenter(halfWidth, halfHeight + radius);
  const double theta = asin(double(width()) / (2 * radius));
  const double thetaDeg = theta * 180.0 / PI;
  const double rho = (PI / 2.0) - theta;

  //===================================================================================
  // Draw the speed dial with the new width & height
  {// Draw the upper semicircle
    p.setBrush(Qt::white);  // Fill colour
     // Draw Outer pie shape including the outline
    p.drawPie(QRectF(halfWidth - radius, 0, 2.0 * radius, 2.0 * radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
    // Draw a dividing arc within the pie.
    p.drawArc(QRectF(halfWidth - (radius / 2.0), radius / 2.0, radius, radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
  }

  p.setPen(palette().color(QPalette::Foreground));
  p.setBrush(palette().color(QPalette::Foreground));

  {//Draw the text labels and text labels
    p.setPen(palette().color(QPalette::Foreground));

    double speedStep = (2 * theta) / 14.0;
    speedLabelCounter = 0;
    const char* speedLabelLookup[13] = { "12   ", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0" };

    for (int j = 0; j < 13; j++) {
      double i = rho + speedStep * (j + 1);
      qreal x = speedCenter.x() + radius * cos(i);
      qreal y = speedCenter.y() - radius * sin(i);
      QPointF start(x, y);
      halfFontWidth = fm.width(speedLabelLookup[j]) / 2;
      if (width() < 175) { // Small dial
        if ((j % 4) == 0) {
          // Bigger marking + text label
          p.setPen(pen);  // Border
          p.drawLine(start, start + 0.05 * (speedCenter - start));

          QPointF pt = start + 0.08 * (speedCenter - start);
          p.setPen(palette().color(QPalette::Foreground));
          p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, speedLabelLookup[j]);
        } else { // Smaller marking, no text label
          p.drawLine(start, start + 0.03 * (speedCenter - start));
        }
      } else if (width() < 300) { // Bigger dial
        if ((j % 2) == 0) { // Bigger marking + text label
          speedLabelLookup[0] = "12";
          p.setPen(pen);  // Border
          p.drawLine(start, start + 0.05 * (speedCenter - start));

          QPointF pt = start + 0.08 * (speedCenter - start);
          p.setPen(palette().color(QPalette::Foreground));
          p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, speedLabelLookup[j]);
        } else { // Smaller marking, no text label
          p.setPen(pen);  // Border
          p.drawLine(start, start + 0.03 * (speedCenter - start));
        }
      } else { // Big dial
        speedLabelLookup[0] = "12";
        p.setPen(pen);  // Border
        p.drawLine(start, start + 0.05 * (speedCenter - start));

        QPointF pt = start + 0.08 * (speedCenter - start);
        p.setPen(palette().color(QPalette::Foreground));
        p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, speedLabelLookup[j]);
      }

      // Draw the "Hz" label
      {
        halfFontWidth = fm.width("Hz") / 2;
        p.setPen(palette().color(QPalette::Foreground));
        p.drawText(speedCenter.x() - halfFontWidth, speedCenter.y() - 0.7 * radius + halfFontHeight, "Hz");
      }
    }
  }

  // Draw the speed needle
  if (speedValueToDraw != 0) {
    // Speed needle value, draw the speed needle this paint
    qreal halfKnobWidth = MAX(radius * 0.02, 1);

    if ((speedValueToDraw > 0) && (speedValueToDraw <= 12)) {
      float hzAngle = (2 * theta) / 14.0;
      float note = PI - rho - (speedValueToDraw * hzAngle) - hzAngle;

      p.setPen(Qt::darkRed);
      QPointF noteX(halfWidth + radius * cos(note), radius - radius * sin(note));
      QPointF knobNote(halfWidth + halfKnobWidth * cos(note), height() - halfKnobWidth * sin(note));
      QPointF knobLeft = speedCenter - QPointF(halfKnobWidth, 0);
      QPointF knobRight = speedCenter + QPointF(halfKnobWidth, 0);

      p.setBrush(Qt::red);
      QPolygonF points;
      points << noteX << knobRight << knobLeft;
      p.drawPolygon(points);
    }
  }

  //===================================================================================
  // Draw the width dial with the new width & height
  p.setPen(pen);  // Border
  // Determine whether the scale needs adjusting
  if ((widthValueToDraw > 0) && (currentNoteNumber == prevNoteNumber)) {
    if (widthValueToDraw > 50) {
      widthLimit = std::max(widthLimit, 100);
    }
    if (widthValueToDraw > 100) {
      widthLimit = std::max(widthLimit, 200);
    }
    if (widthValueToDraw > 200) {
      widthLimit = std::max(widthLimit, 300);
    }
  }
  if (prevNoteNumber != currentNoteNumber) {
    widthLimit = 50;
  }

  {// Draw the upper semicircle
    p.setBrush(Qt::white);  // Fill colour
    // Draw Outer pie shape including the outline
    p.drawPie(QRectF(halfWidth - radius, halfHeight, 2.0 * radius, 2.0 * radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
    // Draw a dividing arc within the pie.
    p.drawArc(QRectF(halfWidth - (radius / 2.0), halfHeight + (radius / 2.0), radius, radius), toInt((90 - thetaDeg) * 16), toInt(2 * thetaDeg * 16));
  }

  p.setPen(palette().color(QPalette::Foreground));
  p.setBrush(palette().color(QPalette::Foreground));

  {//Draw the text labels and text labels
    p.setPen(palette().color(QPalette::Foreground));

    double widthStep = (2 * theta) / 12.0;
    widthLabelCounter = 0;

    for (int j = 0; j < 11; j++) {
      double i = rho + widthStep * (j + 1);
      qreal x = widthCenter.x() + radius * cos(i);
      qreal y = widthCenter.y() - radius * sin(i);
      QPointF start(x, y);
      if (width() < 250) { // Small dial
        if ((j % 2) == 0) {
          // Bigger marking + text label
          p.setPen(pen);  // Border
          p.drawLine(start, start + 0.05 * (widthCenter - start));

          QPointF pt = start + 0.08 * (widthCenter - start);
          sprintf(widthLabel, "%d", widthLimit - ((widthLimit / 10) * j));
          halfFontWidth = fm.width(widthLabel) / 2;
          p.setPen(palette().color(QPalette::Foreground));
          p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, widthLabel);
        } else { // Smaller marking, no text label
          p.drawLine(start, start + 0.03 * (widthCenter - start));
        }
      } else { // Bigger marking + text label
        p.setPen(pen);  // Border
        p.drawLine(start, start + 0.05 * (widthCenter - start));

        QPointF pt = start + 0.08 * (widthCenter - start);
        sprintf(widthLabel, "%d", widthLimit - ((widthLimit / 10) * j));
        halfFontWidth = fm.width(widthLabel) / 2;
        p.setPen(palette().color(QPalette::Foreground));
        p.drawText(pt.x() - halfFontWidth, pt.y() + halfFontHeight, widthLabel);
      }

      // Draw the "Cents" label
      {
        halfFontWidth = fm.width("Cents") / 2;
        p.setPen(palette().color(QPalette::Foreground));
        p.drawText(widthCenter.x() - halfFontWidth, widthCenter.y() - 0.7 * radius + halfFontHeight, "Cents");
      }
    }
  }

  { // Draw the width needle
    if (widthValueToDraw != 0) {
      // Width needle value, draw the width needle this paint
      if ((widthValueToDraw > 0) && (widthValueToDraw <= widthLimit)) {
        float centAngle = (2 * theta) / (widthLimit + 2 * (widthLimit / 10));
        float note = PI - rho - (widthValueToDraw * centAngle) - (widthLimit / 10) * centAngle;
        qreal halfKnobWidth = MAX(radius * 0.02, 1);

        p.setPen(Qt::darkRed);
        QPointF noteX(widthCenter.x() + radius * cos(note), widthCenter.y() - radius * sin(note));
        QPointF knobNote(halfWidth + halfKnobWidth * cos(note), height() - halfKnobWidth * sin(note));
        QPointF knobLeft = widthCenter - QPointF(halfKnobWidth, 0);
        QPointF knobRight = widthCenter + QPointF(halfKnobWidth, 0);

        p.setBrush(Qt::red);
        QPolygonF points;
        points << noteX << knobRight << knobLeft;
        p.drawPolygon(points);
      }

    }
  }

  // Propogate the Note Number after all drawing is done.
  prevNoteNumber = currentNoteNumber;

  endDrawing();
}

void VibratoSpeedWidget::doUpdate()
{
  Channel* active = gdata->getActiveChannel();

  double vibratoSpeed = 0;
  double vibratoWidth = 0;
  currentNoteNumber = -1;

  if (active) {
    AnalysisData* data;
    if (gdata->soundMode & SOUND_REC) data = active->dataAtChunk(active->chunkAtCurrentTime() - active->pronyDelay());
    else data = active->dataAtCurrentChunk();
    if (data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      currentNoteNumber = data->noteIndex;
      vibratoSpeed = data->vibratoSpeed;
      vibratoWidth = 200 * data->vibratoWidth;
    }
  }

  if ((fabs(prevVibratoSpeed - vibratoSpeed) < 0.05) && (fabs(prevVibratoWidth - vibratoWidth) < 0.05)) {
    // Needle values haven't changed (much) , no update needed
    return;

  } else {
    {
    // Needle values have changed
      if ((vibratoSpeed == 0) && (vibratoWidth == 0)) {
        prevNoteNumber = -1;
      }
      if (vibratoSpeed == 0) {
        // No speed needle value, don't draw the speed needle this update
        prevVibratoSpeed = 0;
        speedValueToDraw = 0;
      } else {
        // Speed needle values, draw the speed needle this update
        prevVibratoSpeed = vibratoSpeed;
        speedValueToDraw = vibratoSpeed;
      }
      if (vibratoWidth == 0) {
        // No width needle value, don't draw the width needle this update
        prevVibratoWidth = 0;
        widthValueToDraw = 0;
      } else {
        // Speed needle values, draw the speed needle this update
        prevVibratoWidth = vibratoWidth;
        widthValueToDraw = vibratoWidth;
      }
    }
  }
  update();
}
