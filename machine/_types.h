/*
 *  $Id$
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H
// #include <machine/_default_types.h>
#include <stdint.h>
typedef uint32_t time_t;
typedef uint32_t suseconds_t;
#define	_TIME_T_DECLARED
#define	_SUSECONDS_T_DECLARED
typedef void* clockid_t;
#define _CLOCKID_T_DECLARED

//#warning "Declared __Time_T!!!!!!!!!!!!!"

#endif
