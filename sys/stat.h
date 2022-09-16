#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_
#include <sys/types.h>
#include <sys/_timespec.h>
#if 1
typedef unsigned int _dev_t;
typedef unsigned short _ino_t;
typedef __PTRDIFF_TYPE__ _off_t;

  struct stat {
    _dev_t st_dev;
    _ino_t st_ino;
    unsigned short st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    _dev_t st_rdev;
    _off_t st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
  };
#else

struct stat {
// The st_ino and st_dev fields taken together uniquely identify the file within the system
#if 0
  dev_t         st_dev;
  ino_t         st_ino;
  mode_t        st_mode;
  nlink_t       st_nlink;
  uid_t         st_uid;
  gid_t         st_gid;
  dev_t         st_rdev;
  off_t         st_size;
#endif

	ino_t 	st_ino:22; 	// File serial number.
	dev_t 	st_dev:10; 	// Device ID of device containing file.
	mode_t 	st_mode:16; 	// Mode of file.
	dev_t 	st_rdev:16;	// Device ID (if file is character or block special).
	nlink_t st_nlink;	// Number of hard links to the file.
#if 0
	uid_t 	st_uid;		// User ID of file.
	gid_t 	st_gid;		// Group ID of file.
#endif
	off_t 	st_size;	
	/* 	For regular files, the file size in bytes.
		For symbolic links, the length in bytes of the
		pathname contained in the symbolic link.
		SHM For a shared memory object, the length in bytes.
		TYM For a typed memory object, the length in bytes.
		For other file types, the use of this field is
		unspecified. */
struct timespec st_atim;	// Last data access timestamp.
struct timespec st_mtim;	// Last data modification timestamp.
struct timespec st_ctim;	// Last file status change timestamp.
/*[XSI] blksize_t st_blksize;	*/ /* A file system-specific preferred I/O block size
for this object. In some file system types, this
may vary from file to file.*/
/*	blkcnt_t 	st_blocks; */ /* Number of blocks allocated for this object. */
};
/* For compatibility with earlier versions of this standard, the st_atime macro shall be defined with
the value st_atim.tv_sec. Similarly, st_ctime and st_mtime shall be defined as macros with the
values st_ctim.tv_sec and st_mtim.tv_sec, respectively. */
#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
#endif

#define _IFMT           0170000 /* type of file */
#define         _IFIFO  0010000 /* fifo */
#define         _IFCHR  0020000 /* character special */
#define         _IFDIR  0040000 /* directory */
#define         _IFBLK  0060000 /* block special */
#define         _IFREG  0100000 /* regular */
#define         _IFLNK  0120000 /* symbolic link */
#define         _IFSOCK 0140000 /* socket */

// file types encoded in mode_t
#define S_IFMT  _IFMT	// Type of file.
#define S_IFBLK _IFBLK	// Block special.
#define S_IFCHR _IFCHR	// Character special.
#define S_IFIFO _IFIFO	// FIFO special.
#define S_IFREG _IFREG	// Regular.
#define S_IFDIR _IFDIR	// Directory.
#define S_IFLNK _IFLNK	// Symbolic link.
#define S_IFSOCK _IFSOCK// Socket

#define S_IRWXU 0700 	// Read, write, execute/search by owner.
#define S_IRUSR 0400 	// Read permission, owner.
#define S_IWUSR 0200 	// Write permission, owner.
#define S_IXUSR 0100 	// Execute/search permission, owner.
#define S_IRWXG  070 	// Read, write, execute/search by group.
#define S_IRGRP  040 	// Read permission, group.
#define S_IWGRP  020 	// Write permission, group.
#define S_IXGRP  010 	// Execute/search permission, group.
#define S_IRWXO   07	// Read, write, execute/search by others.
#define S_IROTH   04	// Read permission, others.
#define S_IWOTH   02	// Write permission, others.
#define S_IXOTH   01	// Execute/search permission, others.
#define S_ISUID 04000 	// Set-user-ID on execution.
#define S_ISGID 02000 	// Set-group-ID on execution.
#define S_ISVTX 01000 	// On directories, restricted deletion flag.

#define S_IREAD         0000400 /* read permission, owner */
#define S_IWRITE        0000200 /* write permission, owner */
#define S_IEXEC         0000100 /* execute/search permission, owner */

<<<<<<< HEAD
=======

#define S_IRWXU         (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG         (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO         (S_IROTH | S_IWOTH | S_IXOTH)

>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#define S_ISBLK(m)      (((m)&_IFMT) == _IFBLK)
#define S_ISCHR(m)      (((m)&_IFMT) == _IFCHR)
#define S_ISDIR(m)      (((m)&_IFMT) == _IFDIR)
#define S_ISFIFO(m)     (((m)&_IFMT) == _IFIFO)
#define S_ISREG(m)      (((m)&_IFMT) == _IFREG)
#define S_ISLNK(m)      (((m)&_IFMT) == _IFLNK)
#define S_ISSOCK(m)     (((m)&_IFMT) == _IFSOCK)
/* первое слово в структуре OID, под {ino_t:22,dev_t:10} можно определить тип объекта, 
следом идет mode_t;
*/
#define S_TYPEISMQ(buf)  ((buf)->st_dev == DEV_MQ) // Test for a message queue.
#define S_TYPEISSEM(buf) ((buf)->st_dev == DEV_SEM)	// Test for a semaphore.
#define S_TYPEISSHM(buf) ((buf)->st_dev == DEV_SHM)	// Test for a shared memory object.
#define S_TYPEISTMO(buf) ((buf)->st_dev == DEV_TMO)	// Test macro for a typed memory object
/* Special tv_nsec values for futimens(2) and utimensat(2). */
#define UTIME_NOW       -2L
#define UTIME_OMIT      -1L

<<<<<<< HEAD
int      stat  (const char *restrict __path, struct stat *restrict __sbuf );
int     fstat  (int __fd, struct stat *__sbuf );
int 	fstatat(int __fd, const char *restrict, struct stat *restrict, int);
int      chmod (const char *__path, mode_t __mode );
int     fchmod (int __fd, mode_t __mode);
int 	fchmodat(int, const char *, mode_t, int);
int     mknod  (const char *__path, mode_t __mode, dev_t __dev );
int     mkdir  (const char *__path, mode_t __mode );
int 	mkdirat(int, const char *, mode_t);
int     mkfifo (const char *__path, mode_t __mode );
int 	mkfifoat(int, const char *, mode_t);
mode_t  umask (mode_t __mask );
int     futimens (int __fd, const struct timespec __times[2]);
int 	 utimensat(int __fd, const char *__path, const struct timespec __times[2], int __flag);
=======
int     fstat (int __fd, struct stat *__sbuf );
int     fchmod(int __fd, mode_t __mode);
int     chmod (const char *__path, mode_t __mode );
int     mknod (const char *__path, mode_t __mode, dev_t __dev );
int     mkdir (const char *__path, mode_t __mode );
int     mkfifo(const char *__path, mode_t __mode );
int     stat  (const char *restrict __path, struct stat *restrict __sbuf );
int     lstat (const char *restrict __path, struct stat *restrict __buf );
mode_t  umask (mode_t __mask );
int     futimens (int __fd, const struct timespec *);
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif//_SYS_STAT_H_