
#include <unistd.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <r3_slice.h>/* В версии C2x использовать free_sized() */
// системно-зависимые функции:
extern FILE* dtree_path	 (FILE* f, const char* path, const char**name);
extern FILE* dtree_openat(FILE* f, const char* name, mode_t mode);
extern int 	 dtree_unref (FILE* f);

int 	fclose	(FILE *f){
	int rc = f->dev_class->close(f->phandle);
	if(dtree_unref(f)==0)
		g_slice_free(FILE, f);
	return rc;
}
#define O_BINARY 0
/*
r or rb 		Open file for reading.
w or wb 		Truncate to zero length or create file for writing.
a or ab 		Append; open or create file for writing at end-of-file.
r+ or rb+ or r+b Open file for update (reading and writing).
w+ or wb+ or w+b Truncate to zero length or create file for update.
a+ or ab+ or a+b Append; open or create file for update, writing at end-of-file.
*/
FILE *	fopen	(const char *restrict path, const char *restrict mods)
{
	mode_t mode=0;
	if (mods[0]=='r') mode |= O_RDONLY;
	else
	if (mods[0]=='w') mode |= O_WRONLY|O_CREAT|O_TRUNC;
	else
	if (mods[0]=='a') mode |= O_WRONLY|O_CREAT|O_APPEND;
	mods++;
	if (mods[0]=='b') mode |= O_BINARY, mods++;//
	if (mods[0]=='+') {
		mode &=~(O_RDONLY|O_WRONLY); 
		mode |= O_RDWR;
		mods++;
	}
	if (mods[0]=='b') mode |= O_BINARY;//
	
	const char* name = NULL;
	FILE* dev = dtree_path(NULL, path, &name);
	if (dev==NULL) return NULL;
	if (name!=NULL && (mode & O_CREAT)==0) return NULL;
	dev = dtree_openat(dev, name, mode);
	return dev;
}
int 	vfprintf(FILE *restrict f, const char *restrict format, va_list ap)
{
	// Если определено POSIX MEMORY MAPPING
	char* str = mmap(0, BUFSIZ, PROT_WRITE, MAP_PRIVATE, f->fildes, f->offset);
	int rc = vsnprintf(str, BUFSIZ, format, ap);
	f->size += rc;
	fdatasync(f->fildes);
	munmap(str, BUFSIZ);
	return rc;
}
int 	fprintf	(FILE *restrict f, const char *restrict format, ...)
{
	va_list ap;
	va_start(ap, format);
	int rc = vfprintf(f, format, ap);
	va_end(ap);
	return rc;
}
/*! \breaf put a string on a stream
 	\return Upon successful completion, fputs( ) shall return a non-negative number. Otherwise, it shall

[CX] return EOF, set an error indicator for the stream
*/
int 	fputs	(const char *restrict str, FILE *restrict f) {
	return f->dev_class->write(f->phandle, str, strlen(str));
}
FILE *	tmpfile	(void)
{
	return NULL;
}