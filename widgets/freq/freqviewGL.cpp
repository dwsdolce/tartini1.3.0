/***************************************************************************
						  freqviewGL.cpp  -  description
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

#include "freqviewGL.h"
#include "view.h"
#include "gdata.h"
#include "myscrollbar.h"
#include "freqwidgetGL.h"
#include "amplitudewidgetGL.h"
#include "timeaxis.h"
#include "mainwindow.h"
#include "analysisdata.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qsizegrip.h>
#include <qsplitter.h>
#include <qtooltip.h>
#include <qwt_wheel.h>
#include <qcursor.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

FreqViewGL::FreqViewGL(int viewID_, QWidget* parent)
	: ViewWidget(viewID_, parent)
{
	View* view = gdata->view;

	QBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetNoConstraint);

	QSplitter* splitter = new QSplitter(Qt::Vertical, this);
	QWidget* topWidget = new QWidget(splitter);
	QBoxLayout* topLayout = new QHBoxLayout(topWidget);

	QBoxLayout* topLeftLayout = new QVBoxLayout();
	topLayout->addLayout(topLeftLayout);

	timeAxis = new TimeAxis(topWidget, gdata->leftTime(), gdata->rightTime(), true);
	timeAxis->setWhatsThis("The time in seconds");
	topLeftLayout->addWidget(timeAxis);

	QFrame* freqFrame = new QFrame;
	freqFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QVBoxLayout* freqFrameLayout = new QVBoxLayout;

	freqWidgetGL = new FreqWidgetGL();
	freqWidgetGL->setWhatsThis("The line represents the musical pitch of the sound. A higher pitch moves up, with note names shown at the left, with octave numbers. "
		"The black vertical line shows the current time. This line's position can be moved. "
		"Pitch-lines are drawn connected only because they change a small amount over a small time step. "
		"Note: This may cause semi-tone note changes appear to be joined. Clicking the background and dragging moves the view. "
		"Clicking a pitch-line selects it. Mouse-Wheel scrolls. Shift-Mouse-Wheel zooms");
	freqFrameLayout->addWidget(freqWidgetGL);

	freqFrameLayout->setContentsMargins(0, 0, 0, 0);
	freqFrameLayout->setSpacing(0);
	freqFrame->setLayout(freqFrameLayout);
	topLeftLayout->addWidget(freqFrame);


	QVBoxLayout* topRightLayout = new QVBoxLayout();

	freqWheelY = new QwtWheel(topWidget);
	freqWheelY->setOrientation(Qt::Vertical);
	freqWheelY->setWheelWidth(14);
	freqWheelY->setRange(1.6, 5.0);
	freqWheelY->setSingleStep(0.001);
	freqWheelY->setValue(view->logZoomY());
	freqWheelY->setToolTip("Zoom pitch contour view vertically");
	topRightLayout->addSpacing(20);
	topRightLayout->addWidget(freqWheelY, 0);

	//Create the vertical scrollbar
	freqScrollBar = new MyScrollBar(0, gdata->topPitch() - view->viewHeight(), 0.5, view->viewHeight(), 0, 20, Qt::Vertical, topWidget);
	freqScrollBar->setValue(gdata->topPitch() - view->viewHeight() - view->viewBottom());
	topRightLayout->addWidget(freqScrollBar, 4);

	topLayout->addLayout(topRightLayout);

	//------------bottom half------------------

	QWidget* bottomWidget = new QWidget(splitter);
	QBoxLayout* bottomLayout = new QVBoxLayout(bottomWidget);
	QBoxLayout* bottomTopLayout = new QHBoxLayout();
	bottomLayout->addLayout(bottomTopLayout);

	QFrame* amplitudeFrame = new QFrame;
	amplitudeFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QVBoxLayout* amplitudeFrameLayout = new QVBoxLayout;
	amplitudeWidgetGL = new AmplitudeWidgetGL(NULL);
	amplitudeWidgetGL->setWhatsThis("Shows the volume (or other parameters) at time lined up with the pitch above. Note: You can move the lines to change some thresholds.");
	amplitudeFrameLayout->addWidget(amplitudeWidgetGL);
	amplitudeFrameLayout->setContentsMargins(0, 0, 0, 0);
	amplitudeFrameLayout->setSpacing(0);
	amplitudeFrame->setLayout(amplitudeFrameLayout);
	bottomTopLayout->addWidget(amplitudeFrame);

	QBoxLayout* bottomTopRightLayout = new QVBoxLayout();
	bottomTopLayout->addLayout(bottomTopRightLayout);

	amplitudeWheelY = new QwtWheel(bottomWidget);
	amplitudeWheelY->setOrientation(Qt::Vertical);
	amplitudeWheelY->setWheelWidth(14);
	amplitudeWheelY->setRange(0.2, 1.00);
	amplitudeWheelY->setSingleStep(0.01);
	amplitudeWheelY->setValue(amplitudeWidgetGL->range());
	amplitudeWheelY->setToolTip("Zoom pitch contour view vertically");

	bottomTopRightLayout->addWidget(amplitudeWheelY, 0);

	//Create the vertical scrollbar
	amplitudeScrollBar = new MyScrollBar(0.0, 1.0 - amplitudeWidgetGL->range(), 0.20, amplitudeWidgetGL->range(), 0, 20, Qt::Vertical, bottomWidget);
	bottomTopRightLayout->addWidget(amplitudeScrollBar, 4);

	QBoxLayout* bottomBottomLayout = new QHBoxLayout();
	bottomLayout->addLayout(bottomBottomLayout);

	QComboBox* amplitudeModeComboBox = new QComboBox(bottomWidget);
	amplitudeModeComboBox->setObjectName("amplitudeTypeModeBox");

	amplitudeModeComboBox->setWhatsThis("Select different algorithm parameters to view in the bottom pannel");
	int j;
	QStringList s;
	for (j = 0; j < NUM_AMP_MODES; j++) s << amp_mode_names[j];
	amplitudeModeComboBox->addItems(s);
	connect(amplitudeModeComboBox, SIGNAL(activated(int)), gdata, SLOT(setAmplitudeMode(int)));
	connect(amplitudeModeComboBox, SIGNAL(activated(int)), amplitudeWidgetGL, SLOT(update()));

	QComboBox* pitchContourModeComboBox = new QComboBox(bottomWidget);
	pitchContourModeComboBox->setObjectName("pitchContourModeComboBox");
	pitchContourModeComboBox->setWhatsThis("Select whether the Pitch Contour line fades in/out with clarity/loudness of the sound or is a solid dark line");
	s.clear();
	s << "Clarity fading" << "Note grouping";
	pitchContourModeComboBox->addItems(s);
	connect(pitchContourModeComboBox, SIGNAL(activated(int)), gdata, SLOT(setPitchContourMode(int)));
	connect(pitchContourModeComboBox, SIGNAL(activated(int)), freqWidgetGL, SLOT(update()));

	freqWheelX = new QwtWheel(bottomWidget);
	freqWheelX->setOrientation(Qt::Horizontal);
	freqWheelX->setWheelWidth(16);
	freqWheelX->setRange(0.5, 9.0);
	freqWheelX->setSingleStep(0.001);
	freqWheelX->setValue(2.0);
	freqWheelX->setToolTip("Zoom horizontally");

	bottomBottomLayout->addStretch(2);
	bottomBottomLayout->addWidget(amplitudeModeComboBox, 0);
	bottomBottomLayout->addStretch(2);
	bottomBottomLayout->addWidget(pitchContourModeComboBox, 0);
	bottomBottomLayout->addStretch(2);
	bottomBottomLayout->addWidget(freqWheelX, 1);
	bottomBottomLayout->addSpacing(16);

	//Create the resize grip. The thing in the bottom right hand corner
	QSizeGrip* sizeGrip = new QSizeGrip(bottomWidget);
	sizeGrip->setFixedSize(15, 15);
	sizeGrip->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	bottomBottomLayout->addWidget(sizeGrip);

	//Actually add all the widgets into the widget layout
	mainLayout->addWidget(splitter);

	//Setup all the signals and slots
	//vertical

	connect(freqScrollBar, SIGNAL(sliderMoved(double)), view, SLOT(changeViewY(double)));
	connect(freqScrollBar, SIGNAL(sliderMoved(double)), view, SLOT(doSlowUpdate()));

	connect(view, SIGNAL(viewBottomChanged(double)), freqScrollBar, SLOT(setValue(double)));
	connect(freqWheelY, SIGNAL(valueChanged(double)), view, SLOT(setZoomFactorYQWTWheel(double)));
	connect(view, SIGNAL(logZoomYChanged(double)), freqWheelY, SLOT(setValue(double)));

	//horizontal
	connect(freqWheelX, SIGNAL(valueChanged(double)), view, SLOT(setZoomFactorXQWTWheel(double)));
	connect(view, SIGNAL(logZoomXChanged(double)), freqWheelX, SLOT(setValue(double)));
	connect(amplitudeWheelY, SIGNAL(valueChanged(double)), amplitudeWidgetGL, SLOT(setRangeQWTWheel(double)));
	connect(amplitudeWheelY, SIGNAL(valueChanged(double)), amplitudeWidgetGL, SLOT(update()));

	connect(amplitudeScrollBar, SIGNAL(sliderMoved(double)), amplitudeWidgetGL, SLOT(setOffset(double)));
	connect(amplitudeScrollBar, SIGNAL(sliderMoved(double)), amplitudeWidgetGL, SLOT(update()));

	connect(amplitudeWidgetGL, SIGNAL(rangeChanged(double)), this, SLOT(setAmplitudeZoom(double)));
	connect(amplitudeWidgetGL, SIGNAL(rangeChanged(double)), amplitudeWheelY, SLOT(setValue(double)));
	connect(amplitudeWidgetGL, SIGNAL(offsetChanged(double)), amplitudeScrollBar, SLOT(setValue(double)));

	//make the widgets get updated when the view changes
	connect(gdata->view, SIGNAL(onSlowUpdate(double)), freqWidgetGL, SLOT(update()));
	connect(gdata->view, SIGNAL(onSlowUpdate(double)), amplitudeWidgetGL, SLOT(update()));
	connect(gdata->view, SIGNAL(onSlowUpdate(double)), timeAxis, SLOT(update()));
	connect(gdata->view, SIGNAL(timeViewRangeChanged(double, double)), timeAxis, SLOT(setRange(double, double)));
}

FreqViewGL::~FreqViewGL()
{
  //Qt deletes the child widgets automatically
}

void FreqViewGL::zoomIn()
{
	bool doneIt = false;
	if (gdata->running != STREAM_FORWARD) {
		if (freqWidgetGL->testAttribute(Qt::WA_UnderMouse)) {
			QPoint mousePos = freqWidgetGL->mapFromGlobal(QCursor::pos());
			gdata->view->setZoomFactorX(gdata->view->logZoomX() + 0.1, mousePos.x());
			gdata->view->setZoomFactorY(gdata->view->logZoomY() + 0.1, freqWidgetGL->height() - mousePos.y());
			doneIt = true;
		} else if (amplitudeWidgetGL->testAttribute(Qt::WA_UnderMouse)) {
			QPoint mousePos = amplitudeWidgetGL->mapFromGlobal(QCursor::pos());
			gdata->view->setZoomFactorX(gdata->view->logZoomX() + 0.1, mousePos.x());
			doneIt = true;
		}
	}
	if (!doneIt) {
		gdata->view->setZoomFactorX(gdata->view->logZoomX() + 0.1);
		gdata->view->setZoomFactorY(gdata->view->logZoomY() + 0.1);
		doneIt = true;
	}
}

void FreqViewGL::zoomOut()
{
	gdata->view->setZoomFactorX(gdata->view->logZoomX() - 0.1);
	if (!amplitudeWidgetGL->testAttribute(Qt::WA_UnderMouse)) {
		gdata->view->setZoomFactorY(gdata->view->logZoomY() - 0.1);
	}
}

void FreqViewGL::setAmplitudeZoom(double newRange)
{
	amplitudeScrollBar->setRange(0.0, 1.0 - newRange);
	amplitudeScrollBar->setPageStep(newRange);
}


