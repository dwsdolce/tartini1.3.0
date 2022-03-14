/***************************************************************************
                          vibratocirclewidget.cpp  -  description
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
#include "vibratocirclewidget.h"
#include <QPainter>
#include <QPolygon>
#include <QDebug>

#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"

VibratoCircleWidget::VibratoCircleWidget(QWidget *parent)
  : DrawWidget(parent)
{
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);

  accuracy = 1.0;
  type = 5;
  lastPeriodToDraw = -1;
  doPaint = true;
  prevPeriods.resize(6);
  patternColorVector.resize(6);
}

VibratoCircleWidget::~VibratoCircleWidget()
{
  patternVertices.clear();
  for (int p = 0; p < 6; p++) {
    prevPeriods[p].clear();
  }
}

void VibratoCircleWidget::resizeEvent(QResizeEvent*)
{
  // Do forced update on resize
  doPaint = false;
  doUpdate();
  doPaint = true;
}

void VibratoCircleWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();

  // Draw the reference circle
  const qreal halfWidth = 0.5 * width();
  const qreal halfHeight = 0.5 * height();

  QPen halfBlackPen(QColor(0, 0, 0, 64), 2.0);
  QPen blackPen(Qt::black, 2.0);

  p.setPen(halfBlackPen);

  // The horizontal line
  p.drawLine(QPointF(0.0, halfHeight), QPointF(width(), halfHeight));

  // The vertical line
  p.drawLine(QPointF(halfWidth, 0.0), QPointF(halfWidth, height()));

  p.setPen(blackPen);
  // The circle
  p.drawEllipse(QPointF(halfWidth, halfHeight), 0.8 * halfWidth, 0.8 * halfHeight);

  if ((type == 1) || (type == 2)) {
    // Draw a line using the patternVertices and the patternColor
    p.setPen(patternColor);
    p.drawPolyline(patternVertices);
  } else if ((type == 4) || (type == 5)) {
    // Draw a polygon using the patternVertices and the patternColor
    p.setPen(patternColor);
    p.setBrush(QBrush(patternColor));
    p.drawPolygon(patternVertices, Qt::WindingFill);
  } else if (type == 3) {
    // Draw the last 6 periods using the patterVerticesVector and the patternColorVector
    for (int i = 5; i >= 0; i--) {
      if (lastPeriodToDraw >= i) {
        p.setPen(patternColorVector.at(i));
        p.setBrush(QBrush(patternColorVector.at(i)));
        p.drawPolygon(prevPeriods.at(i));
      }
    }
  }

  endDrawing();
}

void VibratoCircleWidget::doUpdate()
{
  Channel* active = gdata->getActiveChannel();

  patternVertices.clear();

  int leftMinimumTime = -1;
  int maximumTime = -1;
  int rightMinimumTime = -1;
  int leftMinimumAt = -1;
  int maximumAt = -1;
  if ((active) && (active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
    AnalysisData* data = active->dataAtCurrentChunk();
    if (data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData* note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      // Determine which delay to use
      size_t smoothDelay = active->pitchBigSmoothingFilter->delay();
      large_vector<float> pitchLookupUsed = active->pitchLookupSmoothed;

      int currentTime = int(active->chunkAtCurrentTime() * active->framesPerChunk() + smoothDelay);
      size_t maximaSize = note->maxima->size();
      size_t minimaSize = note->minima->size();

      // Determine which period to show: the 2 rightmost minima + the maximum in between
      if ((maximaSize >= 1) && (minimaSize == 2)) {
        // Only 2 minima
        if (currentTime >= note->minima->at(1)) {
          leftMinimumTime = note->minima->at(0);
          rightMinimumTime = note->minima->at(1);
          leftMinimumAt = 0;
        }
      } else if ((maximaSize >= 1) && (minimaSize > 2)) {
        // More than 2 minima
        for (int i = 2; i < minimaSize; i++) {
          if ((currentTime >= note->minima->at(i - 1)) && (currentTime <= note->minima->at(i))) {
            leftMinimumTime = note->minima->at(i - 2);
            rightMinimumTime = note->minima->at(i - 1);
            leftMinimumAt = i - 2;
            break;
          }
        }
        if (currentTime > note->minima->at(minimaSize - 1)) {
          leftMinimumTime = note->minima->at(minimaSize - 2);
          rightMinimumTime = note->minima->at(minimaSize - 1);
          leftMinimumAt = int(minimaSize - 2);
        }
      }

      // The maximum in between
      for (int i = 0; i < maximaSize; i++) {
        if ((note->maxima->at(i) >= leftMinimumTime) && (note->maxima->at(i) <= rightMinimumTime)) {
          maximumTime = note->maxima->at(i);
          maximumAt = i;
          break;
        }
      }

      if (maximumTime > 0) {
        // There is at least one full period before this one
        const float halfWidth = 0.5 * width();
        const float halfHeight = 0.5 * height();

        if ((type == 1) || (type == 2)) {
          const int stepSize = active->rate() / 1000;  // Draw element for every 0.001s
          const int prevPeriodDuration = rightMinimumTime - leftMinimumTime;
          const int currentChunk = active->chunkAtCurrentTime();

          float prevMinimumPitch = (pitchLookupUsed.at(leftMinimumTime) >
            pitchLookupUsed.at(rightMinimumTime))
            ? pitchLookupUsed.at(rightMinimumTime)
            : pitchLookupUsed.at(leftMinimumTime);  // Minimum pitch of previous period
          float prevMaximumPitch = pitchLookupUsed.at(maximumTime);       // Maximum pitch of previous period
          float prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
          float prevWidth = prevMaximumPitch - prevMinimumPitch;

          size_t end_i = std::min((currentChunk + 1) * active->framesPerChunk() + smoothDelay, pitchLookupUsed.size());
          for (int i = rightMinimumTime; i < end_i; i += stepSize) {

            if (i > (int)pitchLookupUsed.size()) {
              // Break out of loop when end of smoothed pitchlookup is reached
              break;
            }

            if ((i - rightMinimumTime) > prevPeriodDuration) {
              // Break out of loop when current period is longer than previous period, 'circle' is complete then
              break;
            }

            float currentPitch = pitchLookupUsed.at(i);
            float prevPitch = pitchLookupUsed.at(i - prevPeriodDuration);

            switch (type) {
              case 1:
              {
                qreal phase = qreal(i - rightMinimumTime) / prevPeriodDuration;
                qreal cosPhase = cos(phase * twoPI);
                qreal sinPhase = sin(phase * twoPI);
                qreal comparison = (currentPitch - prevPitch) / prevWidth;

                patternVertices << QPointF(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase),
                                    halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase));
                patternColor = QColor(255, 0, 0, 255);
                break;
              }
              case 2:
              {
                qreal phase = qreal(i - rightMinimumTime) / prevPeriodDuration;
                qreal cosPhase = cos(phase * twoPI);
                qreal sinPhase = sin(phase * twoPI);
                qreal comparison = (currentPitch - (prevAvgPitch + (-cosPhase * prevWidth * 0.5))) / prevWidth;

                patternVertices << QPointF(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase),
                                           halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase));

                patternColor = QColor(255, 0, 0, 255);
                break;
              }
              default:
              {
                break;
              }
            }
          }
          // The patternVertices define a line of the indicated patternColor.
        } else if (type == 3) {
          for (int p = 0; p < 6; p++) {
            prevPeriods[p].clear();
          }
          const int stepSize = active->rate() / 1000;  // Draw element for every 0.001s
          for (int p = 0; p < 6; p++) {
            if (leftMinimumAt - p < 0) {
              lastPeriodToDraw = p - 1;
              break;
            }

            leftMinimumTime = note->minima->at(leftMinimumAt - p);
            rightMinimumTime = note->minima->at(leftMinimumAt - (p - 1));
            for (int j = 0; j < note->maxima->size(); j++) {
              if ((note->maxima->at(j) >= leftMinimumTime) && (note->maxima->at(j) <= rightMinimumTime)) {
                maximumTime = note->maxima->at(j);
                break;
              }
            }

            qreal prevMinimumPitch = (pitchLookupUsed.at(leftMinimumTime) >
              pitchLookupUsed.at(rightMinimumTime))
              ? pitchLookupUsed.at(rightMinimumTime)
              : pitchLookupUsed.at(leftMinimumTime);
            qreal prevMaximumPitch = pitchLookupUsed.at(maximumTime);
            qreal prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
            qreal prevWidth = prevMaximumPitch - prevMinimumPitch;

            for (int i = leftMinimumTime; i <= rightMinimumTime; i += stepSize) {

              qreal pitch = pitchLookupUsed.at(i);

              qreal phase = qreal(i - leftMinimumTime) / float(rightMinimumTime - leftMinimumTime);
              qreal cosPhase = cos(phase * twoPI);
              qreal sinPhase = sin(phase * twoPI);
              qreal comparison = (pitch - (prevAvgPitch + (-cosPhase * prevWidth * 0.5))) / prevWidth;

              prevPeriods[p] << QPointF(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase),
                                        halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase));
            }
            patternColorVector[p] = QColor(255, 0, 0, toInt(qreal(1 / pow(2, p)) * 255));

            // The patternVertices define a line with each segment being a different color.
            lastPeriodToDraw = p;
          }
        } else if (type == 4) {

          const int periodDuration = rightMinimumTime - leftMinimumTime;

          for (int j = 0; j < 360; j++) {
            int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
            if (i > rightMinimumTime) {
              i = rightMinimumTime;
            }

            qreal pitch = pitchLookupUsed.at(i);

            qreal phase = qreal(i - leftMinimumTime) / qreal(periodDuration);
            qreal cosPhase = cos(phase * twoPI);
            qreal sinPhase = sin(phase * twoPI);
            qreal minStep = qreal(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
            qreal thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
            qreal thisMaximumPitch = pitchLookupUsed.at(maximumTime);
            qreal thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
            qreal thisWidth = thisMaximumPitch - thisMinimumPitch;
            qreal comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;

            patternVertices << QPointF(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase),
                                       halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase));
          }
          patternColor = QColor(255, 0, 0, 200);

          // The patternVertices define a filled polygon.
        } else if (type == 5) {
          if (leftMinimumAt == 0) {  // Fade in first period
            const int periodDuration = rightMinimumTime - leftMinimumTime;
            const int fadeAlphaInt = 200 * (currentTime - rightMinimumTime) / periodDuration;
            const int fadeAlpha = (fadeAlphaInt > 255) ? 255 : fadeAlphaInt;

            for (int j = 0; j < 360; j++) {
              int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
              if (i > rightMinimumTime) {
                i = rightMinimumTime;
              }

              qreal pitch = pitchLookupUsed.at(i);

              qreal phase = float(i - leftMinimumTime) / float(periodDuration);
              qreal cosPhase = cos(phase * twoPI);
              qreal sinPhase = sin(phase * twoPI);
              qreal minStep = float(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
              qreal thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
              qreal thisMaximumPitch = pitchLookupUsed.at(maximumTime);
              qreal thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
              qreal thisWidth = thisMaximumPitch - thisMinimumPitch;
              qreal comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;

              patternVertices << QPointF(halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase),
                                         halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase));
            }
            patternColor = QColor(255, 0, 0, fadeAlpha);
            
          } else if (leftMinimumAt > 0) {  // Morph from previous period to current period

            const int prevLeftMinimumTime = note->minima->at(leftMinimumAt - 1);
            const int prevRightMinimumTime = leftMinimumTime;
            const int prevMaximumTime = note->maxima->at(maximumAt - 1);
            const int prevPeriodDuration = prevRightMinimumTime - prevLeftMinimumTime;
            qreal prevMaximumPitch = pitchLookupUsed.at(prevMaximumTime);
            const int periodDuration = rightMinimumTime - leftMinimumTime;
            qreal thisMaximumPitch = pitchLookupUsed.at(maximumTime);
            qreal distanceRatio = (currentTime - rightMinimumTime) / float(periodDuration);
            if (distanceRatio > 1) {
              distanceRatio = 1;
            }

            for (int j = 0; j < 360; j++) {
              int prevI = prevLeftMinimumTime + toInt(floor((j / float(360)) * prevPeriodDuration));
              if (prevI > prevRightMinimumTime) {
                prevI = prevRightMinimumTime;
              }
              int i = leftMinimumTime + toInt(floor((j / float(360)) * periodDuration));
              if (i > rightMinimumTime) {
                i = rightMinimumTime;
              }

              qreal prevPitch = pitchLookupUsed.at(prevI);
              qreal prevPhase = float(prevI - prevLeftMinimumTime) / float(prevPeriodDuration);
              qreal prevCosPhase = cos(prevPhase * twoPI);
              qreal prevSinPhase = sin(prevPhase * twoPI);
              qreal prevMinStep = float(prevI - prevLeftMinimumTime) * ((pitchLookupUsed.at(prevRightMinimumTime) - pitchLookupUsed.at(prevLeftMinimumTime)) / prevPeriodDuration);
              qreal prevMinimumPitch = pitchLookupUsed.at(prevLeftMinimumTime) + prevMinStep;
              qreal prevAvgPitch = (prevMinimumPitch + prevMaximumPitch) * 0.5;
              qreal prevWidth = prevMaximumPitch - prevMinimumPitch;
              qreal prevComparison = (prevPitch - (prevAvgPitch + (-prevCosPhase * prevWidth * 0.5))) / prevWidth;
              qreal prevX = halfWidth + halfWidth * (0.8 * prevCosPhase + accuracy * prevComparison * prevCosPhase);
              qreal prevY = halfHeight - halfHeight * (0.8 * prevSinPhase + accuracy * prevComparison * prevSinPhase);

              qreal pitch = pitchLookupUsed.at(i);
              qreal phase = float(i - leftMinimumTime) / float(periodDuration);
              qreal cosPhase = cos(phase * twoPI);
              qreal sinPhase = sin(phase * twoPI);
              qreal minStep = float(i - leftMinimumTime) * ((pitchLookupUsed.at(rightMinimumTime) - pitchLookupUsed.at(leftMinimumTime)) / periodDuration);
              qreal thisMinimumPitch = pitchLookupUsed.at(leftMinimumTime) + minStep;
              qreal thisAvgPitch = (thisMinimumPitch + thisMaximumPitch) * 0.5;
              qreal thisWidth = thisMaximumPitch - thisMinimumPitch;
              qreal comparison = (pitch - (thisAvgPitch + (-cosPhase * thisWidth * 0.5))) / thisWidth;
              qreal thisX = halfWidth + halfWidth * (0.8 * cosPhase + accuracy * comparison * cosPhase);
              qreal thisY = halfHeight - halfHeight * (0.8 * sinPhase + accuracy * comparison * sinPhase);

              patternVertices << QPointF(prevX + distanceRatio * (thisX - prevX), prevY + distanceRatio * (thisY - prevY));
            }

            patternColor = QColor(255, 0, 0, 200);

          }
          // The patternVertices define a filled polygon.
        }
      }
    }
  }

  if (doPaint) {
    update();
  }
}

void VibratoCircleWidget::setAccuracy(int value)
{
  accuracy = value / 10.0;
  doUpdate();
}

void VibratoCircleWidget::setType(int value)
{
  type = value;
  doUpdate();
}
