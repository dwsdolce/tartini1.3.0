/***************************************************************************
                          drawwidget.cpp  -  description
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
 
#include <qthread.h>
#include <QPolygon>
#include <utility>
#include <algorithm>
#include <qpixmap.h>

#include "drawwidget.h"
#include "channel.h"
#include "view.h"
#include "gdata.h"
#include "analysisdata.h"
#include "useful.h"
#include "conversions.h"
#include "myqt.h"

int DrawWidget::lineWidth = 3;
int DrawWidget::lineTopHalfWidth = 2;
int DrawWidget::lineBottomHalfWidth = 1;

DrawWidget::DrawWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  paintDevice = this;
  setLineWidth(3);
  
  setAttribute(Qt::WA_OpaquePaintEvent);
}

DrawWidget::~DrawWidget()
{
}

void DrawWidget::beginDrawing(bool clearBackground_)
{
  p.begin(paintDevice);

  if(clearBackground_) clearBackground();
}

void DrawWidget::endDrawing()
{
  p.end();
}

void DrawWidget::clearBackground()
{
  p.fillRect(0, 0, width(), height(), gdata->backgroundColor());
}

void DrawWidget::fillBackground(const QColor &color)
{
  p.fillRect(0, 0, width(), height(), color);
}

void DrawWidget::setLineWidth(int width)
{
  lineWidth = width;
  lineTopHalfWidth = lineWidth/2 + (lineWidth % 2);
  lineBottomHalfWidth = lineWidth/2;
}

/** Gives a color between a and b, using ratio (a number between 0 and 1)
*/
/*
QColor DrawWidget::colorBetween(QColor a, QColor b, double ratio)
{
  //myassert(ratio >= 0.0 && ratio <= 1.0);
  ratio = MAX(MIN(ratio, 1.0), 0.0);
  return QColor(
    a.red()+int(ratio*float(b.red()-a.red())),
    a.green()+int(ratio*float(b.green()-a.green())),
    a.blue()+int(ratio*float(b.blue()-a.blue())));
}
*/

void DrawWidget::drawChannel(QPaintDevice &pd, Channel *ch, QPainter &p, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
{
  ZoomLookup *z;
  if(viewType == DRAW_VIEW_SUMMARY) z = &ch->summaryZoomLookup;
  else z = &ch->normalZoomLookup;

  ChannelLocker channelLocker(ch);

  QColor current = ch->color;
 	QColor invert(255 - current.red(), 255 - current.green(), 255 - current.blue());
 	p.setPen(current);

 	int viewBottomOffset = toInt(viewBottom / zoomY);
  viewBottom = double(viewBottomOffset) * zoomY;
  
  // baseX is the no. of chunks a pixel must represent.
 	double baseX = zoomX / ch->timePerChunk();

  z->setZoomLevel(baseX);
  
  double currentChunk = ch->chunkFractionAtTime(currentTime);
  double leftFrameTime = currentChunk - ((currentTime - leftTime) / ch->timePerChunk());
  
  //double leftFrameTime = leftTime / ch->timePerChunk();

 	double frameTime = leftFrameTime;
  //if(frameTime < 0.0) frameTime = 0.0;
  int n = 0;
  int baseElement = int(floor(frameTime / baseX));
  if(baseElement < 0) { n -= baseElement; baseElement = 0; }
  int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));
  
  QPolygon pointArray(pd.width()*2);
 	if (baseX > 1) { // More samples than pixels
    int theWidth = pd.width();
    if(lastBaseElement > z->size()) z->setSize(lastBaseElement);
    for(; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement &ze = z->at(baseElement);
      if(!ze.isValid()) {
        if(calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }
     
      if(ze.high() != 0.0f && ze.high() - ze.low() < 1.0) { //if range is closer than one semi-tone then draw a line between them
        p.setPen(ze.color());
        //Note: lineTo doen't draw a pixel on the last point of the line
        p.drawLine(n, pd.height() - lineTopHalfWidth - toInt(ze.high() / zoomY) + viewBottomOffset, n, pd.height() + lineBottomHalfWidth - toInt(ze.low() / zoomY) + viewBottomOffset);
      }
    }
 	} else { // More pixels than samples
    float err = 0.0, pitch = 0.0, prevPitch = 0.0, vol;
    int intChunk = (int) floor(frameTime); // Integer version of frame time
    if(intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y;
  
    //double start = 0 - stepSize;
    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = pd.width() + (2 * stepSize);
    int squareSize = (int(sqrt(stepSize)) / 2) * 2 + 1; //make it an odd number
    int halfSquareSize = squareSize/2;
    int penX=0, penY=0;
    
    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData *data = ch->dataAtChunk(intChunk);
      err = data->correlation();
      vol = dB2Normalised(data->logrms(), ch->rmsCeiling, ch->rmsFloor);
      
      if(gdata->pitchContourMode() == 0)
        if(viewType == DRAW_VIEW_PRINT)
          p.setPen(QPen(colorBetween(QColor(255, 255, 255), ch->color, err*vol), lineWidth));
        else
          p.setPen(QPen(colorBetween(gdata->backgroundColor(), ch->color, err*vol), lineWidth));
      else
        p.setPen(QPen(ch->color, lineWidth));
      
      x = toInt(n);
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;
      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      y = pd.height() - 1 - toInt(pitch / zoomY) + viewBottomOffset;
      if(pitch > 0.0f) {
        if(fabs(prevPitch - pitch) < 1.0 && n != start) { //if closer than one semi-tone from previous then draw a line between them
          p.drawLine(penX, penY, x, y);
          penX = x; penY = y;
        } else {
          p.drawPoint(x, y);
          penX = x; penY = y;
        }
        if(stepSize > 10) { //draw squares on the data points
          p.setBrush(Qt::NoBrush);
          p.drawRect(x - halfSquareSize, y - halfSquareSize, squareSize, squareSize);
        }
      }
      prevPitch = pitch;
    }
  }
}

