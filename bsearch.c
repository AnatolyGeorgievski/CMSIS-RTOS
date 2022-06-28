#include <stdint.h>
#include <stdlib.h>
void *bsearch(const void *key, const void *base, size_t num, size_t size,
              int (*cmp)(const void *key, const void *))
{
	size_t l = 0, u = num;
	while (l < u) {
		register const size_t mid = (l + u)>>1;
		register const char* p = (const char*)base + mid * size;
		register int result = cmp(key, p);
		if (result < 0)
			u = mid;
		else if (result > 0)
			l = mid + 1;
		else
			return (void *)p;
	}
	return NULL;
}
#ifdef TEST_BSEARCH
#include <stdio.h>
static int op_count = 0;
static int cmp16(const void *a, const void *b)
{
	printf("+(%d,%d)", *(uint16_t*)a,*(uint16_t*)b);
	op_count++;
	return *(uint16_t*)a - *(uint16_t*)b;
}

int main()
{
	uint16_t testA[] = {
	 2,  3,  5,  7, 11, 13, 17, 19, 23, 29,	31,	37,	41,	43,	47,	53,	59,	61,	67,	71,
	73,	79,	83,	89,	97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173};
	const size_t width = sizeof(typeof(testA[0]));
	uint16_t key;
	for (key=1; key<24; key+=2){
		uint16_t * res = bsearch(&key, testA, sizeof(testA)/width, width, cmp16);
		printf("%2d: res=%d\n", key, res?*res:0);
	}
	printf("op count=%d\n", op_count);
}
#endif // TEST_BSEARCH
