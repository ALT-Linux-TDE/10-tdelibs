/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KPUSHBUTTON_H
#define KPUSHBUTTON_H

#include <tqpoint.h>
#include <tqpushbutton.h>
#include <kguiitem.h>
#include <kstdguiitem.h> 

class TQDragObject;
/**
 * This is nothing but a TQPushButton with drag-support and KGuiItem support. You have to call
 * setDragEnabled( true ) and override the virtual method
 * dragObject() to specify the TQDragObject to be used.
 *
 * @short A TQPushButton with drag-support and KGuiItem support
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEUI_EXPORT KPushButton : public TQPushButton
{
    TQ_OBJECT
    TQ_PROPERTY(int stdItem READ guiItm WRITE setGuiItm )
    TQ_PROPERTY(bool isDragEnabled READ isDragEnabled WRITE setDragEnabled)

public:

    /**
     * Default constructor.
     */
    KPushButton( TQWidget *parent, const char *name=0 );

    /**
     * Constructor, that sets the button-text to @p text
     */
    KPushButton( const TQString &text, TQWidget *parent, const char *name=0);

    /**
     * Constructor, that sets an icon and the button-text to @p text
     */
    KPushButton( const TQIconSet &icon, const TQString &text,
		     TQWidget *parent, const char *name=0 );

    /**
     * Constructor that takes a KGuiItem for the text, the icon, the tooltip
     * and the what's this help
     */
    KPushButton( const KGuiItem &item, TQWidget *parent, const char *name = 0 );

    /**
     * Destructs the button.
     */
    ~KPushButton();

    /**
     * Enables/disables drag-support. Default is disabled.
     */
    void setDragEnabled( bool enable );

    /**
     * @returns if drag support is enabled or not.
     */
    bool isDragEnabled() const { return m_dragEnabled; }

    /**
     * Sets the KGuiItem for this button.
     */
    void setGuiItem( const KGuiItem& item );

    /** 
    * Sets the standard KGuiItem for this button.
    * @since 3.4
    */
    void setGuiItem( KStdGuiItem::StdItem item );

    /**
    * Reads the standard KGuiItem for this button. 
    * @since 3.4
    */
    KStdGuiItem::StdItem guiItem() const;

    // Hack for Qt designer
    void setGuiItm(int itm ) { setGuiItem( (KStdGuiItem::StdItem)itm );}
    int guiItm() const { return (int)guiItem(); }

    /**
     * Sets the Icon Set for this button. It also takes into account hte
     * TDEGlobalSettings::showIconsOnPushButtons() setting.
     */
    void setIconSet( const TQIconSet &iconSet );

    /**
    * Sets the text of the button
    */
    void setText( const TQString &text );

protected:
    /**
     * Reimplement this and return the TQDragObject that should be used
     * for the drag.
     * Default implementation returns 0L, so that no drag is initiated.
     */
    virtual TQDragObject * dragObject();

    /**
     * Reimplemented to add drag-support
     */
    virtual void mousePressEvent( TQMouseEvent * );
    /**
     * Reimplemented to add drag-support
     */
    virtual void mouseMoveEvent( TQMouseEvent * );

    /**
     * Starts a drag (dragCopy() by default) using dragObject()
     */
    virtual void startDrag();

private:
    bool m_dragEnabled;
    TQPoint startPos;

private slots:
    void slotSettingsChanged( int category );

private:
    /**
     * Internal.
     * Initialize the KPushButton instance
     */
    void init( const KGuiItem &item );

    void readSettings();
    static bool s_useIcons;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KPushButtonPrivate;
    KPushButtonPrivate *d;

};

#endif // KPUSHBUTTON_H
