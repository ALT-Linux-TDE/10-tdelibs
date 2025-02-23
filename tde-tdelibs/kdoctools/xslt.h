#ifndef _MEIN_XSLT_H_
#define _MEIN_XSLT_H_

#include <libxml/parser.h>
#include <tqvaluevector.h>

TQString transform(const TQString &file, const TQString& stylesheet,
                  const TQValueVector<const char *> &params = TQValueVector<const char *>());
TQString splitOut(const TQString &parsed, int index);
void fillInstance(TDEInstance &ins, const TQString &srcdir = TQString::null );
bool saveToCache( const TQString &contents, const TQString &filename );
TQString lookForCache( const TQString &filename );
TQCString fromUnicode( const TQString &data );
void replaceCharsetHeader( TQString &output );

extern bool warnings_exist;
extern TQString *SRCDIR;

/**
 * Compares two files and returns true if @param newer exists and is newer than
 * @param older
 **/
bool compareTimeStamps( const TQString &older, const TQString &newer );
#endif
