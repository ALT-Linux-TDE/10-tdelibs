/*  This file is part of the KDE libraries
    Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// $Id$

#include <kdatastream.h> // Do not remove, needed for correct bool serialization
#include <kurl.h>
#include <tdeapplication.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <tdefilemetainfo.h>
#include <tdelocale.h>
#include <stdlib.h>

#include "metainfo.h"

// Recognized metadata entries:
// mimeType     - the mime type of the file, so we need not extra determine it
// what         - what to load
 
using namespace TDEIO;

extern "C"
{
    TDE_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    TDEApplication app(argc, argv, "tdeio_metainfo", false, true, false);

    if (argc != 4)
    {
        kdError() << "Usage: tdeio_metainfo protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    MetaInfoProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    return 0;
}

MetaInfoProtocol::MetaInfoProtocol(const TQCString &pool, const TQCString &app)
    : SlaveBase("metainfo", pool, app)
{
}

MetaInfoProtocol::~MetaInfoProtocol()
{
}

void MetaInfoProtocol::get(const KURL &url)
{
    TQString mimeType = metaData("mimeType");
    KFileMetaInfo info(url.path(), mimeType);
    
    TQByteArray arr;
    TQDataStream stream(arr, IO_WriteOnly);

    stream << info;

    data(arr);
    finished();
}

void MetaInfoProtocol::put(const KURL& url, int, bool, bool) 
{
    TQString mimeType = metaData("mimeType");
    KFileMetaInfo info;
    
    TQByteArray arr;
    readData(arr);
    TQDataStream stream(arr, IO_ReadOnly);
    
    stream >> info;

    if (info.isValid())
    {
        info.applyChanges();
    } 
    else 
    {
        error(ERR_NO_CONTENT, i18n("No metainfo for %1").arg(url.path()));
        return;
    }
    finished();
}  
