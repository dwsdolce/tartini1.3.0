/***************************************************************************
              freqwidget.cpp  -  description
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

#include <QCoreApplication>
#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include "shader.h"

// zoom cursors
#include "pics/zoomx.xpm"
#include "pics/zoomy.xpm"
#include "pics/zoomxout.xpm"
#include "pics/zoomyout.xpm"

#include "freqwidget.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "useful.h"
#include "myqt.h"
#include "array1d.h"
#include "musicnotes.h"

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

FreqWidget::FreqWidget(QWidget* parent) : DrawWidget(parent)
{
  setMouseTracking(true);

  dragMode = DragNone;
  mouseX = 0; mouseY = 0;
  downTime = 0.0; downNote = 0.0;

  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setSizePolicy(sizePolicy);

  setFocusPolicy(Qt::StrongFocus);
  gdata->view->setPixelHeight(height());
}

void FreqWidget::resizeEvent(QResizeEvent* q)
{
  if (q->size() == q->oldSize()) return;

  View* v = gdata->view;

  double oldViewWidth = double(v->viewWidth());

  v->setPixelHeight(height());
  v->setPixelWidth(width());
  //printf("resizeEvent width() = %d\n", width());

  // Work out what the times/heights of the view should be based on the zoom factors
  float newYHeight = height() * v->zoomY();
  float newYBottom = v->viewBottom() - ((newYHeight - v->viewHeight()) / 2.0);

  //printf("1 viewOffset() = %d\n", v->viewOffset());
  v->setViewOffset(v->viewOffset() / oldViewWidth * v->viewWidth());
  v->setViewBottom(newYBottom);
}

void FreqWidget::drawChannelFilled(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
{
  viewBottom += gdata->semitoneOffset();
  ZoomLookup* z;
  if (viewType == DRAW_VIEW_SUMMARY) z = &ch->summaryZoomLookup;
  else z = &ch->normalZoomLookup;

  ChannelLocker channelLocker(ch);

  int viewBottomOffset = toInt(viewBottom / zoomY);
  viewBottom = double(viewBottomOffset) * zoomY;

  // baseX is the no. of chunks a pixel must represent.
  double baseX = zoomX / ch->timePerChunk();

  z->setZoomLevel(baseX);

  double currentChunk = ch->chunkFractionAtTime(currentTime);
  double leftFrameTime = currentChunk - ((currentTime - leftTime) / ch->timePerChunk());

  double frameTime = leftFrameTime;
  int n = 0;
  int baseElement = int(floor(frameTime / baseX));
  if (baseElement < 0) {
    n -= baseElement; baseElement = 0;
  }
  int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));

  int firstN = n;
  int lastN = firstN;

  QPolygonF bottomPoints;
  bool isNoteRectEven;

  if (baseX > 1) { // More samples than pixels
    QVector<QLineF> evenMidPoints;
    QVector<QLineF> oddMidPoints;
    QVector<QLineF> evenMidPoints2;
    QVector<QLineF> oddMidPoints2;

    int theWidth = width();
    if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
    bottomPoints << QPointF(firstN, height());

    for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement& ze = z->at(baseElement);
      if (!ze.isValid()) {
        if (!calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }

      int y = height() - (1 + toInt(ze.high() / zoomY) - viewBottomOffset);
      int y2, y3;
      if (ze.noteIndex() != -1 && ch->dataAtChunk(ze.midChunk())->noteIndex != -1) {
        myassert(ze.noteIndex() >= 0);
        myassert(ze.noteIndex() < int(ch->noteData.size()));
        myassert(ch->isValidChunk(ze.midChunk()));
        AnalysisData* data = ch->dataAtChunk(ze.midChunk());

        if (gdata->showMeanVarianceBars()) {
            //longTermMean bars
          y2 = height() - (1 + toInt((data->longTermMean + data->longTermDeviation) / zoomY) - viewBottomOffset);
          y3 = height() - (1 + toInt((data->longTermMean - data->longTermDeviation) / zoomY) - viewBottomOffset);
          if (ze.noteIndex() % 2 == 0) {
            evenMidPoints << QLineF(QPointF(n, y2), QPointF(n, y3));
          } else {
            oddMidPoints << QLineF(QPointF(n, y2), QPointF(n, y3));
          }

          //shortTermMean bars
          y2 = height() - (1 + toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) - viewBottomOffset);
          y3 = height() - (1 + toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) - viewBottomOffset);
          if (ze.noteIndex() % 2 == 0) {
            evenMidPoints2 << QLineF(QPointF(n, y2), QPointF(n, y3));
          } else {
            oddMidPoints2 << QLineF(QPointF(n, y2), QPointF(n, y3));
          }
        }
      }
      bottomPoints << QPointF(n, y);
      lastN = n;
    }
    bottomPoints << QPointF(lastN, height());

    QRectF rect = QRectF(QPointF(firstN, 0), QPointF(lastN, height()));
    p.setPen(gdata->shading1Color());
    p.setBrush(gdata->shading1Color());
    p.drawRect(rect);

    if (bottomPoints.count() > 1) {
      p.setPen(gdata->shading2Color());
      p.setBrush(gdata->shading2Color());
      p.drawPolygon(bottomPoints);
    }

    if (gdata->showMeanVarianceBars()) {
      QPen yellowPen = QPen(Qt::yellow);
      yellowPen.setWidth(3.0);
      QPen greenPen(Qt::green);
      greenPen.setWidth(3.0);

      //shortTermMean bars
      if (evenMidPoints2.count() > 1) {
        p.setPen(greenPen);
        p.drawLines(evenMidPoints2);
      }
      if (oddMidPoints2.count() > 1) {
        p.setPen(yellowPen);
        p.drawLines(oddMidPoints2);
      }

      //longTermMean bars

      if (evenMidPoints.count() > 1) {
        p.setPen(yellowPen);
        p.drawLines(evenMidPoints);
      }

      if (oddMidPoints.count() > 1) {
        p.setPen(greenPen);
        p.drawLines(oddMidPoints);
      }
    }
  } else { // More pixels than samples
    QVector<QRectF> evenMidPoints;
    QVector<QRectF> oddMidPoints;
    QVector<QRectF> evenMidPoints2;
    QVector<QRectF> oddMidPoints2;

    float pitch = 0.0, prevPitch = 0.0;
    int intChunk = (int)floor(frameTime); // Integer version of frame time
    if (intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y, y2, y3, x2;

    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = width() + (2 * stepSize);
    lastN = firstN = toInt(start);
    bottomPoints << QPointF(firstN, height());
    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData* data = ch->dataAtChunk(intChunk);

      x = toInt(n);
      lastN = x;
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;

      if (data->noteIndex >= 0) {
        isNoteRectEven = ((data->noteIndex % 2) == 0);

        if (gdata->showMeanVarianceBars()) {
          x2 = toInt(n + stepSize);

          //longTermMean bars
          y2 = height() - (1 + toInt((data->longTermMean + data->longTermDeviation) / zoomY) - viewBottomOffset);
          y3 = height() - (1 + toInt((data->longTermMean - data->longTermDeviation) / zoomY) - viewBottomOffset);

          // Since Quads are no long supported define 2 triangles
          if ((data->noteIndex % 2) == 0) {
            evenMidPoints << QRectF(QPointF(x, y2), QPointF(x2, y3));
          } else {
            oddMidPoints << QRectF(QPointF(x, y2), QPointF(x2, y3));
          }

          //shortTermMean bars
          y2 = height() - (1 + toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) - viewBottomOffset);
          y3 = height() - (1 + toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) - viewBottomOffset);
          if ((data->noteIndex % 2) == 0) {
            evenMidPoints2 << QRectF(QPointF(x, y2), QPointF(x2, y3));
          } else {
            oddMidPoints2 << QRectF(QPointF(x, y2), QPointF(x2, y3));
          }
        }
      }

      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      y = height() - (1 + toInt(pitch / zoomY) - viewBottomOffset);
      bottomPoints << QPointF(x, y);
      prevPitch = pitch;
    }

    bottomPoints << QPointF(lastN, height());

    QRectF rect = QRectF(QPointF(firstN, 0), QPointF(lastN, height()));
    p.setPen(gdata->shading1Color());
    p.setBrush(gdata->shading1Color());
    p.drawRect(rect);

    if (bottomPoints.count() > 1) {
      p.setPen(gdata->shading2Color());
      p.setBrush(gdata->shading2Color());
      p.drawPolygon(bottomPoints);
    }

    if (gdata->showMeanVarianceBars()) {
       //shortTermMean bars
      if (evenMidPoints2.count() > 1) {
        p.setPen(Qt::green);
        p.setBrush(Qt::green);
        p.drawRects(evenMidPoints2);
      }
      if (oddMidPoints2.count() > 1) {
        p.setPen(Qt::yellow);
        p.setBrush(Qt::yellow);
        p.drawRects(oddMidPoints2);
      }

      //longTermMean bars
      QColor seeThroughYellow = Qt::yellow;
      seeThroughYellow.setAlpha(255);
      QColor seeThroughGreen = Qt::green;
      seeThroughGreen.setAlpha(255);

      if (evenMidPoints.count() > 1) {
        p.setPen(seeThroughYellow);
        p.setBrush(seeThroughYellow);
        p.drawRects(evenMidPoints);
      }
      if (oddMidPoints.count() > 1) {
        p.setPen(seeThroughGreen);
        p.setBrush(seeThroughGreen);
        p.drawRects(oddMidPoints);
      }
    }
  }
}

void FreqWidget::drawReferenceLines(double viewBottom, double zoomY, int /*viewType*/)
{
  // Draw the lines and notes
  QFontMetrics fm = fontMetrics();
  int fontHeightSpace = fm.height() / 4;
  int fontWidth = fm.horizontalAdvance("C#0") + 3;

  MusicKey& musicKey = gMusicKeys[gdata->temperedType()];
  MusicScale& musicScale = gMusicScales[gdata->musicKeyType()];

  int keyRoot = cycle(gMusicKeyRoot[gdata->musicKey()] + musicScale.semitoneOffset(), 12);
  int viewBottomNote = (int)viewBottom - keyRoot;
  int remainder = cycle(viewBottomNote, 12);
  int lowRoot = viewBottomNote - remainder + keyRoot;
  int rootOctave = lowRoot / 12;
  int rootOffset = cycle(lowRoot, 12);
  int numOctaves = int(ceil(zoomY * (double)height() / 12.0)) + 1;
  int topOctave = rootOctave + numOctaves;
  double lineY;
  double curRoot;
  double curPitch;
  QString noteLabel;
  int nameIndex;

  QVector<QLineF> refRoot;
  QVector<QLineF> refAccidental;
  QVector<QLineF> refNote;

  //Draw the horizontal reference lines
  for (int octave = rootOctave; octave < topOctave; ++octave) {
    curRoot = double(octave * 12 + rootOffset);
    for (int j = 0; j < musicKey.size(); j++) {
      if (musicScale.hasSemitone(musicKey.noteType(j))) { //draw it
        curPitch = curRoot + musicKey.noteOffset(j);
        lineY = height() - double(myround((curPitch - viewBottom) / zoomY));
        if (j == 0) { //root note
          refRoot << QLineF(fontWidth, lineY, width() - 1, lineY);
        } else if ((gdata->musicKeyType() == MusicScale::Chromatic) && !gMusicScales[MusicScale::Major].hasSemitone(musicKey.noteType(j))) {
          refAccidental << QLine(fontWidth, lineY, width() - 1, lineY);
        } else {
          refNote << QLine(fontWidth, lineY, width() - 1, lineY);
        }
      }
      if (zoomY > 0.1) break;
    }
  }


  // Draw the lines for the root of the octave
  p.setPen(Qt::black);
  p.drawLines(refRoot);

  // Draw the lines for the accidentals
  QPen pen = p.pen();
  QPen oldPen = pen;
  QVector<qreal> dashes;
  dashes << 6 << 2;
  pen.setDashPattern(dashes);
  pen.setCapStyle(Qt::FlatCap);
  pen.setColor(QColor(25, 125, 170, 128));
  p.setPen(pen);
  p.drawLines(refAccidental);

  // Draw the lines for the notes
  dashes.clear();
  dashes << 14 << 2;
  pen.setDashPattern(dashes);
  pen.setColor(QColor(25, 125, 170, 196));
  p.setPen(pen);
  p.drawLines(refNote);

  pen.setColor(Qt::black);
  p.setPen(pen);

  //Draw the text labels down the left hand side
  for (int octave = rootOctave; octave < topOctave; ++octave) {
    curRoot = double(octave * 12 + rootOffset);
    for (int j = 0; j < musicKey.size(); j++) {
      if (musicScale.hasSemitone(musicKey.noteType(j))) { //draw it
        curPitch = curRoot + musicKey.noteOffset(j);
        lineY = double(height()) - myround((curPitch - viewBottom) / zoomY);
        nameIndex = toInt(curPitch);

        noteLabel.asprintf("%s%d", noteName(nameIndex), noteOctave(nameIndex));
        p.drawText(2, toInt(lineY) + fontHeightSpace, noteLabel);
      }
      if (zoomY > 0.1) break;
    }
  }
  // Restore pen
  p.setPen(oldPen);

  //Draw the vertical reference lines
  double timeStep = timeWidth() / double(width()) * 150.0; //time per 150 pixels
  double timeScaleBase = pow10(floor(log10(timeStep))); //round down to the nearest power of 10

  //choose a timeScaleStep which is a multiple of 1, 2 or 5 of timeScaleBase
  int largeFreq;
  if (timeScaleBase * 5.0 < timeStep) {
    largeFreq = 5;
  } else if (timeScaleBase * 2.0 < timeStep) {
    largeFreq = 2;
  } else {
    largeFreq = 2; timeScaleBase /= 2;
  }

  double timePos = floor(leftTime() / (timeScaleBase * largeFreq)) * (timeScaleBase * largeFreq); //calc the first one just off the left of the screen
  int x, largeCounter = -1;
  double ratio = double(width()) / timeWidth();
  double lTime = leftTime();

  QVector<QLineF> refVertDark;
  QVector<QLineF> refVertLight;

  for (; timePos <= rightTime(); timePos += timeScaleBase) {
    x = toInt((timePos - lTime) * ratio);
    if (++largeCounter == largeFreq) {
      largeCounter = 0;
      refVertDark << QLineF(QPointF(x, 0), QPointF(x, height() - 1));
    } else {
      refVertLight << QLineF(QPointF(x, 0), QPointF(x, height() - 1));
    }
  }
  p.setPen(Qt::black);
  p.drawLines(refVertDark);

  p.setPen(QColor(25, 125, 170, 128));
  p.drawLines(refVertLight);
}

