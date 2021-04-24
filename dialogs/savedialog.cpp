/***************************************************************************
                          savedialog.cpp  -  description
                             -------------------
    begin                : Feb 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#include "savedialog.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include "gdata.h"

/*
const char *sampleRateStr[] = { "8000", "11025", "22050", "44100", "48000", "96000", NULL };
const char *channelsBoxStr[] = { "Mono", "Stereo", NULL };
const char *bitsBoxStr[] = { "8", "16", NULL };
const char *windowSizeBoxStr[] = { "64", "128", "256", "512", "1024", "2048", "4096", "8192", NULL };
*/

SaveDialog::SaveDialog(/*const QString & dirName, const QString & filter, */QWidget * parent)
 : QFileDialog(parent, NULL, QDir::toNativeSeparators(gdata->qsettings->value("Dialogs/saveFilesFolder", QDir::currentPath()).toString()), "Wave files (*.wav)")
{
  setModal(true);
  setWindowTitle("Choose a filename to save under");
  setFileMode(QFileDialog::AnyFile);
  setOption(QFileDialog::DontUseNativeDialog);
  
  QWidget *baseWidget = new QWidget(this);
  this->layout()->addWidget(baseWidget);
  QBoxLayout *baseLayout = new QVBoxLayout(baseWidget);

  appendWavCheckBox =      new QCheckBox("Append .wav extension if needed", baseWidget);
  rememberFolderCheckBox = new QCheckBox("Remember current folder", baseWidget);
  appendWavCheckBox->setChecked(gdata->qsettings->value("Dialogs/appendWav", true).toBool());
  rememberFolderCheckBox->setChecked(gdata->qsettings->value("Dialogs/rememberSaveFolder", true).toBool());
  baseLayout->addSpacing(10);
  baseLayout->addWidget(appendWavCheckBox);
  baseLayout->addWidget(rememberFolderCheckBox);

  //baseLayout->setColSpacing(2, 30);
}

SaveDialog::~SaveDialog()
{
}

void SaveDialog::accept()
{
  bool remember = rememberFolderCheckBox->isChecked();
  //gdata->settings.setBool("Dialogs", "rememberSaveFolder", remember);
  gdata->qsettings->setValue("Dialogs/rememberSaveFolder", remember);
  if(remember == true) {
    const QDir curDir = directory();
    //gdata->settings.setString("Dialogs", "saveFilesFolder", curDir->absPath());
    gdata->qsettings->setValue("Dialogs/saveFilesFolder", curDir.absolutePath());
  }
  bool appendWav = appendWavCheckBox->isChecked();
  //gdata->settings.setBool("Dialogs", "appendWav", appendWav);
  gdata->qsettings->setValue("Dialogs/appendWav", appendWav);
  if(appendWav == true) {
    QStringList sList = selectedFiles();
    QString s = sList[0];
    if (!s.toLower().endsWith(".wav")) {
        s += ".wav";
    }
    selectFile(s);
  }
  QFileDialog::accept();
}

QString SaveDialog::getSaveWavFileName(QWidget *parent)
{
  SaveDialog d(parent);
  if(d.exec() != QDialog::Accepted) return QString();
  return d.selectedFiles()[0];
}

