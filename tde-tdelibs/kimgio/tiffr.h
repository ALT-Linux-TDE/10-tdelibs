/**
* TQImageIO Routines to read/write TIFF images.
* Sirtaj Singh Kang, Oct 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*
* $Id$
*/

#ifndef KIMG_TIFFR_H
#define KIMG_TIFFR_H

class TQImageIO;

extern "C" {
void kimgio_tiff_read( TQImageIO *io );
void kimgio_tiff_write( TQImageIO *io );
}

#endif