void FreqWidget::drawChannel(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
{
  viewBottom += gdata->semitoneOffset();
  float lineWidth = 3.0f;
  float lineHalfWidth = lineWidth / 2;
  ZoomLookup* z;
  if (viewType == DRAW_VIEW_SUMMARY) z = &ch->summaryZoomLookup;
  else z = &ch->normalZoomLookup;

  ChannelLocker channelLocker(ch);

  int viewBottomOffset = toInt(viewBottom / zoomY);
  viewBottom = double(viewBottomOffset) * zoomY;

  // baseX is the no. of chunks a pixel must represent.
  double baseX = zoomX / ch->timePerChunk();

  z->setZoomLevel(baseX);

  double currentChunk = ch->chunkFractionAtTime(currentTime);
  double leftFrameTime = currentChunk - ((currentTime - leftTime) / ch->timePerChunk());

  double frameTime = leftFrameTime;
  int n = 0;
  int baseElement = int(floor(frameTime / baseX));
  if (baseElement < 0) {
    n -= baseElement; baseElement = 0;
  }
  int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));

  QColor lineColor;

  if (baseX > 1) { // More samples than pixels
    int theWidth = width();
    if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
    for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement& ze = z->at(baseElement);
      if (!ze.isValid()) {
        if (!calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }

      if (ze.high() != 0.0f && ze.high() - ze.low() < 1.0) {
        //if range is closer than one semi-tone then draw a line between them
        QPen linePen(ze.color());
        linePen.setWidth(3.0);
        p.setPen(linePen);
        p.drawLine(n, height() - ((ze.low() / zoomY) - lineHalfWidth - viewBottomOffset),
                   n, height() - ((ze.high() / zoomY) + lineHalfWidth - viewBottomOffset));
      }
    }
  } else { // More pixels than samples
    float err = 0.0, pitch = 0.0, prevPitch = 0.0, vol;
    int intChunk = (int)floor(frameTime); // Integer version of frame time
    if (intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y;

    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = width() + (2 * stepSize);
    int squareSize = (int(sqrt(stepSize)) / 2) * 2 + 1; //make it an odd number
    int halfSquareSize = squareSize / 2;
    halfSquareSize = 4;
    int penX = 0, penY = 0;

    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData* data = ch->dataAtChunk(intChunk);
      err = data->correlation();
      vol = dB2Normalised(data->logrms(), ch->rmsCeiling, ch->rmsFloor);

      QColor lineColor;
      if (gdata->pitchContourMode() == 0) {
        if (viewType == DRAW_VIEW_PRINT) {
          lineColor = colorBetween(QColor(255, 255, 255), ch->color, err * vol);
        } else {
          lineColor = colorBetween(gdata->backgroundColor(), ch->color, err * vol);
        }
      } else {
        lineColor = ch->color;
      }

      x = toInt(n);
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;
      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      y = height() - (1 + toInt(pitch / zoomY) - viewBottomOffset);
      if (pitch > 0.0f) {
        if (fabs(prevPitch - pitch) < 1.0 && n != start) {
          //if closer than one semi-tone from previous then draw a line between them
          QPen linePen(lineColor);
          linePen.setWidth(3.0);
          p.setPen(linePen);
          p.drawLine(penX, penY, x, y);
          penX = x; penY = y;
        } else {
          penX = x; penY = y;
        }
        if (stepSize > 10) { //draw squares on the data points
          p.setPen(lineColor);
          p.setBrush(lineColor);
          p.drawRect(QRectF(QPointF(x - halfSquareSize, y - halfSquareSize), QPointF(x + halfSquareSize, y + halfSquareSize)));
        }
      }
      prevPitch = pitch;
    }
  }
}


