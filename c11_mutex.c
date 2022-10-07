//#include <cmsis_os.h>
#include <threads.h>
#include <sys/thread.h>
#include <svc.h>

typedef struct _thread osThread_t;
#define THREAD_PTR(x) ((osThread_t*)(x))

/*!	\defgroup _mtx_  C11 Mutex
	\ingroup _system _libc
	\{

 */
#include "semaphore.h"
/*! \brief Приватоное определение мьютекса */
typedef struct os_mutex_cb mtx_t;
struct os_mutex_cb {
	volatile int count;
	thrd_t owner;// идентификатор треда
	// тип мьютекса mtx_plain mtx_recursive
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
//	thrd_t thr = thrd_current();
//	if (mtx->owner!=thr ) return thrd_error;
	semaphore_leave(&mtx->count);
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
	mtx->owner = NULL;
	semaphore_init(&mtx->count, 1);
	return thrd_success;
}
int mtx_lock(mtx_t *mtx) {
	return mtx_timedlock(mtx, NULL);
}
/*! 
The \b mtx_timedlock function endeavors to block until it locks the mutex pointed to by
\b mtx or until after the TIME_UTC-based calendar time pointed to by \b ts. The specified
mutex shall support timeout. If the operation succeeds, prior calls to \b mtx_unlock on
the same mutex shall synchronize with this operation.
*/
int  mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts) {
	int res = mtx_trylock(mtx);
	if (res==thrd_busy) {
		svc4(SVC_CLOCK_WAIT, osEventSemaphore, (void*)&mtx->count, TIME_UTC, ts);
		thrd_t thr = thrd_current();
		int status = THREAD_PTR(thr)->process.event.status;
		if (status & osEventSemaphore) {
			mtx->owner = thr;
			res = thrd_success;
		} else
		if (status & osEventTimeout  ) res = thrd_timedout;
		// else res = thrd_error;// может прерываться 
	}
	return res;

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
	thrd_t thr = thrd_current();
	if (count>0) {
		mtx->owner = thr;
		return thrd_success;
	}
	if (mtx->owner == thr) {
		atomic_fetch_sub(&mtx->count, 1);
		return thrd_success;
	}
	return thrd_busy;
}
void mtx_destroy(mtx_t *mtx)
{
	mtx->owner = NULL;
	semaphore_init(&mtx->count, 0);
}
//! \}
