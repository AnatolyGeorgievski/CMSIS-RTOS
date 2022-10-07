#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

// symbolic constants for use as protection options:
#define PROT_EXEC 	4	/* Page can be executed. */
#define PROT_NONE 	0	/* Page cannot be accessed. */
#define PROT_READ	1	/* Page can be read. */
#define PROT_WRITE	2	/* Page can be written. */

// symbolic constants for use as flag options:
#define MAP_FIXED	2	//    Interpret addr exactly.
#define MAP_PRIVATE 0	//    Changes are private.
#define MAP_SHARED	1	//     Share changes. 

#define MAP_FAILED	((void*)0)

// symbolic constants for the msync( ) function:
#define MS_ASYNC 		2// Perform asynchronous writes.
#define MS_INVALIDATE 	1// Invalidate mappings.
#define MS_SYNC 		0// Perform synchronous writes.
// ADV _POSIX_ADVISORY_INFO
// symbolic constants for the advice argument to the posix_madvise( ) function as follows:
#define POSIX_MADV_DONTNEED	0
/*!< The application expects that it will not access the specified range in the near future. */
#define POSIX_MADV_NORMAL	0
/*!< The application has no advice to give on its behavior with respect to the specified range. It
is the default characteristic if no advice is given for a range of memory. */
#define POSIX_MADV_RANDOM	0
/*!< The application expects to access the specified range in a random order. */
#define POSIX_MADV_SEQUENTIAL	1
/*!< The application expects to access the specified range sequentially from lower addresses to
higher addresses. */
#define POSIX_MADV_WILLNEED		2
/*!< The application expects to access the specified range in the near future. */
// symbolic constants for use as flags for the posix_typed_mem_open( ) function:
#define POSIX_TYPED_MEM_ALLOCATE		0	//!< Allocate on mmap( ).
#define POSIX_TYPED_MEM_ALLOCATE_CONTIG	1	//!< Allocate contiguously on mmap( ).
#define POSIX_TYPED_MEM_MAP_ALLOCATABLE	2	//!< Map on mmap( ), without affecting allocatability

void  *mmap  (void *, size_t, int, int, int, off_t);
int    mprotect(void *, size_t, int);
int    munmap(void *, size_t);
// SHM _POSIX_SHARED_MEMORY_OBJECTS
int    shm_open(const char *, int, mode_t);
int    shm_unlink(const char *);
// ADV
int 	posix_madvise(void *, size_t, int);
// TYM
int 	posix_mem_offset(const void *restrict, size_t, off_t *restrict, size_t *restrict, int *restrict);

struct 	posix_typed_mem_info {
	size_t posix_tmi_length;	// Maximum length which may be allocated from a typed memory object.
};

int 	posix_typed_mem_get_info(int, struct posix_typed_mem_info *);
int 	posix_typed_mem_open(const char *, int, int);
#endif//_SYS_MMAN_H_