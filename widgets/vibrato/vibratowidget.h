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
#include <QPaintEvent>
#include <QPixmap>

class VibratoWidget : public DrawWidget
{
  Q_OBJECT

public:
  VibratoWidget(QWidget* parent, int nls);
  virtual ~VibratoWidget();

  void paintEvent(QPaintEvent* event);

  QSize sizeHint() const
  {
    return QSize(300, 100);
  }

private:
  int noteLabelOffset; // The horizontal space in pixels a note label requires
  double zoomFactorX;
  double zoomFactorY;
  int offsetY;

  QFont vibratoFont;

  QVector<QRectF> m_verticalPeriodBarsShading1;
  QColor m_verticalPeriodBarsShading1Color;

  QVector<QRectF> m_verticalPeriodBarsShading2;
  QColor m_verticalPeriodBarsShading2Color;

  QVector<QLineF> m_verticalSeparatorLines;
  QColor m_verticalSeparatorLinesColor;

  QVector<QLineF> m_referenceLines;
  QColor m_referenceLinesColor;

  QVector<QPolygonF> m_pronyWidthBand;
  QColor m_pronyWidthBandColor;

  QPolygonF m_pronyAveragePitch;
  QColor m_pronyAveragePitchColor;

  QPolygonF m_vibratoPolyline;
  QColor m_vibratoPolylineColor;

  QVector<QRectF> m_currentWindowBand;
  QColor m_currentWindowBandColor;

  QLineF m_currentTimeLine;
  QColor m_currentTimeLineColor;

  QVector<QRectF> m_maximaPoints;
  QColor m_maximaPointsColor;

  QVector<QRectF> m_minimaPoints;
  QColor m_minimaPointsColor;

  int noteLabelCounter;
  struct noteLabelStruct
  {
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
