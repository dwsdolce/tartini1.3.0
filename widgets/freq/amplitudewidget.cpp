/***************************************************************************
						  amplitudewidget.cpp  -  description
							 -------------------
	begin                : 19 Mar 2005
	copyright            : (C) 2005 by Philip McLeod
	email                : pmcleod@cs.otago.ac.nz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Please read LICENSE.txt for details.
 ***************************************************************************/
#include "amplitudewidget.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QVector3D>
#include <QDebug>

#include "gdata.h"
#include "channel.h"
#include "zoomlookup.h"
#include "qcursor.h"

#include "analysisdata.h"
#include "conversions.h"

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

AmplitudeWidget::AmplitudeWidget(QWidget* parent) : DrawWidget(parent)
{
	setMouseTracking(true);

	dragMode = DragNone;

	setRange(0.8);
	setOffset(0.0);
	setAttribute(Qt::WA_OpaquePaintEvent);
}

AmplitudeWidget::~AmplitudeWidget()
{
}

void AmplitudeWidget::drawChannelAmplitudeFilled(Channel* ch)
{
	View* view = gdata->view;

	ChannelLocker channelLocker(ch);
	ZoomLookup* z = &ch->amplitudeZoomLookup;

	// baseX is the no. of chunks a pixel must represent.
	double baseX = view->zoomX() / ch->timePerChunk();

	z->setZoomLevel(baseX);

	double currentChunk = ch->chunkFractionAtTime(view->currentTime());
	double leftFrameTime = currentChunk - ((view->currentTime() - view->viewLeft()) / ch->timePerChunk());
	int n = 0, lastn = 0;
	int baseElement = int(floor(leftFrameTime / baseX));
	if (baseElement < 0) {
		n -= baseElement; baseElement = 0;
	}
	int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));
	double heightRatio = double(height()) / range();

	QPolygonF ch_amp;
	ch_amp << QPointF(n, height());
	if (baseX > 1) { // More samples than pixels
		int theWidth = width();
		if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
		for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
			myassert(baseElement >= 0);
			ZoomElement& ze = z->at(baseElement);
			if (!ze.isValid()) {
				if (!calcZoomElement(ze, ch, baseElement, baseX)) continue;
			}

			double y = height() - (1 + (ze.high() - offsetInv()) * heightRatio);
			ch_amp << QPointF(n, y);
			lastn = n;
		}
		ch_amp << QPointF(lastn, height());
	} else { //baseX <= 1
		float val = 0.0;
		int intChunk = (int)floor(leftFrameTime); // Integer version of frame time
		double stepSize = 1.0 / baseX; // So we skip some pixels

		double start = (double(intChunk) - leftFrameTime) * stepSize;
		double stop = width() + (2 * stepSize);

		double dn = start, lastdn = start;
		int totalChunks = ch->totalChunks();
		if (intChunk < 0) {
			dn += stepSize * -intChunk; intChunk = 0;
		}

		for (; dn < stop && intChunk < totalChunks; dn += stepSize, intChunk++) {
			AnalysisData* data = ch->dataAtChunk(intChunk);
			myassert(data);

			if (!data) continue;
			
			val = calculateElement(data);

			ch_amp << QPointF(dn, height() - (1 + ((val - offsetInv()) * heightRatio)));
			lastdn = dn;
		}
		ch_amp << QPointF(lastdn, height());
	}

	p.setPen(gdata->shading2Color());
	p.setBrush(gdata->shading2Color());
	p.drawPolygon(ch_amp);
}

void AmplitudeWidget::drawVerticalRefLines()
{
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

	QVector<QLineF> refLinesDark;
	QVector<QLineF> refLinesLight;

	for (; timePos <= rightTime(); timePos += timeScaleBase) {
		x = toInt((timePos - lTime) * ratio);
		if (++largeCounter == largeFreq) {
			largeCounter = 0;
			refLinesDark << QLineF(QPointF(x, 0.0), QPointF(x, height() - 1.0));
		} else {
			refLinesLight << QLineF(QPointF(x, 0.0), QPointF(x, height() - 1.0));
		}
	}

	// Draw the dark lines
	p.setPen(QColor(25, 125, 170, 128));
	p.drawLines(refLinesDark);

	// Draw the light lines
	p.setPen(QColor(25, 125, 170, 64));
	p.drawLines(refLinesLight);
}

