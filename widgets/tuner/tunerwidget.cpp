/***************************************************************************
						  tunerwidget.cpp  -  description
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
#include "tunerwidget.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include "shader.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "musicnotes.h"
#include <QPainter>

TunerWidget::TunerWidget(QWidget* parent)
	: QOpenGLWidget(parent)
{
	needleValueToDraw = -999;
	prevNeedleValue = -999;
	prevClosePitch = 0;
	curPitch = 0.0;
	// DWS curPitch = 55.0;
	tunerLabelCounter = 0;
	for (int i = 0; i < 100; i++) {
		tunerLabels[i].label = QString(8, ' ');
		tunerLabels[i].x = 0.0f;
		tunerLabels[i].y = 0.0f;
	}
	tunerFont = QFont();
	tunerFont.setPointSize(9);
}

TunerWidget::~TunerWidget()
{
  // Remove display lists
	QOpenGLContext* c = QOpenGLContext::currentContext();
	if (c) {
		makeCurrent();
	}

	m_vao_dial[VBO_DIAL].destroy();
	m_vao_dial[VBO_ARC].destroy();
	m_vao_dial[VBO_MARKERS].destroy();
	m_vao_needle.destroy();
	m_vbo_dial[VBO_DIAL].destroy();
	m_vbo_dial[VBO_ARC].destroy();
	m_vbo_needle.destroy();

	m_program.deleteLater();
	doneCurrent();
}

void TunerWidget::initializeGL()
{
	initializeOpenGLFunctions();

	// build and compile our shader program
	// ------------------------------------
	try {
		Shader ourShader(&m_program, ":/shader.vs.glsl", ":/shader.fs.glsl");
	} catch (...) {
		close();
	}

	m_vao_needle.create();
	m_vbo_needle.create();
	m_vao_dial[VBO_DIAL].create();
	m_vao_dial[VBO_ARC].create();
	m_vao_dial[VBO_MARKERS].create();
	m_vbo_dial[VBO_DIAL].create();
	m_vbo_dial[VBO_ARC].create();
	m_vbo_dial[VBO_MARKERS].create();

	// Initialize so that no needle is drawn at the start.
	m_vbo_needle_count = 0;
}

// Resize creates a new Dial composed of:
//		dial (VBO_DIAL)				- triangle_fan 
//		dial_outline (VBO_DIAL)	- line
//		dial_arc (VBO_ARC)			- line
//		label_markers (VBO_MARKERS)	- line
// We need a separate vao/vbo for each of these (3 in total).


void TunerWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, (GLint)w, (GLint)h);

	// Create model transformation matrix to go from:
	//		x: 0 to width
	//		y: 0 to height
	//	to:
	//		x: -1.0f to 1.0f
	//		y: -1.0f to 1.0f
	m_model.setToIdentity();
	m_model.translate(QVector3D(-1.0f, -1.0f, 0.0f));
	m_model.scale(2.0f / width(), 2.0f / height(), 1.0f);

	// Calculate the dial with the new width & height

	// The following calculates in 0 to width and 0 to height in X and Y
	const float halfWidth = 0.5 * width();
	const float halfHeight = 0.5 * height();
	const float radius = 1.8 * MAX(halfHeight, halfWidth);
	const float theta = asin(float(width()) / (2 * radius));
	const float rho = 0.5 * PI - theta;
	const float centerX = halfWidth;
	const float centerY = height() - radius;
	
	m_program.bind();

	// Calculate the white semicircle of the dial and the black outline of the dial
	QVector<QVector3D> upperArc;
	float upperArcTheta = -theta;
	float upperArcThetaStep = (2 * theta) / width();
	upperArc << QVector3D((float)centerX, (float)centerY, 0.0);
	for (int x = 0; x < width(); x++) {
		upperArc << QVector3D((float)x , (float)(centerY + (sin(0.5 * PI - upperArcTheta) * radius)), 0.0f);
		upperArcTheta += upperArcThetaStep;
	}
	m_vbo_dial_count[VBO_DIAL] = upperArc.count();
	m_vao_dial[VBO_DIAL].bind();
	m_vbo_dial[VBO_DIAL].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_DIAL].bind();
	m_vbo_dial[VBO_DIAL].allocate(upperArc.constData(), upperArc.count()*3*sizeof(float));
	m_program.enableAttributeArray(0);
	m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

	// Calculate the dial
	QVector<QVector3D> lowerArc;
	float lowerArcTheta = -theta;
	float lowerArcThetaStep = (2 * theta) / halfWidth;
	for (float x = 0.25 * width(); x < 0.75 * width(); x++) {
		lowerArc << QVector3D(x, centerY + (0.5 * sin(0.5 * PI - lowerArcTheta) * radius), 0);
		lowerArcTheta += lowerArcThetaStep;
	}
	m_vbo_dial_count[VBO_ARC] = lowerArc.count();
	m_vao_dial[VBO_ARC].bind();
	m_vbo_dial[VBO_ARC].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_ARC].bind();
	m_vbo_dial[VBO_ARC].allocate(lowerArc.constData(), lowerArc.count() * 3 * sizeof(float));
	m_program.enableAttributeArray(0);
	m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

	// Calculate line markings & text labels
	QVector<QVector3D> lineMarking;
	float step = (2 * theta) / 12.0;
	tunerLabelCounter = 0;
	const char* tunerLabelLookup[11] = { "+50   ", "+40", "+30", "+20", "+10", "0",
								   "-10", "-20", "-30", "-40", "   -50" };

	for (int j = 0; j < 11; j++) {
		float i = rho + step * (j + 1);
		float startX = centerX + radius * cos(i);
		float startY = centerY + radius * sin(i);
		if (width() < 350) {  // Small dial
			if ((j == 0) || (j == 3) || (j == 5) || (j == 7) || (j == 10)) {
			    // Bigger marking + text label
				lineMarking << QVector3D(startX, startY, 0.0);
				lineMarking << QVector3D(startX + 0.05 * (centerX - startX), startY + 0.05 * (centerY - startY), 0.0);

				tunerLabels[tunerLabelCounter].x = startX + 0.08 * (centerX - startX);
				tunerLabels[tunerLabelCounter].y = startY + 0.08 * (centerY - startY);
				tunerLabels[tunerLabelCounter++].label = tunerLabelLookup[j];
			} else {
			  // Smaller marking, no text label
				lineMarking << QVector3D(startX, startY, 0.0);
				lineMarking << QVector3D(startX + 0.03 * (centerX - startX), startY + 0.03 * (centerY - startY), 0.0);
			}
		} else {  // Big dial
		    // Bigger marking + text label
			tunerLabelLookup[0] = "+50";
			tunerLabelLookup[10] = "-50";
			lineMarking << QVector3D(startX, startY, 0.0);
			lineMarking << QVector3D(startX + 0.05 * (centerX - startX), startY + 0.05 * (centerY - startY), 0.0);

			tunerLabels[tunerLabelCounter].x = startX + 0.08 * (centerX - startX);
			tunerLabels[tunerLabelCounter].y = startY + 0.08 * (centerY - startY);
			tunerLabels[tunerLabelCounter++].label = tunerLabelLookup[j];
		}
	}
	m_vbo_dial_count[VBO_MARKERS] = lineMarking.count();
	m_vao_dial[VBO_MARKERS].bind();
	m_vbo_dial[VBO_MARKERS].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_MARKERS].bind();
	m_vbo_dial[VBO_MARKERS].allocate(lineMarking.constData(), lineMarking.count() * 3 * sizeof(float));
	m_program.enableAttributeArray(0);
	m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

	// Calculate the position of the "Cents" label
	centsLabelX = centerX;
	centsLabelY = centerY + 0.7 * radius;

	// Do forced update on resize
	needleValueToDraw = -999;
	prevNeedleValue = -999;
	prevClosePitch = -1;
	doUpdate(curPitch);
}

void TunerWidget::paintGL()
{
	QColor bg = gdata->backgroundColor();
	glClearColor(double(bg.red()) / 256.0, double(bg.green()) / 256.0, double(bg.blue()) / 256.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_program.bind();

	// Draw the dial
	glLineWidth(1.5);

	m_program.setUniformValue("model", m_model);

	// Draw each of the dial VBOs.
	// Dial Face
	m_vao_dial[VBO_DIAL].bind();
	m_program.setUniformValue("color", QVector3D(255 / 255.0f, 255 / 255.0f, 255 / 255.0f));
	glDrawArrays(GL_TRIANGLE_FAN, 0, m_vbo_dial_count[VBO_DIAL]);
	// Dial Outline
	m_program.setUniformValue("color", QVector3D(0, 0, 0));
	glDrawArrays(GL_LINE_LOOP, 0, m_vbo_dial_count[VBO_DIAL]);

	// Lower Arc
	m_vao_dial[VBO_ARC].bind();
	m_program.setUniformValue("color", QVector3D(0, 0, 0));
	glDrawArrays(GL_LINE_STRIP, 0, m_vbo_dial_count[VBO_ARC]);

	// Line Markings
	m_vao_dial[VBO_MARKERS].bind();
	m_program.setUniformValue("color", QVector3D(0, 0, 0));
	glDrawArrays(GL_LINES, 0, m_vbo_dial_count[VBO_MARKERS]);

	// Draw the needle inside
	//glLineWidth(1.5);

	if (m_vbo_needle_count != 0) {
		m_vao_needle.bind();
		m_vbo_needle.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		m_vbo_needle.bind();
		m_vbo_needle.allocate(m_needleVectors.constData(), m_needleVectors.count() * 3 * sizeof(float));
		m_program.enableAttributeArray(0);
		m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

		// Needle Inside
		m_vao_needle.bind();
		// Needle Inside
		m_program.setUniformValue("color", QVector3D(255 / 255.0f, 0, 0));
		glDrawArrays(GL_TRIANGLES, 0, m_vbo_needle_count);

		// Needle Outside;
		m_program.setUniformValue("color", QVector3D(127 / 255.0f, 0, 0));
		glDrawArrays(GL_LINE_LOOP, 0, m_vbo_needle_count);
	}
	m_program.release();

	QPainter p;
	p.begin(this);

	// Draw the labels
	QFontMetrics fm = QFontMetrics(tunerFont);
	setFont(tunerFont);
	// Take into account that QPainter origin is upper left while OpenGL is lower left.
	p.drawText(centsLabelX - (0.5 * fm.width("Cents")), height() - centsLabelY, "Cents");

	for (int i = 0; i < tunerLabelCounter; i++) {
		// Take into account that QPainter origin is upper left while OpenGL is lower left.
		p.drawText(tunerLabels[i].x - (0.5 * fm.width(tunerLabels[i].label)), height() - (tunerLabels[i].y - 8), tunerLabels[i].label);
	}
	p.end();
}

void TunerWidget::doUpdate(double thePitch)
{
	curPitch = thePitch;

	Channel* active = gdata->getActiveChannel();

	float needleValue = 0;
	int closePitch = 0;
	bool isPitchBlackNote = true;

	if (active) {
		AnalysisData* data = active->dataAtCurrentChunk();
		if (data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
			NoteData* note = new NoteData();
			note = &(active->noteData[data->noteIndex]);
		}
	}

	closePitch = toInt(thePitch);
	isPitchBlackNote = isBlackNote(closePitch);
	needleValue = 100 * (thePitch - float(closePitch));

	if ((fabs(prevNeedleValue - needleValue) < 0.05) && (prevClosePitch == closePitch)) {
		// Pitch hasn't changed (much), no update needed

	} else {
		// Pitch has changed
		if (closePitch == 0) {
		    // No pitch, don't draw the needle this update
			m_vbo_needle_count = 0;

			prevNeedleValue = -999;
			prevClosePitch = 0;

			needleValueToDraw = -999;

			resetLeds();
		} else {
			// Pitch, draw the needle this update
			m_needleVectors.clear();

			int VTLEDLetterLookup[12] = { 2, 2, 3, 3, 4, 5, 5, 6, 6, 0, 0, 1 };
			resetLeds();
			emit(ledSet(VTLEDLetterLookup[noteValue(closePitch)], true));
			if (isBlackNote(closePitch)) {
				emit(ledSet(7, true));
			}

			prevNeedleValue = needleValue;
			prevClosePitch = closePitch;
			prevIsPitchBlackNote = isPitchBlackNote;

			needleValueToDraw = needleValue;

			// Calculate the needle, if needleValueToDraw is set to sensible value
			if ((needleValueToDraw > -60.0) && (needleValueToDraw < 60.0)) {
				const float halfWidth = 0.5 * width();
				const float halfHeight = 0.5 * height();
				const float radius = 1.8 * MAX(halfHeight, halfWidth);
				const float theta = asin(float(width()) / (2 * radius));
				const float rho = (PI * 0.5) - theta;
				const float centerX = halfWidth;
				const float centerY = height() - radius;

				float centAngle = (2 * theta) / 120;
				float note = rho + (fabs(needleValueToDraw - 60) * centAngle);
				int halfKnobWidth = MAX(toInt(radius * 0.02), 1);

				float noteX = centerX + radius * cos(note);
				float noteY = centerY + radius * sin(note);
				float knobLeftX = centerX - halfKnobWidth;
				float knobLeftY = centerY;
				float knobRightX = centerX + halfKnobWidth;
				float knobRightY = centerY;

				// Inside and outside of the needle
				m_needleVectors << QVector3D(noteX, noteY, 0);
				m_needleVectors << QVector3D(knobLeftX, knobLeftY, 0);
				m_needleVectors << QVector3D(knobRightX, knobRightY, 0);

				m_vbo_needle_count = m_needleVectors.count();
			} else {
				m_vbo_needle_count = 0;
			}
		}
	}
	update();
}

void TunerWidget::resetLeds()
{
	for (int i = 0; i < 8; i++) {
		emit(ledSet(i, false));
	}
}
