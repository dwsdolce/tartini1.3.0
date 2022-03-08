/***************************************************************************
						  freqwidgetGL.cpp  -  description
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


#include <QOpenGLContext>
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

#include "freqwidgetGL.h"
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

FreqWidgetGL::FreqWidgetGL()
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

FreqWidgetGL::~FreqWidgetGL()
{
	QOpenGLContext* c = QOpenGLContext::currentContext();
	if (c) {
		makeCurrent();
	}

	m_vao_all_fill.destroy();
	m_vbo_all_fill.destroy();
	m_vao_channel_fill.destroy();
	m_vbo_channel_fill.destroy();
	m_vao_varianace[VarYellow].destroy();
	m_vbo_varianace[VarYellow].destroy();
	m_vao_varianace[VarGreen].destroy();
	m_vbo_varianace[VarGreen].destroy();
	m_vao_varianace2[VarYellow].destroy();
	m_vbo_varianace2[VarYellow].destroy();
	m_vao_varianace2[VarGreen].destroy();
	m_vbo_varianace2[VarGreen].destroy();
	m_vao_ref_lines[RefRoot].destroy();
	m_vbo_ref_lines[RefRoot].destroy();
	m_vao_ref_lines[RefVertDark].destroy();
	m_vbo_ref_lines[RefVertDark].destroy();
	m_vao_ref_lines[RefVertLight].destroy();
	m_vbo_ref_lines[RefVertLight].destroy();
	m_vao_channel.destroy();
	m_vbo_channel.destroy();
	m_vbo_channel_color.destroy();
	m_vao_channel_point.destroy();
	m_vbo_channel_point.destroy();
	m_vbo_channel_point_color.destroy();
	m_vao_active_band.destroy();
	m_vbo_active_band.destroy();
	m_vao_time_line.destroy();
	m_vbo_time_line.destroy();

	m_program.deleteLater();
	m_program_color.deleteLater();
	m_program_line.deleteLater();
	m_program_line_color.deleteLater();
	doneCurrent();
}

void FreqWidgetGL::initializeGL()
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
		Shader ourShader(&m_program_color, ":/color.vs.glsl", ":/color.fs.glsl");
	} catch (...) {
		close();
	}
	try {
		Shader ourShader(&m_program_line, ":/shader.vs.glsl", ":/shader.fs.glsl", ":/shader.gs.glsl");
	} catch (...) {
		close();
	}
	try {
		Shader ourShader(&m_program_line_color, ":/line.vs.glsl", ":/line.fs.glsl", ":/line.gs.glsl");
	} catch (...) {
		close();
	}

	m_vao_all_fill.create();
	m_vbo_all_fill.create();
	m_vao_channel_fill.create();
	m_vbo_channel_fill.create();
	m_vao_varianace[VarYellow].create();
	m_vbo_varianace[VarYellow].create();
	m_vao_varianace[VarGreen].create();
	m_vbo_varianace[VarGreen].create();
	m_vao_varianace2[VarYellow].create();
	m_vbo_varianace2[VarYellow].create();
	m_vao_varianace2[VarGreen].create();
	m_vbo_varianace2[VarGreen].create();
	m_vao_ref_lines[RefRoot].create();
	m_vbo_ref_lines[RefRoot].create();
	m_vao_ref_lines[RefVertDark].create();
	m_vbo_ref_lines[RefVertDark].create();
	m_vao_ref_lines[RefVertLight].create();
	m_vbo_ref_lines[RefVertLight].create();
	m_vao_channel.create();
	m_vbo_channel.create();
	m_vbo_channel_color.create();
	m_vao_channel_point.create();
	m_vbo_channel_point.create();
	m_vbo_channel_point_color.create();
	m_vao_active_band.create();
	m_vbo_active_band.create();
	m_vao_time_line.create();
	m_vbo_time_line.create();
}

void FreqWidgetGL::resizeEvent(QResizeEvent* q)
{
	QOpenGLWidget::resizeEvent(q);

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

void FreqWidgetGL::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	// Create model transformation matrix to go from:
	//		x: 0 to width
	//		y: 0 to height
	//	to:
	//		x: -1.0f to 1.0f
	//		y: -1.0f to 1.0f
	QMatrix4x4 model;
	model.setToIdentity();
	model.translate(QVector3D(-1.0f, -1.0f, 0.0f));
	model.scale(2.0f / width, 2.0f / height, 1.0f);

	m_program.bind();
	m_program.setUniformValue("model", model);
	m_program.release();

	m_program_color.bind();
	m_program_color.setUniformValue("model", model);
	m_program_color.release();

	m_program_line.bind();
	m_program_line.setUniformValue("model", model);
	m_program_line.setUniformValue("screen_size", QVector2D(width, height));
	m_program_line.release();

	m_program_line_color.bind();
	m_program_line_color.setUniformValue("model", model);
	m_program_line_color.setUniformValue("screen_size", QVector2D(width, height));
	m_program_line_color.release();
}

void FreqWidgetGL::drawChannelFilledGL(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
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

	QVector<QVector3D> bottomPoints;
	QVector<QVector3D> evenMidPoints;
	QVector<QVector3D> oddMidPoints;
	QVector<QVector3D> evenMidPoints2;
	QVector<QVector3D> oddMidPoints2;

	bool isNoteRectEven;

	if (baseX > 1) { // More samples than pixels
		int theWidth = width();
		if (lastBaseElement > z->size()) z->setSize(lastBaseElement);
		for (; n < theWidth && baseElement < lastBaseElement; n++, baseElement++) {
			myassert(baseElement >= 0);
			ZoomElement& ze = z->at(baseElement);
			if (!ze.isValid()) {
				if (!calcZoomElement(ch, ze, baseElement, baseX)) continue;
			}

			int y = 1 + toInt(ze.high() / zoomY) - viewBottomOffset;
			int y2, y3;
			if (ze.noteIndex() != -1 && ch->dataAtChunk(ze.midChunk())->noteIndex != -1) {
				myassert(ze.noteIndex() >= 0);
				myassert(ze.noteIndex() < int(ch->noteData.size()));
				myassert(ch->isValidChunk(ze.midChunk()));
				AnalysisData* data = ch->dataAtChunk(ze.midChunk());

				if (gdata->showMeanVarianceBars()) {
				    //longTermMean bars
					y2 = 1 + toInt((data->longTermMean + data->longTermDeviation) / zoomY) - viewBottomOffset;
					y3 = 1 + toInt((data->longTermMean - data->longTermDeviation) / zoomY) - viewBottomOffset;
					if (ze.noteIndex() % 2 == 0) {
						evenMidPoints << QVector3D(n, y2, 0);
						evenMidPoints << QVector3D(n, y3, 0);
					} else {
						oddMidPoints << QVector3D(n, y2, 0);
						oddMidPoints << QVector3D(n, y3, 0);
					}

					//shortTermMean bars
					y2 = 1 + toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) - viewBottomOffset;
					y3 = 1 + toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) - viewBottomOffset;
					if (ze.noteIndex() % 2 == 0) {
						evenMidPoints2 << QVector3D(n, y2, 0);
						evenMidPoints2 << QVector3D(n, y3, 0);
					} else {
						oddMidPoints2 << QVector3D(n, y2, 0);
						oddMidPoints2 << QVector3D(n, y3, 0);
					}
				}
			}
			bottomPoints << QVector3D(n, 0, 0.0f);
			bottomPoints << QVector3D(n, y, 0.0f);
			lastN = n;
		}

		QVector<QVector3D> rect;
		rect << QVector3D(firstN, 0,        0);
		rect << QVector3D(firstN, height(), 0);
		rect << QVector3D(lastN,  0,        0);
		rect << QVector3D(lastN,  height(), 0);

		m_vao_all_fill.bind();
		m_vbo_all_fill.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_all_fill.bind();
		m_vbo_all_fill.allocate(rect.constData(), rect.count() * 3 * sizeof(float));
		MyGL::DrawShape(m_program, m_vao_all_fill, m_vbo_all_fill, rect.count(), GL_TRIANGLE_STRIP, gdata->shading1Color());

		if (bottomPoints.count() > 1) {
			m_vao_channel_fill.bind();
			m_vbo_channel_fill.setUsagePattern(QOpenGLBuffer::DynamicDraw);
			m_vbo_channel_fill.bind();
			m_vbo_channel_fill.allocate(bottomPoints.constData(), bottomPoints.count() * 3 * sizeof(float));
			MyGL::DrawShape(m_program, m_vao_channel_fill, m_vbo_channel_fill, bottomPoints.count(), GL_TRIANGLE_STRIP, gdata->shading2Color());
		}

		if (gdata->showMeanVarianceBars()) {
		    //shortTermMean bars
			if (evenMidPoints2.count() > 1) {
				m_vao_varianace2[VarGreen].bind();
				m_vbo_varianace2[VarGreen].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace2[VarGreen].bind();
				m_vbo_varianace2[VarGreen].allocate(evenMidPoints2.constData(), evenMidPoints2.count() * 3 * sizeof(float));
				MyGL::DrawLine(m_program_line, m_vao_varianace2[VarGreen], m_vbo_varianace2[VarGreen], evenMidPoints2.count(), GL_LINES, 3.0, QColor(Qt::green));
			}
			if (oddMidPoints2.count() > 1) {
				m_vao_varianace2[VarYellow].bind();
				m_vbo_varianace2[VarYellow].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace2[VarYellow].bind();
				m_vbo_varianace2[VarYellow].allocate(oddMidPoints2.constData(), oddMidPoints2.count() * 3 * sizeof(float));
				MyGL::DrawLine(m_program_line, m_vao_varianace2[VarYellow], m_vbo_varianace2[VarYellow], oddMidPoints2.count(), GL_LINES, 3.0, QColor(Qt::yellow));
			}

			//longTermMean bars
			if (evenMidPoints.count() > 1) {
				m_vao_varianace[VarYellow].bind();
				m_vbo_varianace[VarYellow].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace[VarYellow].bind();
				m_vbo_varianace[VarYellow].allocate(evenMidPoints.constData(), evenMidPoints.count() * 3 * sizeof(float));
				MyGL::DrawLine(m_program_line, m_vao_varianace[VarYellow], m_vbo_varianace[VarYellow], evenMidPoints.count(), GL_LINES, 3.0, QColor(Qt::yellow));
			}
			if (oddMidPoints.count() > 1) {
				m_vao_varianace[VarGreen].bind();
				m_vbo_varianace[VarGreen].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace[VarGreen].bind();
				m_vbo_varianace[VarGreen].allocate(oddMidPoints.constData(), oddMidPoints.count() * 3 * sizeof(float));
				MyGL::DrawLine(m_program_line, m_vao_varianace[VarGreen], m_vbo_varianace[VarGreen], oddMidPoints.count(), GL_LINES, 3.0, QColor(Qt::green));
			}
		}
	} else { // More pixels than samples
		float pitch = 0.0, prevPitch = 0.0;
		int intChunk = (int)floor(frameTime); // Integer version of frame time
		if (intChunk < 0) intChunk = 0;
		double stepSize = 1.0 / baseX; // So we skip some pixels
		int x = 0, y, y2, y3, x2;

		double start = (double(intChunk) - frameTime) * stepSize;
		double stop = width() + (2 * stepSize);
		lastN = firstN = toInt(start);
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
					y2 = 1 + toInt((data->longTermMean + data->longTermDeviation) / zoomY) - viewBottomOffset;
					y3 = 1 + toInt((data->longTermMean - data->longTermDeviation) / zoomY) - viewBottomOffset;

					// Since Quads are no long supported define 2 triangles
					if ((data->noteIndex % 2) == 0) {
						evenMidPoints << QVector3D(x,  y2, 0);
						evenMidPoints << QVector3D(x,  y3, 0);
						evenMidPoints << QVector3D(x2, y3, 0);
						evenMidPoints << QVector3D(x2, y3, 0);
						evenMidPoints << QVector3D(x2, y2, 0);
						evenMidPoints << QVector3D(x,  y2, 0);
					} else {
						oddMidPoints << QVector3D(x, y2, 0);
						oddMidPoints << QVector3D(x, y3, 0);
						oddMidPoints << QVector3D(x2, y3, 0);
						oddMidPoints << QVector3D(x2, y3, 0);
						oddMidPoints << QVector3D(x2, y2, 0);
						oddMidPoints << QVector3D(x, y2, 0);
					}

					//shortTermMean bars
					y2 = 1 + toInt((data->shortTermMean + data->shortTermDeviation) / zoomY) - viewBottomOffset;
					y3 = 1 + toInt((data->shortTermMean - data->shortTermDeviation) / zoomY) - viewBottomOffset;
					if ((data->noteIndex % 2) == 0) {
						evenMidPoints2 << QVector3D(x, y2, 0);
						evenMidPoints2 << QVector3D(x, y3, 0);
						evenMidPoints2 << QVector3D(x2, y3, 0);
						evenMidPoints2 << QVector3D(x2, y3, 0);
						evenMidPoints2 << QVector3D(x2, y2, 0);
						evenMidPoints2 << QVector3D(x, y2, 0);
					} else {
						oddMidPoints2 << QVector3D(x, y2, 0);
						oddMidPoints2 << QVector3D(x, y3, 0);
						oddMidPoints2 << QVector3D(x2, y3, 0);
						oddMidPoints2 << QVector3D(x2, y3, 0);
						oddMidPoints2 << QVector3D(x2, y2, 0);
						oddMidPoints2 << QVector3D(x, y2, 0);
					}
				}
			}

			myassert(pitch >= 0.0 && pitch <= gdata->topPitch());
			y = 1 + toInt(pitch / zoomY) - viewBottomOffset;
			bottomPoints << QVector3D(x, 0, 0);
			bottomPoints << QVector3D(x, y, 0);

			prevPitch = pitch;
		}

		QVector<QVector3D> rect;
		rect << QVector3D(firstN, 0,        0);
		rect << QVector3D(firstN, height(), 0);
		rect << QVector3D(lastN,  0,        0);
		rect << QVector3D(lastN,  height(), 0);

		m_vao_all_fill.bind();
		m_vbo_all_fill.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_all_fill.bind();
		m_vbo_all_fill.allocate(rect.constData(), rect.count() * 3 * sizeof(float));
		MyGL::DrawShape(m_program, m_vao_all_fill, m_vbo_all_fill, rect.count(), GL_TRIANGLE_STRIP, gdata->shading1Color());

		if (bottomPoints.count() > 1) {
			m_vao_channel_fill.bind();
			m_vbo_channel_fill.setUsagePattern(QOpenGLBuffer::DynamicDraw);
			m_vbo_channel_fill.bind();
			m_vbo_channel_fill.allocate(bottomPoints.constData(), bottomPoints.count() * 3 * sizeof(float));
			MyGL::DrawShape(m_program, m_vao_channel_fill, m_vbo_channel_fill, bottomPoints.count(), GL_TRIANGLE_STRIP, gdata->shading2Color());
		}

		if (gdata->showMeanVarianceBars()) {
		     //shortTermMean bars
			if (evenMidPoints2.count() > 1) {
				m_vao_varianace2[VarGreen].bind();
				m_vbo_varianace2[VarGreen].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace2[VarGreen].bind();
				m_vbo_varianace2[VarGreen].allocate(evenMidPoints2.constData(), evenMidPoints2.count() * 3 * sizeof(float));
				MyGL::DrawShape(m_program, m_vao_varianace2[VarGreen], m_vbo_varianace2[VarGreen], evenMidPoints2.count(), GL_TRIANGLES, QColor(Qt::green));
			}
			if (oddMidPoints2.count() > 1) {
				m_vao_varianace2[VarYellow].bind();
				m_vbo_varianace2[VarYellow].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace2[VarYellow].bind();
				m_vbo_varianace2[VarYellow].allocate(oddMidPoints2.constData(), oddMidPoints2.count() * 3 * sizeof(float));
				MyGL::DrawShape(m_program, m_vao_varianace2[VarYellow], m_vbo_varianace2[VarYellow], oddMidPoints2.count(), GL_TRIANGLES, QColor(Qt::yellow));
			}

			//longTermMean bars
			QColor seeThroughYellow = Qt::yellow;
			seeThroughYellow.setAlpha(255);
			QColor seeThroughGreen = Qt::green;
			seeThroughGreen.setAlpha(255);
			if (evenMidPoints.count() > 1) {
				m_vao_varianace[VarYellow].bind();
				m_vbo_varianace[VarYellow].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace[VarYellow].bind();
				m_vbo_varianace[VarYellow].allocate(evenMidPoints.constData(), evenMidPoints.count() * 3 * sizeof(float));
				MyGL::DrawShape(m_program, m_vao_varianace[VarYellow], m_vbo_varianace[VarYellow], evenMidPoints.count(), GL_TRIANGLES, seeThroughYellow);
			}
			if (oddMidPoints.count() > 1) {
				m_vao_varianace[VarGreen].bind();
				m_vbo_varianace[VarGreen].setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_varianace[VarGreen].bind();
				m_vbo_varianace[VarGreen].allocate(oddMidPoints.constData(), oddMidPoints.count() * 3 * sizeof(float));
				MyGL::DrawShape(m_program, m_vao_varianace[VarGreen], m_vbo_varianace[VarGreen], oddMidPoints.count(), GL_TRIANGLES, seeThroughGreen);
			}
		}
	}
}

void FreqWidgetGL::drawReferenceLinesGL(QPainter &p, double viewBottom, double zoomY, int /*viewType*/)
{
  // Draw the lines and notes
	QFontMetrics fm = fontMetrics();
	int fontHeightSpace = fm.height() / 4;
	int fontWidth = fm.width("C#0") + 3;

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

	QVector<QVector3D> refRoot;
	QVector<QLineF> refAccidental;
	QVector<QLineF> refNote;

	//Draw the horizontal reference lines
	for (int octave = rootOctave; octave < topOctave; ++octave) {
		curRoot = double(octave * 12 + rootOffset);
		for (int j = 0; j < musicKey.size(); j++) {
			if (musicScale.hasSemitone(musicKey.noteType(j))) { //draw it
				curPitch = curRoot + musicKey.noteOffset(j);
				lineY = double(myround((curPitch - viewBottom) / zoomY));
				if (j == 0) { //root note
					refRoot << QVector3D(fontWidth,   lineY, 0);
					refRoot << QVector3D(width() - 1, lineY, 0);
				} else if ((gdata->musicKeyType() == MusicScale::Chromatic) && !gMusicScales[MusicScale::Major].hasSemitone(musicKey.noteType(j))) {
					refAccidental << QLine(fontWidth, height() - lineY, width() - 1, height() - lineY);
				} else {
					refNote << QLine(fontWidth, height() - lineY, width() - 1, height() - lineY);
				}
			}
			if (zoomY > 0.1) break;
		}
	}

	// Draw the lines for the root of the octave
	m_vao_ref_lines[RefRoot].bind();
	m_vbo_ref_lines[RefRoot].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ref_lines[RefRoot].bind();
	m_vbo_ref_lines[RefRoot].allocate(refRoot.constData(), refRoot.count() * 3 * sizeof(float));
	MyGL::DrawShape(m_program, m_vao_ref_lines[RefRoot], m_vbo_ref_lines[RefRoot], refRoot.count(), GL_LINES, QColor(Qt::black));

	p.endNativePainting();
	
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
				
				noteLabel.sprintf("%s%d", noteName(nameIndex), noteOctave(nameIndex));
				p.drawText(2, toInt(lineY) + fontHeightSpace, noteLabel);
			}
			if (zoomY > 0.1) break;
		}
	}
	// Restore pen
	p.setPen(oldPen);

	p.beginNativePainting();

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

	QVector<QVector3D> refVertDark;
	QVector<QVector3D> refVertLight;

	for (; timePos <= rightTime(); timePos += timeScaleBase) {
		x = toInt((timePos - lTime) * ratio);
		if (++largeCounter == largeFreq) {
			largeCounter = 0;
			refVertDark << QVector3D(x, 0, 0);
			refVertDark << QVector3D(x, height() - 1, 0);
		} else {
			refVertLight << QVector3D(x, 0, 0);
			refVertLight << QVector3D(x, height() - 1, 0);
		}
	}
	m_vao_ref_lines[RefVertDark].bind();
	m_vbo_ref_lines[RefVertDark].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ref_lines[RefVertDark].bind();
	m_vbo_ref_lines[RefVertDark].allocate(refVertDark.constData(), refVertDark.count() * 3 * sizeof(float));
	MyGL::DrawShape(m_program, m_vao_ref_lines[RefVertDark], m_vbo_ref_lines[RefVertDark], refVertDark.count(), GL_LINES, QColor(Qt::black));

	m_vao_ref_lines[RefVertLight].bind();
	m_vbo_ref_lines[RefVertLight].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_ref_lines[RefVertLight].bind();
	m_vbo_ref_lines[RefVertLight].allocate(refVertLight.constData(), refVertLight.count() * 3 * sizeof(float));
	MyGL::DrawShape(m_program, m_vao_ref_lines[RefVertLight], m_vbo_ref_lines[RefVertLight], refVertLight.count(), GL_LINES, QColor(25, 125, 170, 128));
}

