/* This header file provides the reentrancy.  */

/* WARNING: All identifiers here must begin with an underscore.  This file is
   included by stdio.h and others and we therefore must only use identifiers
   in the namespace allotted to us.  */
#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

#include <_ansi.h>
#include <stddef.h>
#include <sys/_types.h>
typedef struct _FILE __FILE;
struct _FILE {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
//  short	_flags;		/* flags, below; this FILE is free if 0 */
//  short	_file;		/* fileno, if Unix descriptor, else -1 */
	uint32_t object_identifier;
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */
  void* media;
};

extern struct _reent* _REENT;
struct _reent {
	struct _FILE* _stdin;
	struct _FILE* _stdout;
};


#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */