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

#include "drawwidget.h"

#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QPaintEvent>

#include "channel.h"

class AnalysisData;

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

	void paintEvent(QPaintEvent* event);

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
};

#endif