void DrawWidget::drawChannelFilled(Channel *ch, QPainter &p, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
{
  ZoomLookup *z;
  if(viewType == DRAW_VIEW_SUMMARY) z = &ch->summaryZoomLookup;
  else z = &ch->normalZoomLookup;
    
  ChannelLocker channelLocker(ch);

  QColor current = ch->color;
  QColor invert(255 - current.red(), 255 - current.green(), 255 - current.blue());
  p.setPen(current);

  int viewBottomOffset = toInt(viewBottom / zoomY);
  viewBottom = double(viewBottomOffset) * zoomY;
  
  // baseX is the no. of chunks a pixel must represent.
  double baseX = zoomX / ch->timePerChunk();

  z->setZoomLevel(baseX);
  
  double currentChunk = ch->chunkFractionAtTime(currentTime);
  double leftFrameTime = currentChunk - ((currentTime - leftTime) / ch->timePerChunk());
  
  //double leftFrameTime = leftTime / ch->timePerChunk();

  double frameTime = leftFrameTime;
  int n = 0;
  int baseElement = int(floor(frameTime / baseX));
  if(baseElement < 0) { n -= baseElement; baseElement = 0; }
  int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));
  
  int firstN = n;
  int lastN = firstN;
  
  QPolygon bottomPoints(width()*2);
  QPolygon evenMidPoints(width()*2);
  QPolygon oddMidPoints(width()*2);
  QPolygon evenMidPoints2(width()*2);
  QPolygon oddMidPoints2(width()*2);
  std::vector<QRect> noteRect(width()*2);
  std::vector<QRect> noteRect2(width()*2);
  std::vector<bool> isNoteRectEven(width()*2);
  int pointIndex = 0;
  int evenMidPointIndex = 0;
  int oddMidPointIndex = 0;
  int evenMidPointIndex2 = 0;
  int oddMidPointIndex2 = 0;
  int rectIndex = 0;
  int rectIndex2 = 0;

  if (baseX > 1) { // More samples than pixels
    int theWidth = width();
    if(lastBaseElement > z->size()) z->setSize(lastBaseElement);
    for(; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement &ze = z->at(baseElement);
      if(!ze.isValid()) {
        if(!calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }
     
      int y = height() - 1 - toInt(ze.high() / zoomY) + viewBottomOffset;
      int y2, y3;
      if(ze.noteIndex() != -1 && ch->dataAtChunk(ze.midChunk())->noteIndex != -1) {
        myassert(ze.noteIndex() >= 0);
        myassert(ze.noteIndex() < int(ch->noteData.size()));
        myassert(ch->isValidChunk(ze.midChunk()));
        AnalysisData *data = ch->dataAtChunk(ze.midChunk());

        if(gdata->showMeanVarianceBars()) {
          //longTermMean bars
          y2 = height() - 1 - toInt((data->longTermMean + data->longTermDeviation) / zoomY) + viewBottomOffset;
          y3 = height() - 1 - toInt((data->longTermMean - data->longTermDeviation) / zoomY) + viewBottomOffset;
          if(ze.noteIndex() % 2 == 0) {
            evenMidPoints.setPoint(evenMidPointIndex++, n, y2);
            evenMidPoints.setPoint(evenMidPointIndex++, n, y3);
          } else {
            oddMidPoints.setPoint(oddMidPointIndex++, n, y2);
            oddMidPoints.setPoint(oddMidPointIndex++, n, y3);
          }
  
          //shortTermMean bars
          y2 = height() - 1 - toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) + viewBottomOffset;
          y3 = height() - 1 - toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) + viewBottomOffset;
          if(ze.noteIndex() % 2 == 0) {
            evenMidPoints2.setPoint(evenMidPointIndex2++, n, y2);
            evenMidPoints2.setPoint(evenMidPointIndex2++, n, y3);
          } else {
            oddMidPoints2.setPoint(oddMidPointIndex2++, n, y2);
            oddMidPoints2.setPoint(oddMidPointIndex2++, n, y3);
          }
        }
      }
      bottomPoints.setPoint(pointIndex++, n, y);
      bottomPoints.setPoint(pointIndex++, n, height());
      lastN = n;
    }
    p.setPen(Qt::NoPen);
    p.setBrush(gdata->shading1Color());
    p.drawRect(firstN, 0, lastN, height());
    p.setPen(gdata->shading2Color());
    if(pointIndex > 1) p.drawLines(bottomPoints.constData(), pointIndex/2);

    if(gdata->showMeanVarianceBars()) {
      p.setPen(Qt::green);
      if(evenMidPointIndex2 > 1) p.drawLines(evenMidPoints2.constData(), evenMidPointIndex2/2);
      p.setPen(Qt::yellow);
      if(oddMidPointIndex2 > 1) p.drawLines(oddMidPoints2.constData(), oddMidPointIndex2/2);

      //longTermMean bars
      p.setPen(Qt::yellow);
      if(evenMidPointIndex > 1) p.drawLines(evenMidPoints.constData(), evenMidPointIndex/2);
      p.setPen(Qt::green);
      if(oddMidPointIndex > 1) p.drawLines(oddMidPoints.constData(), oddMidPointIndex/2);
    }
  } else { // More pixels than samples
    float err = 0.0, pitch = 0.0, prevPitch = 0.0;
    int intChunk = (int) floor(frameTime); // Integer version of frame time
    if(intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y, y2, y3;
  
    //double start = 0 - stepSize;
    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = width() + (2 * stepSize);
    bottomPoints.setPoint(pointIndex++, toInt(start), height());
    lastN = firstN = toInt(start);
    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData *data = ch->dataAtChunk(intChunk);
      err = data->correlation();
      if(gdata->pitchContourMode() == 0)
        p.setPen(QPen(colorBetween(QColor(255, 255, 255), ch->color, err*dB2ViewVal(data->logrms())), lineWidth));
      else
        p.setPen(QPen(ch->color, lineWidth));
      
      x = toInt(n);
      lastN = x;
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;
      if(data->noteIndex >= 0) {
        isNoteRectEven[rectIndex] = (data->noteIndex % 2) == 0;
        if(gdata->showMeanVarianceBars()) {
          //longTermMean bars
          y2 = height() - 1 - toInt((data->longTermMean + data->longTermDeviation) / zoomY) + viewBottomOffset;
          y3 = height() - 1 - toInt((data->longTermMean - data->longTermDeviation) / zoomY) + viewBottomOffset;
          noteRect[rectIndex].setLeft(x);
          noteRect[rectIndex].setRight(toInt(n+stepSize));
          noteRect[rectIndex].setTop(y2);
          noteRect[rectIndex++].setBottom(y3);
  
          //shortTermMean bars
          y2 = height() - 1 - toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) + viewBottomOffset;
          y3 = height() - 1 - toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) + viewBottomOffset;
          noteRect2[rectIndex2].setLeft(x);
          noteRect2[rectIndex2].setRight(toInt(n+stepSize));
          noteRect2[rectIndex2].setTop(y2);
          noteRect2[rectIndex2++].setBottom(y3);
        }
      }
      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      y = height() - 1 - toInt(pitch / zoomY) + viewBottomOffset;
      bottomPoints.setPoint(pointIndex++, x, y);
      prevPitch = pitch;
    }
    bottomPoints.setPoint(pointIndex, bottomPoints.point(pointIndex-1).x(), height());
    pointIndex++;

    myassert(pointIndex <= width()*2);
    p.setPen(Qt::NoPen);
    p.setBrush(gdata->shading1Color());
    p.drawRect(firstN, 0, lastN, height());
    p.setBrush(gdata->shading2Color());
    p.drawPolygon(bottomPoints.constData(), pointIndex, Qt::OddEvenFill);

    if(gdata->showMeanVarianceBars()) {
      //shortTermMean bars
      for(int j=0; j<rectIndex2; j++) {
        if(isNoteRectEven[j]) p.setBrush(Qt::green);
        else p.setBrush(Qt::yellow);
        p.drawRect(noteRect2[j]);
      }
      //longTermMean bars
      QColor seeThroughYellow = Qt::yellow;
      seeThroughYellow.setAlpha(255);
      QColor seeThroughGreen = Qt::green;
      seeThroughGreen.setAlpha(255);
      for(int j=0; j<rectIndex; j++) {
        if(isNoteRectEven[j]) p.setBrush(seeThroughYellow);
        else p.setBrush(seeThroughGreen);
        p.drawRect(noteRect[j]);
      }
    }
  }
}

