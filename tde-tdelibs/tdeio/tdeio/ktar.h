/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __ktar_h
#define __ktar_h

#include <sys/stat.h>
#include <sys/types.h>

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdict.h>

#include <karchive.h>

/**
 * A class for reading / writing (optionally compressed) tar archives.
 *
 * KTar allows you to read and write tar archives, including those 
 * that are compressed using gzip, bzip2, or xz.
 * 
 * @author Torben Weis <weis@kde.org>, David Faure <faure@kde.org>
 */
class TDEIO_EXPORT KTar : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/weis/myfile.tgz")
     * @param mimetype "application/x-gzip", "application/x-bzip2",
     * or "application/x-xz"
     * Do not use application/x-tgz or similar - you only need to
     * specify the compression layer !  If the mimetype is omitted, it
     * will be determined from the filename.
     */
    KTar( const TQString& filename, const TQString & mimetype = TQString::null );

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (TQFile, etc.).
     * @warning Do not assume that giving a TQFile here will decompress the file,
     * in case it's compressed!
     * @param dev the device to read from. If the source is compressed, the
     * TQIODevice must take care of decompression
     */
    KTar( TQIODevice * dev );

    /**
     * If the tar ball is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KTar();

    /**
     * The name of the tar file, as passed to the constructor
     * Null if you used the TQIODevice constructor.
     * @return the name of the file, or TQString::null if unknown
     */
    TQString fileName() { return m_filename; } // TODO KDE4 const

    /**
     * Special function for setting the "original file name" in the gzip header,
     * when writing a tar.gz file. It appears when using in the "file" command,
     * for instance. Should only be called if the underlying device is a KFilterDev!
     * @param fileName the original file name
     */
    void setOrigFileName( const TQCString & fileName );

    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeSymLink(const TQString &name, const TQString &target,
    			const TQString &user, const TQString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);
    virtual bool writeDir( const TQString& name, const TQString& user, const TQString& group );
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeDir( const TQString& name, const TQString& user, const TQString& group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime );
    virtual bool prepareWriting( const TQString& name, const TQString& user, const TQString& group, uint size );
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool prepareWriting( const TQString& name, const TQString& user,
    			const TQString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime );
    virtual bool doneWriting( uint size );

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * @param mode the mode of the file
     */
    virtual bool openArchive( int mode );
    virtual bool closeArchive();

private:
    /**
     * @internal
     */
    void prepareDevice( const TQString & filename, const TQString & mimetype, bool forced = false );

    /**
     * @internal
     * Fills @p buffer for writing a file as required by the tar format
     * Has to be called LAST, since it does the checksum
     * (normally, only the name has to be filled in before)
     * @param mode is expected to be 6 chars long, [uname and gname 31].
     */
    void fillBuffer( char * buffer, const char * mode, int size, time_t mtime,
    		char typeflag, const char * uname, const char * gname );

    /**
     * @internal
     * Writes an overlong name into a special longlink entry. Call this
     * if the file name or symlink target (or both) are longer than 99 chars.
     * @p buffer buffer at least 0x200 bytes big to be used as a write buffer
     * @p name 8-bit encoded file name to be written
     * @p typeflag specifying the type of the entry, 'L' for filenames or
     *		'K' for symlink targets.
     * @p uname user name
     * @p gname group name
     */
    void writeLonglink(char *buffer, const TQCString &name, char typeflag,
			const char *uname, const char *gname);

    TQ_LONG readRawHeader(char *buffer);
    bool readLonglink(char *buffer,TQCString &longlink);
    TQ_LONG readHeader(char *buffer,TQString &name,TQString &symlink);

    TQString m_filename;
protected:
    virtual void virtual_hook( int id, void* data );
    bool prepareWriting_impl(const TQString& name, const TQString& user,
    			const TQString& group, uint size, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime);
    bool writeDir_impl(const TQString& name, const TQString& user,
    			const TQString& group, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime );
    bool writeSymLink_impl(const TQString &name, const TQString &target,
    			const TQString &user, const TQString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);
private:
    class KTarPrivate;
    KTarPrivate * d;
};

/**
 * Old, deprecated naming
 */
#define KTarGz KTar
#define KTarEntry KArchiveEntry
#define KTarFile KArchiveFile
#define KTarDirectory KArchiveDirectory

#endif
