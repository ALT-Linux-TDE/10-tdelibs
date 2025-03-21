/*  This file is part of the KDE libraries
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef _KCUSTOMMENUEDITOR_H_
#define _KCUSTOMMENUEDITOR_H_

#include <kdialogbase.h>

class TDEListView;
class TDEConfigBase;

 /*
  * Dialog for editing custom menus.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @since 3.1
  */
class TDEIO_EXPORT KCustomMenuEditor : public KDialogBase
{
    TQ_OBJECT
public:
    /**
     * Create a dialog for editing a custom menu
     */
    KCustomMenuEditor(TQWidget *parent);
    ~KCustomMenuEditor();
    /**
     * load the custom menu
     */
    void load(TDEConfigBase *);

    /**
     * save the custom menu
     */
    void save(TDEConfigBase *);

public slots:
    void slotNewItem();
    void slotRemoveItem();
    void slotMoveUp();
    void slotMoveDown();
    void refreshButton();

protected:
    class Item;
    TDEListView *m_listView;

    class KCustomMenuEditorPrivate;
    KCustomMenuEditorPrivate *d;
};

#endif
