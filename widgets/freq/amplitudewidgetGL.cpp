/***************************************************************************
						  amplitudewidgetGL.cpp  -  description
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
#include "amplitudewidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include "shader.h"
#include "mygl.h"

#include "gdata.h"
#include "channel.h"
#include "zoomlookup.h"
#include "qcursor.h"

#include "analysisdata.h"
#include "conversions.h"

#include <qpixmap.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

AmplitudeWidgetGL::AmplitudeWidgetGL(QWidget* /*parent*/, const char* /*name*/)
{
	setMouseTracking(true);

	dragMode = DragNone;

	setRange(0.8);
	setOffset(0.0);
	setAttribute(Qt::WA_OpaquePaintEvent);
}

AmplitudeWidgetGL::~AmplitudeWidgetGL()
{
	QOpenGLContext* c = QOpenGLContext::currentContext();
	if (c) {
		makeCurrent();
	}

	m_vao_ch_amp.destroy();
	m_vao_ref_dark.destroy();
	m_vao_ref_light.destroy();
	m_vao_time_line.destroy();
	m_vao_blk_ref.destroy();
	m_vao_red_ref.destroy();
	
	m_vbo_ch_amp.destroy();
	m_vbo_ref_dark.destroy();
	m_vbo_ref_light.destroy();
	m_vbo_time_line.destroy();
	m_vbo_blk_ref.destroy();
	m_vbo_red_ref.destroy();

	m_program.deleteLater();
	m_program_line.deleteLater();
	doneCurrent();
}

void AmplitudeWidgetGL::initializeGL()
{
	initializeOpenGLFunctions();

	// build and compile our shader program
	// ------------------------------------
	try {
		Shader ourShader(&m_program, ":/shader.vs.glsl", ":/shader.fs.glsl");
	} catch (...) {
		close();
	}
	try {
		Shader ourShader(&m_program_line, ":/shader.vs.glsl", ":/shader.fs.glsl", ":/shader.gs.glsl");
	} catch (...) {
		close();
	}

	m_vao_ch_amp.create();
	m_vao_ref_dark.create();
	m_vao_ref_light.create();
	m_vao_time_line.create();
	m_vao_blk_ref.create();
	m_vao_red_ref.create();

	m_vbo_ch_amp.create();
	m_vbo_ref_dark.create();
	m_vbo_ref_light.create();
	m_vbo_time_line.create();
	m_vbo_blk_ref.create();
	m_vbo_red_ref.create();
}

void AmplitudeWidgetGL::resizeGL(int w, int h)
{
	glViewport(0, 0, (GLint)w, (GLint)h);

	// Create model transformation matrix to go from:
	//		x: 0 to width
	//		y: 0 to height
	//	to:
	//		x: -1.0f to 1.0f
	//		y: -1.0f to 1.0f
	QMatrix4x4 model;
	model.setToIdentity();
	model.translate(QVector3D(-1.0f, -1.0f, 0.0f));
	model.scale(2.0f / width(), 2.0f / height(), 1.0f);

	m_program.bind();
	m_program.setUniformValue("model", model);
	m_program.release();

	m_program_line.bind();
	m_program_line.setUniformValue("model", model);
	m_program_line.setUniformValue("screen_size", QVector2D(w, h));
	m_program_line.release();

}

void AmplitudeWidgetGL::drawChannelAmplitudeFilledGL(Channel* ch)
{
	View* view = gdata->view;

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

	QVector<QVector3D> vertexArray;
	if (baseX > 1) { // More samples than pixels
		int theWidth = width();
		if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
		for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
			myassert(baseElement >= 0);
			ZoomElement& ze = z->at(baseElement);
			if (!ze.isValid()) {
				if (!calcZoomElement(ze, ch, baseElement, baseX)) continue;
			}

			int y = 1 + toInt((ze.high() - offsetInv()) * heightRatio);
			vertexArray << QVector3D((float)n, 0.0f, 0.0f);
			vertexArray << QVector3D((float)n, (float)y, 0.0f);
		}
	} else { //baseX <= 1
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

			vertexArray << QVector3D((float)dn, 0.0f, 0.0f);
			vertexArray << QVector3D((float)dn, (float)(1 + ((val - offsetInv()) * heightRatio)), 0.0f);
		}
	}
	m_vao_ch_amp.bind();
	m_vbo_ch_amp.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ch_amp.bind();
	m_vbo_ch_amp.allocate(vertexArray.constData(), static_cast<int>(vertexArray.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_ch_amp, m_vbo_ch_amp, vertexArray.count(), GL_TRIANGLE_STRIP, gdata->shading2Color());
	m_vao_ch_amp.release();
	m_vbo_ch_amp.release();
}

