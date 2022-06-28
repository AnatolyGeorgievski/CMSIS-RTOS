// r3_stdlib
#ifndef R3_STDLIB_H
#define R3_STDLIB_H

/* затычка для совместимости с sys/types.h, stdint.h и C99 inttypes.h */

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#if 0
//typedef unsigned long size_t;
typedef struct _list R3list;
struct _list {
	R3list *next;
	char data[0];
};



//-------------------------------------------------------
//! \ingroup r3_queue
//! \brief структура для очереди
typedef struct _queue R3queue;
struct _queue {
	R3list* first;
	R3list* last;
	unsigned int done;
//	unsigned int mask;		//!< размер очереди	2^n маска = (2^n)-1
//	unsigned int write_pos; //!< индекс записи
//	unsigned int read_pos;	//!< индекс чтения
//	unsigned int lock;		//!< блокировка записи используется только при одновременном доступе на запись.
};

//Пусть очереди тут поживут
void	r3_queue_init(R3queue * queue);
void	r3_queue_push(R3queue * queue, R3list* item);
R3list*	r3_queue_pop (R3queue * queue);



typedef struct _Stack Stack;
//! \ingroup _stack
//! \brief Складирование объектов на стеке
struct _Stack {
    Stack *next;    //!< следующий элемент в колоде
    void*  data;    //!< данные
};
Stack*  r3_stack_push   (Stack* st, void* data);
Stack * r3_stack_pop    (Stack* st);
void    r3_stack_clear  (Stack* st);



static inline void r3_list_prepend(R3list ** list, R3list *item)
{
    item->next = *list;
    *list = item;
}
#endif

//void* 	r3_alloc(unsigned int size);
unsigned char * bin2hex(unsigned char *buffer, unsigned int word);
unsigned char *	bin2str(void * buffer, unsigned char * str, int size);
static inline unsigned char bin2char(unsigned char c)
{
	c &= 0x0F;
	if(c < 10) c+='0';
	else c+='A'-10;
	return c;
}
int vsnprintf(char *pStr, size_t length, const char *pFormat, va_list ap);
int snprintf(char *pString, size_t length, const char *pFormat, ...);
void fp_print(char * buffer, long a, int n, int f);
// работа с динамической памятью
void  r3_mem_init();
void* r3_mem_alloc(unsigned int size);
void* r3_mem_alloc_aligned(unsigned int size, unsigned long align);
void  r3_mem_free(void* data);
void  r3_mem_status();
//#define r3_mem_alloc(size) r3_mem_alloc_align(size, 4)

//int snprintf(char *pString, size_t length, const char *pFormat, ...);

#define IS_SPACE(c) ((c)==' ' || (c)=='\r' || (c)=='\n' || (c)=='\t')
#define IS_DIGITAL(c) ((c)>='0' && (c)<='9')
#define IS_XDIGITAL(c) (((c)>='0' && (c)<='9') || ((c)>='A' && (c)<='F'))
#define IS_ALPHA(c)	(((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z') || (c)=='_')
#define IS_ALPHA_NUM(c)	(((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z') || ((c)>='0' && (c)<='9') || (c)=='_')

#define ASCII_UPPER(ch) (((ch) >='a' && (ch) <= 'z')? (ch) ^ 0x20: (ch))

#define ISSET(register, flags)      (((register) & (flags)) == (flags))
#define CLEAR(register, flags)      ((register) &= ~(flags))
#define ISCLEARED(register, flags)  (((register) & (flags)) == 0)

#ifndef offsetof
#   define offsetof(s,m) ((size_t)&(((s*)0)->m))
#endif
#if 0
/*! работа с динамической памятью, удобно использовать для списков и деревьев */
extern void  r3_slice_init();
//extern void  r3_slice_test();
extern void* r3_slice_alloc  (unsigned int size);
extern void* r3_slice_alloc0 (unsigned int size);
extern void  r3_slice_free   (unsigned int size, void* data);
void r3_slice_status();

#define r3_slice_new(type)  r3_slice_alloc(sizeof(type))
#define r3_slice_new0(type)  r3_slice_alloc0(sizeof(type))

#undef g_slice_new0
#undef g_slice_free
#define g_slice_new0(type)  \
    (type* ) r3_slice_alloc0(sizeof(type))

#define g_slice_new(type)  \
    (type* ) r3_slice_alloc(sizeof(type))

#define g_slice_free(type, d)  \
    r3_slice_free(sizeof(type), d)
#endif
/*! для переносимости кода */
#undef puts
#undef printf
#undef putchar
#undef g_free
#undef g_malloc
//#define puts(str)   TRACE_log(TRACE_DEBUG, str)
//#define
int puts(const char* str);
int printf(const char* fmt,...);
int putchar(int ch);
#define g_malloc r3_mem_alloc
#define g_free r3_mem_free


#endif //R3_STDLIB_H
