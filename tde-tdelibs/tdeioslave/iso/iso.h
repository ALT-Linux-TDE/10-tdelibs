/***************************************************************************
                                iso.h
                             -------------------
    begin                : Oct 25 2002
    copyright            : (C) 2002 by Szombathelyi György
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /* This file is heavily based on tar.h from tdebase
  * (c) David Faure <faure@kde.org>
  */

#ifndef _ISO_H
#define _ISO_H

#include <tdeio/slavebase.h>
#include <sys/types.h>
#include "kisofile.h"

class KIso;

class tdeio_isoProtocol : public TDEIO::SlaveBase
{
public:
    tdeio_isoProtocol( const TQCString &pool, const TQCString &app );
    virtual ~tdeio_isoProtocol();

    virtual void listDir( const KURL & url );
    virtual void stat( const KURL & url );
    virtual void get( const KURL & url );

protected:
    void getFile( const KIsoFile *isoFileEntry, const TQString &path );
    void createUDSEntry( const KArchiveEntry * isoEntry, TDEIO::UDSEntry & entry );
    bool checkNewFile( TQString fullPath, TQString & path, int startsec );

    KIso * m_isoFile;
    time_t m_mtime;
    int m_mode;
};

#endif
