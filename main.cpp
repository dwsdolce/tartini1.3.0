/***************************************************************************
                          main.cpp  -  description
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

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QSurfaceFormat>

#ifdef MACX
#include <CoreFoundation/CoreFoundation.h>
QString macxPathString;
#endif


#include "gdata.h"
#include "myassert.h"

#if defined(QT_NEEDS_QMAIN)
int qMain( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
#ifdef MACX
	fixColours();
  
  CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
  const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
  macxPathString = QString(pathPtr);
  if(!macxPathString.endsWith("/")) macxPathString.append('/');
  CFRelease(pluginRef);
  CFRelease(macPath);
#endif
	
  QApplication a( argc, argv );
  Q_INIT_RESOURCE(pitch);

  qDebug() << "QT_VERSION_STR = " << QT_VERSION_STR;
  qDebug() << "QT_VERSION = " << QT_VERSION;

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  a.setApplicationName("Tartini");
  a.setApplicationVersion("1.3.0");

  //Create one instance only of the global data
  gdata = new GData();
  /* The view needs to be created here, not in GData's constructor because of the View's
  autofollow method and the signals in the View class. It can try to reference the GData 
  object before the constructor finishes, which causes an access violation in 
  Visual Studio 6. */
  gdata->view = new View();

  mainWindow = new MainWindow();

  //call init after we know the windows size
  gdata->view->init();

  mainWindow->showMaximized();
    
  mainWindow->show();

  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );

  if(!mainWindow->loadViewGeometry()) {
    mainWindow->aboutTartini(); //open the tartini dialog on first time open
  }

  //open any files on the command line
  if(argc >= 2) {
    for(int j=1; j<argc; j++) mainWindow->openFile(argv[j]);
  }
  
  int ret = a.exec();
    
  delete gdata->view;
  delete gdata;
  
  return ret;
}
