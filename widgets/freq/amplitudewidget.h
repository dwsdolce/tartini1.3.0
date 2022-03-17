/***************************************************************************


						  widget.h  -  description
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

#ifndef AMPLITUDEWIDGET_H
#define AMPLITUDEWIDGET_H

#ifdef DWS
#include <QCoreApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#endif
#include "drawwidget.h"
#include <QPixmap>

#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QPaintEvent>

#include "channel.h"

class AnalysisData;

#ifdef DWS
class AmplitudeWidget : public QOpenGLWidget, protected QOpenGLFunctions {
#endif
class AmplitudeWidget : public DrawWidget
	{

	Q_OBJECT

public:
	enum DragModes {
		DragNone = 0,
		DragChannel = 1,
		DragBackground = 2,
		DragTimeBar = 3,
		DragNoiseThreshold = 4
	};

	AmplitudeWidget(QWidget* parent);
	virtual ~AmplitudeWidget();

#ifdef DWS
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
#endif
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);

	void drawVerticalRefLines();
	bool calcZoomElement(ZoomElement& ze, Channel* ch, int baseElement, double baseX);
	double calculateElement(AnalysisData* data);
	double getCurrentThreshold(int index);
	void setCurrentThreshold(double newThreshold, int index);
	QString getCurrentThresholdString();
	void drawChannelAmplitude(Channel* ch);
	void drawChannelAmplitudeFilled(Channel* ch);
	QSize sizeHint() const {
		return QSize(400, 100);
	}
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	double range() {
		return _range;
	}
	double maxOffset() {
		return 1.0 - range();
	}
	double offset() {
		return _offset;
	}
	double offsetInv() {
		return _offsetInv;
	}

public slots:
	void setRange(double newRange);
	void setRangeQWTWheel(double newRange);
	void setOffset(double newOffset);

signals:
	void rangeChanged(double);
	void offsetChanged(double);
	void offsetInvChanged(double);

private:
	double leftTime() {
		return gdata->view->viewLeft();
	}
	double rightTime() {
		return gdata->view->viewRight();
	}
	double timeWidth() {
		return gdata->view->viewTotalTime();
	}

	int dragMode;
	int mouseX, mouseY;
	double downTime, downNote;
	double downOffset;
	int thresholdIndex;

	QPixmap* buffer;
	double _range;
	double _offset;
	double _offsetInv;
	float lineWidth;
	float halfLineWidth;

#ifdef DWS
	QOpenGLShaderProgram m_program;
	QOpenGLShaderProgram m_program_line;

	QOpenGLVertexArrayObject m_vao_ch_amp;
	QOpenGLBuffer m_vbo_ch_amp;

	QOpenGLVertexArrayObject m_vao_ref_dark;
	QOpenGLBuffer m_vbo_ref_dark;

	QOpenGLVertexArrayObject m_vao_ref_light;
	QOpenGLBuffer m_vbo_ref_light;

	QOpenGLVertexArrayObject m_vao_time_line;
	QOpenGLBuffer m_vbo_time_line;

	QOpenGLVertexArrayObject m_vao_blk_ref;
	QOpenGLBuffer m_vbo_blk_ref;

	QOpenGLVertexArrayObject m_vao_red_ref;
	QOpenGLBuffer m_vbo_red_ref;
#endif
};

#endif
