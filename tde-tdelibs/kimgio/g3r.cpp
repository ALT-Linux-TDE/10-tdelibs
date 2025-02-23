// This library is distributed under the conditions of the GNU LGPL.

#include "config.h"

#ifdef HAVE_LIBTIFF

#include <tiffio.h>

#include <tqimage.h>
#include <tqfile.h>

#include "g3r.h"

TDE_EXPORT void kimgio_g3_read( TQImageIO *io )
{
    // This won't work if io is not a TQFile !
  TIFF *tiff = TIFFOpen(TQFile::encodeName(io->fileName()), "r");  
  if (!tiff)
    return;
 
  uint32 width, height;
  tsize_t scanlength;

  if( TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width ) != 1
      || TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height ) != 1 )
      return;
  scanlength = TIFFScanlineSize(tiff);

  TQImage image(width, height, 1, 0, TQImage::BigEndian);
  
  if (image.isNull() || scanlength != image.bytesPerLine())
    {
      TIFFClose(tiff);
      return;
    }

  for (uint32 y=0; y < height; y++)
    TIFFReadScanline(tiff, image.scanLine(y), y);

  TIFFClose(tiff);
  
  io->setImage(image);
  io->setStatus(0);
}


TDE_EXPORT void kimgio_g3_write(TQImageIO *)
{
	// TODO: stub
}


#endif
