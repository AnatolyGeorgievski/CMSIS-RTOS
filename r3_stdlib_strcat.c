#include <string.h>
/*! \brief сшивание строк
 */
//char* strcat(char *a, const char *b) __attribute__((optimize("O3")));
char* strcat(char *a, const char *b) 
{
	register char* s = a;
	while (s[0]!='\0') s++;
	while ((*s++ = *b++) != '\0');
	return a;
}