void FreqWidget::paintEvent(QPaintEvent*)
{
  beginDrawing();

  View* view = gdata->view;

  if (view->autoFollow() && gdata->getActiveChannel() && gdata->running == STREAM_FORWARD) {
    setChannelVerticalView(gdata->getActiveChannel(), view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY());
  }

  //draw the red/blue background color shading if needed
  if (view->backgroundShading() && gdata->getActiveChannel()) {
    drawChannelFilled(gdata->getActiveChannel(), view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);
  }

  drawReferenceLines(view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);

  //draw all the visible channels
  for (uint i = 0; i < gdata->channels.size(); i++) {
    Channel* ch = gdata->channels.at(i);
    if (!ch->isVisible()) continue;
    drawChannel(ch, view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);
  }

  // Draw a light grey band indicating which time is being used in the current window
  if (gdata->getActiveChannel()) {
    QColor lineColor = palette().color(QPalette::WindowText);
    lineColor.setAlpha(50);
    Channel* ch = gdata->getActiveChannel();
    double halfWindowTime = (double)ch->size() / (double)(ch->rate() * 2);
    int pixelLeft = view->screenPixelX(view->currentTime() - halfWindowTime);
    int pixelRight = view->screenPixelX(view->currentTime() + halfWindowTime);

    p.setPen(lineColor);
    p.setBrush(lineColor);
    p.drawRect(QRectF(QPointF(pixelLeft, 0.0), QPointF(pixelRight, height() - 1.0)));
  }

  // Draw the current time line
  //int curTimePixel = view->screenPixelX(view->currentTime());
  double curScreenTime = (view->currentTime() - view->viewLeft()) / view->zoomX();

  p.setPen(palette().color(QPalette::WindowText));
  p.drawLine(curScreenTime, 0.0, curScreenTime, height() - 1.0);

  endDrawing();
  }

