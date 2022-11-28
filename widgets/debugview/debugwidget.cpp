/***************************************************************************
                          debugwidget.cpp  -  description
                             -------------------
    begin                : Wed Feb 22 2006
    copyright            : (C) 2006 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <qpixmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>

#include "debugwidget.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "useful.h"

DebugWidget::DebugWidget(QWidget *parent)
  : DrawWidget(parent)
{
  textY = 0;
}

DebugWidget::~DebugWidget()
{
}

void DebugWidget::printString(const QString &s) {
  textY += fontHeight;
  p.drawText(0, textY, s);
}

void DebugWidget::paintEvent( QPaintEvent * )
{
  Channel *ch = gdata->getActiveChannel();

  beginDrawing();

  // Drawing code goes here
  QFontMetrics fm = p.fontMetrics();
  fontHeight = fm.height();

  if (ch != NULL && ch->isValidChunk(ch->currentChunk())) {
    textY = 0; //move text position back to the top
    int chunk = ch->currentChunk();
    AnalysisData &data = *ch->dataAtChunk(chunk);
    QString s;
    printString(s.asprintf("Chunk = %d", chunk));
    printString(s.asprintf("period = %f", data.period));
    printString(s.asprintf("fundamentalFreq = %f", data.fundamentalFreq));
    printString(s.asprintf("pitch = %f", data.pitch));
    printString(s.asprintf("pitchSum = %f", data.pitchSum));
    printString(s.asprintf("pitch2Sum = %f", data.pitch2Sum));
    printString(s.asprintf("_freqCentroid = %f", data._freqCentroid));
    printString(s.asprintf("shortTermMean = %f", data.shortTermMean));
    printString(s.asprintf("shortTermDeviation = %f", data.shortTermDeviation));
    printString(s.asprintf("longTermMean = %f", data.longTermMean));
    printString(s.asprintf("longTermDeviation = %f", data.longTermDeviation));
    printString(s.asprintf("highestCorrelationIndex = %d", data.highestCorrelationIndex));
    printString(s.asprintf("chosenCorrelationIndex = %d", data.chosenCorrelationIndex));
    //printString(s.sprintf("periodOctaveEstimate = %f", data.periodOctaveEstimate));
    printString(s.asprintf("periodOctaveEstimate = %f", ch->periodOctaveEstimate(chunk)));
    printString(s.asprintf("noteIndex = %d", data.noteIndex));
    printString(s.asprintf("done = %s", data.done ? "true" : "false"));
    printString(s.asprintf("reason = %d", data.reason));
    printString(s.asprintf("notePlaying = %d", data.notePlaying));
    printString(s.asprintf("spread = %f", data.spread));
    printString(s.asprintf("spread2 = %f", data.spread2));
    printString(s.asprintf("logrms = %f", data.logrms()));
    printString(s.asprintf("normalised_logrms = %f", dB2Normalised(data.logrms())));
    printString(s.asprintf("detailedPeriod.size() = %d", ch->detailedPitchData.size()));
    printString(s.asprintf("vibratoPitch = %f", data.vibratoPitch));
    printString(s.asprintf("vibratoWidth = %f", data.vibratoWidth));
    printString(s.asprintf("vibratoSpeed = %f", data.vibratoSpeed));
    printString(s.asprintf("vibratoPhase = %f", data.vibratoPhase));
    printString(s.asprintf("vibratoError = %f", data.vibratoError));
    printString(s.asprintf("vibratoWidthAdjust = %f", data.vibratoWidthAdjust));
    printString(s.asprintf("periodRatio = %f", data.periodRatio));
  }

  endDrawing();
}
