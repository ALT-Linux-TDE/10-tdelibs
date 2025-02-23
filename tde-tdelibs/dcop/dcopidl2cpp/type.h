#ifndef TYPE_H
#define TYPE_H

#include <tqtextstream.h>
#include <tqdom.h>

static TQString writeType( TQTextStream& str, const TQDomElement& r )
{
  Q_ASSERT( r.tagName() == "TYPE" );
  if ( r.hasAttribute( "qleft" ) )
    str << r.attribute("qleft") << " ";
  TQString t = r.firstChild().toText().data();
  t = t.replace( ">>", "> >" );
  str << t;
  if ( r.hasAttribute( "qright" ) )
    str << r.attribute("qright") << " ";
  else
    str << " ";
  return t;
}

#endif
