#ifndef _DIRENT_H_
#define _DIRENT_H_

struct dirent {
//	ino_t d_ino; // File serial number
	const char *d_name; // Filename string of entry
};

struct posix_dent { 
	ino_t d_ino;			// File serial number.
	unsigned char d_type;	// File type or unknown-file-type indication. |
	reclen_t d_reclen;		// Length of this entry, including trailing
	char d_name[];			// Filename string of this entry. 
};
typedef struct _dir DIR;
struct _dir {
	struct _dentry * list;
	const struct _htable * htable;
};
/* определить d_type для структуры директории posix_dent
DT_BLK Block special. +
DT_CHR Character special. +
DT_DIR Directory. +
DT_FIFO FIFO special. +
DT_LNK Symbolic link. +
DT_REG Regular. +
DT_SOCK Socket. +
DT_UNKNOWN 
DT_MQ Message queue.
DT_SEM Semaphore. +
DT_SHM Shared memory object. +
DT_TMO Typed memory object. 
*/

int alphasort(const struct dirent **, const struct dirent **);
int closedir(DIR *);
int dirfd(DIR *);
DIR *fdopendir(int);
DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
// POSIX-202x Issue 8
ssize_t posix_getdents(int, void *, size_t, int);

#endif//_DIRENT_H_