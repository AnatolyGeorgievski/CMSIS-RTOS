#ifndef SYS__TYPES_H
#define SYS__TYPES_H

#define __need_wint_t

#include <stddef.h>
#include "machine/_types.h"

#define __need_wint_t/* Conversion state information.  */
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;            /* Value so far.  */
} _mbstate_t;

typedef unsigned long useconds_t;
typedef int32_t ssize_t;
typedef long _off_t;
typedef long _fpos_t;              /* XXX must match off_t in <sys/types.h> */

#endif
