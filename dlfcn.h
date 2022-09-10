#ifndef _DLFCN_H_
#define _DLFCN_H_
<<<<<<< HEAD
/*! \defgroup POSIX_DYNAMIC_LINKING POSIX: Dynamic Linking
	\ingroup _posix

dlclose( ), dlerror( ), dlopen( ), dlsym( )
	\{ */
=======

>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
// dlopen( ) mode argument:
#define RTLD_LAZY 	2	//!< Relocations are performed at an implementation-defined time.
#define RTLD_NOW 	0	//!< Relocations are performed when the object is loaded.
#define RTLD_GLOBAL	1	//!< All symbols are available for relocation processing of other modules.
#define RTLD_LOCAL	0	//!< All symbols are not made available for relocation processing by other modules.

int   dlclose(void *);
char *dlerror(void);
void *dlopen(const char *, int);
void *dlsym (void *restrict, const char *restrict);
<<<<<<< HEAD
	//! \}
=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif//_DLFCN_H_