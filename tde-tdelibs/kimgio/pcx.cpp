/* This file is part of the KDE project
   Copyright (C) 2002-2005 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "pcx.h"

#include <tqimage.h>

#include <kdebug.h>

static TQDataStream &operator>>( TQDataStream &s, RGB &rgb )
{
  s >> rgb.r >> rgb.g >> rgb.b;

  return s;
}

static TQDataStream &operator>>( TQDataStream &s, Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s >> pal.rgb[ i ];

  return s;
}

static TQDataStream &operator>>( TQDataStream &s, PCXHEADER &ph )
{
  s >> ph.Manufacturer;
  s >> ph.Version;
  s >> ph.Encoding;
  s >> ph.Bpp;
  s >> ph.XMin >> ph.YMin >> ph.XMax >> ph.YMax;
  s >> ph.HDpi >> ph.YDpi;
  s >> ph.ColorMap;
  s >> ph.Reserved;
  s >> ph.NPlanes;
  s >> ph.BytesPerLine;
  s >> ph.PaletteInfo;
  s >> ph.HScreenSize;
  s >> ph.VScreenSize;

  // Skip the rest of the header
  TQ_UINT8 byte;
  while ( s.device()->at() < 128 )
    s >> byte;

  return s;
}

static TQDataStream &operator<<( TQDataStream &s, const RGB &rgb )
{
  s << rgb.r << rgb.g << rgb.b;

  return s;
}

static TQDataStream &operator<<( TQDataStream &s, const Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s << pal.rgb[ i ];

  return s;
}

static TQDataStream &operator<<( TQDataStream &s, const PCXHEADER &ph )
{
  s << ph.Manufacturer;
  s << ph.Version;
  s << ph.Encoding;
  s << ph.Bpp;
  s << ph.XMin << ph.YMin << ph.XMax << ph.YMax;
  s << ph.HDpi << ph.YDpi;
  s << ph.ColorMap;
  s << ph.Reserved;
  s << ph.NPlanes;
  s << ph.BytesPerLine;
  s << ph.PaletteInfo;
  s << ph.HScreenSize;
  s << ph.VScreenSize;

  TQ_UINT8 byte = 0;
  for ( int i=0; i<54; ++i )
    s << byte;

  return s;
}

PCXHEADER::PCXHEADER()
{
  // Initialize all data to zero
  TQByteArray dummy( 128 );
  dummy.fill( 0 );
  TQDataStream s( dummy, IO_ReadOnly );
  s >> *this;
}

static void readLine( TQDataStream &s, TQByteArray &buf, const PCXHEADER &header )
{
  TQ_UINT32 i=0;
  TQ_UINT32 size = buf.size();
  TQ_UINT8 byte, count;

  if ( header.isCompressed() )
  {
    // Uncompress the image data
    while ( i < size )
    {
      count = 1;
      s >> byte;
      if ( byte > 0xc0 )
      {
        count = byte - 0xc0;
        s >> byte;
      }
      while ( count-- && i < size )
        buf[ i++ ] = byte;
    }
  }
  else
  {
    // Image is not compressed (possible?)
    while ( i < size )
    {
      s >> byte;
      buf[ i++ ] = byte;
    }
  }
}

static void readImage1( TQImage &img, TQDataStream &s, const PCXHEADER &header )
{
  TQByteArray buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 1, 2, TQImage::BigEndian ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf, header );
    uchar *p = img.scanLine( y );
    unsigned int bpl = TQMIN((header.width()+7)/8, header.BytesPerLine);
    for ( unsigned int x=0; x< bpl; ++x )
      p[ x ] = buf[x];
  }

  // Set the color palette
  img.setColor( 0, tqRgb( 0, 0, 0 ) );
  img.setColor( 1, tqRgb( 255, 255, 255 ) );
}

static void readImage4( TQImage &img, TQDataStream &s, const PCXHEADER &header )
{
  TQByteArray buf( header.BytesPerLine*4 );
  TQByteArray pixbuf( header.width() );

  if(!img.create( header.width(), header.height(), 8, 16 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    pixbuf.fill( 0 );
    readLine( s, buf, header );

    for ( int i=0; i<4; i++ )
    {
      TQ_UINT32 offset = i*header.BytesPerLine;
      for ( unsigned int x=0; x<header.width(); ++x )
        if ( buf[ offset + ( x/8 ) ] & ( 128 >> ( x%8 ) ) )
          pixbuf[ x ] = static_cast<const char>(pixbuf.at(x)) + ( 1 << i );
    }

    uchar *p = img.scanLine( y );
    for ( unsigned int x=0; x<header.width(); ++x )
      p[ x ] = pixbuf[ x ];
  }

  // Read the palette
  for ( int i=0; i<16; ++i )
    img.setColor( i, header.ColorMap.color( i ) );
}

static void readImage8( TQImage &img, TQDataStream &s, const PCXHEADER &header )
{
  TQByteArray buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 8, 256 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf, header );

    uchar *p = img.scanLine( y );
    unsigned int bpl = TQMIN(header.BytesPerLine, header.width());
    for ( unsigned int x=0; x<bpl; ++x )
      p[ x ] = buf[ x ];
  }

  TQ_UINT8 flag;
  s >> flag;
  kdDebug( 399 ) << "Palette Flag: " << flag << endl;

  if ( flag == 12 && ( header.Version == 5 || header.Version == 2 ) )
  {
    // Read the palette
    TQ_UINT8 r, g, b;
    for ( int i=0; i<256; ++i )
    {
      s >> r >> g >> b;
      img.setColor( i, tqRgb( r, g, b ) );
    }
  }
}

static void readImage24( TQImage &img, TQDataStream &s, const PCXHEADER &header )
{
  TQByteArray r_buf( header.BytesPerLine );
  TQByteArray g_buf( header.BytesPerLine );
  TQByteArray b_buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 32 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, r_buf, header );
    readLine( s, g_buf, header );
    readLine( s, b_buf, header );

    uint *p = ( uint * )img.scanLine( y );
    for ( unsigned int x=0; x<header.width(); ++x )
      p[ x ] = tqRgb( r_buf[ x ], g_buf[ x ], b_buf[ x ] );
  }
}

TDE_EXPORT void kimgio_pcx_read( TQImageIO *io )
{
  TQDataStream s( io->ioDevice() );
  s.setByteOrder( TQDataStream::LittleEndian );

  if ( s.device()->size() < 128 )
  {
    io->setStatus( -1 );
    return;
  }

  PCXHEADER header;

  s >> header;

  if ( header.Manufacturer != 10 || s.atEnd())
  {
    io->setStatus( -1 );
    return;
  }

  int w = header.width();
  int h = header.height();

  kdDebug( 399 ) << "Manufacturer: " << header.Manufacturer << endl;
  kdDebug( 399 ) << "Version: " << header.Version << endl;
  kdDebug( 399 ) << "Encoding: " << header.Encoding << endl;
  kdDebug( 399 ) << "Bpp: " << header.Bpp << endl;
  kdDebug( 399 ) << "Width: " << w << endl;
  kdDebug( 399 ) << "Height: " << h << endl;
  kdDebug( 399 ) << "Window: " << header.XMin << "," << header.XMax << "," 
                 << header.YMin << "," << header.YMax << endl;
  kdDebug( 399 ) << "BytesPerLine: " << header.BytesPerLine << endl;
  kdDebug( 399 ) << "NPlanes: " << header.NPlanes << endl;

  TQImage img;

  if ( header.Bpp == 1 && header.NPlanes == 1 )
  {
    readImage1( img, s, header );
  }
  else if ( header.Bpp == 1 && header.NPlanes == 4 )
  {
    readImage4( img, s, header );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 1 )
  {
    readImage8( img, s, header );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 3 )
  {
    readImage24( img, s, header );
  }

  kdDebug( 399 ) << "Image Bytes: " << img.numBytes() << endl;
  kdDebug( 399 ) << "Image Bytes Per Line: " << img.bytesPerLine() << endl;
  kdDebug( 399 ) << "Image Depth: " << img.depth() << endl;

  if ( !img.isNull() )
  {
    io->setImage( img );
    io->setStatus( 0 );
  }
  else
  {
    io->setStatus( -1 );
  }
}

static void writeLine( TQDataStream &s, TQByteArray &buf )
{
  TQ_UINT32 i = 0;
  TQ_UINT32 size = buf.size();
  TQ_UINT8 count, data;
  char byte;

  while ( i < size )
  {
    count = 1;
    byte = buf[ i++ ];

    while ( ( i < size ) && ( TQChar(byte) == buf.at(i) ) && ( count < 63 ) )
    {
      ++i;
      ++count;
    }

    data = byte;

    if ( count > 1 || data >= 0xc0 )
    {
      count |= 0xc0;
      s << count;
    }

    s << data;
  }
}

static void writeImage1( TQImage &img, TQDataStream &s, PCXHEADER &header )
{
  img = img.convertBitOrder( TQImage::BigEndian );

  header.Bpp = 1;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  s << header;

  TQByteArray buf( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    TQ_UINT8 *p = img.scanLine( y );

    // Invert as TQImage uses reverse palette for monochrome images?
    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = ~p[ i ];

    writeLine( s, buf );
  }
}

static void writeImage4( TQImage &img, TQDataStream &s, PCXHEADER &header )
{
  header.Bpp = 1;
  header.NPlanes = 4;
  header.BytesPerLine = header.width()/8;

  for ( int i=0; i<16; ++i )
    header.ColorMap.setColor( i, img.color( i ) );

  s << header;

  TQByteArray buf[ 4 ];

  for ( int i=0; i<4; ++i )
      buf[ i ].resize( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    TQ_UINT8 *p = img.scanLine( y );

    for ( int i=0; i<4; ++i )
      buf[ i ].fill( 0 );

    for ( unsigned int x=0; x<header.width(); ++x )
    {
      for ( int i=0; i<4; ++i )
        if ( *( p+x ) & ( 1 << i ) )
          buf[ i ][ x/8 ] = buf[ i ].at(x/8) | 1 << ( 7-x%8 );
    }

    for ( int i=0; i<4; ++i )
      writeLine( s, buf[ i ] );
  }
}

static void writeImage8( TQImage &img, TQDataStream &s, PCXHEADER &header )
{
  header.Bpp = 8;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  s << header;

  TQByteArray buf( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    TQ_UINT8 *p = img.scanLine( y );

    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = p[ i ];

    writeLine( s, buf );
  }

  // Write palette flag
  TQ_UINT8 byte = 12;
  s << byte;

  // Write palette
  for ( int i=0; i<256; ++i )
    s << RGB( img.color( i ) );
}

static void writeImage24( TQImage &img, TQDataStream &s, PCXHEADER &header )
{
  header.Bpp = 8;
  header.NPlanes = 3;
  header.BytesPerLine = header.width();

  s << header;

  TQByteArray r_buf( header.width() );
  TQByteArray g_buf( header.width() );
  TQByteArray b_buf( header.width() );

  for ( int y=0; y<header.height(); ++y )
  {
    uint *p = ( uint * )img.scanLine( y );

    for ( unsigned int x=0; x<header.width(); ++x )
    {
      TQRgb rgb = *p++;
      r_buf[ x ] = tqRed( rgb );
      g_buf[ x ] = tqGreen( rgb );
      b_buf[ x ] = tqBlue( rgb );
    }

    writeLine( s, r_buf );
    writeLine( s, g_buf );
    writeLine( s, b_buf );
  }
}

TDE_EXPORT void kimgio_pcx_write( TQImageIO *io )
{
  TQDataStream s( io->ioDevice() );
  s.setByteOrder( TQDataStream::LittleEndian );

  TQImage img = io->image();

  int w = img.width();
  int h = img.height();

  kdDebug( 399 ) << "Width: " << w << endl;
  kdDebug( 399 ) << "Height: " << h << endl;
  kdDebug( 399 ) << "Depth: " << img.depth() << endl;
  kdDebug( 399 ) << "BytesPerLine: " << img.bytesPerLine() << endl;
  kdDebug( 399 ) << "Num Colors: " << img.numColors() << endl;

  PCXHEADER header;

  header.Manufacturer = 10;
  header.Version = 5;
  header.Encoding = 1;
  header.XMin = 0;
  header.YMin = 0;
  header.XMax = w-1;
  header.YMax = h-1;
  header.HDpi = 300;
  header.YDpi = 300;
  header.Reserved = 0;
  header.PaletteInfo =1;

  if ( img.depth() == 1 )
  {
    writeImage1( img, s, header );
  }
  else if ( img.depth() == 8 && img.numColors() <= 16 )
  {
    writeImage4( img, s, header );
  }
  else if ( img.depth() == 8 )
  {
    writeImage8( img, s, header );
  }
  else if ( img.depth() == 32 )
  {
    writeImage24( img, s, header );
  }

  io->setStatus( 0 );
}
