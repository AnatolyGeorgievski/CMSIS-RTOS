#ifndef SYS__TYPES_H
#define SYS__TYPES_H

#include <stddef.h>


#include "machine/_types.h"
typedef unsigned int wint_t;
#define WEOF ((wint_t)-1)
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;            /* Value so far.  */
} _mbstate_t;

typedef unsigned long useconds_t; // системное определение для функции usleep
<<<<<<< HEAD
=======
typedef long _off_t;
typedef _off_t off_t;		/* XXX must match off_t in <sys/types.h> */
typedef long _fpos_t;       /* XXX must match fpos_t in <sys/types.h> */
//typedef void* pid_t;
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif
