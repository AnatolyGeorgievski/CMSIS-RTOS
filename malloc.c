//-------------------------------------------------------
/*! \ingroup _system
    \defgroup _malloc Менеджер памяти
    \brief Функции распределения дианамической памяти

	01.04.2020 обновил имена функций. Соответствие C11

    \{
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "atomic.h"
#include "module.h"
/// выравнивание памяти по длине указателей

typedef struct _R3MemoryBlock R3MemoryBlock;
struct _R3MemoryBlock{
	unsigned int size:30;// размер кратный 4
	unsigned int adv :1;// Advisory info POSIX_MADV_SEQUENTIAL - резервируется для последовательного выделения
	unsigned int free:1;
//	volatile unsigned int lock:1;
	//unsigned char data[0];
} __attribute__((packed, aligned(4)));

#define R3_MEM_ALIGN __alignof__(void*)

//unsigned int heap[4*1024];
//R3MemoryBlock heap_base0[2000] __attribute__((section("HEAP"))); // куча
R3MemoryBlock heap_base[0] __attribute__((section("HEAP"))); // куча
static R3MemoryBlock * last_block  = heap_base;
static R3MemoryBlock * first_free_block  = heap_base;
static R3MemoryBlock * max_block  = heap_base;
//static R3MemoryBlock * first_block = heap_base;
#ifndef SRAM_SIZE
#define SRAM_SIZE 16000000
#endif
#ifndef SRAM_BASE
#define SRAM_BASE 0x20000000UL
#endif
/// \todo считать указатель стека и сипользовать как heap_size
unsigned long heap_size = SRAM_SIZE;//AT91C_ISRAM_SIZE;

/*! \brief сбор мусора в кучу

    Процедура сбора мусора выполняется периодически по расписанию

    \todo сделать адаптивный механизм
*/

static void r3_mem_garbage(void *  data)
{	// сбор мусора
	R3MemoryBlock * block = first_free_block;
	if (block >= last_block){ // свободен
	    first_free_block = heap_base;// начать сканирование
		//printf("grabage");
	    return;
	}
	R3MemoryBlock * next_block = (void*)((char*)block + block->size);
	if (!block->free) {
	    first_free_block = next_block;//(void*)((char*)block + block->size);
	    return;
    }
	int count = 5;
	do	{ // укрупняем обрезки
        if (next_block==last_block)	{
            last_block = block;
            first_free_block = heap_base;
            return;
        }
		if (!next_block->free) {
		    first_free_block= (void*)((char*)next_block + next_block->size);
			return; // цепочка оборвалась, следующий полный
		}
        block->size += next_block->size; // объединили блоки
        next_block =( void*)((char*)block + block->size);
	} while (--count);
//	first_free_block = block;
}
MODULE(Garbage_Collector, NULL, r3_mem_garbage);
/*! \brief высвободить память

Освобождение памяти не должно за собой тянуть сборку мусора
освобождать надо за фиксированное время, так же как и выделять.
*/
void free(void* data) 
{
	R3MemoryBlock * block = (R3MemoryBlock *)data -1;// -sizeof (R3MemoryBlock);
	block->free=1;
	return;
	R3MemoryBlock * next_block = block + block->size;
    if (next_block == last_block)
    {
		last_block = block;

    } else {

   		if (next_block->free){
			block->size += next_block->size; // объединили блоки
   		}
        if (first_free_block > block)
            first_free_block = block; // выдали задание для сбора мусора
    }
	//block->free=1;
//	r3_mem_garbage(block); - отдельный процесс
}
#if 0
void r3_mem_test()
{
	printf("heap base=%08lX"" size=%08lX", (uint32_t)heap_base, (uint32_t)last_block - (uint32_t)heap_base);
    printf(" max=%08lX""\r\n", (uint32_t)max_block-(uint32_t)heap_base);
}
#endif
extern  void r3_mem_init()
{	// вообще-то не плохо было бы вычесть стек
	heap_size = SRAM_SIZE - ((void*)&heap_base - (void*)SRAM_BASE);
    //r3_mem_test(NULL,NULL);
}


