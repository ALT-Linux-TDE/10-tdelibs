/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 David Faure <faure@kde.org>

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

#ifndef TDEPARTS_STATUSBAREXTENSION_H
#define TDEPARTS_STATUSBAREXTENSION_H

#include <tqwidget.h>
#include <tqvaluelist.h>

#include <tdelibs_export.h>

class KStatusBar;
class TDEMainWindow;
class TQEvent;

namespace KParts
{

  class ReadOnlyPart;

  // Defined in impl
  class StatusBarItem;


  /**
   * @short An extension for KParts that allows more sophisticated statusbar handling
   *
   * Every part can use this class to customize the statusbar as long as it is active.
   * Add items via addStatusBarItem() and remove an item with removeStatusBarItem().
   *
   * IMPORTANT: do NOT add any items immediately after constructing the extension.
   * Give the application time to set the statusbar in the extension if necessary.
   *
   * @since 3.2
   */
  class TDEPARTS_EXPORT StatusBarExtension : public TQObject
  {
    TQ_OBJECT

    public:
      StatusBarExtension( KParts::ReadOnlyPart *parent, const char *name=0L );
      ~StatusBarExtension();

      /**
       * This adds a widget to the statusbar for this part.
       * If you use this method instead of using statusBar() directly,
       * this extension will take care of removing the items when the parts GUI
       * is deactivated and will re-add them when it is reactivated.
       * The parameters are the same as TQStatusBar::addWidget().
       *
       * Note that you can't use KStatusBar methods (inserting text items by id)
       * but you can create a KStatusBarLabel with a dummy id instead, and use
       * it directly in order to get the same look and feel.
       *
       * @param widget the widget to add
       * @param stretch the stretch factor. 0 for a minimum size.
       * @param permanent passed to TQStatusBar::addWidget as the "permanent" bool.
       * Note that the item isn't really permanent though, it goes away when
       * the part is unactivated. This simply controls where temporary messages
       * hide the @p widget, and whether it's added to the left or to the right side.
       *
       * IMPORTANT: do NOT add any items immediately after constructing the extension.
       * Give the application time to set the statusbar in the extension if necessary.
       */
      void addStatusBarItem( TQWidget * widget, int stretch, bool permanent );

      /**
       * Remove a @p widget from the statusbar for this part.
       */
      void removeStatusBarItem( TQWidget * widget );

      /**
       * @return the statusbar of the TDEMainWindow in which this part is currently embedded.
       * WARNING: this could return 0L
       */
      KStatusBar* statusBar() const;

      /**
       * This allows the hosting application to set a particular KStatusBar
       * for this part. If it doesn't do this, the statusbar used will be
       * the one of the TDEMainWindow in which the part is embedded.
       * Konqueror uses this to assign a view-statusbar to the part.
       * The part should never call this method!
       */
      void setStatusBar( KStatusBar* status );

      /**
       * Queries @p obj for a child object which inherits from this
       * BrowserExtension class. Convenience method.
       */
      static StatusBarExtension *childObject( TQObject *obj );

      /** @internal */
      virtual bool eventFilter( TQObject *watched, TQEvent* ev );

    private:

     TQValueList<StatusBarItem> m_statusBarItems; // Our statusbar items
     mutable KStatusBar* m_statusBar;

     // for future extensions
     class StatusBarExtensionPrivate;
     StatusBarExtensionPrivate *d;
  };

}
#endif // TDEPARTS_STATUSBAREXTENSION_H
