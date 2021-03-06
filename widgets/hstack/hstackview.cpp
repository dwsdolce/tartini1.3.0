/***************************************************************************
                          hstackview.cpp  -  description
                             -------------------
    begin                : Mon Jan 10 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include "hstackview.h"
#include "hstackwidget.h"
#include "channel.h" 
#include "analysisdata.h"
#include "useful.h" 
#include "myscrollbar.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qsizegrip.h>
#include <qsplitter.h>
#include <qtooltip.h>
#include <qwt_wheel.h>
#include <qcursor.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>


HStackView::HStackView( int viewID_, QWidget *parent )
 : ViewWidget( viewID_, parent)
{
  setWindowTitle("Harmonic Stack");

  QBoxLayout *mainLayout = new QHBoxLayout(this);

  QBoxLayout *leftLayout = new QVBoxLayout();
  mainLayout->addLayout(leftLayout);
  QBoxLayout *rightLayout = new QVBoxLayout();
  mainLayout->addLayout(rightLayout);

  QFrame *waveFrame = new QFrame(this);
  waveFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  leftLayout->addWidget(waveFrame);
  hStackWidget = new HStackWidget(waveFrame);
  
  QBoxLayout *bottomLayout = new QHBoxLayout();
  leftLayout->addLayout(bottomLayout);

  QwtWheel* dbRangeWheel = new QwtWheel(this);
  dbRangeWheel->setOrientation(Qt::Vertical);
  dbRangeWheel->setWheelWidth(14);
  dbRangeWheel->setRange(5, 160.0);
  dbRangeWheel->setSingleStep(0.1);
  dbRangeWheel->setPageStepCount(100);
  dbRangeWheel->setValue(100);
  dbRangeWheel->setToolTip("Zoom dB range vertically");
  rightLayout->addWidget(dbRangeWheel, 0);
  rightLayout->addStretch(2);

  QwtWheel* windowSizeWheel = new QwtWheel(this);
  windowSizeWheel->setOrientation(Qt::Horizontal);
  windowSizeWheel->setWheelWidth(14);
  windowSizeWheel->setRange(32, 1024);
  windowSizeWheel->setSingleStep(2);
  windowSizeWheel->setValue(128);
  windowSizeWheel->setToolTip("Zoom windowsize horizontally");
  bottomLayout->addWidget(windowSizeWheel, 0);
  bottomLayout->addStretch(2); 

  connect(dbRangeWheel, SIGNAL(valueChanged(double)), hStackWidget, SLOT(setDBRange(double)));
  connect(dbRangeWheel, SIGNAL(valueChanged(double)), hStackWidget, SLOT(update()));

  connect(windowSizeWheel, SIGNAL(valueChanged(double)), hStackWidget, SLOT(setWindowSize(double)));
  connect(windowSizeWheel, SIGNAL(valueChanged(double)), hStackWidget, SLOT(update()));


/*


  QBoxLayout *topLeftLayout = new QVBoxLayout(mainLayout);
  QBoxLayout *topRightLayout = new QVBoxLayout(mainLayout);

  QWidget *qw1 = new QWidget(this);
  QWidget *qw2 = new QWidget(this);

  QwtWheel *freqWheelY = new QwtWheel(qw1);
  freqWheelY->setOrientation(Qt::Vertical);
  freqWheelY->setWheelWidth(14);
  freqWheelY->setRange(1.6, 5.0, 0.001, 1);
  freqWheelY->setValue(3.2);

  QwtWheel *freqWheelX = new QwtWheel(qw2);
  freqWheelX->setOrientation(Qt::Horizontal);
  freqWheelX->setWheelWidth(14);
  freqWheelX->setRange(1.6, 5.0, 0.001, 1);
  freqWheelX->setValue(3.2);
  
  
  QGrid *qw = new QGrid(1, this);
  


  topLeftLayout->addWidget(qw);
  topLeftLayout->addWidget(qw2);

  //hStackWidget->show();
*/
}

HStackView::~HStackView()
{
  delete hStackWidget;
}
