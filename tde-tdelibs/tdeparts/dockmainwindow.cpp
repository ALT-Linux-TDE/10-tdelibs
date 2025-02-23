/* This file is part of the KDE project
   Copyright (C) 2000 Falk Brettschneider <gigafalk@yahoo.com>
             (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
*/

#include <tdeparts/dockmainwindow.h>
#include <tdeparts/event.h>
#include <tdeparts/part.h>
#include <tdeaccel.h>
#include <tdeparts/plugin.h>
#include <kstatusbar.h>
#include <kinstance.h>
#include <khelpmenu.h>
#include <kstandarddirs.h>
#include <tqapplication.h>

#include <kdebug.h>
#include <kxmlguifactory.h>

#include <assert.h>

using namespace KParts;

namespace KParts
{
class DockMainWindowPrivate
{
public:
  DockMainWindowPrivate()
  {
    m_activePart = 0;
    m_bShellGUIActivated = false;
    m_helpMenu = 0;
  }
  ~DockMainWindowPrivate()
  {
  }

  TQGuardedPtr<Part> m_activePart;
  bool m_bShellGUIActivated;
  KHelpMenu *m_helpMenu;
};
}

DockMainWindow::DockMainWindow( TQWidget* parent, const char *name, WFlags f )
  : KDockMainWindow( parent, name, f )
{
  d = new DockMainWindowPrivate();
  PartBase::setPartObject( this );
}

DockMainWindow::~DockMainWindow()
{
  delete d;
}

void DockMainWindow::createGUI( Part * part )
{
  kdDebug(1000) << TQString(TQString("DockMainWindow::createGUI for %1").arg(part?part->name():"0L")) << endl;

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  TQPtrList<Plugin> plugins;

  if ( d->m_activePart )
  {
    kdDebug(1000) << TQString(TQString("deactivating GUI for %1").arg(d->m_activePart->name())) << endl;

    GUIActivateEvent ev( false );
    TQApplication::sendEvent( d->m_activePart, &ev );

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, TQ_SIGNAL( setWindowCaption( const TQString & ) ),
             this, TQ_SLOT( setCaption( const TQString & ) ) );
    disconnect( d->m_activePart, TQ_SIGNAL( setStatusBarText( const TQString & ) ),
             this, TQ_SLOT( slotSetStatusBarText( const TQString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    loadPlugins( this, this, TDEGlobal::instance() );
    createShellGUI();
    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    // do this before sending the activate event
    connect( part, TQ_SIGNAL( setWindowCaption( const TQString & ) ),
             this, TQ_SLOT( setCaption( const TQString & ) ) );
    connect( part, TQ_SIGNAL( setStatusBarText( const TQString & ) ),
             this, TQ_SLOT( slotSetStatusBarText( const TQString & ) ) );

    factory->addClient( part );

    GUIActivateEvent ev( true );
    TQApplication::sendEvent( part, &ev );

  }

  setUpdatesEnabled( true );

  d->m_activePart = part;
}

void DockMainWindow::slotSetStatusBarText( const TQString & text )
{
  statusBar()->message( text );
}

void DockMainWindow::createShellGUI( bool create )
{
    bool bAccelAutoUpdate = accel()->setAutoUpdate( false );
    assert( d->m_bShellGUIActivated != create );
    d->m_bShellGUIActivated = create;
    if ( create )
    {
        if ( isHelpMenuEnabled() )
            d->m_helpMenu = new KHelpMenu( this, instance()->aboutData(), true, actionCollection() );

        TQString f = xmlFile();
        setXMLFile( locate( "config", "ui/ui_standards.rc", instance() ) );
        if ( !f.isEmpty() )
            setXMLFile( f, true );
        else
        {
            TQString auto_file( instance()->instanceName() + "ui.rc" );
            setXMLFile( auto_file, true );
        }

        GUIActivateEvent ev( true );
        TQApplication::sendEvent( this, &ev );

        guiFactory()->addClient( this );

    }
    else
    {
        GUIActivateEvent ev( false );
        TQApplication::sendEvent( this, &ev );

        guiFactory()->removeClient( this );
    }
    accel()->setAutoUpdate( bAccelAutoUpdate );
}

#include "dockmainwindow.moc"