Channel* FreqWidget::channelAtPixel(int x, int y)
{
  double time = mouseTime(x);
  float pitch = mousePitch(y);
  float tolerance = 6 * gdata->view->zoomY(); //10 pixel tolerance

  std::vector<Channel*> channels;

  //loop through channels in reverse order finding which one the user clicked on
  for (std::vector<Channel*>::reverse_iterator it = gdata->channels.rbegin(); it != gdata->channels.rend(); it++) {
    if ((*it)->isVisible()) {
      AnalysisData* data = (*it)->dataAtTime(time);
      if (data && within(tolerance, data->pitch, pitch)) return *it;
    }
  }
  return NULL;
}

/*
 * If control or alt is pressed, zooms. If shift is also pressed, it 'reverses' the zoom: ie ctrl+shift zooms x
 * out, alt+shift zooms y out. Otherwise, does internal processing.
 *
 * @param e the QMouseEvent to respond to.
 */
void FreqWidget::mousePressEvent(QMouseEvent* e)
{
  View* view = gdata->view;
  int timeX = toInt(view->viewOffset() / view->zoomX());
  bool zoomed = false;
  dragMode = DragNone;


  //Check if user clicked on center bar, to drag it
  if (within(4, e->x(), timeX)) {
    dragMode = DragTimeBar;
    mouseX = e->x();
    return;
  }

  // If the control or alt keys are pressed, zoom in or out on the correct axis, otherwise scroll.
  if (e->modifiers() & Qt::ControlModifier) {
    // Do we zoom in or out?
    if (e->modifiers() & Qt::ShiftModifier) {
      //view->viewZoomOutX();
    } else {
      //view->viewZoomInX();
    }
    zoomed = true;
  } else if (e->modifiers() & Qt::AltModifier) {
     // Do we zoom in or out?
    if (e->modifiers() & Qt::ShiftModifier) {
      //view->viewZoomOutY();
    } else {
      //view->viewZoomInY();
    }
    zoomed = true;
  } else {
    //mouseDown = true;
    mouseX = e->x();
    mouseY = e->y();

    Channel* ch = channelAtPixel(e->x(), e->y());
    if (ch) { //Clicked on a channel
      gdata->setActiveChannel(ch);
      dragMode = DragChannel;
    } else {
       //Must have clicked on background
      dragMode = DragBackground;
      downTime = view->currentTime();
      downNote = view->viewBottom();
    }
  }
}

