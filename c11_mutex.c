#include <cmsis_os.h>
#include <threads.h>
/*! \ingroup _system
	\defgroup _mtx_  C11 Mutex
	
	\{
	
 */
#include "semaphore.h"
/*! \brief Приватоное определение мьютекса */
struct os_mutex_cb {
	volatile int count;
};
static inline uint32_t div1M(uint32_t v) {// деление на миллион, справедливо для все целых uint32_t
#if defined (__ARM_ARCH_8M_BASE__)
    return v/1000000UL;
#else
    return (v*0x8637BD06ULL)>>51;
#endif
}

int  mtx_unlock (mtx_t *mtx) 
{
	semaphore_init(&mtx->count, 1);
	__DMB();
	return thrd_success;
}
/*! \brief Инициализация блокировки (мьютекса)
	\param type
	\arg \b mtx_plain
		which is passed to mtx_init to create a mutex object that supports neither timeout nor test and return;
	\arg \b mtx_recursive
		which is passed to mtx_init to create a mutex object that supports recursive locking;
	\arg \b mtx_timed
		which is passed to mtx_init to create a mutex object that supports timeout;
*/
int  mtx_init(mtx_t *mtx, int type)
{
	semaphore_init(&mtx->count, 1);
	return thrd_success;
}
int  mtx_lock(mtx_t *mtx)
{
	int count = semaphore_enter(&mtx->count);
/*	int count;
	do {// атомарно добавляем в список
		count = atomic_int_get(mtx);
	} while (!atomic_int_compare_and_exchange(mtx, count, 0)); */
	if (count) return thrd_success;

	osEvent event = {.status = osEventSemaphore,.value ={.p = (void*)&mtx->count}};
	return osEventWait(&event, osWaitForever);
//	return (event.status& osEventTimeout)?thrd_timedout:thrd_success;
}
/*! 
The \b mtx_timedlock function endeavors to block until it locks the mutex pointed to by
\b mtx or until after the TIME_UTC-based calendar time pointed to by \b ts. The specified
mutex shall support timeout. If the operation succeeds, prior calls to \b mtx_unlock on
the same mutex shall synchronize with this operation.
*/
int  mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts)
{
	int count = semaphore_enter(&mtx->count);
	if (count) return thrd_success;

	osEvent event = {.status = osEventSemaphore,.value ={.p = (void*)&mtx->count}};
	return osEventTimedWait(&event, ts);
	//return (event.status & osEventTimeout)?thrd_timedout: thrd_success;
	
}
/*! \brief Неблокирующий вызов мьютекса

The \b mtx_trylock function endeavors to lock the mutex pointed to by mtx. If the mutex is already
locked, the function returns without blocking. If the operation succeeds, prior calls to mtx_unlock
on the same mutex synchronize with this operation.
	\return \b thrd_success on success, or \b thrd_busy if the resource requested
is already in use, or thrd_error if the request could not be honored. mtx_trylock may spuriously
fail to lock an unused resource, in which case it returns thrd_busy.
*/
int  mtx_trylock(mtx_t *mtx)
{
	int count = semaphore_enter(&mtx->count);
	return (count)? thrd_success: thrd_busy;
}
void mtx_destroy(mtx_t *mtx)
{
	semaphore_init(&mtx->count, 0);
}
//! \}
