/*
   This file is part of the KDE libraries.
   Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSWAP_H
#define KSWAP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqglobal.h>

/** 
 * \defgroup KSWAP Byte-swapping functions
 * kswap.h contains functions that will help converting
 * 16, 32 and 64 bit length data between little-endian and
 * big-endian representations.
 *
 * The KSWAP_16, KSWAP_32 and KSWAP_64 functions are always
 * swaps the byte order of the supplied argument (which should be
 * 16, 32 or 64 bit wide). These functions are inline, and tries to
 * use the most optimized function of the underlying system
 * (bswap_xx functions from byteswap.h in GLIBC, or ntohs and ntohl
 * on little-endian machines, and if neither are applicable, some fast
 * custom code).
 *
 * The KFromTo{Little|Big}Endian functions are for converting big-endian and 
 * little-endian data to and from the machine endianness.
 */

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>

  inline TQ_UINT16 KSWAP_16( TQ_UINT16 b ) { return bswap_16( b ); }
  inline TQ_INT16 KSWAP_16( TQ_INT16 b ) { return bswap_16( (TQ_UINT16)b ); }
  inline TQ_UINT32 KSWAP_32( TQ_UINT32 b ) { return bswap_32( b ); }
  inline TQ_INT32 KSWAP_32( TQ_INT32 b ) { return bswap_32( (TQ_UINT32)b ); }
  inline TQ_UINT64 KSWAP_64( TQ_UINT64 b ) { return bswap_64( b ); }
  inline TQ_INT64 KSWAP_64( TQ_INT64 b ) { return bswap_64( (TQ_UINT64)b ); }

#else /* HAVE_BYTESWAP_H */
#ifdef WORDS_BIGENDIAN
  inline TQ_UINT16 KSWAP_16( TQ_UINT16 b ) 
  { 
    return (((b) & 0x00ff) << 8 | ((b) & 0xff00) >> 8); 
  }

  inline TQ_INT16 KSWAP_16( TQ_INT16 b ) 
  { 
    return ((((TQ_UINT16)b) & 0x00ff) << 8 | (((TQ_UINT16)b) & 0xff00) >> 8); 
  }

  inline TQ_UINT32 KSWAP_32( TQ_UINT32 b ) 
  {
    return
      ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
       (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24)); 
  }

  inline TQ_INT32 KSWAP_32( TQ_INT32 b ) 
  {
    return 
      (((((TQ_UINT32)b) & 0xff000000) >> 24) | ((((TQ_UINT32)b) & 0x00ff0000) >>  8) | \
       ((((TQ_UINT32)b) & 0x0000ff00) <<  8) | ((((TQ_UINT32)b) & 0x000000ff) << 24)); 
  }
#else /* WORDS_BIGENDIAN */
#include <sys/types.h>
#include <netinet/in.h>

  inline TQ_UINT16 KSWAP_16( TQ_UINT16 b ) { return htons(b); }
  inline TQ_INT16 KSWAP_16( TQ_INT16 b ) { return htons((TQ_UINT16)b); }
  inline TQ_UINT32 KSWAP_32( TQ_UINT32 b ) { return htonl(b); }
  inline TQ_INT32 KSWAP_32( TQ_INT32 b ) { return htonl((TQ_UINT32)b); }
#endif
  inline TQ_UINT64 KSWAP_64( TQ_UINT64 b ) 
  {
    union { 
        TQ_UINT64 ll;
        TQ_UINT32 l[2]; 
    } w, r;
    w.ll = b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }

  inline TQ_INT64 KSWAP_64( TQ_INT64 b ) 
  {
    union { 
        TQ_UINT64 ll;
        TQ_UINT32 l[2]; 
    } w, r;
    w.ll = (TQ_UINT64) b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }
#endif	/* !HAVE_BYTESWAP_H */

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline TQ_UINT16 KFromToBigEndian( TQ_UINT16 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_UINT16 *out, TQ_UINT16 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline TQ_UINT32 KFromToBigEndian( TQ_UINT32 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_UINT32 *out, TQ_UINT32 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline TQ_UINT64 KFromToBigEndian( TQ_UINT64 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_UINT64 *out, TQ_UINT64 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline TQ_INT16 KFromToBigEndian( TQ_INT16 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_INT16 *out, TQ_INT16 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline TQ_INT32 KFromToBigEndian( TQ_INT32 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_INT32 *out, TQ_INT32 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline TQ_INT64 KFromToBigEndian( TQ_INT64 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( TQ_INT64 *out, TQ_INT64 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline TQ_UINT16 KFromToLittleEndian( TQ_UINT16 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_UINT16 *out, TQ_UINT16 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline TQ_UINT32 KFromToLittleEndian( TQ_UINT32 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_UINT32 *out, TQ_UINT32 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline TQ_UINT64 KFromToLittleEndian( TQ_UINT64 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_UINT64 *out, TQ_UINT64 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline TQ_INT16 KFromToLittleEndian( TQ_INT16 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_INT16 *out, TQ_INT16 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline TQ_INT32 KFromToLittleEndian( TQ_INT32 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_INT32 *out, TQ_INT32 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline TQ_INT64 KFromToLittleEndian( TQ_INT64 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( TQ_INT64 *out, TQ_INT64 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

#endif /* KSWAP_H */