void FreqWidget::mouseMoveEvent(QMouseEvent* e)
{
  View* view = gdata->view;
  int pixelAtCurrentTimeX = toInt(view->viewOffset() / view->zoomX());

  switch (dragMode) {
    case DragTimeBar:
    {
      int newX = pixelAtCurrentTimeX + (e->x() - mouseX);
      view->setViewOffset(double(newX) * view->zoomX());
      mouseX = e->x();
      view->doSlowUpdate();
    }
    break;
    case DragBackground:
      view->setViewBottom(downNote - (mouseY - e->y()) * view->zoomY());
      gdata->updateActiveChunkTime(downTime - (e->x() - mouseX) * view->zoomX());
      view->doSlowUpdate();
      break;
    case DragNone:
      if (within(4, e->x(), pixelAtCurrentTimeX)) {
        setCursor(QCursor(Qt::SplitHCursor));
      } else if (channelAtPixel(e->x(), e->y()) != NULL) {
        setCursor(QCursor(Qt::PointingHandCursor));
      } else {
        unsetCursor();
      }
  }
}

void FreqWidget::mouseReleaseEvent(QMouseEvent*)
{
  dragMode = DragNone;
}

/**
 Calculates at what time the mouse is.
 @param x the mouse's x co-ordinate
 @return the time the mouse is positioned at.
 */