void AmplitudeWidgetGL::drawVerticalRefLines()
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

	QVector<QVector3D> refLinesDark;
	QVector<QVector3D> refLinesLight;
	for (; timePos <= rightTime(); timePos += timeScaleBase) {
		x = toInt((timePos - lTime) * ratio);
		if (++largeCounter == largeFreq) {
			largeCounter = 0;
			refLinesDark << QVector3D(x, 0.0f, 0.0f);
			refLinesDark << QVector3D(x, (float)(height() - 1.0f), 0.0f);
		} else {
			refLinesLight << QVector3D(x, 0.0f, 0.0f);
			refLinesLight << QVector3D(x, (float)(height() - 1.0f), 0.0f);
		}
	}

	// Draw the dark lines 
	m_vao_ref_dark.bind();
	m_vbo_ref_dark.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ref_dark.bind();
	m_vbo_ref_dark.allocate(refLinesDark.constData(), static_cast<int>(refLinesDark.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_ref_dark, m_vbo_ref_dark, refLinesDark.count(), GL_LINES, QColor(25, 125, 170, 128));

	// Draw the light lines
	m_vao_ref_light.bind();
	m_vbo_ref_light.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ref_light.bind();
	m_vbo_ref_light.allocate(refLinesLight.constData(), static_cast<int>(refLinesLight.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_ref_light, m_vbo_ref_light, refLinesLight.count(), GL_LINES, QColor(25, 125, 170, 64));
}

void AmplitudeWidgetGL::drawChannelAmplitudeGL(Channel* ch)
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

	QVector<QVector3D> vertexArray;
	
	// Half the linewidth for lines.
	float lineWidth = 2.0f;
	float halfLineWidth = lineWidth/2.0f;

	if (baseX > 1) { // More samples than pixels
		int theWidth = width();
		if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
		for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
			myassert(baseElement >= 0);
			ZoomElement& ze = z->at(baseElement);
			if (!ze.isValid()) {
				if (!calcZoomElement(ze, ch, baseElement, baseX)) continue;
			}
			vertexArray << QVector3D((float)n, (float)(1 + ((ze.high() - offsetInv()) * heightRatio)) + halfLineWidth, 0.0f);
			vertexArray << QVector3D((float)n, (float)(1 + ((ze.low() - offsetInv()) * heightRatio)) - halfLineWidth, 0.0f);
		}
		myassert(vertexArray.count() <= width() * 2);

		m_vao_time_line.bind();
		m_vbo_time_line.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_time_line.bind();
		m_vbo_time_line.allocate(vertexArray.constData(), static_cast<int>(vertexArray.count() * 3 * sizeof(float)));

		MyGL::DrawShape(m_program_line, m_vao_time_line, m_vbo_time_line, vertexArray.count(), GL_LINES, ch->color);
	} else { //baseX <= 1
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

			vertexArray << QVector3D((float)dn, (float)(1 + ((val - offsetInv()) * heightRatio)), 0.0f);
		}
		myassert(vertexArray.count() <= width() * 2);

		m_vao_time_line.bind();
		m_vbo_time_line.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_time_line.bind();
		m_vbo_time_line.allocate(vertexArray.constData(), static_cast<int>(vertexArray.count() * 3 * sizeof(float)));

		MyGL::DrawLine(m_program_line, m_vao_time_line, m_vbo_time_line, vertexArray.count(), GL_LINE_STRIP, lineWidth, ch->color);
	}
}

