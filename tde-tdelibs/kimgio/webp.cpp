// WebP read support
// © 2024 Alexander Hajnal
// Based loosely on jp2.cpp
//
// If implementing write support it's suggested to use lossless mode with exact 
// mode enabled when the quality setting is 100 and for other qualities to use 
// lossy mode with the default settings.
//
// This library is distributed under the conditions of the GNU LGPL.
#include "config.h"

#include <tdetempfile.h>
#include <tqfile.h>
#include <tqimage.h>

#include <webp/decode.h>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

TDE_EXPORT void kimgio_webp_read( TQImageIO* io )
{
  int width, height;
  FILE* in;
  
  // === Read the source file ===
  // Based on code in jp2.cpp
  
  // for QIODevice's other than TQFile, a temp. file is used.
  KTempFile* tempf = 0;
  
  TQFile* qf = 0;
  if( ( qf = dynamic_cast<TQFile*>( io->ioDevice() ) ) ) {
    // great, it's a TQFile. Let's just take the filename.
    in = fopen( TQFile::encodeName( qf->name() ), "rb" );
  } else {
    // not a TQFile. Copy the whole data to a temp. file.
    tempf = new KTempFile();
    if( tempf->status() != 0 ) {
      delete tempf;
      return;
    } // if
    tempf->setAutoDelete( true );
    TQFile* out = tempf->file();
    // 4096 (=4k) is a common page size.
    TQByteArray b( 4096 );
    TQ_LONG size;
    // 0 or -1 is EOF / error
    while( ( size = io->ioDevice()->readBlock( b.data(), 4096 ) ) > 0 ) {
        // in case of a write error, still give the decoder a try
        if( ( out->writeBlock( b.data(), size ) ) == -1 ) break;
    } // while
    // flush everything out to disk
    out->flush();
    
    in = fopen( TQFile::encodeName( tempf->name() ), "rb" );
  } // else
  if( ! in ) {
      delete tempf;
      return;
  } // if
  
  // File is now open
  
  // === Load compressed data ===
  
  // Find file's size
  fseek(in, 0L, SEEK_END);   // Seek to end of file
  long size = ftell(in);     // Get position (i.e. the file size)
  fseek(in, 0L, SEEK_SET);   // Seek back to start of file
  
  // Sanity check
  if ( size > SIZE_MAX ) {
    // File size is larger than a size_t can hold
    fclose( in );
    delete tempf;
    return;
  }
  
  // Allocate a buffer for the compressed data
  uint8_t* compressed_image = (uint8_t*)malloc(size);
  if( ! compressed_image ) {
    // malloc failed
    fclose( in );
    delete tempf;
    return;
  } // if
  
  // Read compressed image into buffer
  size_t bytes_read = fread( compressed_image, sizeof(uint8_t), size, in );
  
  // Close the compressed image file
  fclose( in );
  delete tempf;
  
  if ( bytes_read < size ) {
    // Read failed
    free( compressed_image );
    return;
  }
  
  // === Decompress image ===
  
  // Get image dimensions
  if ( ! WebPGetInfo( compressed_image, size, &width, &height ) ) {
    // Error
    free( compressed_image );
    return;
  }
  
  // Create an appropriately sized image
  TQImage image;
  if( ! image.create( width, height, 32 ) ) {
    // Error
    free( compressed_image );
    return;
  }
  
  // Enable alpha channel
  image.setAlphaBuffer(true);
  
  // Get the image buffer
  uint32_t* data = (uint32_t*)image.bits();
  
  // Decompress the image
#ifdef WORDS_BIGENDIAN
  if ( ! WebPDecodeARGBInto( compressed_image, size, (uint8_t*)data, width*height*4, width*4) ) {
#else
  if ( ! WebPDecodeBGRAInto( compressed_image, size, (uint8_t*)data, width*height*4, width*4) ) {
#endif
    // Error
    free( compressed_image );
    return;
  }
  
  // Free the compressed image buffer
  free( compressed_image );
  
  // Finalize load
  io->setImage( image );
  io->setStatus( 0 );
} // kimgio_webp_read

#ifdef __cplusplus
}
#endif
