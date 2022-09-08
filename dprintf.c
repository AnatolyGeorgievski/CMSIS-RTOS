#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
int dprintf(int fildes, const char *pFormat, ...)
{
    va_list ap;
    int rc;

    va_start(ap, pFormat);
    rc = vdprintf(fildes, pFormat, ap);
    va_end(ap);

    return rc;
}
int vdprintf(int fildes, const char *pFormat, va_list ap)
{
	struct stat st;
	int rc;
	
	//fstat(fildes, &st);
	Device_t * dev = DEV_PTR(fildes);
	FILE* file = (FILE*)(dev+1);
	char *addr = mmap(0, BUFSIZ, PROT_WRITE, MAP_PRIVATE, fildes, file->offset);
    rc = vsnprintf(addr, BUFSIZ, pFormat, ap);
	fdatasync(fildes);
	munmap(addr, BUFSIZ);
    return rc;
}