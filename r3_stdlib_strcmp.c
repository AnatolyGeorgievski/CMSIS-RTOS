#include <string.h>
/*! \brief сравнение строк */
#if 0
int strcmp(const char *a, const char *b)
{
  const unsigned char *s1 = (const unsigned char *) a;
  const unsigned char *s2 = (const unsigned char *) b;

	int dif=0;
	while(*s1 && *s2) {
		if((dif = *s1++ - *s2++)) break;
	}
	return dif;
}
#endif

int strcmp (const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;
	do {
	  c1 = (unsigned char) *s1++;
	  c2 = (unsigned char) *s2++;
	  if (c1 == '\0')
		break;
	} while (c1 == c2);
	return c1 - c2;
}
