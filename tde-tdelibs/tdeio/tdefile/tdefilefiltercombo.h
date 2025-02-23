/* This file is part of the KDE libraries
    Copyright (C) Stephan Kulow <coolo@kde.org>

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

#ifndef TDEFILEFILTERCOMBO_H
#define TDEFILEFILTERCOMBO_H

#include <tqstringlist.h>
#include <tqptrdict.h>

#include <kcombobox.h>
#include <kmimetype.h>

class KFileFilterComboPrivate;

class TDEIO_EXPORT KFileFilterCombo : public KComboBox
{
    TQ_OBJECT

 public:
    KFileFilterCombo(TQWidget *parent= 0, const char *name= 0);
    ~KFileFilterCombo();

    void setFilter(const TQString& filter);

    /**
     * @returns the current filter, either something like "*.cpp *.h"
     * or the current mimetype, like "text/html", or a list of those, like
     " "text/html text/plain image/png", all separated with one space.
     */
    TQString currentFilter() const;

    /**
     * Sets the current filter. Filter must match one of the filter items
     * passed before to this widget.
     * @since 3.4
     */
    void setCurrentFilter( const TQString& filter );

    /**
     * Sets a list of mimetypes.
     * If @p defaultType is set, it will be set as the current item.
     * Otherwise, a first item showing all the mimetypes will be created.
     */
    void setMimeFilter( const TQStringList& types, const TQString& defaultType );

    /**
     * @return true if the filter's first item is the list of all mimetypes
     */
    bool showsAllTypes() const { return m_allTypes; }

    /**
     * This method allows you to set a default-filter, that is used when an
     * empty filter is set. Make sure you call this before calling
     * setFilter().
     *
     * By default, this is set to i18n("*|All Files")
     * @see defaultFilter
     */
    void setDefaultFilter( const TQString& filter );

    /**
     * @return the default filter, used when an empty filter is set.
     * @see setDefaultFilter
     */
    TQString defaultFilter() const;

 protected:
    virtual bool eventFilter( TQObject *o, TQEvent *e );

// KDE4: those variables are private. filters() was added
    TQStringList filters;
    bool m_allTypes;

 signals:
    void filterChanged();

private slots:
    void slotFilterChanged();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    friend class KFileDialog; // gone in KDE4
    class KFileFilterComboPrivate;
    KFileFilterComboPrivate *d;
};

#endif