void AmplitudeWidget::drawChannelAmplitude(Channel* ch)
{
	View* view = gdata->view;
	QVector<QVector3D> amp;

	ChannelLocker channelLocker(ch);
	ZoomLookup* z = &ch->amplitudeZoomLookup;

	// baseX is the no. of chunks a pixel must represent.
	double baseX = view->zoomX() / ch->timePerChunk();

	z->setZoomLevel(baseX);

	double currentChunk = ch->chunkFractionAtTime(view->currentTime());
	double leftFrameTime = currentChunk - ((view->currentTime() - view->viewLeft()) / ch->timePerChunk());
	int n = 0;
	int baseElement = int(floor(leftFrameTime / baseX));
	if (baseElement < 0) {
		n -= baseElement; baseElement = 0;
	}
	int lastBaseElement = int(floor(double(ch->totalChunks()) / baseX));
	double heightRatio = double(height()) / range();

	// Half the linewidth for lines.
	double lineWidth = 2.0f;
	double halfLineWidth = lineWidth/2.0f;

	QPen chPen(ch->color);
	chPen.setWidth(lineWidth);

	if (baseX > 1) { // More samples than pixels
		QVector<QLineF> ch_amp;

		int theWidth = width();
		if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
		for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
			myassert(baseElement >= 0);
			ZoomElement& ze = z->at(baseElement);
			if (!ze.isValid()) {
				if (!calcZoomElement(ze, ch, baseElement, baseX)) continue;
			}
			ch_amp << QLineF(QPointF(n, height() - (1 + ((ze.high() - offsetInv()) * heightRatio) + halfLineWidth)),
				               QPointF(n, height() - (1 + ((ze.low() - offsetInv()) * heightRatio) - halfLineWidth)));
		}
		p.setPen(chPen);
		p.drawLines(ch_amp);
	} else { //baseX <= 1
		QPolygonF ch_amp;

		float val = 0.0;
		int intChunk = (int)floor(leftFrameTime); // Integer version of frame time
		double stepSize = 1.0 / baseX; // So we skip some pixels

		double start = (double(intChunk) - leftFrameTime) * stepSize;
		double stop = width() + (2 * stepSize);
		double dn = start;
		int totalChunks = ch->totalChunks();
		if (intChunk < 0) {
			dn += stepSize * -intChunk; intChunk = 0;
		}
		for (; dn < stop && intChunk < totalChunks; dn += stepSize, intChunk++) {
			AnalysisData* data = ch->dataAtChunk(intChunk);
			myassert(data);

			if (!data) continue;
			val = calculateElement(data);
			ch_amp << QPointF(dn, height() - (1 + ((val - offsetInv()) * heightRatio)));
		}
		p.setPen(chPen);
		p.drawPolyline(ch_amp);
	}
}

