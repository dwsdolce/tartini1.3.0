/***************************************************************************
                          tunerview.cpp  -  description
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
#include <qpixmap.h>
#include <qwt_slider.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <QGridLayout>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QFrame>

#include "tunerview.h"
#include "tunerwidget.h"
#include "ledindicator.h"
#include "useful.h"
#include "gdata.h"
#include "channel.h"
#include "musicnotes.h"

TunerView::TunerView( int viewID_, QWidget *parent )
 : ViewWidget( viewID_, parent)
{
  QGridLayout *layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetNoConstraint);

  // Tuner widget goes from (0, 0) to (0, 8);
  QGridLayout* tunerLayout = new QGridLayout;
  tunerLayout->setMargin(0);
  tunerLayout->setSpacing(0);

  // The label
  QFrame* tunerFrame = new QFrame;
  tunerFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  QVBoxLayout* tunerFrameLayout = new QVBoxLayout;
  tunerWidget = new TunerWidget(0);

  tunerWidget->setWhatsThis("Indicates pitch deviation from standard not plus the standard note that is being played.");
  tunerFrameLayout->addWidget(tunerWidget);
  tunerFrameLayout->setMargin(0);
  tunerFrameLayout->setSpacing(0);
  tunerFrame->setLayout(tunerFrameLayout);

  tunerLayout->addWidget(tunerFrame, 0, 0, 1, 1);

  QWidget* topWidget2 = new QWidget;
  topWidget2->setLayout(tunerLayout);
  layout->addWidget(topWidget2, 0, 0, 1, 9);

  // Slider goes from (2,0) to (2,8)
  slider = new QwtSlider(this);
  slider->setOrientation(Qt::Horizontal);
  slider->setScalePosition(QwtSlider::LeadingScale);
  slider->setTrough(true);

  slider->setScale(0, 2);
  slider->setReadOnly(false);
  layout->addWidget(slider, 1, 0, 1, 9);
  slider->setToolTip("Increase slider to smooth the pitch over a longer time period");

  leds.push_back(new LEDIndicator(this, "A"));
  leds.push_back(new LEDIndicator(this, "B"));
  leds.push_back(new LEDIndicator(this, "C"));
  leds.push_back(new LEDIndicator(this, "D"));
  leds.push_back(new LEDIndicator(this, "E"));
  leds.push_back(new LEDIndicator(this, "F"));
  leds.push_back(new LEDIndicator(this, "G"));

  leds.push_back(new LEDIndicator(this, "#"));

  // Add the leds for note names into the positions (1, 0) to (1, 6)
  for (int n = 0; n < 7; n++) {
    layout->addWidget(leds.at(n), 2, n);
  }

  // (1, 7) is blank
  
  // Add the sharp led
  layout->addWidget(leds.at(7), 2, 8);

  layout->setRowStretch( 0, 4 );
  layout->setRowStretch( 1, 1 );
  layout->setRowStretch( 2, 0 ); 
    
  connect(gdata, SIGNAL(onChunkUpdate()), this, SLOT(doUpdate()));
  connect(tunerWidget, SIGNAL(ledSet(int, bool)), this, SLOT(setLed(int, bool)));

  QSize s = size();
}

TunerView::~TunerView()
{
  delete slider;
  for (uint i = 0; i < leds.size(); i++) {
      delete leds[i];
  }
  delete tunerWidget;
}

void TunerView::resetLeds()
{
 for (uint i = 0; i < leds.size(); i++) {
    leds[i]->setOn(false);
  }
}

void TunerView::setLed(int index, bool value)
{
  leds[index]->setOn(value);
}

void TunerView::doUpdate()
{
  Channel *active = gdata->getActiveChannel();
  if (active == NULL || !active->hasAnalysisData()) {
    tunerWidget->doUpdate(0.0);
    return;
  }
  ChannelLocker channelLocker(active);
  double time = gdata->view->currentTime();

  // To work out note:
  //   * Find the slider's value. This tells us how many seconds to average over.
  //   * Start time is currentTime() - sliderValue.
  //   * Finish time is currentTime().
  //   * Calculate indexes for these times, and use them to call average.
  //

  double sliderVal = slider->value();

  double pitch = 0.0;
  if (sliderVal == 0) {
    int chunk = active->currentChunk();
    if(chunk >= active->totalChunks()) chunk = active->totalChunks()-1;
    if(chunk >= 0)
      pitch = active->dataAtChunk(chunk)->pitch;
  } else {
    double startTime = time - sliderVal;
    double stopTime = time;
  
    int startChunk = active->chunkAtTime(startTime);
    int stopChunk = active->chunkAtTime(stopTime)+1;
    pitch = active->averagePitch(startChunk, stopChunk);
  }

  tunerWidget->doUpdate(pitch);
}