void FreqWidgetGL::drawChannelGL(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType)
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

	QVector<QVector3D> lineVector;
	QVector<QVector4D> lineColorVector;
	QVector<QVector3D> pointVector;
	QVector<QVector4D> pointColorVector;

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
				lineVector << QVector3D(float(n), float((ze.low() / zoomY) - lineHalfWidth - viewBottomOffset), 0.0f);
				lineVector << QVector3D(float(n), float((ze.high() / zoomY) + lineHalfWidth - viewBottomOffset), 0.0f);
				lineColorVector << QVector4D(ze.color().red() / 255.0f, ze.color().green() / 255.0f, ze.color().blue() / 255.0f, ze.color().alpha() / 255.0f);
				lineColorVector << QVector4D(ze.color().red() / 255.0f, ze.color().green() / 255.0f, ze.color().blue() / 255.0f, ze.color().alpha() / 255.0f);
			}
		}
		m_vao_channel.bind();
		m_vbo_channel.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel.bind();
		m_vbo_channel.allocate(lineVector.constData(), lineVector.count() * 3 * sizeof(float));

		m_vbo_channel_color.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel_color.bind();
		m_vbo_channel_color.allocate(lineColorVector.constData(), lineColorVector.count() * 4 * sizeof(float));

		MyGL::DrawLine(m_program_line_color, m_vao_channel, m_vbo_channel, m_vbo_channel_color, lineVector.count(), GL_LINES, lineWidth);
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

			glLineWidth(lineWidth);
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
			y = 1 + toInt(pitch / zoomY) - viewBottomOffset;
			if (pitch > 0.0f) {
				if (fabs(prevPitch - pitch) < 1.0 && n != start) {
					//if closer than one semi-tone from previous then draw a line between them
					lineVector << QVector3D((float)penX, (float)penY, 0.0f);
					lineVector << QVector3D((float)x, (float)y, 0.0f);
					lineColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					lineColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					penX = x; penY = y;
				} else {
					penX = x; penY = y;
				}
				if (stepSize > 10) { //draw squares on the data points
					pointVector << QVector3D(x - halfSquareSize, y - halfSquareSize, 0.0f);
					pointVector << QVector3D(x - halfSquareSize, y + halfSquareSize, 0.0f);
					pointVector << QVector3D(x + halfSquareSize, y + halfSquareSize, 0.0f);

					pointVector << QVector3D(x + halfSquareSize, y + halfSquareSize, 0.0f);
					pointVector << QVector3D(x + halfSquareSize, y - halfSquareSize, 0.0f);
					pointVector << QVector3D(x - halfSquareSize, y - halfSquareSize, 0.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
					pointColorVector << QVector4D(lineColor.red() / 255.0f, lineColor.green() / 255.0f, lineColor.blue() / 255.0f, lineColor.alpha() / 255.0f);
				}
			}
			prevPitch = pitch;
		}
		m_vao_channel.bind();
		m_vbo_channel.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel.bind();
		m_vbo_channel.allocate(lineVector.constData(), lineVector.count() * 3 * sizeof(float));

		m_vbo_channel_color.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel_color.bind();
		m_vbo_channel_color.allocate(lineColorVector.constData(), lineColorVector.count() * 4 * sizeof(float));

		m_vao_channel_point.bind();
		m_vbo_channel_point.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel_point.bind();
		m_vbo_channel_point.allocate(pointVector.constData(), pointVector.count() * 3 * sizeof(float));

		m_vbo_channel_point_color.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_channel_point_color.bind();
		m_vbo_channel_point_color.allocate(pointColorVector.constData(), pointColorVector.count() * 4 * sizeof(float));

		MyGL::DrawLine(m_program_line_color, m_vao_channel, m_vbo_channel, m_vbo_channel_color, lineVector.count(), GL_LINES, lineWidth);
		MyGL::DrawShape(m_program_color, m_vao_channel_point, m_vbo_channel_point, m_vbo_channel_point_color, pointVector.count(), GL_TRIANGLES);
	}
}

