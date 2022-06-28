#include <string.h>
/*! \brief копирование строк заданной длины */
char *strncpy(char * restrict out, const char * restrict src, size_t length)
{
	char * dst = out;
//	if(length) while ((*dst++ =*src++) && --length);
	while (length-- && (*dst++ =*src++));
	return out;
}
