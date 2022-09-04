#ifndef _STDIO_H_
#define _STDIO_H_

#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stdio.h>// Системно - зависимые определения

typedef _fpos_t fpos_t;
typedef struct _Device FILE;
#ifndef SEEK_SET
#define SEEK_SET        0       /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR        1       /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END        2       /* set file offset to EOF plus offset */
#endif

#define _IOFBF 2// Input/output fully buffered.
#define _IOLBF 1// Input/output line buffered.
#define _IONBF 0// Input/output unbuffered.
#define _ATTRIBUTE __attribute__

void 	clearerr(FILE *);
int 	dprintf(int, const char *restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
int 	fclose(FILE *);
FILE *	fdopen(int, const char *);
//int feof	(FILE *);
int ferror	(FILE *);
int fflush	(FILE *);
int fgetc	(FILE *);
int fgetpos	(FILE *restrict, fpos_t *restrict);
char *	fgets	(char *restrict, int, FILE *restrict);
//int 	fileno	(FILE *);
void 	flockfile(FILE *);
FILE *	fmemopen(void *restrict, size_t, const char *restrict);
FILE *	fopen	(const char *restrict, const char *restrict);
int 	fprintf(FILE *restrict, const char *restrict, ...);
int 	fputc	(int, FILE *);
int 	fputs	(const char *restrict, FILE *restrict);
//size_t 	fread	(void *restrict, size_t, size_t, FILE *restrict);
FILE *	freopen(const char *restrict, const char *restrict, FILE *restrict);
int fscanf(FILE *restrict, const char *restrict, ...);
int fseek(FILE *, long, int);
//int fseeko(FILE *, off_t, int);
int fsetpos(FILE *, const fpos_t *);
long ftell(FILE *);
//off_t ftello(FILE *);
int ftrylockfile(FILE *);
void funlockfile(FILE *);
//size_t fwrite(const void *restrict, size_t, size_t, FILE *restrict);
int getc	(FILE *);
int getchar	(void);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
ssize_t getdelim(char **restrict, size_t *restrict, int, FILE *restrict);
ssize_t getline(char **restrict, size_t *restrict, FILE *restrict);
int pclose	(FILE *);
void perror	(const char *);
FILE *popen	(const char *, const char *);
int printf	(const char *restrict, ...)
				_ATTRIBUTE ((__format__ (__printf__, 1, 2)));
int putc	(int, FILE *);
int putchar	(int);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);
int puts	(const char *);
int remove	(const char *);
int rename	(const char *, const char *);
int renameat(int, const char *, int, const char *);
void rewind	(FILE *);
int  scanf	(const char *restrict, ...);
void setbuf	(FILE *restrict, char *restrict);
int setvbuf	(FILE *restrict, char *restrict, int, size_t);
int snprintf(char *restrict, size_t, const char *restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));
int sprintf	(char *restrict, const char *restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
int sscanf	(const char *restrict, const char *restrict, ...);
FILE *tmpfile(void);
int ungetc	(int, FILE *);
int vdprintf(int, const char *restrict, va_list)
				_ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int vfprintf(FILE *restrict, const char *restrict, va_list);
int vfscanf	(FILE *restrict, const char *restrict, va_list);
int vprintf	(const char *restrict, va_list);
int vscanf	(const char *restrict, va_list);
int vsnprintf(char *restrict, size_t, const char *restrict, va_list);
int vsprintf(char *restrict, const char *restrict, va_list);
int vsscanf	(const char *restrict, const char *restrict, va_list);
static inline 
size_t fread	(      void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	return f->dev_class->read(f->phandle, ptr, size*nitems);
}
static inline 
size_t fwrite	(const void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	return f->dev_class->write(f->phandle, ptr, size*nitems);
}

static inline int 	fileno	(FILE *f){
	return f->fildes;
}
static inline off_t 	ftello	(FILE *f){
	return f->offset;
}
static inline int 	feof	(FILE *f){
	return f->offset == f->size;
}
static inline int     fseeko 	(FILE *f, off_t offset, int whence){
	f->dev_class->seek(f->phandle, offset, whence);
/*
	switch (whence) {
	case SEEK_SET: f->offset =offset; break;
	case SEEK_CUR: f->offset+=offset; break;
	case SEEK_END: f->offset =f->size + offset; 
		break;
	default: 
		return -1;
	}*/
	return 0;
}
#endif//_STDIO_H_