/***************************************************************************
                          openfiles.h  -  description
                             -------------------
    begin                : Thu Dec 16 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef OPENFILES_H
#define OPENFILES_H

#include "viewwidget.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QResizeEvent>

class Channel;

class OpenFiles : public ViewWidget {

  Q_OBJECT

  public:
    OpenFiles(int id, QWidget *parent);
    virtual ~OpenFiles();

  //private: // Although it should be private, really
    QTableWidget *filesTable;

    void resizeEvent(QResizeEvent *);

    QSize sizeHint() const { return QSize(200, 300); }

  public slots:
    void refreshChannelList();
    void tableWidgetChanged(QTableWidgetItem*);
    void slotCurrentChanged(QTableWidgetItem*);
    //void slotAddFilename(QString s);
	  void slotActiveChannelChanged(Channel *active);

};
#endif
