// This library is distributed under the conditions of the GNU LGPL.
#ifndef XVIEW_H
#define XVIEW_H

class TQImageIO;

extern "C" {   
void kimgio_xv_read( TQImageIO * );
void kimgio_xv_write( TQImageIO * );
}

#endif