double FreqWidget::mouseTime(int x)
{
  return gdata->view->viewLeft() + gdata->view->zoomX() * x;

}

/**
 Calculates at what note pitch the mouse is at.
 @param x the mouse's y co-ordinate
 @return the pitch the mouse is positioned at.
 */
double FreqWidget::mousePitch(int y)
{
  return gdata->view->viewBottom() + gdata->view->zoomY() * (height() - y);
}

void FreqWidget::wheelEvent(QWheelEvent* e)
{
  View* view = gdata->view;
  double amount = double(e->angleDelta().y()) / WHEEL_DELTA * 0.15;
  bool isZoom = gdata->mouseWheelZooms();
  if (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) isZoom = !isZoom;

  if (isZoom) {
    if (e->angleDelta().y() >= 0) { //zooming in
      double before = view->logZoomY();
      view->setZoomFactorY(view->logZoomY() + amount, height() - e->position().y());
      amount = view->logZoomY() - before;
      if (gdata->running == STREAM_FORWARD) {
        view->setZoomFactorX(view->logZoomX() + amount);
      } else { //zoom toward mouse pointer
        view->setZoomFactorX(view->logZoomX() + amount, e->position().x());
      }
    } else { //zoom out toward center
      double before = view->logZoomY();
      view->setZoomFactorY(view->logZoomY() + amount, height() / 2);
      amount = view->logZoomY() - before;
      if (gdata->running == STREAM_FORWARD) {
        view->setZoomFactorX(view->logZoomX() + amount);
      } else {
        view->setZoomFactorX(view->logZoomX() + amount, width() / 2);
      }
    }
  } else { //mouse wheel scrolls
    view->setViewBottom(view->viewBottom() + amount * 0.75 * view->viewHeight());
  }
  view->doSlowUpdate();

  e->accept();
}

