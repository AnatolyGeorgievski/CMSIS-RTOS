
struct _QuarkEntry {
	const char* _name;// смещение в таблице имен
	uint32_t _next;// индекс в хеш таблице
};

uint32_t quark_try_string(const char* name)
{
	uint32_t key = elf_hash(name);
	uint32_t y = htable->bucket[key & Nbucket_MASK];
	//const uint32_t *quarks = htable->bucket + Nbucket;
	while (y<htable->nchain && y!=0) {
		if (strcmp(quarks[y]._name, name)==0) 
			return y;
		y = quarks[y]._next;
	}
	return 0;
}
const char* quark_to_string(uint32_t y){
	return quarks[y]._name;
}
uint32_t _quark_new(char* name)
{
	uint32_t key = elf_hash(name);
	uint32_t y = atomic_fetch_add(htable->nchain, 1);
	quarks[y]._name = name;
	uint32_t* head = &htable->bucket[key & Nbucket_MASK];
	do {
		next = atomic_int_get(head);
		quarks[y]._next = next;
		atomic_mb();
	} while (!atomic_int_compare_and_exchange(head, next, y));
	return y;
}
