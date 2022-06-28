#include <stdint.h>
#include <string.h>
/*! \brief сравнение двух буферов */
int memcmp(const void * restrict s1, const void * restrict s2, size_t n)
{
	const uint8_t* c2 = s2;
	const uint8_t* c1 = s1;
	int dif=0;
	while ((n--) && (dif = c2[n]-c1[n])==0);
	return dif;
}
