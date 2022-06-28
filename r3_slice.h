#ifndef R3_SLICE_H_INCLUDED
#define R3_SLICE_H_INCLUDED
#include <stdlib.h>


/*! работа с динамической памятью, удобно использовать для списков и деревьев */
//void  g_slice_test();
void  g_slice_init   (size_t size, void* array, size_t length);
void* g_slice_alloc  (size_t size);
static inline void* g_slice_alloc0 (size_t size){
	void* mem = g_slice_alloc(size);
	__builtin_bzero(mem, size);
	return mem;
}
void  g_slice_free1  (size_t size, void* data);
void  g_slice_status ();

#define g_slice_new(type) ((type*)g_slice_alloc(sizeof(type)))
#define g_slice_new0(type) ((type*)g_slice_alloc0(sizeof(type)))
#define g_slice_free(type, data) g_slice_free1(sizeof(type), data)


#endif // R3_SLICE_H_INCLUDED
