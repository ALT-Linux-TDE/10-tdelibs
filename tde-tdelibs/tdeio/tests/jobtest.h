/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#ifndef JOBTEST_H
#define JOBTEST_H

#include <tqstring.h>
#include <tqobject.h>
#include <tdeio/job.h>

class JobTest : public TQObject
{
    TQ_OBJECT

public:
    JobTest() {}
    void setup();
    void runAll();
    void cleanup();

    // Local tests (tdeio_file only)
    void get();
    void copyFileToSamePartition();
    void copyDirectoryToSamePartition();
    void copyDirectoryToExistingDirectory();
    void copyFileToOtherPartition();
    void copyDirectoryToOtherPartition();
    void listRecursive();
    void moveFileToSamePartition();
    void moveDirectoryToSamePartition();
    void moveFileToOtherPartition();
    void moveSymlinkToOtherPartition();
    void moveDirectoryToOtherPartition();
    void moveFileNoPermissions();
    void moveDirectoryNoPermissions();

    // Remote tests
    void copyFileToSystem();

private slots:
    void slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList& lst );
    void slotGetResult( TDEIO::Job* );
    void slotMimetype(TDEIO::Job*,const TQString&);

private:
    TQString homeTmpDir() const;
    TQString otherTmpDir() const;
    TQString realSystemPath() const;
    KURL systemTmpDir() const;
    enum { AlreadyExists = 1 };
    void copyLocalFile( const TQString& src, const TQString& dest );
    void copyLocalDirectory( const TQString& src, const TQString& dest, int flags = 0 );
    void moveLocalFile( const TQString& src, const TQString& dest );
    void moveLocalDirectory( const TQString& src, const TQString& dest );
    void copyFileToSystem( bool resolve_local_urls );

    int m_result;
    TQByteArray m_data;
    TQStringList m_names;
    TQString m_mimetype;
};

#endif
