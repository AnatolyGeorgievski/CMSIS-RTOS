#include <string.h>
/*! \brief заполнение сравнение первых n байт строк */
#if 0
int strncmp(const char *a, const char *b, size_t length)
{
	char dif=0;
	if(length) do{
		if((dif = *b++ - *a)) break;
	}while (*a++ && --length);
	return dif;
}
#endif
int strncmp (const char *s1, const char *s2, size_t n)
{
    unsigned char c1 = '\0';
    unsigned char c2 = '\0';
    while (n > 0){
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			break;
		n--;
    }
    return c1 - c2;
}
