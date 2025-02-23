/*
    This file is part of libtdeabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCEDIR_H
#define KABC_RESOURCEDIR_H

#include <tdeconfig.h>
#include <kdirwatch.h>

#include <sys/types.h>

#include <tdeabc/resource.h>

class TQTimer;

namespace TDEABC {

class FormatPlugin;
class Lock;

/**
  @internal
*/
class KABC_EXPORT ResourceDir : public Resource
{
  TQ_OBJECT

  public:
    ResourceDir( const TDEConfig* );
    ResourceDir( const TQString &path, const TQString &type = "vcard" );
    ~ResourceDir();

    virtual void writeConfig( TDEConfig* );

    virtual bool doOpen();
    virtual void doClose();
  
    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket* ticket );
    virtual bool asyncSave( Ticket* ticket );

    /**
      Set path to be used for saving.
     */
    void setPath( const TQString & );

    /**
      Return path used for loading and saving the address book.
     */
    TQString path() const;

    /**
      Set the format by name.
     */
    void setFormat( const TQString &format );

    /**
      Returns the format name.
     */
    TQString format() const;
  
    /**
      Remove a addressee from its source.
      This method is mainly called by TDEABC::AddressBook.
     */
    virtual void removeAddressee( const Addressee& addr );

  protected slots:
    void pathChanged();

  protected:
    void init( const TQString &path, const TQString &format );

  private:
    FormatPlugin *mFormat;

    KDirWatch mDirWatch;

    TQString mPath;
    TQString mFormatName;

    Lock *mLock;

    bool mAsynchronous;

    class ResourceDirPrivate;
    ResourceDirPrivate *d;
};

}

#endif