void AmplitudeWidget::paintEvent(QPaintEvent*)
{
	beginDrawing();

	View* view = gdata->view;

	//draw the red/blue background color shading if needed
	if (view->backgroundShading() && gdata->getActiveChannel()) {
		drawChannelAmplitudeFilled(gdata->getActiveChannel());
	}
	drawVerticalRefLines();

	//draw all the visible channels
	for (uint i = 0; i < gdata->channels.size(); i++) {
		Channel* ch = gdata->channels.at(i);
		if (!ch->isVisible()) continue;
		drawChannelAmplitude(ch);
	}

	// Draw the current time line
	double curScreenTime = (view->currentTime() - view->viewLeft()) / view->zoomX();

	QLineF timeLine = QLineF(QPointF(curScreenTime, 0.0), QPointF(curScreenTime, height() - 1.0));
	
	p.setPen(palette().color(QPalette::Foreground));
	p.drawLine(timeLine);

	// Draw a horizontal line at the current threshold.
	double y;
	double heightRatio = double(height()) / range();

	// Draw the horizontal Black line
	QLineF blkRef;

	y = height() - (1 + toInt((getCurrentThreshold(0) - offsetInv()) * heightRatio));
	blkRef = QLineF(QPointF(0.0, y), QPointF(width(), y));

	p.setPen(Qt::black);
	p.drawLine(blkRef);

	// Draw the horizontal Red line
	QLineF redRef;

	y = height() - (1 + toInt((getCurrentThreshold(1) - offsetInv()) * heightRatio));
	redRef = QLineF(QPointF(0.0, y), QPointF(width(), y));

	p.setPen(Qt::red);
	p.drawLine(redRef);

	p.setPen(Qt::black);
	p.drawText(2, height() - 3, getCurrentThresholdString());

	endDrawing();
}


void AmplitudeWidget::setRange(double newRange)
{
	if (_range != newRange) {
		_range = bound(newRange, 0.0, 1.0);
		setOffset(offset());
		emit rangeChanged(_range);
	}
}

// This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
// Mouse Motion.
void AmplitudeWidget::setRangeQWTWheel(double newRange)
{
	if (_range != newRange) {
		_range = bound(newRange, 0.0, 1.0);
		setOffset(offset());
	}
}

void AmplitudeWidget::setOffset(double newOffset)
{
	newOffset = bound(newOffset, 0.0, maxOffset());
	_offset = newOffset;
	_offsetInv = maxOffset() - _offset;
	emit offsetChanged(_offset);
	emit offsetInvChanged(offsetInv());
}


/** This function has the side effect of changing ze
*/
bool AmplitudeWidget::calcZoomElement(ZoomElement& ze, Channel* ch, int baseElement, double baseX)
{
	int startChunk = int(floor(double(baseElement) * baseX));
	myassert(startChunk <= ch->totalChunks());
	int finishChunk = int(floor(double(baseElement + 1) * baseX)) + 1;
	myassert(finishChunk <= ch->totalChunks());
	if (startChunk == finishChunk) return false;

	myassert(startChunk < finishChunk);

	int mode = gdata->amplitudeMode();
	std::pair<large_vector<AnalysisData>::iterator, large_vector<AnalysisData>::iterator> a =
		minMaxElement(ch->dataIteratorAtChunk(startChunk), ch->dataIteratorAtChunk(finishChunk), lessValue(mode));
	myassert(a.second != ch->dataIteratorAtChunk(finishChunk));
	float low = (*amp_mode_func[mode])(a.first->values[mode]);
	float high = (*amp_mode_func[mode])(a.second->values[mode]);

	ze.set(low, high, 0, ch->color, NO_NOTE, (startChunk + finishChunk) / 2);
	return true;
}

double AmplitudeWidget::calculateElement(AnalysisData* data)
{
	double val = (*amp_mode_func[gdata->amplitudeMode()])(data->values[gdata->amplitudeMode()]);
	return val;
}

double AmplitudeWidget::getCurrentThreshold(int index)
{
	return (*amp_mode_func[gdata->amplitudeMode()])(gdata->ampThreshold(gdata->amplitudeMode(), index));
}

void AmplitudeWidget::setCurrentThreshold(double newThreshold, int index)
{
	newThreshold = bound(newThreshold, 0.0, 1.0);
	if (newThreshold < offsetInv()) setOffset(maxOffset() - newThreshold);
	else if (newThreshold > offsetInv() + range()) setOffset(maxOffset() - (newThreshold - range()));

	gdata->setAmpThreshold(gdata->amplitudeMode(), index, (*amp_mode_inv_func[gdata->amplitudeMode()])(newThreshold));
}

