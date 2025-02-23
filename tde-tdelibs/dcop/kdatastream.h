#ifndef __kdatastream__h
#define __kdatastream__h

#include <tqdatastream.h>

inline TQDataStream & operator << (TQDataStream & str, bool b)
{
  str << TQ_INT8(b);
  return str;
}

inline TQDataStream & operator >> (TQDataStream & str, bool & b)
{
  TQ_INT8 l;
  str >> l;
  b = bool(l);
  return str;
}

#endif
