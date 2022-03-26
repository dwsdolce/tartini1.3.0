/***************************************************************************
						  freqwidget.h  -  description
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

#ifndef FREQWIDGET_H
#define FREQWIDGET_H

#include "drawwidget.h"

#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QPaintEvent>

#include "gdata.h"
#include "channel.h"

#define DRAW_VIEW_NORMAL   0
#define DRAW_VIEW_SUMMARY  1
#define DRAW_VIEW_PRINT    2

class FreqWidget : public DrawWidget
	{

	Q_OBJECT

public:
	enum DragModes {
		DragNone = 0,
		DragChannel = 1,
		DragBackground = 2,
		DragTimeBar = 3
	};

	FreqWidget(QWidget* parent);

	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);

	void drawReferenceLines(double viewBottom, double zoomY, int viewType);
	void drawChannel(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType);
	void drawChannelFilled(Channel* ch, double leftTime, double currentTime, double zoomX, double viewBottom, double zoomY, int viewType);
	bool calcZoomElement(Channel* ch, ZoomElement& ze, int baseElement, double baseX);

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
};

#endif