QString AmplitudeWidget::getCurrentThresholdString()
{
	QString thresholdStr;
	thresholdStr.sprintf(amp_display_string[gdata->amplitudeMode()], gdata->ampThreshold(gdata->amplitudeMode(), 0), gdata->ampThreshold(gdata->amplitudeMode(), 1));
	return thresholdStr;
}

void AmplitudeWidget::mousePressEvent(QMouseEvent* e)
{
	View* view = gdata->view;
	int timeX = toInt(view->viewOffset() / view->zoomX());
	int pixelAtCurrentNoiseThresholdY;
	dragMode = DragNone;

	//Check if user clicked on center bar, to drag it
	if (within(4, e->x(), timeX)) {
		dragMode = DragTimeBar;
		mouseX = e->x();
		return;
	}
	//Check if user clicked on a threshold bar
	for (int j = 0; j < 2; j++) {
		pixelAtCurrentNoiseThresholdY = height() - 1 - toInt((getCurrentThreshold(j) - offsetInv()) / range() * double(height()));
		if (within(4, e->y(), pixelAtCurrentNoiseThresholdY)) {
			dragMode = DragNoiseThreshold;
			thresholdIndex = j; //remember which thresholdIndex the user clicked
			mouseY = e->y();
			return;
		}
	}
	//Otherwise user has clicked on background
	{
		mouseX = e->x();
		mouseY = e->y();
		dragMode = DragBackground;
		downTime = view->currentTime();
		downOffset = offset();
	}
}

void AmplitudeWidget::mouseMoveEvent(QMouseEvent* e)
{
	View* view = gdata->view;
	int pixelAtCurrentTimeX = toInt(view->viewOffset() / view->zoomX());
	int pixelAtCurrentNoiseThresholdY;

	switch (dragMode) {
	case DragTimeBar:
	{
		int newX = pixelAtCurrentTimeX + (e->x() - mouseX);
		view->setViewOffset(double(newX) * view->zoomX());
		mouseX = e->x();
		view->doSlowUpdate();
	}
	break;
	case DragNoiseThreshold:
	{
		int newY = e->y();
		setCurrentThreshold(double(height() - 1 - newY) / double(height()) * range() + offsetInv(), thresholdIndex);
		mouseY = e->y();
		gdata->view->doSlowUpdate();
	}
	break;
	case DragBackground:
		gdata->updateActiveChunkTime(downTime - (e->x() - mouseX) * view->zoomX());
		setOffset(downOffset - (double(e->y() - mouseY) / double(height()) * range()));
		view->doSlowUpdate();
		break;
	case DragNone:
		if (within(4, e->x(), pixelAtCurrentTimeX)) {
			setCursor(QCursor(Qt::SplitHCursor));
		} else {
			bool overThreshold = false;
			for (int j = 0; j < 2; j++) {
				pixelAtCurrentNoiseThresholdY = height() - 1 - toInt((getCurrentThreshold(j) - offsetInv()) / range() * double(height()));
				if (within(4, e->y(), pixelAtCurrentNoiseThresholdY)) overThreshold = true;
			}
			if (overThreshold) setCursor(QCursor(Qt::SplitVCursor));
			else unsetCursor();
		}
	}
}

void AmplitudeWidget::mouseReleaseEvent(QMouseEvent*)
{
	dragMode = DragNone;
}

void AmplitudeWidget::wheelEvent(QWheelEvent* e)
{
	View* view = gdata->view;
	if (!(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
		if (gdata->running == STREAM_FORWARD) {
			view->setZoomFactorX(view->logZoomX() + double(e->delta() / WHEEL_DELTA) * 0.3);
		} else {
			if (e->delta() < 0) {
				view->setZoomFactorX(view->logZoomX() + double(e->delta() / WHEEL_DELTA) * 0.3, width() / 2);
			} else {
				view->setZoomFactorX(view->logZoomX() + double(e->delta() / WHEEL_DELTA) * 0.3, e->x());
			}
		}
		view->doSlowUpdate();
	}

	e->accept();
}