void FreqWidgetGL::paintGL()
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
	glEnableClientState(GL_VERTEX_ARRAY);

	View* view = gdata->view;

	if (view->autoFollow() && gdata->getActiveChannel() && gdata->running == STREAM_FORWARD) {
		setChannelVerticalView(gdata->getActiveChannel(), view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY());
	}

	//draw the red/blue background color shading if needed
	if (view->backgroundShading() && gdata->getActiveChannel()) {
		drawChannelFilledGL(gdata->getActiveChannel(), view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);
	}

	drawReferenceLinesGL(p, view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);

	glEnable(GL_LINE_SMOOTH);
	//draw all the visible channels
	for (uint i = 0; i < gdata->channels.size(); i++) {
		Channel* ch = gdata->channels.at(i);
		if (!ch->isVisible()) continue;
		drawChannelGL(ch, view->viewLeft(), view->currentTime(), view->zoomX(), view->viewBottom(), view->zoomY(), DRAW_VIEW_NORMAL);
	}

	glDisable(GL_LINE_SMOOTH);

	// Draw a light grey band indicating which time is being used in the current window
	if (gdata->getActiveChannel()) {
		p.endNativePainting();
		QColor lineColor = palette().color(QPalette::Foreground);
		lineColor.setAlpha(50);
		Channel* ch = gdata->getActiveChannel();
		double halfWindowTime = (double)ch->size() / (double)(ch->rate() * 2);
		int pixelLeft = view->screenPixelX(view->currentTime() - halfWindowTime);
		int pixelRight = view->screenPixelX(view->currentTime() + halfWindowTime);
#ifdef DWS
		QVector<QVector3D> rect;
		rect << QVector3D(pixelLeft, 0.0f, 0.0f);
		rect << QVector3D(pixelLeft, height() - 1, 0.0f);
		rect << QVector3D(pixelRight, 0.0f, 0.0f);
		rect << QVector3D(pixelRight, height() - 1, 0.0f);

		m_vao_active_band.bind();
		m_vbo_active_band.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_active_band.bind();
		m_vbo_active_band.allocate(rect.constData(), rect.count() * 3 * sizeof(float));
		MyGL::DrawShape(m_program, m_vao_active_band, m_vbo_active_band, rect.count(), GL_TRIANGLE_STRIP, lineColor);
#endif
		p.setBrush(lineColor);
		p.drawRect(pixelLeft, 0, pixelRight - pixelLeft, height());
		p.beginNativePainting();
	}

	// Draw the current time line
	//int curTimePixel = view->screenPixelX(view->currentTime());
	double curScreenTime = (view->currentTime() - view->viewLeft()) / view->zoomX();

	QVector<QVector3D> timeLine;
	timeLine << QVector3D((float)curScreenTime, 0.0f, 0.0f);
	timeLine << QVector3D((float)curScreenTime, height() - 1, 0.0f);

	m_vao_time_line.bind();
	m_vbo_time_line.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_time_line.bind();
	m_vbo_time_line.allocate(timeLine.constData(), timeLine.count() * 3 * sizeof(float));
	MyGL::DrawShape(m_program, m_vao_time_line, m_vbo_time_line, timeLine.count(), GL_LINES, palette().color(QPalette::Foreground));
	
	p.endNativePainting();
	p.end();
}

