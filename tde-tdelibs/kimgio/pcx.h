/* This file is part of the KDE project
   Copyright (C) 2002-2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PCX_H
#define PCX_H

#include <tqglobal.h>
#include <tqdatastream.h>
#include <tqcolor.h>

class TQImageIO;

extern "C"
{
  void kimgio_pcx_read( TQImageIO * );
  void kimgio_pcx_write( TQImageIO * );
}

class RGB
{
  public:
    RGB() { }

    RGB( const TQRgb color )
    {
      r = tqRed( color );
      g = tqGreen( color );
      b = tqBlue( color );
    }

    TQ_UINT8 r;
    TQ_UINT8 g;
    TQ_UINT8 b;
};

class Palette
{
  public:
    Palette() { }

    void setColor( int i, const TQRgb color )
    {
      rgb[ i ] = RGB( color );
    }

    TQRgb color( int i ) const
    {
      return tqRgb( rgb[ i ].r, rgb[ i ].g, rgb[ i ].b );
    }

    struct RGB rgb[ 16 ];
};

class PCXHEADER
{
  public:
    PCXHEADER();

    inline int width() const { return ( XMax-XMin ) + 1; }
    inline int height() const { return ( YMax-YMin ) + 1; }
    inline bool isCompressed() const { return ( Encoding==1 ); }

    TQ_UINT8  Manufacturer;    // Constant Flag, 10 = ZSoft .pcx
    TQ_UINT8  Version;         // Version information
                              // 0 = Version 2.5 of PC Paintbrush
                              // 2 = Version 2.8 w/palette information
                              // 3 = Version 2.8 w/o palette information
                              // 4 = PC Paintbrush for Windows(Plus for
                              //     Windows uses Ver 5)
                              // 5 = Version 3.0 and > of PC Paintbrush
                              //     and PC Paintbrush +, includes
                              //     Publisher's Paintbrush . Includes
                              //     24-bit .PCX files
    TQ_UINT8  Encoding;        // 1 = .PCX run length encoding
    TQ_UINT8  Bpp;             // Number of bits to represent a pixel
                              // (per Plane) - 1, 2, 4, or 8
    TQ_UINT16 XMin;
    TQ_UINT16 YMin;
    TQ_UINT16 XMax;
    TQ_UINT16 YMax;
    TQ_UINT16 HDpi;
    TQ_UINT16 YDpi;
    Palette  ColorMap;
    TQ_UINT8  Reserved;        // Should be set to 0.
    TQ_UINT8  NPlanes;         // Number of color planes
    TQ_UINT16 BytesPerLine;    // Number of bytes to allocate for a scanline
                              // plane.  MUST be an EVEN number.  Do NOT
                              // calculate from Xmax-Xmin.
    TQ_UINT16 PaletteInfo;     // How to interpret palette- 1 = Color/BW,
                              // 2 = Grayscale ( ignored in PB IV/ IV + )
    TQ_UINT16 HScreenSize;     // Horizontal screen size in pixels. New field
                              // found only in PB IV/IV Plus
    TQ_UINT16 VScreenSize;     // Vertical screen size in pixels. New field
                              // found only in PB IV/IV Plus
} TDE_PACKED;

#endif // PCX_H
