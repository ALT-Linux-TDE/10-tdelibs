/*

(C) Daniel M. Duley <mosfet@kde.org>
(C) Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef __PANELAPPMENU_H
#define __PANELAPPMENU_H "$Id$"

#include <dcopobject.h>


/**
 * DCop client menu class for the Kicker panel that will insert a submenu into
 * the %TDE menu.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class TDEUI_EXPORT KPanelAppMenu : public TQObject, public DCOPObject
{
    TQ_OBJECT
public:
    /**
     * Constructs a new menu with the given title.
     */
    KPanelAppMenu(const TQString &title, TQObject *parent = 0, const char *name = 0 );
    /**
     * Same as above, but with an icon for the submenu item.
     */
    KPanelAppMenu(const TQPixmap &icon, const TQString &title, TQObject *parent,
              const char *name);
    /**
     * Constructs a new (sub)menu with a given id.
     */
    KPanelAppMenu(TQObject *parent, const char *name);

    /**
     * Make sure you actually destroy this thing or the menu will never go
     * away until this application quits or Kicker restarts ;-)
     */
    ~KPanelAppMenu();

    /**
     * Inserts an item into the menu.
     */
    int insertItem(const TQString &text, int id = -1);
    /**
     * Inserts an item with a icon. The icon name must be a full path.
     */
    int insertItem(const TQPixmap &icon, const TQString &text, int id = -1 );
    /**
     * Inserts a sub menu with a icon. The icon name must be a full path.
     */
    KPanelAppMenu *insertMenu(const TQPixmap &icon, const TQString &text, int id=-1);

    /**
     * Clears the menu.
     */
    void clear();

    /**
     * @internal
     */
    bool process(const TQCString &fun, const TQByteArray &data,
		 TQCString &replyType, TQByteArray &replyData);

signals:
    /**
     * Emitted when the user selects an item from the Kicker menu.
     */
    void activated(int id);

protected:
    void init(const TQPixmap &icon, const TQString &title);

private:
    TQCString realObjId;
};


#endif
