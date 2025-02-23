
/*
 * $Id$
 * kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 *
 */   

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include <tqimage.h>
#include <tqbitmap.h>
#include <tqapplication.h>
#include <tqmemarray.h>
#include <tqpaintdevicemetrics.h>

#include <tdelibs_export.h>

#include "ico.h"

namespace
{
    // Global header
    struct IcoHeader
    {
        enum Type { Icon = 1, Cursor };
        TQ_UINT16 reserved;
        TQ_UINT16 type;
        TQ_UINT16 count;
    };

    inline TQDataStream& operator >>( TQDataStream& s, IcoHeader& h )
    {
        return s >> h.reserved >> h.type >> h.count;
    }

    // Based on qt_read_dib et al. from qimage.cpp
    // (c) 1992-2002 Trolltech AS.
    struct BMP_INFOHDR
    {
        static const TQ_UINT32 Size = 40;
        TQ_UINT32  biSize;                // size of this struct
        TQ_UINT32  biWidth;               // pixmap width
        TQ_UINT32  biHeight;              // pixmap height
        TQ_UINT16  biPlanes;              // should be 1
        TQ_UINT16  biBitCount;            // number of bits per pixel
        enum Compression { RGB = 0 };
        TQ_UINT32  biCompression;         // compression method
        TQ_UINT32  biSizeImage;           // size of image
        TQ_UINT32  biXPelsPerMeter;       // horizontal resolution
        TQ_UINT32  biYPelsPerMeter;       // vertical resolution
        TQ_UINT32  biClrUsed;             // number of colors used
        TQ_UINT32  biClrImportant;        // number of important colors
    };
    const TQ_UINT32 BMP_INFOHDR::Size;
  
    TQDataStream& operator >>( TQDataStream &s, BMP_INFOHDR &bi )
    {
        s >> bi.biSize;
        if ( bi.biSize == BMP_INFOHDR::Size )
        {
            s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
            s >> bi.biCompression >> bi.biSizeImage;
            s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
            s >> bi.biClrUsed >> bi.biClrImportant;
        }
        return s;
    }

#if 0
    TQDataStream &operator<<( TQDataStream &s, const BMP_INFOHDR &bi )
    {
        s << bi.biSize;
        s << bi.biWidth << bi.biHeight;
        s << bi.biPlanes;
        s << bi.biBitCount;
        s << bi.biCompression;
        s << bi.biSizeImage;
        s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
        s << bi.biClrUsed << bi.biClrImportant;
        return s;
    }
#endif

    // Header for every icon in the file
    struct IconRec
    {
        unsigned char width;
        unsigned char height;
        TQ_UINT16 colors;
        TQ_UINT16 hotspotX;
        TQ_UINT16 hotspotY;
        TQ_UINT32 size;
        TQ_UINT32 offset;
    };

    inline TQDataStream& operator >>( TQDataStream& s, IconRec& r )
    {
        return s >> r.width >> r.height >> r.colors
                 >> r.hotspotX >> r.hotspotY >> r.size >> r.offset;
    }

    struct LessDifference
    {
        LessDifference( unsigned s, unsigned c )
            : size( s ), colors( c ) {}

        bool operator ()( const IconRec& lhs, const IconRec& rhs ) const
        {
            // closest size match precedes everything else
            if ( std::abs( int( lhs.width - size ) ) <
                 std::abs( int( rhs.width - size ) ) ) return true;
            else if ( std::abs( int( lhs.width - size ) ) >
                 std::abs( int( rhs.width - size ) ) ) return false;
            else if ( colors == 0 )
            {
                // high/true color requested
                if ( lhs.colors == 0 ) return true;
                else if ( rhs.colors == 0 ) return false;
                else return lhs.colors > rhs.colors;
            }
            else
            {
                // indexed icon requested
                if ( lhs.colors == 0 && rhs.colors == 0 ) return false;
                else if ( lhs.colors == 0 ) return false;
                else return std::abs( int( lhs.colors - colors ) ) <
                            std::abs( int( rhs.colors - colors ) );
            }
        }
        unsigned size;
        unsigned colors;
    };

