#include <string.h>
/*! \brief сшивание строк
 */
//char* strncat(char *a, const char *b, size_t length) __attribute__((optimize("O3")));
char* strncat(char *a, const char *b, size_t length)
{
	register char* s = a;
	register char c;
	while (*s != '\0') s++;
	if(length!=0) {
		do {
			c = *b++;
			if (c=='\0') break;
			*s++ = c;
		} while (--length);
		*s = '\0';
	}
	return a;
}
