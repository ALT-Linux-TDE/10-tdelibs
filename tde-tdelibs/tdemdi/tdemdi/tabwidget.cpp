/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Based on:

   //----------------------------------------------------------------------------
   //    Project              : KDE MDI extension
   //
   //    begin                : 07/1999       by Szymon Stefanek as part of kvirc
   //                                         (an IRC application)
   //    changes              : 09/1999       by Falk Brettschneider to create an
   //                           - 06/2000     stand-alone Qt extension set of
   //                                         classes and a Qt-based library
   //                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
   //                           2000-2003     maintained by the KDevelop project
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#include <ktabbar.h>
#include <tdepopupmenu.h>
#include <kdebug.h>

#include <tqobjectlist.h>

#include "tabwidget.h"
#include "tabwidget.moc"

namespace KMDIPrivate
{
  class TabWidgetPrivate
  {

  };
}

namespace KMDI
{

TabWidget::TabWidget(TQWidget* parent, const char* name)
 : KTabWidget(parent,name)
 , m_visibility (KMDI::ShowWhenMoreThanOneTab)
 , d (new KMDIPrivate::TabWidgetPrivate())
{
  installEventFilter (this);

  tabBar()->hide();

  setHoverCloseButton(true);

  connect(this, TQ_SIGNAL(closeRequest(TQWidget*)), this, TQ_SLOT(closeTab(TQWidget*)));
}

TabWidget::~TabWidget()
{
  delete d;
  d = 0;
}

bool TabWidget::eventFilter(TQObject *obj, TQEvent *e )
{
  if(e->type() == TQEvent::FocusIn)
  {
    emit focusInEvent ();
  }
  else if (e->type() == TQEvent::ChildRemoved)
  {
    // if we lost a child we uninstall ourself as event filter for the lost
    // child and its children
    TQObject* pLostChild = ((TQChildEvent*)e)->child();
    if ((pLostChild != 0L) && (pLostChild->isWidgetType())) {
       TQObjectList *list = pLostChild->queryList( "TQWidget" );
       list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
       TQObjectListIt it( *list );          // iterate over all lost child widgets
       TQObject * o;
       while ( (o=it.current()) != 0 ) { // for each found object...
          TQWidget* widg = (TQWidget*)o;
          ++it;
          widg->removeEventFilter(this);
       }
       delete list;                        // delete the list, not the objects
    }
  }
  else if (e->type() == TQEvent::ChildInserted)
  {
    // if we got a new child and we are attached to the MDI system we
    // install ourself as event filter for the new child and its children
    // (as we did when we were added to the MDI system).
    TQObject* pNewChild = ((TQChildEvent*)e)->child();
    if ((pNewChild != 0L) && (pNewChild->isWidgetType()))
    {
       TQWidget* pNewWidget = (TQWidget*)pNewChild;
       if (pNewWidget->testWFlags((WFlags)(WType_Dialog | WShowModal)))
           return false;
       TQObjectList *list = pNewWidget->queryList( "TQWidget" );
       list->insert(0, pNewChild);         // add the new child to the list too, just to save code
       TQObjectListIt it( *list );          // iterate over all new child widgets
       TQObject * o;
       while ( (o=it.current()) != 0 ) { // for each found object...
          TQWidget* widg = (TQWidget*)o;
          ++it;
          widg->installEventFilter(this);
          connect(widg, TQ_SIGNAL(destroyed()), this, TQ_SLOT(childDestroyed()));
       }
       delete list;                        // delete the list, not the objects
    }
  }

  return KTabWidget::eventFilter (obj, e);
}

void TabWidget::childDestroyed()
{
  // if we lost a child we uninstall ourself as event filter for the lost
  // child and its children
  const TQObject* pLostChild = sender();
  if ((pLostChild != 0L) && (pLostChild->isWidgetType()))
  {
     TQObjectList *list = ((TQObject*)(pLostChild))->queryList("TQWidget");
     list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
     TQObjectListIt it( *list );          // iterate over all lost child widgets
     TQObject * obj;
     while ( (obj=it.current()) != 0 ) { // for each found object...
       TQWidget* widg = (TQWidget*)obj;
       ++it;
       widg->removeEventFilter(this);
     }
     delete list;                        // delete the list, not the objects
  }
}

void TabWidget::closeTab(TQWidget* w)
{
  w->close();
}

void TabWidget::addTab ( TQWidget * child, const TQString & label )
{
  KTabWidget::addTab(child,label);
  showPage(child);
  maybeShow();
}

void TabWidget::addTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label )
{
  KTabWidget::addTab(child,iconset,label);
  showPage(child);
  maybeShow();
}

void TabWidget::addTab ( TQWidget * child, TQTab * tab )
{
  KTabWidget::addTab(child,tab);
  showPage(child);
  maybeShow();
}

void TabWidget::insertTab ( TQWidget * child, const TQString & label, int index)
{
  KTabWidget::insertTab(child,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::insertTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index )
{
  KTabWidget::insertTab(child,iconset,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::insertTab ( TQWidget * child, TQTab * tab, int index)
{
  KTabWidget::insertTab(child,tab,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::removePage ( TQWidget * w )
{
  KTabWidget::removePage(w);
  maybeShow();
}

void TabWidget::updateIconInView( TQWidget *w, TQPixmap icon )
{
  changeTab(w,icon,tabLabel(w));
}

void TabWidget::updateCaptionInView( TQWidget *w, const TQString &caption )
{
  changeTab(w, caption);
}

void TabWidget::maybeShow()
{
  switch (m_visibility)
  {
    case KMDI::AlwaysShowTabs:
      tabBar()->show();

      // show/hide corner widgets
      if (count() == 0)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case KMDI::ShowWhenMoreThanOneTab:
      if (count()<2) tabBar()->hide();
      else tabBar()->show();

      // show/hide corner widgets
      if (count() < 2)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case KMDI::NeverShowTabs:
      tabBar()->hide();
      break;
  }
}

void TabWidget::setCornerWidgetVisibility(bool visible) {
  // there are two corner widgets: on TopLeft and on TopTight!

  if (cornerWidget(TQt::TopLeft) ) {
    if (visible)
      cornerWidget(TQt::TopLeft)->show();
    else
      cornerWidget(TQt::TopLeft)->hide();
  }

  if (cornerWidget(TQt::TopRight) ) {
    if (visible)
      cornerWidget(TQt::TopRight)->show();
    else
      cornerWidget(TQt::TopRight)->hide();
  }
}

void TabWidget::setTabWidgetVisibility( KMDI::TabWidgetVisibility visibility )
{
  m_visibility = visibility;
  maybeShow();
}

KMDI::TabWidgetVisibility TabWidget::tabWidgetVisibility( ) const
{
  return m_visibility;
}

}
