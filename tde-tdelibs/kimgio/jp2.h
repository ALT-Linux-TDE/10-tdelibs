// This library is distributed under the conditions of the GNU LGPL.
#ifndef KIMG_JP2_H
#define KIMG_JP2_H

class TQImageIO;

extern "C" {
	void kimgio_jp2_read( TQImageIO* io );
	void kimgio_jp2_write( TQImageIO* io );
} // extern "C"

#endif

