/***************************************************************************
						  freqwidgetGL.h  -  description
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

#ifndef FREQWIDGETGL_H
#define FREQWIDGETGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QPaintDevice>

#include "gdata.h"
#include "channel.h"

#define DRAW_VIEW_NORMAL   0
#define DRAW_VIEW_SUMMARY  1
#define DRAW_VIEW_PRINT    2

class FreqWidgetGL : public QOpenGLWidget, protected QOpenGLFunctions {

	Q_OBJECT

public:
	enum DragModes {
		DragNone = 0,
		DragChannel = 1,
		DragBackground = 2,
		DragTimeBar = 3
	};

	FreqWidgetGL();
	virtual ~FreqWidgetGL();

	void initializeGL();
	void resizeGL(int w, int h);
	void drawReferenceLinesGL(QPainter& p, double viewBottom, double zoomY, int viewType);
	void drawChannelGL(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType);
	void drawChannelFilledGL(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType);
	bool calcZoomElement(Channel* ch, ZoomElement& ze, int baseElement, double baseX);
	void paintGL();
	QSize sizeHint() const {
		return QSize(400, 350);
	}

private:
	int dragMode;
	int mouseX = 0, mouseY = 0;
	double downTime = 0.0, downNote = 0.0;

	void keyPressEvent(QKeyEvent* k);
	void keyReleaseEvent(QKeyEvent* k);
	void leaveEvent(QEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	void resizeEvent(QResizeEvent* q);

	double mouseTime(int x);
	double mousePitch(int y);
	Channel* channelAtPixel(int x, int y);
	void setChannelVerticalView(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY);

	double leftTime() {
		return gdata->view->viewLeft();
	}
	double rightTime() {
		return gdata->view->viewRight();
	}
	double timeWidth() {
		return gdata->view->viewTotalTime();
	}

	QOpenGLShaderProgram m_program;
	QOpenGLShaderProgram m_program_color;
	QOpenGLShaderProgram m_program_line;
	QOpenGLShaderProgram m_program_line_color;

	// area fill
	QOpenGLVertexArrayObject m_vao_all_fill;
	QOpenGLBuffer m_vbo_all_fill;

	// area under channel
	QOpenGLVertexArrayObject m_vao_channel_fill;
	QOpenGLBuffer m_vbo_channel_fill;

	// yellow and green variances for channel
	enum VarianceTypes
	{
		VarYellow = 0,
		VarGreen = 1,
	};
	QOpenGLVertexArrayObject m_vao_varianace[2];
	QOpenGLBuffer m_vbo_varianace[2];
	QOpenGLVertexArrayObject m_vao_varianace2[2];
	QOpenGLBuffer m_vbo_varianace2[2];

	// Horizontal and vertical reference lines
	enum RefTypes
	{
		RefRoot = 0,
		RefVertDark = 1,
		RefVertLight = 2
	};
	QOpenGLVertexArrayObject m_vao_ref_lines[3];
	QOpenGLBuffer m_vbo_ref_lines[3];

	// channel and rectangles on data points
	QOpenGLVertexArrayObject m_vao_channel;
	QOpenGLBuffer m_vbo_channel;
	QOpenGLBuffer m_vbo_channel_color;
	QOpenGLVertexArrayObject m_vao_channel_point;
	QOpenGLBuffer m_vbo_channel_point;
	QOpenGLBuffer m_vbo_channel_point_color;

	QOpenGLVertexArrayObject m_vao_active_band;
	QOpenGLBuffer m_vbo_active_band;

	QOpenGLVertexArrayObject m_vao_time_line;
	QOpenGLBuffer m_vbo_time_line;
};

#endif