/*
 * Changes the cursor icon to be one of the zoom ones depending on if the control or alt keys were pressed.
 * Otherwise, ignores the event.
 *
 * @param k the QKeyEvent to respond to.
 */
void FreqWidget::keyPressEvent(QKeyEvent* k)
{
  switch (k->key()) {
    case Qt::Key_Control:
      setCursor(QCursor(QPixmap(zoomx)));
      break;
    case Qt::Key_Alt:
      setCursor(QCursor(QPixmap(zoomy)));
      break;
    case Qt::Key_Shift:
      if (k->modifiers() & Qt::ControlModifier) {
        setCursor(QCursor(QPixmap(zoomxout)));
      } else if (k->modifiers() & Qt::AltModifier) {
        setCursor(QCursor(QPixmap(zoomyout)));
      } else {
        k->ignore();
      }
      break;
    default:
      k->ignore();
      break;
  }

}

/*
 * Unsets the cursor icon if the control or alt key was released. Otherwise, ignores the event.
 *
 * @param k the QKeyEvent to respond to.
 */
void FreqWidget::keyReleaseEvent(QKeyEvent* k)
{
  switch (k->key()) {
    case Qt::Key_Control: // Unset the cursor if the control or alt keys were released, ignore otherwise
    case Qt::Key_Alt:
      unsetCursor();
      break;
    case Qt::Key_Shift:
      if (k->modifiers() & Qt::ControlModifier) {
        setCursor(QCursor(QPixmap(zoomx)));
      } else if (k->modifiers() & Qt::AltModifier) {
        setCursor(QCursor(QPixmap(zoomy)));
      } else {
        k->ignore();
      }
      break;
    default:
      k->ignore();
      break;
  }
}

void FreqWidget::leaveEvent(QEvent* e)
{
  unsetCursor();
  QWidget::leaveEvent(e);
}

/** calculates elements in the zoom lookup table
  @param ch The channel we are working with
  @param baseElement The element's index in the zoom lookup table
  @param baseX  The number of chunks each pixel represents (can include a fraction part)
  @return false if a zoomElement can't be calculated, else true
*/
bool FreqWidget::calcZoomElement(Channel* ch, ZoomElement& ze, int baseElement, double baseX)
{
  int startChunk = toInt(double(baseElement) * baseX);
  int finishChunk = toInt(double(baseElement + 1) * baseX) + 1;
  if (finishChunk >= (int)ch->totalChunks()) finishChunk--; //dont go off the end
  if (finishChunk >= (int)ch->totalChunks()) return false; //that data doesn't exist yet

  std::pair<large_vector<AnalysisData>::iterator, large_vector<AnalysisData>::iterator> a =
    minMaxElement(ch->dataIteratorAtChunk(startChunk), ch->dataIteratorAtChunk(finishChunk), lessPitch());
  if (a.second == ch->dataIteratorAtChunk(finishChunk)) return false;

  large_vector<AnalysisData>::iterator err = std::max_element(ch->dataIteratorAtChunk(startChunk), ch->dataIteratorAtChunk(finishChunk), lessValue(0));
  if (err == ch->dataIteratorAtChunk(finishChunk)) return false;

  float low, high;
  int noteIndex;
  if (ch->isVisibleChunk(&*err)) {
    low = a.first->pitch;
    high = a.second->pitch;
    noteIndex = a.first->noteIndex;
  } else {
    low = 0;
    high = 0;
    noteIndex = NO_NOTE;
  }
  float corr = err->correlation() * dB2Normalised(err->logrms(), ch->rmsCeiling, ch->rmsFloor);
  QColor theColor = (gdata->pitchContourMode() == 0) ? colorBetween(gdata->backgroundColor(), ch->color, corr) : ch->color;

  ze.set(low, high, corr, theColor, noteIndex, (startChunk + finishChunk) / 2);
  return true;
}

