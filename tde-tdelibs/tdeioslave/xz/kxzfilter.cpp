/* This file is part of the KDE libraries
   Copyright (C) 2007-2008 Per Øyvind Karlsen <peroyvind@mandriva.org>

   Based on kbzip2filter:
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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

#include "kxzfilter.h"
#include <klibloader.h>

#include <config.h>
#if HAVE_XZ_SUPPORT
extern "C" {
	#include <lzma.h>
}

#include <kdebug.h>

#include <tqiodevice.h>

// #define DEBUG_XZ

class KXzFilterFactory : public KLibFactory
{
public:
    KXzFilterFactory() : KLibFactory() {}
    virtual ~KXzFilterFactory(){}
    TQObject *createObject( TQObject *, const char *, const char*, const TQStringList & )
    {
        return new KXzFilter;
    }
};

K_EXPORT_COMPONENT_FACTORY( kxzfilter, KXzFilterFactory )


class KXzFilter::Private
{
public:
    Private()
    : isInitialized(false)
    {
        memset(&zStream, 0, sizeof(zStream));
        mode = 0;
    }

    lzma_stream zStream;
    int mode;
    bool isInitialized;
};

KXzFilter::KXzFilter()
    :d(new Private)
{
}


KXzFilter::~KXzFilter()
{
    delete d;
}

void KXzFilter::init( int mode )
{
    if (d->isInitialized) {
        terminate();
    }
  
    lzma_ret result;
    d->zStream.next_in = 0;
    d->zStream.avail_in = 0;
    if ( mode == IO_ReadOnly )
    {
	/* We set the memlimit for decompression to 100MiB which should be
	 * more than enough to be sufficient for level 9 which requires 65 MiB.
	 */
        result = lzma_auto_decoder(&d->zStream, 100<<20, 0);
    	//kdDebug(7131) << "lzma_auto_decoder returned " << result;
    } else if ( mode == IO_WriteOnly ) {
        result = lzma_easy_encoder(&d->zStream, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC32);
    	//kdDebug(7131) << "lzma_easy_encoder returned " << result;
    } else
        kdWarning(7131) << "Unsupported mode " << mode << ". Only IO_ReadOnly and IO_WriteOnly supported";
    d->mode = mode;
    d->isInitialized = true;
}

int KXzFilter::mode() const
{
    return d->mode;
}

void KXzFilter::terminate()
{
    if (d->mode == IO_ReadOnly || d->mode == IO_WriteOnly) {
        lzma_end(&d->zStream);
    } else {
        kdWarning(7131) << "Unsupported mode " << d->mode << ". Only IO_ReadOnly and IO_WriteOnly supported";
    }
    d->isInitialized = false;
}


void KXzFilter::reset()
{
    //kdDebug(7131) << "KXzFilter::reset";
    // liblzma doesn't have a reset call...
    terminate();
    init( d->mode );
}

void KXzFilter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = (uint8_t *)data;
}

void KXzFilter::setInBuffer( const char *data, unsigned int size )
{
    d->zStream.avail_in = size;
    d->zStream.next_in = (uint8_t *)const_cast<char *>(data);
}

int KXzFilter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}

int KXzFilter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

KXzFilter::Result KXzFilter::uncompress()
{
    //kdDebug(7131) << "Calling lzma_code with avail_in=" << inBufferAvailable() << " avail_out =" << outBufferAvailable();
    lzma_ret result = lzma_code(&d->zStream, LZMA_RUN);
    if ( result != LZMA_OK )
    {
        kdDebug(7131) << "lzma_code returned " << result;
        kdDebug(7131) << "KXzFilter::uncompress " << ( result == LZMA_STREAM_END ? END : ERROR );
    }

    switch (result) {
        case LZMA_OK:
                return OK;
        case LZMA_STREAM_END:
                return END;
        default:
                return ERROR;
    }
}

KXzFilter::Result KXzFilter::compress( bool finish )
{
    //kdDebug(7131) << "Calling lzma_code with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    lzma_ret result = lzma_code(&d->zStream, finish ? LZMA_FINISH : LZMA_RUN );

    switch (result) {
        case LZMA_OK:
                return OK;
                break;
        case LZMA_STREAM_END:
                kdDebug(7131) << "  lzma_code returned " << result;
                return END;
		break;
        default:
                kdDebug(7131) << "  lzma_code returned " << result;
                return ERROR;
                break;
    }
}

#endif  /* HAVE_XZ_SUPPORT */