void AmplitudeWidgetGL::paintGL()
{
	QPainter p;
	p.begin(this);
	p.beginNativePainting();
	QColor bg = gdata->backgroundColor();
	glClearColor(double(bg.red()) / 255.0, double(bg.green()) / 255.0, double(bg.blue()) / 255.0, bg.alpha() / 255.0);

	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);

	View* view = gdata->view;

	//draw the red/blue background color shading if needed
	if (view->backgroundShading() && gdata->getActiveChannel()) {
		drawChannelAmplitudeFilledGL(gdata->getActiveChannel());
	}

	glDisable(GL_LINE_SMOOTH);

	drawVerticalRefLines();

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	//draw all the visible channels
	for (uint i = 0; i < gdata->channels.size(); i++) {
		Channel* ch = gdata->channels.at(i);
		if (!ch->isVisible()) continue;
		drawChannelAmplitudeGL(ch);
	}
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);

	// Draw the current time line
	double curScreenTime = (view->currentTime() - view->viewLeft()) / view->zoomX();

	QVector<QVector3D> timeLine;
	timeLine << QVector3D((float)curScreenTime, 0.0f, 0.0f);
	timeLine << QVector3D((float)curScreenTime, height() - 1, 0.0f);

	m_vao_time_line.bind();
	m_vbo_time_line.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_time_line.bind();
	m_vbo_time_line.allocate(timeLine.constData(), static_cast<int>(timeLine.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_time_line, m_vbo_time_line, timeLine.count(), GL_LINES, palette().color(QPalette::WindowText));

	// Draw a horizontal line at the current threshold.
	float y;
	double heightRatio = double(height()) / range();

	// Draw the horizontal Black line
	QVector<QVector3D> blkRef;
	y = 1 + toInt((getCurrentThreshold(0) - offsetInv()) * heightRatio);
	blkRef << QVector3D(0.0f, y, 0.0f);
	blkRef << QVector3D((float)width(), y, 0.0f);

	m_vao_blk_ref.bind();
	m_vbo_blk_ref.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_blk_ref.bind();
	m_vbo_blk_ref.allocate(blkRef.constData(), static_cast<int>(blkRef.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_blk_ref, m_vbo_blk_ref, blkRef.count(), GL_LINES, QColor(Qt::black));

	// Draw the horizontal Red line
	QVector<QVector3D> redRef;
	y = 1 + toInt((getCurrentThreshold(1) - offsetInv()) * heightRatio);
	redRef << QVector3D(0.0f, y, 0.0f);
	redRef << QVector3D((float)width(), y, 0.0f);

	m_vao_red_ref.bind();
	m_vbo_red_ref.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_red_ref.bind();
	m_vbo_red_ref.allocate(redRef.constData(), static_cast<int>(redRef.count() * 3 * sizeof(float)));

	MyGL::DrawShape(m_program, m_vao_red_ref, m_vbo_red_ref, redRef.count(), GL_LINES, QColor(Qt::red));

	p.endNativePainting();

	p.setPen(Qt::black);
	p.drawText(2, height() - 3, getCurrentThresholdString());
	p.end();
}


void AmplitudeWidgetGL::setRange(double newRange)
{
	if (_range != newRange) {
		_range = bound(newRange, 0.0, 1.0);
		setOffset(offset());
		emit rangeChanged(_range);
	}
}

// This is required since setRange is called when a QWTWheel is changed and rangeChanged sets the valye of the QWTWheel which breaks the
// Mouse Motion.
void AmplitudeWidgetGL::setRangeQWTWheel(double newRange)
{
	if (_range != newRange) {
		_range = bound(newRange, 0.0, 1.0);
		setOffset(offset());
	}
}

void AmplitudeWidgetGL::setOffset(double newOffset)
{
	newOffset = bound(newOffset, 0.0, maxOffset());
	_offset = newOffset;
	_offsetInv = maxOffset() - _offset;
	emit offsetChanged(_offset);
	emit offsetInvChanged(offsetInv());
}

/** This function has the side effect of changing ze
*/
bool AmplitudeWidgetGL::calcZoomElement(ZoomElement& ze, Channel* ch, int baseElement, double baseX)
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

double AmplitudeWidgetGL::calculateElement(AnalysisData* data)
{
	double val = (*amp_mode_func[gdata->amplitudeMode()])(data->values[gdata->amplitudeMode()]);
	return val;
}

double AmplitudeWidgetGL::getCurrentThreshold(int index)
{
	return (*amp_mode_func[gdata->amplitudeMode()])(gdata->ampThreshold(gdata->amplitudeMode(), index));
}

void AmplitudeWidgetGL::setCurrentThreshold(double newThreshold, int index)
{
	newThreshold = bound(newThreshold, 0.0, 1.0);
	if (newThreshold < offsetInv()) setOffset(maxOffset() - newThreshold);
	else if (newThreshold > offsetInv() + range()) setOffset(maxOffset() - (newThreshold - range()));

	gdata->setAmpThreshold(gdata->amplitudeMode(), index, (*amp_mode_inv_func[gdata->amplitudeMode()])(newThreshold));
}

QString AmplitudeWidgetGL::getCurrentThresholdString()
{
	QString thresholdStr;
  thresholdStr = thresholdStr.asprintf(amp_display_string[gdata->amplitudeMode()], gdata->ampThreshold(gdata->amplitudeMode(), 0), gdata->ampThreshold(gdata->amplitudeMode(), 1));
	return thresholdStr;
}

void AmplitudeWidgetGL::mousePressEvent(QMouseEvent* e)
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

void AmplitudeWidgetGL::mouseMoveEvent(QMouseEvent* e)
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

void AmplitudeWidgetGL::mouseReleaseEvent(QMouseEvent*)
{
	dragMode = DragNone;
}

void AmplitudeWidgetGL::wheelEvent(QWheelEvent* e)
{
	View* view = gdata->view;
	if (!(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
		if (gdata->running == STREAM_FORWARD) {
			view->setZoomFactorX(view->logZoomX() + double(e->angleDelta().y() / WHEEL_DELTA) * 0.3);
		} else {
			if (e->angleDelta().y() < 0) {
				view->setZoomFactorX(view->logZoomX() + double(e->angleDelta().y() / WHEEL_DELTA) * 0.3, width() / 2);
			} else {
				view->setZoomFactorX(view->logZoomX() + double(e->angleDelta().y() / WHEEL_DELTA) * 0.3, e->position().x());
			}
		}
		view->doSlowUpdate();
	}

	e->accept();
}
