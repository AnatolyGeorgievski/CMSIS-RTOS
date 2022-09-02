#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

// The <sys/mman.h> header shall define the following symbolic constants for use as protection options:
#define PROT_EXEC 	4	/* Page can be executed. */
#define PROT_NONE 	0	/* Page cannot be accessed. */
#define PROT_READ	1	/* Page can be read. */
#define PROT_WRITE	2	/* Page can be written. */

// The <sys/mman.h> header shall define the following symbolic constants for use as flag options:
#define MAP_FIXED	2	//    Interpret addr exactly.
#define MAP_PRIVATE 0	//    Changes are private.
#define MAP_SHARED	1	//     Share changes. 

void  *mmap(void *, size_t, int, int, int, off_t);
int    mprotect(void *, size_t, int);
int    munmap(void *, size_t);
// SHM _POSIX_SHARED_MEMORY_OBJECTS
int    shm_open(const char *, int, mode_t);
int    shm_unlink(const char *);
#endif//_SYS_MMAN_H_