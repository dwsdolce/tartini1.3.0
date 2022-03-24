/***************************************************************************
						  tunerwidgetGL.cpp  -  description
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
#include "tunerwidgetGL.h"
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QDebug>
#include "shader.h"
#include "mygl.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "musicnotes.h"
#include <QPainter>

TunerWidgetGL::TunerWidgetGL(QWidget* parent)
	: QOpenGLWidget(parent)
{
	needleValueToDraw = -999;
	prevNeedleValue = -999;
	prevClosePitch = 0;
	curPitch = 0.0;
	tunerLabelCounter = 0;
	for (int i = 0; i < 100; i++) {
		tunerLabels[i].label = QString(8, ' ');
		tunerLabels[i].x = 0.0f;
		tunerLabels[i].y = 0.0f;
	}
	tunerFont = QFont();
	tunerFont.setPointSize(9);
}

TunerWidgetGL::~TunerWidgetGL()
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
	m_program_line.deleteLater();
	doneCurrent();
}

void TunerWidgetGL::initializeGL()
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
	m_vao_needle.create();
	m_vbo_needle.create();
	m_vao_dial[VBO_DIAL].create();
	m_vao_dial[VBO_ARC].create();
	m_vao_dial[VBO_MARKERS].create();
	m_vbo_dial[VBO_DIAL].create();
	m_vbo_dial[VBO_ARC].create();
	m_vbo_dial[VBO_MARKERS].create();
}

// Resize creates a new Dial composed of:
//		dial (VBO_DIAL)				- triangle_fan 
//		dial_outline (VBO_DIAL)	- line
//		dial_arc (VBO_ARC)			- line
//		label_markers (VBO_MARKERS)	- line
// We need a separate vao/vbo for each of these (3 in total).
void TunerWidgetGL::resizeGL(int w, int h)
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

	// Calculate the dial with the new width & height

	// The following calculates in 0 to width and 0 to height in X and Y
	const float halfWidth = 0.5 * width();
	const float halfHeight = 0.5 * height();
	const float radius = 1.8 * MAX(halfHeight, halfWidth);
	const float theta = asin(float(width()) / (2 * radius));
	const float rho = 0.5 * PI - theta;
	const float centerX = halfWidth;
	const float centerY = height() - radius;

	// Calculate the white semicircle of the dial and the black outline of the dial
	float upperArcTheta = -theta;
	float upperArcThetaStep = (2 * theta) / width();
	QVector<QVector3D> upperArc;
	upperArc << QVector3D((float)centerX, (float)centerY, 0.0);
	for (int x = 0; x < width(); x++) {
		upperArc << QVector3D((float)x , (float)(centerY + (sin(0.5 * PI - upperArcTheta) * radius)), 0.0f);
		upperArcTheta += upperArcThetaStep;
	}
	m_vao_dial[VBO_DIAL].bind();
	m_vbo_dial[VBO_DIAL].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_DIAL].bind();
	m_vbo_dial[VBO_DIAL].allocate(upperArc.constData(), upperArc.count() * 3 * sizeof(float));
	m_vao_dial[VBO_DIAL].release();
	m_vbo_dial[VBO_DIAL].release();
	m_dial_count[VBO_DIAL] = upperArc.count();

	// Calculate the dial arc
	float lowerArcTheta = -theta;
	float lowerArcThetaStep = (2 * theta) / halfWidth;
	QVector<QVector3D> lowerArc;
	for (float x = 0.25 * width(); x < 0.75 * width(); x++) {
		lowerArc << QVector3D(x, centerY + (0.5 * sin(0.5 * PI - lowerArcTheta) * radius), 0);
		lowerArcTheta += lowerArcThetaStep;
	}
	m_vao_dial[VBO_ARC].bind();
	m_vbo_dial[VBO_ARC].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_ARC].bind();
	m_vbo_dial[VBO_ARC].allocate(lowerArc.constData(), lowerArc.count() * 3 * sizeof(float));
	m_vao_dial[VBO_ARC].release();
	m_vbo_dial[VBO_ARC].release();
	m_dial_count[VBO_ARC] = lowerArc.count();

	// Calculate line markings & text labels
	float step = (2 * theta) / 12.0;
	tunerLabelCounter = 0;
	const char* tunerLabelLookup[11] = { "+50   ", "+40", "+30", "+20", "+10", "0",
								   "-10", "-20", "-30", "-40", "   -50" };
	QVector<QVector3D> lineMarking;
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
	m_vao_dial[VBO_MARKERS].bind();
	m_vbo_dial[VBO_MARKERS].setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_vbo_dial[VBO_MARKERS].bind();
	m_vbo_dial[VBO_MARKERS].allocate(lineMarking.constData(), lineMarking.count() * 3 * sizeof(float));
	m_vao_dial[VBO_MARKERS].release();
	m_vbo_dial[VBO_MARKERS].release();
	m_dial_count[VBO_MARKERS] = lineMarking.count();

	// Calculate the position of the "Cents" label
	centsLabelX = centerX;
	centsLabelY = centerY + 0.7 * radius;

	// Do forced update on resize
	needleValueToDraw = -999;
	prevNeedleValue = -999;
	prevClosePitch = -1;
	doUpdate(curPitch);
}

void TunerWidgetGL::paintGL()
{
	QColor bg = gdata->backgroundColor();
	glClearColor(double(bg.red()) / 255.0, double(bg.green()) / 255.0, double(bg.blue()) / 255.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	QPainter p;
	p.begin(this);

	p.beginNativePainting();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw each of the dial VBOs.
	// Dial Face
	MyGL::DrawShape(m_program, m_vao_dial[VBO_DIAL], m_vbo_dial[VBO_DIAL], m_dial_count[VBO_DIAL], GL_TRIANGLE_FAN, QColor(Qt::white));

    // Draw the Dial Outline using a line width
	MyGL::DrawLine(m_program_line, m_vao_dial[VBO_DIAL], m_vbo_dial[VBO_DIAL], m_dial_count[VBO_DIAL], GL_LINE_LOOP, 3.0, QColor(Qt::black));

	// Lower Arc with line width
	MyGL::DrawLine(m_program_line, m_vao_dial[VBO_ARC], m_vbo_dial[VBO_ARC], m_dial_count[VBO_ARC], GL_LINE_STRIP, 3.0, QColor(Qt::black));

	// Line Markings with line width
	MyGL::DrawLine(m_program_line, m_vao_dial[VBO_MARKERS], m_vbo_dial[VBO_MARKERS], m_dial_count[VBO_MARKERS], GL_LINES, 3.0, QColor(Qt::black));

	p.endNativePainting();

	// Draw the labels
	QFontMetrics fm = QFontMetrics(tunerFont);
	setFont(tunerFont);
	// Take into account that QPainter origin is upper left while OpenGL is lower left.
	p.drawText(centsLabelX - (0.5 * fm.width("Cents")), height() - centsLabelY, "Cents");

	for (int i = 0; i < tunerLabelCounter; i++) {
		// Take into account that QPainter origin is upper left while OpenGL is lower left.
		p.drawText(tunerLabels[i].x - (0.5 * fm.width(tunerLabels[i].label)), height() - (tunerLabels[i].y - 8), tunerLabels[i].label);
	}

	p.beginNativePainting();
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (m_draw_needle) {
		// Draw the needle inside
		MyGL::DrawShape(m_program, m_vao_needle, m_vbo_needle, m_needle_count, GL_TRIANGLES, QColor(Qt::red));
		// Draw the needle outside with a line width
		MyGL::DrawLine(m_program_line, m_vao_needle, m_vbo_needle, m_needle_count, GL_LINE_LOOP, 3.0, QColor(Qt::darkRed));
	}
	p.endNativePainting();

	p.end();
}

void TunerWidgetGL::doUpdate(double thePitch)
{
	curPitch = thePitch;

	float needleValue = 0;
	int closePitch = 0;

	closePitch = toInt(thePitch);
	needleValue = 100 * (thePitch - float(closePitch));

	if ((fabs(prevNeedleValue - needleValue) < 0.05) && (prevClosePitch == closePitch)) {
		// Pitch hasn't changed (much), no update needed

	} else {
		// Pitch has changed
		if (closePitch == 0) {
		    // No pitch, don't draw the needle this update
			m_draw_needle = false;

			prevNeedleValue = -999;
			prevClosePitch = 0;

			needleValueToDraw = -999;

			resetLeds();
		} else {
			// Pitch, draw the needle this update
			QVector<QVector3D> needleVectors;

			int VTLEDLetterLookup[12] = { 2, 2, 3, 3, 4, 5, 5, 6, 6, 0, 0, 1 };
			resetLeds();
			emit(ledSet(VTLEDLetterLookup[noteValue(closePitch)], true));
			if (isBlackNote(closePitch)) {
				emit(ledSet(7, true));
			}

			prevNeedleValue = needleValue;
			prevClosePitch = closePitch;

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
				needleVectors << QVector3D(noteX, noteY, 0);
				needleVectors << QVector3D(knobLeftX, knobLeftY, 0);
				needleVectors << QVector3D(knobRightX, knobRightY, 0);

				m_vao_needle.bind();
				m_vbo_needle.setUsagePattern(QOpenGLBuffer::DynamicDraw);
				m_vbo_needle.bind();
				m_vbo_needle.allocate(needleVectors.constData(), needleVectors.count() * 3 * sizeof(float));
				m_vao_needle.release();
				m_vbo_needle.release();
				m_needle_count = needleVectors.count();

				m_draw_needle = true;
			} else {
				m_draw_needle = false;
			}
		}
	}
	update();
}

void TunerWidgetGL::resetLeds()
{
	for (int i = 0; i < 8; i++) {
		emit(ledSet(i, false));
	}
}
