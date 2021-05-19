/***************************************************************************
                          openfiles.cpp  -  description
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



#include <qapplication.h>
#include <QListView>
#include <qpixmap.h>
//Added by qt3to4:
#include <QResizeEvent>

#include "openfiles.h"
#include "gdata.h"
#include "channel.h"
#include "mainwindow.h"
#include "soundfile.h"

OpenFiles::OpenFiles(int id, QWidget *parent) : ViewWidget(id, parent)
{
  //Create the list of channels down the left hand side
  filesTable = new QTableWidget(this);
  filesTable->setColumnCount(2);
 
  QTableWidgetItem* col_1 = new QTableWidgetItem("Filename (Channel)");
  col_1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::AlignCenter);
  col_1->setText("Filename (Channel)");
  filesTable->setHorizontalHeaderItem(0, col_1);

  QTableWidgetItem* col_2 = new QTableWidgetItem("A");
  col_2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::AlignCenter);
  col_2->setText("A");
  filesTable->setHorizontalHeaderItem(1, col_2);

  filesTable->horizontalHeader()->setStretchLastSection(false);
  filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  filesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  filesTable->setWhatsThis("A list of all open channels in all open sounds. "
    "The current active channel is marked with an 'A' beside it. "
    "The tick specifies if a channel should be visible or not in the multi-channel views");


  filesTable->setSelectionMode(QAbstractItemView::NoSelection);
  
  filesTable->verticalHeader()->setVisible(false);


  connect(gdata, SIGNAL(channelsChanged()), this, SLOT(refreshChannelList()));
  connect(gdata, SIGNAL(activeChannelChanged(Channel*)), this, SLOT(slotActiveChannelChanged(Channel *)));
  connect(filesTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableWidgetChanged(QTableWidgetItem*)));
  connect(filesTable, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(slotCurrentChanged(QTableWidgetItem*)));

  refreshChannelList();
}

OpenFiles::~OpenFiles()
{
}
  
void OpenFiles::refreshChannelList()
{
    // Block all signals while the table is reconstructed.
    const QSignalBlocker blocker(filesTable);

    //put in any channel items that already exist
    filesTable->clearContents();
    filesTable->setRowCount(0);

    QString s;
    for (std::vector<Channel*>::iterator it = gdata->channels.begin(); it != gdata->channels.end(); it++) {
        const int row = filesTable->rowCount();
        filesTable->setRowCount(row + 1);
        s = (*it)->getUniqueFilename();

        QTableWidgetItem* fileItem = new QTableWidgetItem(s);
        fileItem->setData(Qt::UserRole, s);
        fileItem->setFlags((fileItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
        filesTable->setItem(row, 0, fileItem);

        QTableWidgetItem* active = new QTableWidgetItem("");
        if ((*it) == gdata->getActiveChannel()) {
            active->setText("A");
            filesTable->setCurrentItem(active);
        }
        active->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::AlignCenter);
        active->setFlags(active->flags() & (~Qt::ItemIsSelectable) & (~Qt::ItemIsEnabled));
        filesTable->setItem(row, 1, active);

        if ((*it)->isVisible()) {
            fileItem->setCheckState(Qt::Checked);
        }
        else {
            fileItem->setCheckState(Qt::Unchecked);
        }
    }

}

//TODO: Tidy this method up
void OpenFiles::slotActiveChannelChanged(Channel *active)
{
  int index = 0;
	bool found = false;

	// Find the index of the active channel
	for (index = 0; index < int(gdata->channels.size()); index++) {
		if (gdata->channels.at(index) == active) {
			found = true;
			break;
		}
	}

	// Set the active marker for each item on or off, depending on what it should be.
	// This depends on them being in the same order as the channels list.
	if (found) {
		// Go through all the elements in the table and turn the active channel 
		// markers off, or on if we find the right index
        filesTable->setCurrentCell(index, 0);
        QTableWidgetItem* fileItem = filesTable->itemAt(index, 0);
        slotCurrentChanged(fileItem);
	}
}

/**
 * Toggles a channel on or off for a specified item.
 *
 * @param item the channel to toggle.
 **/
void OpenFiles::tableWidgetChanged(QTableWidgetItem*)
{
    for (int row = 0; row < filesTable->rowCount(); ++row) {
        const QTableWidgetItem* fileItem = filesTable->item(row, 0);

        bool state = fileItem->checkState() == Qt::Checked;
        // row can be greater than the channel size during the closing of the app.
        if (row < int(gdata->channels.size())) {
            if (gdata->channels.at(row)->isVisible() != state) gdata->channels.at(row)->setVisible(state);
        }
    }
   gdata->view->doUpdate();
}

/**
 * Changes the active channel to the item.
 *
 * @param item the channel to toggle.
 **/
void OpenFiles::slotCurrentChanged(QTableWidgetItem*item)
{
  if (item == NULL) return;

  for (int idx = 0; idx < filesTable->rowCount(); idx++) {
      const QTableWidgetItem* fileItem = filesTable->item(idx, 0);
      QTableWidgetItem* active = filesTable->item(idx, 1);
      if (fileItem == item) {
          const QSignalBlocker blocker(gdata);
          gdata->setActiveChannel(gdata->channels.at(idx));
          active->setText("A");
      }
      else {
          active->setText("");
      }
  }
}

void OpenFiles::resizeEvent(QResizeEvent *)
{
    filesTable->resize(size());
}

