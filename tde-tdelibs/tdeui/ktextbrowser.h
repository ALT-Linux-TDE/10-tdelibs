/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef _KTEXT_BROWSER_H_
#define _KTEXT_BROWSER_H_

#include <tqtextbrowser.h>

#include <tdelibs_export.h>

/**
 * @short Extended TQTextBrowser.
 *
 * An extended TQTextBrowser.
 *
 * By default it will
 * invoke the system mailer or the system browser when a link is
 * activated, or it can emit the signal urlClick() or mailClick()
 * when a link is activated.
 *
 * \image html ktextbrowser.png "KDE Text Browser"
 *
 * @author Espen Sand (espensa@online.no)
 */

class TDEUI_EXPORT KTextBrowser : public TQTextBrowser
{
  TQ_OBJECT
  TQ_PROPERTY( bool notifyClick READ isNotifyClick WRITE setNotifyClick )

  public:
    /**
     * Constructor.
     *
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param notifyClick @p true causes signals to be emitted.
     */
    KTextBrowser( TQWidget *parent=0, const char *name=0,
		  bool notifyClick=false );

    /**
     * Destructor.
     */
    ~KTextBrowser( void );

    /**
     * Decide whether a click on a link should be handled internally
     * or if a signal should be emitted.
     *
     * @param notifyClick @p true causes signals to be emitted.
     */
    void setNotifyClick( bool notifyClick );
    /**
     * Returns whether a click on a link should be handled internally
     * or if a signal should be emitted.
     */
    bool isNotifyClick() const;

  protected:
    /**
       Reimplemented to NOT set the source but to do the special handling.
       Do not call.
     */
    void setSource(const TQString& name);

    /**
     * Makes sure Key_Escape is ignored
     */
    virtual void keyPressEvent(TQKeyEvent *e);

    /**
     * Make sure we use our own hand cursor
     */
    virtual void viewportMouseMoveEvent( TQMouseEvent* e);

    /**
     * Reimplemented to support Qt2 behavior (Ctrl-Wheel = fast scroll)
     */
    virtual void contentsWheelEvent( TQWheelEvent *e );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See TQLineEdit::createPopupMenu().
    */
    virtual TQPopupMenu *createPopupMenu( const TQPoint &pos );

  signals:
    /**
     * Emitted when a mail link has been activated and the widget has
     * been configured to emit the signal.
     *
     * @param name The destination name. It is TQString::null at the moment.
     * @param address The destination address.
     */
    void mailClick( const TQString &name, const TQString &address );

    /**
     * Emitted if mailClick() is not emitted and the widget has been
     * configured to emit the signal.
     *
     * @param url The destination address.
     */
    void urlClick( const TQString &url );

  private:
    bool    mNotifyClick;

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KTextBrowserPrivate;
    KTextBrowserPrivate *d;
};

#endif
