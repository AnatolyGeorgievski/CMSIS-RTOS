#include <string.h>
/*! \brief копирование строки */
//char *strcpy(char * restrict out, const char * restrict src)  __attribute__((optimize("O3")));
char *strcpy(char * restrict out, const char * restrict src)
{
	char * dst = out;
	register char c; 
	c=*src; 
	*dst=c; 
	if(c) do{ 
		c=*++src; 
		*++dst=c; 
	}while (c != '\0');
	return out;
}