    bool loadFromDIB( TQDataStream& stream, const IconRec& rec, TQImage& icon )
    {
        BMP_INFOHDR header;
        stream >> header;
        if ( stream.atEnd() || header.biSize != BMP_INFOHDR::Size ||
             header.biSize > rec.size ||
             header.biCompression != BMP_INFOHDR::RGB ||
             ( header.biBitCount != 1 && header.biBitCount != 4 &&
               header.biBitCount != 8 && header.biBitCount != 24 &&
               header.biBitCount != 32 ) ) return false;

        unsigned paletteSize, paletteEntries;

        if (header.biBitCount > 8)
        {
            paletteEntries = 0;
            paletteSize    = 0;
        }
        else
        {
            paletteSize    = (1 << header.biBitCount);
            paletteEntries = paletteSize;
            if (header.biClrUsed && header.biClrUsed < paletteSize)
                paletteEntries = header.biClrUsed;
        }
        
        // Always create a 32-bit image to get the mask right
        // Note: this is safe as rec.width, rec.height are bytes
        icon.create( rec.width, rec.height, 32 );
        if ( icon.isNull() ) return false;
        icon.setAlphaBuffer( true );

        TQMemArray< TQRgb > colorTable( paletteSize );
        
        colorTable.fill( TQRgb( 0 ) );
        for ( unsigned i = 0; i < paletteEntries; ++i )
        {
            unsigned char rgb[ 4 ];
            stream.readRawBytes( reinterpret_cast< char* >( &rgb ),
                                 sizeof( rgb ) );
            colorTable[ i ] = tqRgb( rgb[ 2 ], rgb[ 1 ], rgb[ 0 ] );
        }

        unsigned bpl = ( rec.width * header.biBitCount + 31 ) / 32 * 4;
        
        unsigned char* buf = new unsigned char[ bpl ];
        unsigned char** lines = icon.jumpTable();
        for ( unsigned y = rec.height; !stream.atEnd() && y--; )
        {
            stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
            unsigned char* pixel = buf;
            TQRgb* p = reinterpret_cast< TQRgb* >( lines[ y ] );
            switch ( header.biBitCount )
            {
                case 1:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = colorTable[
                            ( pixel[ x / 8 ] >> ( 7 - ( x & 0x07 ) ) ) & 1 ];
                    break;
                case 4:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        if ( x & 1 ) *p++ = colorTable[ pixel[ x / 2 ] & 0x0f ];
                        else *p++ = colorTable[ pixel[ x / 2 ] >> 4 ];
                    break;
                case 8:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = colorTable[ pixel[ x ] ];
                    break;
                case 24:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = tqRgb( pixel[ 3 * x + 2 ],
                                     pixel[ 3 * x + 1 ],
                                     pixel[ 3 * x ] );
                    break;
                case 32:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = tqRgba( pixel[ 4 * x + 2 ],
                                      pixel[ 4 * x + 1 ],
                                      pixel[ 4 * x ],
                                      pixel[ 4 * x  + 3] );
                    break;
            }
        }
        delete[] buf;

        if ( header.biBitCount < 32 )
        {
            // Traditional 1-bit mask
            bpl = ( rec.width + 31 ) / 32 * 4;
            buf = new unsigned char[ bpl ];
            for ( unsigned y = rec.height; y--; )
            {
                stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
                TQRgb* p = reinterpret_cast< TQRgb* >( lines[ y ] );
                for ( unsigned x = 0; x < rec.width; ++x, ++p )
                    if ( ( ( buf[ x / 8 ] >> ( 7 - ( x & 0x07 ) ) ) & 1 ) )
                        *p &= TQT_RGB_MASK;
            }
            delete[] buf;
        }
        return true;
    }
}

