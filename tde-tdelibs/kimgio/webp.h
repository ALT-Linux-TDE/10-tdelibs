// WebP read support
// © 2024 Alexander Hajnal
// Based on jp2.h
//
// This library is distributed under the conditions of the GNU LGPL.
#ifndef KIMG_WEBP_H
#define KIMG_WEBP_H

class TQImageIO;

extern "C" {
  void kimgio_webp_read( TQImageIO* io );
//  void kimgio_webp_write( TQImageIO* io ); // Not currently supported
} // extern "C"

#endif

