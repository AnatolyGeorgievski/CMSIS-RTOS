#ifndef SVC_H
#define SVC_H

enum {
	SVC_YIELD=0,
	SVC_KILL, //!< выход из треда, параметр osThreadId
	SVC_USLEEP,
	SVC_EVENT_WAIT,
	SVC_MALLOC,
	SVC_FREE,
	SVC_OPEN,
	SVC_CLOSE,
	SVC_READ,
	SVC_WRITE,
	SVC_COUNT
};
/*! \brief аргумент системного запроса, TODO через прерывание SVC */
#define svc_arg(n,value) 
/*! \brief выполнить системный запрос, через прерывание SVC */
#define svc(code) __asm volatile ("svc %[immediate]"::[immediate] "I" (code):"r0")

static inline void __YIELD()
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}


typedef void (*SvcCallback) (unsigned int *frame, int svc_number);
void svc_handler_callback(SvcCallback cb, int svc_number);
#endif // SVC_H
