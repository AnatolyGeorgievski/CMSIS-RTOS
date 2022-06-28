#include <string.h>
/*! \brief копирование буфера 
	Внимание: компилятор пытается оптимизировать путем вызова memcpy 
	-fno-tree-loop-distribute-patterns
*/
void *memcpy(void * restrict out, const void * restrict in, size_t n)  __attribute__((optimize("02","no-tree-loop-distribute-patterns")));
void *memcpy(void * restrict out, const void * restrict in, size_t n)
{
	uint8_t * dst = out;
	const uint8_t * src = in;
	if (n) do {
		*dst++ = *src++;
	} while (--n) ;
	return out;
}
