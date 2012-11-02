/* See md5.c for explanation and copyright information.  */

#ifndef MBUS_MD5_H
#define MBUS_MD5_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Unlike previous versions of this code, uint32 need not be exactly
   32 bits, merely 32 bits or more.  Choosing a data type which is 32
   bits instead of 64 is not important; speed is considerably more
   important.  ANSI guarantees that "unsigned long" will be big enough,
   and always using it seems to have few disadvantages.  */

struct MD5Context {
  guint32	buf[ 4 ];
  guint32	bits[ 2 ];
  guchar	in[ 64 ];
};

void MD5Init( struct MD5Context * context );
void MD5Update( struct MD5Context *context, guchar const * buf, guint len );
void MD5Final( guchar digest[ 16 ], struct MD5Context * context );
void MD5Transform( guint32 buf[ 4 ], const guchar in[ 64 ] );

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#ifdef __cplusplus
}
#endif

#endif /* MBUS_MD5_H */