extern "C" TDE_EXPORT void kimgio_ico_read( TQImageIO* io )
{
    TQIODevice::Offset offset = io->ioDevice()->at();

    TQDataStream stream( io->ioDevice() );
    stream.setByteOrder( TQDataStream::LittleEndian );
    IcoHeader header;
    stream >> header;
    if ( stream.atEnd() || !header.count ||
         ( header.type != IcoHeader::Icon && header.type != IcoHeader::Cursor) )
        return;

    TQPaintDeviceMetrics metrics( TQApplication::desktop() );
    unsigned requestedSize = 32;
    unsigned requestedColors = metrics.depth() > 8 ? 0 : metrics.depth();
    int requestedIndex = -1;
    if ( io->parameters() )
    {
        TQStringList params = TQStringList::split( ';', io->parameters() );
        TQMap< TQString, TQString > options;
        for ( TQStringList::ConstIterator it = params.begin();
              it != params.end(); ++it )
        {
            TQStringList tmp = TQStringList::split( '=', *it );
            if ( tmp.count() == 2 ) options[ tmp[ 0 ] ] = tmp[ 1 ];
        }
        if ( options[ "index" ].toUInt() )
            requestedIndex = options[ "index" ].toUInt();
        if ( options[ "size" ].toUInt() )
            requestedSize = options[ "size" ].toUInt();
        if ( options[ "colors" ].toUInt() )
            requestedColors = options[ "colors" ].toUInt();
    }

    typedef std::vector< IconRec > IconList;
    IconList icons;
    for ( unsigned i = 0; i < header.count; ++i )
    {
        if ( stream.atEnd() ) return;
        IconRec rec;
        stream >> rec;
        icons.push_back( rec );
    }
    IconList::const_iterator selected;
    if (requestedIndex >= 0) {
        selected = std::min( icons.begin() + requestedIndex, icons.end() );
    } else {
        selected = std::min_element( icons.begin(), icons.end(),
                          LessDifference( requestedSize, requestedColors ) );
    }
    if ( stream.atEnd() || selected == icons.end() ||
         offset + selected->offset > io->ioDevice()->size() )
        return;

    io->ioDevice()->at( offset + selected->offset );
    TQImage icon;
    if ( loadFromDIB( stream, *selected, icon ) )
    {
        icon.setText( "X-Index", 0, TQString::number( selected - icons.begin() ) );
        if ( header.type == IcoHeader::Cursor )
        {
            icon.setText( "X-HotspotX", 0, TQString::number( selected->hotspotX ) );
            icon.setText( "X-HotspotY", 0, TQString::number( selected->hotspotY ) );
        }
        io->setImage(icon);
        io->setStatus(0);
    }
}

#if 0
void kimgio_ico_write(TQImageIO *io)
{
    if (io->image().isNull())
        return;

    TQByteArray dibData;
    TQDataStream dib(dibData, IO_ReadWrite);
    dib.setByteOrder(TQDataStream::LittleEndian);

    TQImage pixels = io->image();
    TQImage mask;
    if (io->image().hasAlphaBuffer())
        mask = io->image().createAlphaMask();
    else
        mask = io->image().createHeuristicMask();
    mask.invertPixels();
    for ( int y = 0; y < pixels.height(); ++y )
        for ( int x = 0; x < pixels.width(); ++x )
            if ( mask.pixel( x, y ) == 0 ) pixels.setPixel( x, y, 0 );

    if (!qt_write_dib(dib, pixels))
        return;

   uint hdrPos = dib.device()->at();
    if (!qt_write_dib(dib, mask))
        return;
    memmove(dibData.data() + hdrPos, dibData.data() + hdrPos + BMP_WIN + 8, dibData.size() - hdrPos - BMP_WIN - 8);
    dibData.resize(dibData.size() - BMP_WIN - 8);
        
    TQDataStream ico(io->ioDevice());
    ico.setByteOrder(TQDataStream::LittleEndian);
    IcoHeader hdr;
    hdr.reserved = 0;
    hdr.type = Icon;
    hdr.count = 1;
    ico << hdr.reserved << hdr.type << hdr.count;
    IconRec rec;
    rec.width = io->image().width();
    rec.height = io->image().height();
    if (io->image().numColors() <= 16)
        rec.colors = 16;
    else if (io->image().depth() <= 8)
        rec.colors = 256;
    else
        rec.colors = 0;
    rec.hotspotX = 0;
    rec.hotspotY = 0;
    rec.dibSize = dibData.size();
    ico << rec.width << rec.height << rec.colors
        << rec.hotspotX << rec.hotspotY << rec.dibSize;
    rec.dibOffset = ico.device()->at() + sizeof(rec.dibOffset);
    ico << rec.dibOffset;

    BMP_INFOHDR dibHeader;
    dib.device()->at(0);
    dib >> dibHeader;
    dibHeader.biHeight = io->image().height() << 1;
    dib.device()->at(0);
    dib << dibHeader;

    ico.writeRawBytes(dibData.data(), dibData.size());
    io->setStatus(0);
}
#endif
