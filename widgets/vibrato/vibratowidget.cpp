/***************************************************************************
                          vibratowidget.cpp  -  description
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
#include <QPainter>

#include "vibratowidget.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "musicnotes.h"

VibratoWidget::VibratoWidget(QWidget *parent, int nls)
  : DrawWidget(parent)
{
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);

  noteLabelOffset = nls; // The horizontal space in pixels a note label requires
  zoomFactorX = 2.0;
  zoomFactorY = 1.0;
  offsetY = 0;
  noteLabelCounter = 0;
  for (int i = 0; i < 100; i++) {
    noteLabels[i].label = QString(8,' ');
    noteLabels[i].y = 0.0;
  }
  vibratoFont = QFont();
  vibratoFont.setPointSize(9);

  // Set the colors for all of the items with static color
  verticalSeparatorLinesColor = QColor(131, 144, 159);
  referenceLinesColor = QColor(144, 156, 170);
  pronyWidthBandColor = QColor(0, 0, 0, 64);
  pronyAveragePitchColor = QColor(0, 0, 0, 127);
  vibratoPolylineColor = QColor(127, 0, 0);
  currentWindowBandColor = QColor(palette().color(QPalette::Foreground).red(), palette().color(QPalette::Foreground).green(), palette().color(QPalette::Foreground).blue(), 65);
  currentTimeLineColor = QColor(Qt::black);
  maximaPointsColor = QColor(255, 255, 0);
  minimaPointsColor = QColor(0, 255, 0);
}

VibratoWidget::~VibratoWidget()
{
  // Remove display lists
  verticalPeriodBarsShading1.clear();
  verticalPeriodBarsShading2.clear();
  verticalSeparatorLines.clear();
  referenceLines.clear();
  int count = pronyWidthBand.count();
  for (int p = 0; p < count; p++) {
    pronyWidthBand[p].clear();
  }
  pronyWidthBand.clear();
  pronyAveragePitch.clear();
  vibratoPolyline.clear();
  currentWindowBand.clear();
  maximaPoints.clear();
  minimaPoints.clear();
}

void VibratoWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();

  doUpdate();

  // Draw the vertical bars that indicate the vibrato periods
  p.setBrush(QBrush(verticalPeriodBarsShading1Color));
  p.drawRects(verticalPeriodBarsShading1);
  p.setBrush(verticalPeriodBarsShading2Color);
  p.drawRects(verticalPeriodBarsShading2);

  // Draw the vertical separator lines through the extrema
  p.setPen(verticalSeparatorLinesColor);
  p.drawLines(verticalSeparatorLines);

  // Draw the horizontal reference lines
  QPen stipplePen(referenceLinesColor);
  stipplePen.setStyle(Qt::DashDotDotLine);
  p.setPen(stipplePen);
  p.drawLines(referenceLines);

  // Draw the light grey band indicating the vibratowidth according to the Prony-algorithm
  p.setBrush(QBrush(pronyWidthBandColor));
  int count = pronyWidthBand.count();
  for (int index = 0; index < count; index++) {
    p.drawPolygon(pronyWidthBand[index]);
  }

  // Draw the average pitch according to the Prony-algorithm
  p.setPen(pronyAveragePitchColor);
  p.drawPolyline(pronyAveragePitch);

  // Draw the vibrato-polyline
  QPen vpen(vibratoPolylineColor, 2.0);
  p.setPen(vpen);
  p.drawPolyline(vibratoPolyline);

  // Draw the light grey band indicating which time is being used in the current window
  p.setPen(currentWindowBandColor);
  p.setBrush(QBrush(currentWindowBandColor));
  p.drawRects(currentWindowBand);

  // Draw the current timeline
  p.setPen(currentTimeLineColor);
  p.drawLine(currentTimeLine);

  // Draw the maxima & minima
  p.setPen(maximaPointsColor);
  p.setBrush(maximaPointsColor);
  p.drawRects(maximaPoints);
  p.setPen(minimaPointsColor);
  p.setBrush(minimaPointsColor);
  p.drawRects(minimaPoints);

  // Draw the labels
  QFontMetrics fm = QFontMetrics(vibratoFont);
  setFont(vibratoFont);

  // Draw the note labels
  p.setPen(Qt::black);
  for (int i = 0; i < noteLabelCounter; i++) {
    p.drawText(3, height() - noteLabels[i].y + 4, noteLabels[i].label);
    p.drawText(width() - noteLabelOffset + 3, height() - noteLabels[i].y + 4, noteLabels[i].label);
  }

  endDrawing();
}

void VibratoWidget::doUpdate()
{
  noteLabelCounter = 0;

  Channel *active = gdata->getActiveChannel();

  if (active) {
    ChannelLocker channelLocker(active);
    AnalysisData *data = active->dataAtCurrentChunk();
    if(data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
      NoteData *note = new NoteData();
      note = &(active->noteData[data->noteIndex]);

      const int myStartChunk = note->startChunk();
      const int myEndChunk = note->endChunk();
      const int myCurrentChunk = active->chunkAtCurrentTime();
      const float halfHeight = 0.5 * height();
      const int maximaSize = int(note->maxima->size());
      const int minimaSize = int(note->minima->size());
      const float avgPitch = note->avgPitch();
      const int framesPerChunk = active->framesPerChunk();
      const float zoomFactorYx100 = zoomFactorY * 100;

      float windowOffset;
      large_vector<float> pitchLookupUsed = active->pitchLookupSmoothed;
      int smoothDelay = int(active->pitchBigSmoothingFilter->delay());

      if ((myEndChunk - myStartChunk) * zoomFactorX > width() - 2 * noteLabelOffset) {
        // The vibrato-polyline doesn't fit in the window
        if ((myCurrentChunk - myStartChunk) * zoomFactorX < (width() - 2 * noteLabelOffset)/2) {
          // We're at the left side of the vibrato-polyline
          windowOffset = 0 - noteLabelOffset;
        } else if ((myEndChunk - myCurrentChunk) * zoomFactorX < (width() - 2 * noteLabelOffset)/2) {
          // We're at the right side of the vibrato-polyline
          windowOffset = (myEndChunk - myStartChunk) * zoomFactorX - width() + noteLabelOffset + 1;
        } else {  // We're somewhere in the middle of the vibrato-polyline
          windowOffset = (myCurrentChunk - myStartChunk) * zoomFactorX - width()/2;
        }
      } else {  // The vibrato-polyline does fit in the window
        windowOffset = 0 - noteLabelOffset;
      }

      // Calculate the alternating vertical bars that indicate the vibrato periods
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        int color1Bars = 0;  // No. of bars with the left side at a maximum
        int color2Bars = 0;  // No. of bars with the left side at a minimum
        bool maximumFirst = true;   // The first extremum is a maximum?

        if (maximaSize + minimaSize >= 2) {  // There is at least one bar to calculate
          if (maximaSize == minimaSize) {
            if (note->maxima->at(0) < note->minima->at(0)) {
              color1Bars = maximaSize;
              color2Bars = minimaSize - 1;
              maximumFirst = true;
            } else {
              color1Bars = maximaSize - 1;
              color2Bars = minimaSize;
              maximumFirst = false;
            }
          } else {
            color1Bars = maximaSize > minimaSize ? minimaSize : maximaSize;
            color2Bars = maximaSize > minimaSize ? minimaSize : maximaSize;
            maximumFirst = maximaSize > minimaSize;
          }
        }

        qreal x1, x2;
        // Calculate the bars with the left side at a maximum
        verticalPeriodBarsShading1.clear();
        for (int i = 0; i < color1Bars; i++) {
          x1 = ((((qreal)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
          if (x1 > width() - noteLabelOffset) { break; }
          if (maximumFirst) {
            x2 = ((((qreal)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          } else {
            x2 = ((((qreal)note->minima->at(i+1) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          }
          if (x2 < noteLabelOffset) { continue; }
          if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

          verticalPeriodBarsShading1 << QRectF(QPointF(x1, 0), QPointF(x2, height()));
        }
        verticalPeriodBarsShading1Color = gdata->shading1Color();

        // Calculate the bars with the left side at a minimum
        verticalPeriodBarsShading2.clear();
        for (int i = 0; i < color2Bars; i++) {
          x1 = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
          if (x1 > width() - noteLabelOffset) { break; }
          if (maximumFirst) {
            x2 = ((((float)note->maxima->at(i+1) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          } else {
            x2 = ((((float)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          }
          if (x2 < noteLabelOffset) { continue; }
          if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

          verticalPeriodBarsShading2 << QRectF(QPointF(x1, 0), QPointF(x2, height()));
        }
        verticalPeriodBarsShading2Color = gdata->shading2Color();

        // Calculate the vertical separator lines through the maxima
        verticalSeparatorLines.clear();
        for (int i = 0; i < maximaSize; i++) {
          x1 = ((((float)note->maxima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { continue; }
          if (x1 > width() - noteLabelOffset) { break; }
          verticalSeparatorLines << QLineF(QPointF(x1, 0), QPointF(x1, height()));
        }

        // Calculate the vertical separator lines through the minima
        for (int i = 0; i < minimaSize; i++) {
          x1 = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x1 < noteLabelOffset) { continue; }
          if (x1 > width() - noteLabelOffset) { break; }
          verticalSeparatorLines << QLineF(QPointF(x1, 0), QPointF(x1, height()));
        }
      }

      // Calculate the horizontal reference lines + note labels
      referenceLines.clear();
      const int nearestNote = toInt(avgPitch);
      QString noteLabel;
      const qreal referenceLineX1 = noteLabelOffset;
      const qreal referenceLineX2 = width() - noteLabelOffset;
      qreal referenceLineY;

      // Calculate the nearest reference line + note label
      referenceLineY = halfHeight + (((nearestNote - avgPitch) * zoomFactorYx100) + offsetY);
      referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

      if ((noteOctave(nearestNote) >= 0) && (noteOctave(nearestNote) <= 9)) {
        noteLabel.sprintf("%s%d", noteName(nearestNote), noteOctave(nearestNote));
      } else {
        noteLabel.sprintf("%s ", noteName(nearestNote));
      }
      noteLabels[noteLabelCounter].label = noteLabel;
      noteLabels[noteLabelCounter].y = referenceLineY;
      noteLabelCounter++;

      // Calculate as many reference lines + note labels above the note as can be seen
      for (int i = 1; ; i++) {
        referenceLineY = halfHeight + (((nearestNote + i - avgPitch) * zoomFactorYx100) + offsetY);
        if (referenceLineY > height()) { break; }
        referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

        if ((noteOctave(nearestNote + i) >= 0) && (noteOctave(nearestNote + i) <= 9)) {
          noteLabel.sprintf("%s%d", noteName(nearestNote + i), noteOctave(nearestNote + i));
        } else {
          noteLabel.sprintf("%s ", noteName(nearestNote + i));
        }
        noteLabels[noteLabelCounter].label = noteLabel;
        noteLabels[noteLabelCounter].y = referenceLineY;
        noteLabelCounter++;
      }

      // Calculate as many reference lines + note labels below the note as can be seen
      for (int i = -1; ; i--) {
        referenceLineY = halfHeight + (((nearestNote + i - avgPitch) * zoomFactorYx100) + offsetY);
        if (referenceLineY < 0) { break; }
        referenceLines << QLineF(QPointF(referenceLineX1, height() - referenceLineY), QPointF(referenceLineX2, height() - referenceLineY));

        if ((noteOctave(nearestNote + i) >= 0) && (noteOctave(nearestNote + i) <= 9)) {
          noteLabel.sprintf("%s%d", noteName(nearestNote + i), noteOctave(nearestNote + i));
        } else {
          noteLabel.sprintf("%s ", noteName(nearestNote + i));
        }
        noteLabels[noteLabelCounter].label = noteLabel;
        noteLabels[noteLabelCounter].y = referenceLineY;
        noteLabelCounter++;
      }

      // Calculate the light grey band indicating the vibratowidth according to the Prony-algorithm
      int count = pronyWidthBand.count();
      for (int p = 0; p < count; p++) {
        pronyWidthBand[p].clear();
      }
      pronyWidthBand.clear();
      for (int chunk = myStartChunk; chunk < myEndChunk - 1; chunk++) {
        float x1 = (chunk - myStartChunk) * zoomFactorX - windowOffset;
        if (x1 < noteLabelOffset) { x1 = noteLabelOffset; }
        if (x1 > width() - noteLabelOffset) { break; }
        float x2 = (chunk + 1 - myStartChunk) * zoomFactorX - windowOffset;
        if (x2 < noteLabelOffset) { continue; }
        if (x2 > width() - noteLabelOffset) { x2 = width() - noteLabelOffset; }

        float vibratoPitch1 = active->dataAtChunk(chunk)->vibratoPitch;
        float vibratoWidth1 = active->dataAtChunk(chunk)->vibratoWidth;
        float vibratoPitch2 = active->dataAtChunk(chunk + 1)->vibratoPitch;
        float vibratoWidth2 = active->dataAtChunk(chunk + 1)->vibratoWidth;

        float y1 = halfHeight + ((vibratoPitch1 + vibratoWidth1 - avgPitch) * zoomFactorYx100) + offsetY;
        float y2 = halfHeight + ((vibratoPitch1 - vibratoWidth1 - avgPitch) * zoomFactorYx100) + offsetY;
        float y3 = halfHeight + ((vibratoPitch2 - vibratoWidth2 - avgPitch) * zoomFactorYx100) + offsetY;
        float y4 = halfHeight + ((vibratoPitch2 + vibratoWidth2 - avgPitch) * zoomFactorYx100) + offsetY;

        QPolygonF trap;
        trap << QPointF(x1, height() - y1) << QPointF(x1, height() - y2) << QPointF(x2, height() - y3) << QPointF(x2, height() - y4);
        pronyWidthBand << trap;
      }

      // Calculate the average pitch according to the Prony-algorithm
      pronyAveragePitch.clear();
      for (int chunk = myStartChunk; chunk < myEndChunk; chunk++) {
        float x = (chunk - myStartChunk) * zoomFactorX - windowOffset;
        if (x < noteLabelOffset) { continue; }
        if (x > width() - noteLabelOffset) { break; }
        float y = halfHeight + ((active->dataAtChunk(chunk)->vibratoPitch - avgPitch) * zoomFactorYx100) + offsetY;
        pronyAveragePitch << QPointF(x, height() - y);
      }

      // Calculate the vibrato-polyline
      vibratoPolyline.clear();
      if ((active->doingDetailedPitch()) && (pitchLookupUsed.size() > 0)) {
        // Detailed pitch information available, calculate polyline using this info
        const int pitchLookupUsedSizeLimit = int(pitchLookupUsed.size() - 1);
        const int beginningOfNote = myStartChunk * framesPerChunk;
        const int endOfNote = myEndChunk * framesPerChunk - 1;
        float chunk;
        float y;
        int offset;
        for (int x = noteLabelOffset; x < width() - noteLabelOffset; x++) {
          chunk = ((x + windowOffset) / zoomFactorX + myStartChunk);
          if ((chunk >= myStartChunk) && (chunk <= myEndChunk)) {
            offset = toInt(chunk * framesPerChunk + smoothDelay);
            if(offset > endOfNote) offset = endOfNote;
            if (offset < beginningOfNote + 3*smoothDelay) {
              int timeAtZero = beginningOfNote + smoothDelay;
              float scaleX = (offset - timeAtZero) / float(2*smoothDelay);
              float pitchAtZero = active->dataAtChunk(myStartChunk)->pitch;
              //float pitchAt2SmoothDelay = pitchLookupUsed.at(beginningOfNote + 3*smoothDelay);
              int smoothDelayPos3 = std::min(beginningOfNote + 3*smoothDelay, (int)pitchLookupUsed.size()-1);
              if(smoothDelayPos3 >= 0) {
                float pitchAt2SmoothDelay = pitchLookupUsed.at(smoothDelayPos3);
                y = halfHeight + (pitchAtZero + scaleX * (pitchAt2SmoothDelay - pitchAtZero) - avgPitch) * zoomFactorYx100;
              } else { y = 0; }
            //} else if (offset > endOfNote) {
            //  y = halfHeight + (pitchLookupUsed.at(endOfNote) - avgPitch) * zoomFactorYx100;
            } else {
              offset = std::min(offset, pitchLookupUsedSizeLimit);
              y = halfHeight + (pitchLookupUsed.at(offset) - avgPitch) * zoomFactorYx100;
            }

            y += offsetY;  // Vertical scrollbar offset
            vibratoPolyline << QPointF(x, height() - y);
          }
        }
      } else {  // No detailed pitch information available, calculate polyline using the chunkdata
        float x, y;
        for (int chunk = myStartChunk; chunk < myEndChunk; chunk++) {
          x = (chunk - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((active->dataAtChunk(chunk)->pitch - avgPitch) * zoomFactorYx100) + offsetY;
          vibratoPolyline << QPointF(x, height() - y);
        }
      }

      // Calculate a light grey band indicating which time is being used in the current window
      currentWindowBand.clear();

      const double halfWindowTime = (double)active->size() / (double)(active->rate() * 2);
      int pixelLeft = toInt((active->chunkAtTime(gdata->view->currentTime() - halfWindowTime) - myStartChunk) * zoomFactorX - windowOffset);
      int pixelRight = toInt((active->chunkAtTime(gdata->view->currentTime() + halfWindowTime) - myStartChunk) * zoomFactorX - windowOffset);
      currentWindowBand << QRectF(QPointF(pixelLeft, 0), QPointF(pixelRight, height()));

      // Calculate the current timeline
      const float timeLineX = toInt((myCurrentChunk - myStartChunk) * zoomFactorX - windowOffset);
      currentTimeLine = QLineF(QPointF(timeLineX, 0.0), QPointF(timeLineX, height()));

      // Calculate the points of maxima and minima
      maximaPoints.clear();
      minimaPoints.clear();
      // Calculate the maxima
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        float x, y;
        for (int i = 0; i < maximaSize; i++) {
          x = ((((float)note->maxima->at(i) - smoothDelay)/ framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((pitchLookupUsed.at(note->maxima->at(i)) - avgPitch) * zoomFactorYx100) + offsetY;
          maximaPoints << QRectF(QPointF(x - 1, height() - (y - 1)), QPointF(x + 1, height() - (y + 1)));
        }
      }

      // Calculate the minima
      if ((active->doingDetailedPitch()) && (active->pitchLookupSmoothed.size() > 0)) {
        float x, y;
        for (int i = 0; i < minimaSize; i++) {
          x = ((((float)note->minima->at(i) - smoothDelay) / framesPerChunk) - myStartChunk) * zoomFactorX - windowOffset;
          if (x < noteLabelOffset) { continue; }
          if (x > width() - noteLabelOffset) { break; }
          y = halfHeight + ((pitchLookupUsed.at(note->minima->at(i)) - avgPitch) * zoomFactorYx100) + offsetY;
          minimaPoints << QRectF(QPointF(x - 1, height() - (y - 1)), QPointF(x + 1, height() - (y + 1)));
        }
      }
    }
  }
  update();
}

void VibratoWidget::setZoomFactorX(double x)
{
  zoomFactorX = 2*pow10(log2(x/25));
  update();
}

void VibratoWidget::setZoomFactorY(double y)
{
  zoomFactorY = y;
  update();
}

void VibratoWidget::setOffsetY(int value)
{
  offsetY = value;
  update();
}
