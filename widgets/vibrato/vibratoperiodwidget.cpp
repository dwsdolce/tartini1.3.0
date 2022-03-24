/***************************************************************************
                          vibratoperiodwidget.cpp  -  description
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
#include "vibratoperiodwidget.h"
#include <QPainter>
#include <QDebug>

#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoPeriodWidget::VibratoPeriodWidget(QWidget *parent)
  : DrawWidget(parent)
{
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);

  prevLeftMinimumTime = -1;
  lastPeriodToDraw = -1;

  previousPoly.resize(6);
  previousPolyAlpha.resize(6);

  smoothedPeriods = true;
  drawSineReference = false;
  sineStyle = false;
  drawPrevPeriods = false;
  periodScaling = false;
  drawComparison = false;
  doPaint = true;
}

VibratoPeriodWidget::~VibratoPeriodWidget()
{
  // Remove display lists
  sineReference.clear();
  for (int i = 0; i < 5; i++) {
    previousPoly[i].clear();
  }
  currentPeriod.clear();
  comparisonPoly.clear();
}

void VibratoPeriodWidget::resizeEvent(QResizeEvent*)
{
    // Do forced update on resize
  prevLeftMinimumTime = -1;
  doPaint = false;
  doUpdate();
  doPaint = true;
}

void VibratoPeriodWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();

  // Draw the horizontal reference line
  const qreal halfHeight = 0.5 * height();
  QPen comparisonReferencePen = QPen(QColor(0, 0, 0, 64), 1.5);
  p.setPen(comparisonReferencePen);
  p.drawLine(QPointF(0, halfHeight), QPointF(width(), halfHeight));
    
  // Draw the sinewave
  QPen sineReferencePen = QPen(QColor(255, 255, 0, 255), 2.0);
  p.setPen(sineReferencePen);
  p.drawPolyline(sineReference);

  // Draw the comparison
  QPen comparisonPolyPen = QPen(QColor(0, 255, 0, 255), 2.0);
  p.setPen(comparisonPolyPen);
  p.drawPolyline(comparisonPoly);

  // Draw the previous periods
  for (int i = 4; i >= 0; i--) {
    if (lastPeriodToDraw >= i) {
      QPen previousPolyPen = QPen(QColor(127, 0, 0, previousPolyAlpha[i]), 2.0);
      p.setPen(previousPolyPen);
      p.drawPolyline(previousPoly[i]);
    }
  }

  // Draw the current period
  QPen currentPeriodPen = QPen(QColor(127, 0, 0, 255), 2.0);
  p.setPen(currentPeriodPen);
  p.drawPolyline(currentPeriod);

  endDrawing();
}

void VibratoPeriodWidget::doUpdate()
{
  Channel *active = gdata->getActiveChannel();

  int leftMinimumTime = -1;
  int maximumTime = -1;
  int rightMinimumTime = -1;
  int leftMinimumAt = -1;

  if ((active) && (active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
    AnalysisData *data = active->dataAtCurrentChunk();
    if((data != nullptr) && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData *note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      int smoothDelay = int(active->pitchBigSmoothingFilter->delay());
      int currentTime = active->chunkAtCurrentTime() * active->framesPerChunk() + smoothDelay;
      size_t maximaSize = note->maxima->size();
      size_t minimaSize = note->minima->size();

      // Determine which period to show: the 2 rightmost minima + the maximum in between
      if ((maximaSize >= 1 ) && (minimaSize == 2)) {
        // Only 2 minima
        if (currentTime >= note->minima->at(1)) {
          leftMinimumTime  = note->minima->at(0);
          rightMinimumTime = note->minima->at(1);
          leftMinimumAt = 0;
        }
      } else if ((maximaSize >= 1) && (minimaSize > 2)) {
        // More than 2 minima
        for (int i = 2; i < minimaSize; i++) {
          if ((currentTime >= note->minima->at(i-1)) && (currentTime <= note->minima->at(i))) {
            leftMinimumTime  = note->minima->at(i-2);
            rightMinimumTime = note->minima->at(i-1);
            leftMinimumAt = i-2;
            break;
          }
        }
        if (currentTime > note->minima->at(minimaSize - 1)) {
          leftMinimumTime  = note->minima->at(minimaSize - 2);
          rightMinimumTime = note->minima->at(minimaSize - 1);
          leftMinimumAt = int(minimaSize - 2);
        }
      }
      // The maximum in between
      for (int i = 0; i < maximaSize; i++) {
        if ((note->maxima->at(i) >= leftMinimumTime) && (note->maxima->at(i) <= rightMinimumTime)) {
          maximumTime = note->maxima->at(i);
          break;
        }
      }
    }
  }

  if (prevLeftMinimumTime == leftMinimumTime) {
    // Same period as previous, don't change the polys & no update needed
  } else {
    // Period has changed
    if (leftMinimumTime == -1) {
      // No period to draw, erase polys & update
      sineReference.clear();

      for (int i = 0; i < 5; i++) {
        previousPoly[i].clear();
      }

      currentPeriod.clear();
      comparisonPoly.clear();

      prevLeftMinimumTime = -1;
      if (doPaint) {
        update();
      }
    } else {
      // New period, calculate new polys & update
      sineReference.clear();

      for (int i = 0; i < 5; i++) {
        previousPoly[i].clear();
      }

      currentPeriod.clear();
      comparisonPoly.clear();

      AnalysisData *data = active->dataAtCurrentChunk();
      NoteData *note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      large_vector<float> thePitchLookup;
      int theDelay;
      if(smoothedPeriods) {
        thePitchLookup = active->pitchLookupSmoothed;
        theDelay = 0;
      } else {
        thePitchLookup = active->pitchLookup;
        theDelay = int(active->pitchBigSmoothingFilter->delay() - active->pitchSmallSmoothingFilter->delay());
      }

      int theLeftMinimumTime = leftMinimumTime - theDelay;
      int theRightMinimumTime = rightMinimumTime - theDelay;
      int theMaximumTime = maximumTime - theDelay;
      int periodDuration = theRightMinimumTime - theLeftMinimumTime;

      float minimumPitch = (thePitchLookup.at(theLeftMinimumTime) >
                            thePitchLookup.at(theRightMinimumTime))
                          ? thePitchLookup.at(theRightMinimumTime)
                          : thePitchLookup.at(theLeftMinimumTime);
      float maximumPitch = thePitchLookup.at(theMaximumTime);
      float periodWidth = maximumPitch - minimumPitch;

      const float halfHeight = 0.5 * height();

      // Display either sine- or cosine-style periods
      int theSineDelay = 0;
      if (sineStyle) {
        theSineDelay = toInt((rightMinimumTime - leftMinimumTime) * 0.25);
      }

      // Calculate the sinewave, currentValue, and comparison
      if ((width() > 0) && (height() > 0)) {
        double xValue;
        double ySineValue, yCurrentValue;
        for (float xx = 0; xx < width(); xx++) {
          xValue = 0.05 * width() + 0.9 * xx;
          if (drawSineReference) {
            if (sineStyle) {
              ySineValue = halfHeight - halfHeight * 0.9 * sin((xx / width()) * 2 * PI);
            } else {
              ySineValue = halfHeight - halfHeight * -0.9 * cos((xx / width()) * 2 * PI);
            }
            sineReference << QPointF(xValue, ySineValue);
          }
          yCurrentValue = 0.95 * height() - 0.9 * ((thePitchLookup.at(toInt((xx / width()) * periodDuration + theLeftMinimumTime + theSineDelay)) - minimumPitch) / periodWidth) * height();
          currentPeriod << QPointF(xValue, yCurrentValue);
          if (drawComparison && drawSineReference) {
            comparisonPoly << QPointF(xValue, halfHeight - (ySineValue - yCurrentValue));
          }
        }
      }

      // Calculate the previous period(s)
      if (drawPrevPeriods && (width() > 0) && (height() > 0)) {
        for (int i = 0; i < 5; i++) {
          if (leftMinimumAt - (i+1) < 0) { 
            lastPeriodToDraw = i - 1;
            break;
          }

          int thisPrevLeftMinimumTime = note->minima->at(leftMinimumAt - (i+1)) - theDelay;
          int thisPrevRightMinimumTime = note->minima->at(leftMinimumAt - i) - theDelay;
          int thisPrevDuration = thisPrevRightMinimumTime - thisPrevLeftMinimumTime;
          int thisPrevMaximumTime = 0;
          for (int j = 0; j < note->maxima->size(); j++) {
            if ((note->maxima->at(j) >= thisPrevLeftMinimumTime) && (note->maxima->at(j) <= thisPrevRightMinimumTime)) {
              thisPrevMaximumTime = note->maxima->at(j) - theDelay;
              break;
            }
          }

          float thisPrevMinimumPitch = (thePitchLookup.at(thisPrevLeftMinimumTime) >
                                        thePitchLookup.at(thisPrevRightMinimumTime))
                                      ? thePitchLookup.at(thisPrevRightMinimumTime)
                                      : thePitchLookup.at(thisPrevLeftMinimumTime);
          float thisPrevMaximumPitch = thePitchLookup.at(thisPrevMaximumTime);
          float thisPrevWidth = thisPrevMaximumPitch - thisPrevMinimumPitch;

          if (periodScaling) {
            for (float xx = 0; xx < width(); xx++) {
              int offset = toInt((xx / width()) * thisPrevDuration + thisPrevLeftMinimumTime + theSineDelay);
              previousPoly[i] << QPointF(0.05 * width() + 0.9 * xx,
                                         0.95 * height() - 0.9 * ((thePitchLookup.at(offset) - thisPrevMinimumPitch) / thisPrevWidth) * height());
            }
          } else {
            for (float xx = 0; xx < width(); xx++) {
              int offset = toInt((xx / width()) * thisPrevDuration + thisPrevLeftMinimumTime + theSineDelay);
              float xxx = xx * (float(thisPrevDuration) / periodDuration);
              xxx = xxx + ((0.5 * (periodDuration - thisPrevDuration) / periodDuration) * width());
              previousPoly[i] << QPointF(0.05 * width() + 0.9 * xxx,
                                         0.95 * height() - 0.9 * ((thePitchLookup.at(offset) - minimumPitch) / periodWidth) * height());
            }
          }
          previousPolyAlpha[i] = toInt(float(1 / pow(2, i + 1)) * 255);
          lastPeriodToDraw = i;
        }
      }

      prevLeftMinimumTime = leftMinimumTime;

      if (doPaint) {
        update();
      }
    }
  }
}

void VibratoPeriodWidget::setSmoothedPeriods(bool value)
{
  smoothedPeriods = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidget::setDrawSineReference(bool value)
{
  drawSineReference = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidget::setSineStyle(bool value)
{
  sineStyle = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidget::setDrawPrevPeriods(bool value)
{
  drawPrevPeriods = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidget::setPeriodScaling(bool value)
{
  periodScaling = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}

void VibratoPeriodWidget::setDrawComparison(bool value)
{
  drawComparison = value;
  prevLeftMinimumTime = -1;
  doUpdate();
}
