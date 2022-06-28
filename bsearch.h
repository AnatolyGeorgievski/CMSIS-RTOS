
/*! \brief Функция сделана inline потому что оптимизация должна производиться по выбору параметра size и включению inline функции сравнения для простых типов */
static inline
void *bsearch(const void *key, const void *base, size_t num, size_t size,
              int (*cmp)(const void *key, const void *))
{
	size_t l = 0, u = num;
	while (l < u) {
		register const size_t mid = (l + u)>>1;
		register const uint8_t* p = (const uint8_t*)base + mid * size;
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
