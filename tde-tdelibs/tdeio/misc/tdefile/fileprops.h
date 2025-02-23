/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TDEFILEPROPS_H
#define TDEFILEPROPS_H

#include <tqstring.h>

#include <tdefilemetainfo.h>

class FileProps
{
public:
    FileProps( const TQString& path, const TQStringList& suppliedGroups );
    virtual ~FileProps();

    bool isValid() const;

    TQString fileName() const { return m_info->path(); }
    
    TQStringList supportedGroups() const;
    TQStringList availableGroups() const;
    TQStringList translatedGroups();

    const TQStringList& groupsToUse() const { return m_groupsToUse; }
    bool userSuppliedGroups() const { return m_userSuppliedGroups; }

    TQStringList supportedKeys( const TQString& group ) const;
    TQStringList availableKeys( const TQString& group ) const;
    TQStringList preferredKeys( const TQString& group ) const;

    TQStringList supportedKeys() const { return m_info->supportedKeys(); }
    TQStringList preferredKeys() const { return m_info->preferredKeys(); }

    TQString getValue( const TQString& group, const TQString& key ) const;
    bool setValue( const TQString& group,
                   const TQString& key, const TQString &value );

    TQStringList allValues( const TQString& group ) const;
    TQStringList preferredValues( const TQString& group ) const;

    bool isReadOnly( const TQString& group, const TQString& key );

private:
    static TQString createKeyValue( const KFileMetaInfoGroup& g,
                                   const TQString& key );
    static TQStringList createKeyValueList( const KFileMetaInfoGroup&,
                                           const TQStringList& );
    bool sync();

    KFileMetaInfo *m_info;
    bool m_dirty;
    bool m_userSuppliedGroups;

    TQStringList m_groupsToUse;

};

#endif // TDEFILEPROPS_H