Channel* FreqWidgetGL::channelAtPixel(int x, int y)
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
void FreqWidgetGL::mousePressEvent(QMouseEvent* e)
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

void FreqWidgetGL::mouseMoveEvent(QMouseEvent* e)
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

void FreqWidgetGL::mouseReleaseEvent(QMouseEvent*)
{
	dragMode = DragNone;
}

/**
 Calculates at what time the mouse is.
 @param x the mouse's x co-ordinate
 @return the time the mouse is positioned at.
 */
double FreqWidgetGL::mouseTime(int x)
{
	return gdata->view->viewLeft() + gdata->view->zoomX() * x;

}

/**
 Calculates at what note pitch the mouse is at.
 @param x the mouse's y co-ordinate
 @return the pitch the mouse is positioned at.
 */
double FreqWidgetGL::mousePitch(int y)
{
	return gdata->view->viewBottom() + gdata->view->zoomY() * (height() - y);
}

void FreqWidgetGL::wheelEvent(QWheelEvent* e)
{
	View* view = gdata->view;
	double amount = double(e->delta()) / WHEEL_DELTA * 0.15;
	bool isZoom = gdata->mouseWheelZooms();
	if (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) isZoom = !isZoom;

	if (isZoom) {
		if (e->delta() >= 0) { //zooming in
			double before = view->logZoomY();
			view->setZoomFactorY(view->logZoomY() + amount, height() - e->y());
			amount = view->logZoomY() - before;
			if (gdata->running == STREAM_FORWARD) {
				view->setZoomFactorX(view->logZoomX() + amount);
			} else { //zoom toward mouse pointer
				view->setZoomFactorX(view->logZoomX() + amount, e->x());
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
void FreqWidgetGL::keyPressEvent(QKeyEvent* k)
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
void FreqWidgetGL::keyReleaseEvent(QKeyEvent* k)
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

void FreqWidgetGL::leaveEvent(QEvent* e)
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
bool FreqWidgetGL::calcZoomElement(Channel* ch, ZoomElement& ze, int baseElement, double baseX)
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

void FreqWidgetGL::setChannelVerticalView(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY)
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
