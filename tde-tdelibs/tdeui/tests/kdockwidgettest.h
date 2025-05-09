/***************************************************************************
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDOCKWIDGETTEST_H
#define KDOCKWIDGETTEST_H

#include <kdockwidget.h>
#include <kdockwidget_p.h>
#include <tqtabwidget.h>
#include <tqlabel.h>
class TQPushButton;

class DockApplication : public KDockMainWindow
{TQ_OBJECT
public:
  DockApplication( const char* name );
  ~DockApplication();

public slots:
  void rConfig();
  void wConfig();
  void cap();
  void greenMain();
  void blueMain();
  void nullMain();
  void gSetPix1();
  void gSetPix2();

protected:
  void initMenuBar();
  void initToolBars();
  void initStatusBar();

private:
  void updateButton();
  KDockWidget* dock;
  KDockWidget* dock1;
	KDockWidget* dock4;
	KDockWidget* dock5;
	KDockWidget* dock6;

  TQWidget* mainW;
  TQWidget* l;
  TQPushButton* m_bname;
};

class CTW:public TQTabWidget,public KDockContainer
{
        TQ_OBJECT
public:
        CTW(TQWidget *parent):TQTabWidget(parent,"MyDockContainer"),KDockContainer(){insertTab(new TQLabel("BLAH",this),"BLUP");}
        virtual ~CTW(){;}
        KDockWidget *parentDockWidget(){return ((KDockWidget*)parent());}
        void insertWidget (KDockWidget *w, TQPixmap, const TQString &, int &){tqDebug("widget inserted"); insertTab(w,"NO");}
        void setToolTip (KDockWidget *, TQString &){tqDebug("Tooltip set");}
};


#endif


