/**
* TQImageIO Routines to read/write g3 (fax) images.
* (c) 2000, Matthias Hölzer-Klüpfel
*
* This library is distributed under the conditions of the GNU LGPL.
*
* $Id$
*/

#ifndef KIMG_G3R_H
#define KIMG_G3R_H

class TQImageIO;

extern "C" {
  void kimgio_g3_read( TQImageIO *io );
  void kimgio_g3_write( TQImageIO *io );
}

#endif
