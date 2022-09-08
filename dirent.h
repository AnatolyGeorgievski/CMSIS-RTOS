#ifndef _DIRENT_H_
#define _DIRENT_H_

struct dirent {
//	ino_t d_ino; // File serial number
	const char *d_name; // Filename string of entry
};

typedef struct _dir DIR;
struct _dir {
	struct _dentry * list;
	const struct _htable * htable;
};

int alphasort(const struct dirent **, const struct dirent **);
int closedir(DIR *);
int dirfd(DIR *);
DIR *fdopendir(int);
DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
#endif//_DIRENT_H_