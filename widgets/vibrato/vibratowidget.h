/***************************************************************************
                          vibratowidget.h  -  description
                             -------------------
    begin                : May 18 2005
    copyright            : (C) 2005-2007 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef VIBRATOWIDGET_H
#define VIBRATOWIDGET_H

#include "drawwidget.h"
#include <QPixmap>
#include <QPaintEvent>

class VibratoWidget : public DrawWidget {
  Q_OBJECT

  public:
    VibratoWidget(QWidget *parent, int nls);
    virtual ~VibratoWidget();

    void paintEvent(QPaintEvent* event);

    QSize sizeHint() const { return QSize(300, 100); }

  private:
    int noteLabelOffset; // The horizontal space in pixels a note label requires
    double zoomFactorX;
    double zoomFactorY;
    int offsetY;

    QFont vibratoFont;

    QVector<QRectF> verticalPeriodBarsShading1;
    QColor verticalPeriodBarsShading1Color;
    QVector<QRectF> verticalPeriodBarsShading2;
    QColor verticalPeriodBarsShading2Color;
    QVector<QLineF>verticalSeparatorLines;
    QColor verticalSeparatorLinesColor;
    QVector<QLineF>referenceLines;
    QColor referenceLinesColor;
    QVector<QPolygonF> pronyWidthBand;
    QColor pronyWidthBandColor;
    QPolygonF pronyAveragePitch;
    QColor pronyAveragePitchColor;
    QPolygonF vibratoPolyline;
    QColor vibratoPolylineColor;
    QVector<QRectF> currentWindowBand;
    QColor currentWindowBandColor;
    QLineF currentTimeLine;
    QColor currentTimeLineColor;
    QVector<QRectF> maximaPoints;
    QColor maximaPointsColor;
    QVector<QRectF> minimaPoints;
    QColor minimaPointsColor;

    int noteLabelCounter;
    struct noteLabelStruct {
      QString label;
      qreal y;
    };
    noteLabelStruct noteLabels[100];

  public slots:
    void doUpdate();
    void setZoomFactorX(double x);
    void setZoomFactorY(double y);
    void setOffsetY(int value);
};

#endif
