/**
* TQImageIO Routines to read (and perhaps in the future, write) images
* in the high definition EXR format.
*
* Copyright (c) 2003, Brad Hards <bradh@frogmouth.net>
*
* This library is distributed under the conditions of the GNU LGPL.
*
*/

#ifndef KIMG_EXR_H
#define KIMG_EXR_H

class TQImageIO;

extern "C" {
  void kimgio_exr_read (TQImageIO *io);
  void kimgio_exr_write (TQImageIO *io);
}

#endif
