/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
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
#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <tqptrlist.h>
#include <tdeaction.h>

#include <tdemainwindow.h>

#include <tdeparts/part.h>

class TQString;

namespace KParts
{

class MainWindowPrivate;

/**
 * A KPart-aware main window, whose user interface is described in XML.
 *
 * Inherit your main window from this class
 * and don't forget to call setXMLFile() in the inherited constructor.
 *
 * It implements all internal interfaces in the case of a
 * TDEMainWindow as host: the builder and servant interface (for menu
 * merging).
 */
class TDEPARTS_EXPORT MainWindow : public TDEMainWindow, virtual public PartBase
{
  TQ_OBJECT
 public:
  /**
   * Constructor, same signature as TDEMainWindow.
   */
#ifdef qdoc
  MainWindow( TQWidget* parent,  const char *name = 0L, WFlags f = WType_TopLevel | WDestructiveClose );
#else
  MainWindow( TQWidget* parent,  const char *name = 0L, WFlags f = (WFlags)(WType_TopLevel | WDestructiveClose) );
#endif
  /**
   * Compatibility Constructor
   */
#ifdef qdoc
  MainWindow( const char *name = 0L, WFlags f = WDestructiveClose );
#else
  MainWindow( const char *name = 0L, WFlags f = (WFlags)WDestructiveClose );
#endif
  /**
   * Constructor with creation flags, see TDEMainWindow.
   * @since 3.2
   */
#ifdef qdoc
  MainWindow( int cflags, TQWidget* parent,  const char *name = 0L, WFlags f = WType_TopLevel | WDestructiveClose );
#else
  MainWindow( int cflags, TQWidget* parent,  const char *name = 0L, WFlags f = (WFlags)(WType_TopLevel | WDestructiveClose) );
#endif
  /**
   * Destructor.
   */
  virtual ~MainWindow();

protected slots:

  /**
   * Create the GUI (by merging the host's and the active part's)
   * You _must_ call this in order to see any GUI being created.
   *
   * In a main window with multiple parts being shown (e.g. as in Konqueror)
   * you need to connect this slot to the
   * KPartManager::activePartChanged() signal
   *
   * @param part The active part (set to 0L if no part).
   */
  void createGUI( KParts::Part * part );

  /**
   * Called when the active part wants to change the statusbar message
   * Reimplement if your mainwindow has a complex statusbar
   * (with several items)
   */
  virtual void slotSetStatusBarText( const TQString & );

  /**
   * Rebuilds the GUI after KEditToolbar changed the toolbar layout.
   * @see configureToolbars()
   * KDE4: make this virtual. (For now we rely on the fact that it's called
   * as a slot, so the metaobject finds it here).
   */
  void saveNewToolbarConfig();

protected:
  virtual void createShellGUI( bool create = true );

private:
  MainWindowPrivate *d;
};

}

#endif

