/***************************************************************************
                                kiso.h
                             -------------------
    begin                : Oct 25 2002
    copyright            : (C) 2002 by Szombathelyi Gy�gy
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

 /* This file is heavily based on ktar.h from tdelibs
  * (c) Torben Weis <weis@kde.org>, David Faure <faure@kde.org>
  */

#ifndef KISO_H
#define KISO_H

#include <sys/stat.h>
#include <sys/types.h>

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdict.h>

#include "kisofile.h"
#include "kisodirectory.h"

/**
 * @short A class for reading (optionnally compressed) iso9660 files.
 * @author Gy�gy Szombathelyi <gyurco@users.sourceforge.net>,
 * Torben Weis <weis@kde.org>, David Faure <faure@kde.org>
 */
class KIso : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename.
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/weis/myfile.tgz")
     * @param mimetype "application/x-gzip" or "application/x-bzip2"
     * Do not use application/x-tgz or so. Only the compression layer !
     * If the mimetype is ommitted, it will be determined from the filename.
     */
    KIso( const TQString& filename, const TQString & mimetype = TQString::null );

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (TQFile, etc.).
     * WARNING: don't assume that giving a TQFile here will decompress the file,
     * in case it's compressed!
     */
    KIso( TQIODevice * dev );

    /**
     * If the .iso is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KIso();

    /**
     * The name of the os file, as passed to the constructor
     * Null if you used the TQIODevice constructor.
     */
    TQString fileName() { return m_filename; }

    bool writeDir( const TQString& , const TQString& , const TQString& );
    bool prepareWriting( const TQString& , const TQString& , const TQString& , uint );
    bool doneWriting( uint );

    void setStartSec(int startsec) { m_startsec = startsec; }
    int startSec() { return m_startsec; }

    bool showhidden,showrr;
    int level,joliet;
    KIsoDirectory *dirent;
protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     *
     */
    void readParams();
    virtual bool openArchive( int mode );
    virtual bool closeArchive();

private:
    /**
     * @internal
     */
    void addBoot(struct el_torito_boot_descriptor* bootdesc);
    void prepareDevice( const TQString & filename, const TQString & mimetype, bool forced = false );
    int m_startsec;

    TQString m_filename;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KIsoPrivate;
    KIsoPrivate * d;
};

#endif