void DrawWidget::setChannelVerticalView(Channel *ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY)
{
  ZoomLookup *z = &ch->normalZoomLookup;
    
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
  if(baseElement < 0) { n -= baseElement; baseElement = 0; }
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
    if(lastBaseElement > z->size()) z->setSize(lastBaseElement);
    for(; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
      myassert(baseElement >= 0);
      ZoomElement &ze = z->at(baseElement);
      if(!ze.isValid()) {
        if(!calcZoomElement(ch, ze, baseElement, baseX)) continue;
      }
      if(ze.low() > 0.0f && ze.high() > 0.0f) {
        float weight = ze.corr();
        if(baseElement < currentBaseElement) weight *= double(currentBaseElement - baseElement) / leftBaseWidth;
        else if(baseElement > currentBaseElement) weight *= double(baseElement - currentBaseElement) / rightBaseWidth;
        if(ze.low() < minY) minY = ze.low();
        if(ze.high() > maxY) maxY = ze.high();
        totalY += (ze.low() + ze.high()) / 2.0f * weight;
        numY += weight;
        ys.push_back((ze.low() + ze.high()) / 2.0f);
        weightings.push_back(weight);
      }
      lastN = n;
    }
  } else { // More pixels than samples
    float err = 0.0, pitch = 0.0, prevPitch = 0.0;
    int intChunk = (int) floor(frameTime); // Integer version of frame time
    if(intChunk < 0) intChunk = 0;
    double stepSize = 1.0 / baseX; // So we skip some pixels
    int x = 0, y;
    float corr;
    
    double start = (double(intChunk) - frameTime) * stepSize;
    double stop = width() + (2 * stepSize);
    lastN = firstN = toInt(start);
    for (double n = start; n < stop && intChunk < (int)ch->totalChunks(); n += stepSize, intChunk++) {
      myassert(intChunk >= 0);
      AnalysisData *data = ch->dataAtChunk(intChunk);
      err = data->correlation();

      x = toInt(n);
      lastN = x;
      pitch = (ch->isVisibleChunk(data)) ? data->pitch : 0.0f;
      myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
      corr = data->correlation()*dB2ViewVal(data->logrms());
      if(pitch > 0.0f) {
        float weight = corr;
        if(minY < pitch) minY = pitch;
        if(maxY > pitch) maxY = pitch;
        totalY += pitch * weight;
        numY += weight;
        ys.push_back(pitch);
        weightings.push_back(weight);
      }
      y = height() - 1 - toInt(pitch / zoomY) + viewBottomOffset;
      prevPitch = pitch;
    }
  }
  
  if(!ys.empty()) {
    float meanY = totalY / numY;
    double spred = 0.0;
    myassert(ys.size() == weightings.size());
    //use a linear spred function. not a squared one like standard deviation
    for(uint j=0; j<ys.size(); j++) {
      spred += sq(ys[j] - meanY) * weightings[j];
    }
    spred = sqrt(spred / numY) * 4.0;
    //printf("spred = %f\n", spred);
    if(spred < 12.0) spred = 12.0; //show a minimum of 12 semi-tones
    gdata->view->setViewBottomRaw(meanY - gdata->view->viewHeight() / 2.0);
  }
}

