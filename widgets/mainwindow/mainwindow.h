/***************************************************************************
                          mainwindow.h  -  description
                             -------------------
    begin                : May 2002
    copyright            : (C) 2002-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QDialog>
#include <QKeyEvent>
#include <QEvent>
#include <QLabel>
#include <QSignalMapper>

class Preferences;
class Q3ListView;
class Q3ListViewItem;
class Q3DockWindow;
class QLabel;
class QwtSlider;
class MyScrollBar;
class QTimer;
class QIcon;
class QToolButton;
class Channel;
class MyLabel;
class QComboBox;

struct ViewData
{
	ViewData(QString title_, QString menuName_, QString className_, int menuType_)
	{
		title = title_;
		menuName = menuName_;
		className = className_;
		menuType = menuType_;
	};
  QString title;
  QString menuName;
  QString className;
  int menuType;
};

enum VIEW_MENU_TYPES {
	MENU_TYPE_MAIN,
	MENU_TYPE_TECHNICAL,
	MENU_TYPE_EXPERIMENTAL
};

//DWS #define NUM_VIEWS 20
#define NUM_VIEWS 16
enum VIEW_NAMES {
  VIEW_OPEN_FILES,
#ifdef UNUSED
  DWS
  VIEW_FREQ,
#endif
  VIEW_TUNER,
#ifdef UNUSED
  DWS
  VIEW_HTRACK, VIEW_VIBRATO,
#endif
  VIEW_SCORE,
  VIEW_WAVE, VIEW_CORRELATION, VIEW_FFT, VIEW_CEPSTRUM, VIEW_DEBUGVIEW,
  VIEW_HBLOCK, VIEW_HSTACK, VIEW_HBUBBLE, VIEW_HCIRCLE, VIEW_PITCH_COMPASS,
  VIEW_PIANO, VIEW_SUMMARY, VIEW_VOLUME_METER
};

extern ViewData viewData[NUM_VIEWS];

                         
class MainWindow : public QMainWindow
{
  Q_OBJECT

 public:
  MainWindow();
  virtual ~MainWindow();

  void keyPressEvent ( QKeyEvent * e );
  void message(QString s, int msec);
  
  QMdiArea *mdiArea;

  QMenu *newViewMenu;
  QMenu *windowMenu;

  QMenu *optionsMenu;
  QMenu *helpMenu;
  QAction* closeAct;
  QAction* closeAllAct;
  QAction* tileAct;
  QAction* cascadeAct;
  QAction* nextAct;
  QAction* previousAct;
  MyLabel *timeLabel;
  MyLabel *chunkLabel;
  MyLabel *noteLabel;
  QwtSlider *timeSlider;
  MyScrollBar *timeScrollBar;
  QComboBox *keyTypeComboBox;
    
  //QToolButton *playStopButton;
  QAction *playStopAction;
  //QToolButton *recordButton;
  QAction *recordAction;
  QAction *playRecordAction;
  QIcon *playIconSet;
  QIcon *playRecordIconSet;
  QIcon *stopIconSet;
  QIcon *recordIconSet;
  QTimer *rewindTimer;
  QTimer *fastforwardTimer;

  QSignalMapper *createSignalMapper;
  
 protected:
  //void customEvent( QCustomEvent * e );
  bool event( QEvent * e );

Q_SIGNALS:
  void changedMusicFolder(const QString &);
  //void fileAdded(QString);
  void zoomInPressed();
  void zoomOutPressed();
    
public Q_SLOTS:

  void openFile();
  void openFile(const QString &filename);

  void openRecord();
  void openPlayRecord();
  void openRecord(bool andPlay);
  //void toggleOption(int id);
  void closeEvent(QCloseEvent *event);
  void menuPreferences();

  void updateWindowMenu();
  void newViewUpdate();
  QWidget *openView(int viewID);
  void setTimeLabel(double t);
  void setChunkLabel();
  void setNoteLabel();
  void setTimeRange(double min_, double max_);
  //void setPageStep(double step_);

  void rewindPressed();
  void rewindReleased();
  void playStopClicked();
  void fastforwardPressed();
  void fastforwardReleased();
  void setTitle(Channel *ch);
  void aboutTartini();
  void aboutGPL();
  void aboutQt();
  void showDocumentation();
  void printPitch();
  void exportChannel(int type, QString typeString);
  void exportChannelPlainText();
  void exportChannelMatlab();

  bool loadViewGeometry();
  void saveViewGeometry();


};

class TartiniDialog : public QDialog
{
  Q_OBJECT
 
 public:
  TartiniDialog(QWidget *parent = NULL);
  QSize sizeHint() const { return QSize(600, 600); }
};

class GPLDialog : public QDialog
{
  Q_OBJECT
 
 public:
  GPLDialog(QWidget *parent = NULL);
  QSize sizeHint() const { return QSize(600, 480); }
};

extern MainWindow *mainWindow;
//class MyGLFont;
//extern MyGLFont *mygl_font;

#endif
