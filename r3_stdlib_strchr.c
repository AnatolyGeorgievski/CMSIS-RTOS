#include <string.h>
/*! \brief поиск подходящего символа в строке */
//char *strchr(const char *str, int character)  __attribute__((optimize("O3")));
char *strchr(const char *str, int character)
{
    char ch;
    do {
        ch = *str++;
        if (ch=='\0') return NULL;
    } while (ch != (char)character);
	return (char*)str-1;
}
