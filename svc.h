#ifndef SVC_H
#define SVC_H
#include "board.h"

enum {
	SVC_YIELD=0,
	SVC_SIGNAL, //!< выход из треда, параметр osThreadId
	SVC_USLEEP,
	SVC_EVENT_WAIT,
	SVC_EXIT,
	SVC_COUNT
};
/*! \brief аргумент системного запроса, TODO через прерывание SVC */
#define svc_arg(n,value) 
/*! \brief выполнить системный запрос, через прерывание SVC */
#define svc(code) __asm volatile ("svc %[immediate]"::[immediate] "I" (code):"r0","memory")
#define svc1(code,arg) __extension__({\
	register uint32_t __R0 __asm("r0") = (uint32_t)(arg);\
	__asm volatile ("svc %[immediate]":"=r"(__R0):[immediate] "I" (code), "r"(__R0):"memory"); \
	__R0;})
#define svc2(code,a0,a1) __extension__({\
	register uint32_t __R0 __asm("r0") = (uint32_t)(a0);\
	register uint32_t __R1 __asm("r1") = (uint32_t)(a1);\
	__asm volatile ("svc %[immediate]":"=r"(__R0):[immediate] "I" (code), "r"(__R0), "r"(__R1):"memory"); \
	__R0;})
#define svc3(code,a0,a1,a2) __extension__({\
	register uint32_t __R0 __asm("r0") = (uint32_t)(a0);\
	register uint32_t __R1 __asm("r1") = (uint32_t)(a1);\
	register uint32_t __R2 __asm("r2") = (uint32_t)(a2);\
	__asm volatile ("svc %[immediate]":"=r"(__R0):[immediate] "I" (code), "r"(__R0), "r"(__R1), "r"(__R2):"memory"); \
	__R0;})

static inline void __YIELD()
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}


typedef void (*SvcCallback) (unsigned int *frame, int svc_number);
void svc_handler_callback(SvcCallback cb, int svc_number);
#endif // SVC_H