/*! \brief Выделение динамической памяти с выравниванием

    Используется для выделения памяти под DMA буферы, выравнивание производится на длину линию кеша (32 байты) или длину трансфера по шине.
    \param [in] size - размер памяти для выделения
    \param [in] align - выравнивание 2^n байт, где n > 2
    \return указатель на выделенный объект или NULL
*/
void* aligned_alloc(size_t align, size_t size)
{
//printf("malloc base %08X %08X %08lX\n", (unsigned int)last_block, size, align);
    if (align <R3_MEM_ALIGN) align = R3_MEM_ALIGN;
    const unsigned align_mask = align-1;

	size = (size + align_mask) & ~((size_t)align_mask); // выравниваем на слово
    size += sizeof (R3MemoryBlock);     // добавляем длину шапки -- накладные расходы
	R3MemoryBlock block;// = last_block; // пробуем выделить память с конца

	void* addr_block;
	void* next_block;
	unsigned head_align;
	align -= sizeof (R3MemoryBlock);
    volatile void ** ptr = (volatile void **)&last_block;
	do {
		addr_block = (void*)atomic_pointer_get(ptr);
        head_align = align - (((unsigned)addr_block) & align_mask);// - sizeof (R3MemoryBlock);
        next_block = (void*)addr_block + head_align + size;
	} while (!atomic_pointer_compare_and_exchange(ptr, addr_block, next_block));
//printf("malloc next %08X %08X %08lX\n", (unsigned int)addr_block, (unsigned int)next_block, head_align);

	block.size = size;
	block.free = 0;
	*(volatile R3MemoryBlock*)(addr_block + head_align) = block;
	if (head_align > 0)
	{// надо отравнять шапку
		atomic_mb();// fence
	    block.size = head_align;
	    block.free = 1;// пустой блок
		*(volatile R3MemoryBlock*)addr_block = block;
//printf("malloc head %08X\n", (unsigned int)addr_block);
    }
//printf("malloc addr %08X\n", (unsigned int)addr_block + sizeof (R3MemoryBlock));
#if 0
	if (max_block< last_block) max_block = addr_block+size;
//    TRACE_log(TRACE_DEBUG, "mod=\"%s\" alloc=%08lX [%08X]""\r\n", module?module->name:"(null)", (u32)block, size);
#endif
	return (addr_block + sizeof (R3MemoryBlock));
}

/*! \brief Выделение динамической памяти

    Память можно выделять кусками произвольной длины. Рекомендуется
    возвращать обратно в течение той же сессии или не возвращать никогда.
    Обычно процедура выделения памяти используется для вделения блоками под буферы.
    Для выделения памяти под динамические структуры данных, такие как деревья и списки
    рекомендуется использовать механизм слайсов.
*/
void* malloc(size_t size)
{
	return aligned_alloc(R3_MEM_ALIGN, size);
}
void* realloc(void* mem, size_t size)
{
	return NULL;//aligned_alloc(R3_MEM_ALIGN, size);
}

//void  free(void* data)  __attribute__((weak, alias("r3_mem_free")));
//void* malloc(size_t size)  __attribute__((weak, alias("r3_mem_alloc")));
void* calloc(size_t nelem, size_t elsize)
{
	return aligned_alloc(R3_MEM_ALIGN, nelem*elsize);
}
/*! \} */
/*! \defgroup ADV POSIX: Advisory Information (ADV)
	\ingroup _posix
	\{
 */
int posix_madvise(void *data, size_t size, int adv){
	R3MemoryBlock* block = (R3MemoryBlock*)data -1;
	
	// резервировать свободжный блок дальше с тем же признаком
	int reserved = size - block->size;
	if (reserved>0) {
		block = (R3MemoryBlock*) ((void*)block + size);
		if (block->free){// НАДО атомарно, дописать
			block->adv = adv;
		}
	}
	// 
	return 0;
}
/*! \} */