/** calculates elements in the zoom lookup table
  @param ch The channel we are working with
  @param baseElement The element's index in the zoom lookup table
  @param baseX  The number of chunks each pixel represents (can include a fraction part)
  @return false if a zoomElement can't be calculated, else true
*/
bool DrawWidget::calcZoomElement(Channel *ch, ZoomElement &ze, int baseElement, double baseX)
{
  int startChunk = toInt(double(baseElement) * baseX);
  int finishChunk = toInt(double(baseElement+1) * baseX) + 1;
  if(finishChunk >= (int)ch->totalChunks()) finishChunk--; //dont go off the end
  if(finishChunk >= (int)ch->totalChunks()) return false; //that data doesn't exist yet
  
  std::pair<large_vector<AnalysisData>::iterator, large_vector<AnalysisData>::iterator> a =
    minMaxElement(ch->dataIteratorAtChunk(startChunk), ch->dataIteratorAtChunk(finishChunk), lessPitch());
  if(a.second == ch->dataIteratorAtChunk(finishChunk)) return false;
  
  large_vector<AnalysisData>::iterator err = std::max_element(ch->dataIteratorAtChunk(startChunk), ch->dataIteratorAtChunk(finishChunk), lessValue(0));
  if(err == ch->dataIteratorAtChunk(finishChunk)) return false;
  
  float low, high;
  int noteIndex;
  if(ch->isVisibleChunk(&*err)) {
    low = a.first->pitch;
    high = a.second->pitch;
    noteIndex = a.first->noteIndex;
  } else {
    low = 0;
    high = 0;
    noteIndex = NO_NOTE;
  }
  float corr = err->correlation()*dB2Normalised(err->logrms(), ch->rmsCeiling, ch->rmsFloor);
  QColor theColor = (gdata->pitchContourMode() == 0) ? colorBetween(gdata->backgroundColor(), ch->color, corr) : ch->color;

  ze.set(low, high, corr, theColor, noteIndex, (startChunk+finishChunk)/2);
  return true;
}

void DrawWidget::drawArray(float *input, int n, int sampleStep, double theZoomY, double offset)
{
  double dh2 = double(height()-1) / 2.0;
  double scaleY = dh2 * theZoomY;
  int w = width() / sampleStep;
  QPolygon  pointArray(w);
  int intStep = int(n / w);
  int remainderStep = n - (intStep * w);
  int pos = 0;
  int remainder = 0;

  for(int j=0; j<w; j++, pos+=intStep, remainder+=remainderStep) {
    if(remainder >= w) {
      pos++;
      remainder -= w;
    }
    myassert(pos < n);
    pointArray.setPoint(j, j*sampleStep, toInt(dh2 - (input[pos] + offset)*scaleY));
  }
  p.drawPolyline(pointArray);
}
