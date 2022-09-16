#ifndef SYS__TYPES_H
#define SYS__TYPES_H

#include <stddef.h>

#ifdef __arm__
#include "machine/_types.h"
#else
typedef __PTRDIFF_TYPE__ off_t;
typedef int     pid_t;
typedef int clockid_t;
typedef unsigned long clock_t;
#endif
typedef struct _mbstate _mbstate_t;
//typedef unsigned int wint_t;
//#define WEOF ((wint_t)-1)
/*
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;            // Value so far.  
} _mbstate_t;
*/
typedef unsigned long useconds_t; // системное определение для функции usleep
#endif
