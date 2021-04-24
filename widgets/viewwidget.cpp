/***************************************************************************
                          viewwidget.cpp  -  description
                             -------------------
    begin                : 2003
    copyright            : (C) 2003-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include "viewwidget.h"
#include "mainwindow.h"

ViewWidget::ViewWidget( int viewID_, QWidget * /*parent*/, Qt::WindowFlags f )
  : QWidget( NULL, f )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  viewID = viewID_;
  setWindowTitle(viewData[viewID].title);

  setAutoFillBackground(true);
}

/*
void ViewWidget::closeEvent( QCloseEvent * e )
{
    //emit aboutToClose(viewID);
    QWidget::closeEvent(e);
}
*/
