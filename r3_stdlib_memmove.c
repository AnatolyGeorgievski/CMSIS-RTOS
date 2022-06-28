#include <string.h>
/*! \brief перемещение буфера */
void *memmove(void *restrict out, const void *restrict in, size_t n)  __attribute__((optimize("02","no-tree-loop-distribute-patterns")));
void *memmove(void *restrict out, const void *restrict in, size_t n)
{
	uint8_t * dst = out;
	const uint8_t * src = in;
	if (out > in){
	    while (n--) {
	        dst[n] = src[n];
        }
    } else {
        if(n) do {
            *dst++ = *src++;
        } while (--n);
    }
	return out;
}