void FreqWidget::setChannelVerticalView(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY)
{
  ZoomLookup* z = &ch->normalZoomLookup;

  ChannelLocker channelLocker(ch);

  int viewBottomOffset = toInt(viewBottom / zoomY);
  viewBottom = double(viewBottomOffset) * zoomY;

  std::vector<float> ys;
  ys.reserve(width());
  std::vector<float> weightings;
  weightings.reserve(width());
  float maxY = 0.0f, minY = gdata->topPitch();
  float totalY = 0.0f;
  float numY = 0.0f;

  // baseX is the no. of chunks a pixel must represent.
  double baseX = zoomX / ch->timePerChunk();

  z->setZoomLevel(baseX);

  double currentChunk = ch->chunkFractionAtTime(currentTime);
  double leftFrameTime = currentChunk - ((currentTime - leftTime) / ch->timePerChunk());

  double frameTime = leftFrameTime;
  int n = 0;
  int currentBaseElement = int(floor(currentChunk / baseX));
  int firstBaseElement = int(floor(frameTime / baseX));
  int baseElement = firstBaseElement;
  if (baseElement < 0) {
    n -= baseElement; baseElement = 0;
  }
  int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));
  double leftBaseWidth = MAX(1.0, double(currentBaseElement - firstBaseElement));
  double rightBaseWidth = MAX(1.0, double(lastBaseElement - currentBaseElement));

  /*
  We calculate the auto follow and scale by averaging all the note elements in view.
  We weight the frequency averaging by a triangle weighting function centered on the current time.
  Also it is weighted by the corr value of each frame.
         /|\
        / | \
       /  |  \
      /   |   \
        ^   ^
   leftBaseWidth rightBaseWidth
  */

  int firstN = n;
  int lastN = firstN;

  if (baseX > 1) { // More samples than pixels
    int theWidth = width();
    if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
    for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement& ze = z->at(baseElement);
      if (!ze.isValid()) {
        if (!calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }
      if (ze.low() > 0.0f && ze.high() > 0.0f) {
        float weight = ze.corr();
        if (baseElement < currentBaseElement) weight *= double(currentBaseElement - baseElement) / leftBaseWidth;
        else if (baseElement > currentBaseElement) weight *= double(baseElement - currentBaseElement) / rightBaseWidth;
        if (ze.low() < minY) minY = ze.low();
        if (ze.high() > maxY) maxY = ze.high();
        totalY += (ze.low() + ze.high()) / 2.0f * weight;
        numY += weight;
        ys.push_back((ze.low() + ze.high()) / 2.0f);
        weightings.push_back(weight);
      }
      lastN = n;
    }
  } else { // More pixels than samples
    float err = 0.0, pitch = 0.0, prevPitch = 0.0;
    int intChunk = (int)floor(frameTime); // Integer version of frame time
    if (intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y;
    float corr;

    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = width() + (2 * stepSize);
    lastN = firstN = toInt(start);
    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData* data = ch->dataAtChunk(intChunk);
      err = data->correlation();

      x = toInt(n);
      lastN = x;
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;
      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      corr = data->correlation() * dB2ViewVal(data->logrms());
      if (pitch > 0.0f) {
        float weight = corr;
        if (minY < pitch) minY = pitch;
        if (maxY > pitch) maxY = pitch;
        totalY += pitch * weight;
        numY += weight;
        ys.push_back(pitch);
        weightings.push_back(weight);
      }
      y = height() - 1 - toInt(pitch / zoomY) + viewBottomOffset;
      prevPitch = pitch;
    }
  }

  if (!ys.empty()) {
    float meanY = totalY / numY;
    double spred = 0.0;
    myassert(ys.size() == weightings.size());
    //use a linear spred function. not a squared one like standard deviation
    for (uint j = 0; j < ys.size(); j++) {
      spred += sq(ys[j] - meanY) * weightings[j];
    }
    spred = sqrt(spred / numY) * 4.0;
    //printf("spred = %f\n", spred);
    if (spred < 12.0) {
      //show a minimum of 12 semi-tones
      spred = 12.0;
    }
    gdata->view->setViewBottomRaw(meanY - gdata->view->viewHeight() / 2.0);
  }
}
