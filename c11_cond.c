#include <cmsis_os.h>
#include <threads.h>
/*! \see ARM® Synchronization Primitives */
#include "semaphore.h"
#include "r3_slice.h"
struct _list_mtx {
	mtx_t *mutex;
	struct _list_mtx *next;
};
struct os_mutex_cb {
	volatile int count;
};
struct os_cond_cb {
	struct _list_mtx * list;
};
/*! \brief

The \b cnd_init function creates a condition variable. If it succeeds it sets the variable pointed to by
cond to a value that uniquely identifies the newly created condition variable. A thread that calls
\b cnd_wait on a newly created condition variable will block.
	\return	\b thrd_success on success, or \b thrd_nomem if no memory could be
allocated for the newly created condition, or \b thrd_error if the request could not be honored.
 */
int cnd_init(cnd_t *cond)
{
	cond->list=NULL;
	return thrd_success;
}
/*! \brief

The \b cnd_destroy function releases all resources used by the condition variable pointed to by cond.
The \b cnd_destroy function requires that no threads be blocked waiting for the condition variable
pointed to by cond.
*/
void cnd_destroy(cnd_t *cond)
{
	/* struct _list_mtx */
	volatile void**head = (volatile void**)&cond->list;
	struct _list_mtx *node;
	while (1) {
		do{// атомарно выталкиваем из списка элемент
			node = atomic_pointer_get(head);
			if (node==NULL) {
				//atomic_free();
				return;
			}
		} while(!atomic_pointer_compare_and_exchange(head, node, node->next));
		__DMB();
		if (node->mutex) {
			semaphore_init(&node->mutex->count, 1);// mtx_unlock(node->mutex);
		}
		g_slice_free(struct _list_mtx, node);
	}
	return;
}
/*! \brief

The \b cnd_signal function unblocks one of the threads that are blocked on the condition variable
pointed to by cond at the time of the call. If no threads are blocked on the condition variable at the
time of the call, the function does nothing and returns success.
	\return \b thrd_success on success or \b thrd_error if the request could not be honored.
 */
int cnd_signal(cnd_t *cond)
{
	volatile void** head = (volatile void**)&cond->list;
	struct _list_mtx *node;
	do{// атомарно выталкиваем из списка элемент
		node = atomic_pointer_get(head);
		if (node==NULL) {
			//atomic_free();
			return thrd_success;
		}
	} while(!atomic_pointer_compare_and_exchange(head, node, node->next));
	__DMB();
	if (node->mutex) {
		semaphore_init(&node->mutex->count, 1);// mtx_unlock(node->mutex);
	}
	g_slice_free(struct _list_mtx, node);
	return thrd_success;
}
/*! \brief

The cnd_broadcast function unblocks all of the threads that are blocked on the condition variable
pointed to by cond at the time of the call. If no threads are blocked on the condition variable pointed
to by cond at the time of the call, the function does nothing.
	\return \b thrd_success on success, or \b thrd_error if the request could not be honored
 */
int cnd_broadcast(cnd_t *cond)
{
	volatile void** head = (volatile void**)&cond->list;
	while (1) {
		struct _list_mtx *node;
		do{// атомарно выталкиваем из списка элемент
			node = atomic_pointer_get(head);
			if (node==NULL) {
				//atomic_free();
				return thrd_success;
			}
		} while(!atomic_pointer_compare_and_exchange(head, node, node->next));
		__DMB();
		if (node->mutex) {
			semaphore_init(&node->mutex->count, 1);// mtx_unlock(node->mutex);
		}
		g_slice_free(struct _list_mtx, node);
	}
	return thrd_success;
}
static inline uint32_t div1000(uint32_t v) {
    return (v*0x83126E98ULL)>>41;
}
static inline uint32_t div1M(uint32_t v) {
#if defined(__ARM_ARCH_8M_BASE__)
	return v/1000000UL;
#else
    return (v*0x8637BD06ULL)>>51;
#endif
}

int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
	volatile void**head = (volatile void**)&cond->list;
	struct _list_mtx *node = g_slice_new(struct _list_mtx);
	node->mutex = mtx;
	int count = semaphore_enter(&mtx->count);
	mtx->count = 0; // lock
	do {// атомарно добавляем в список
		node->next = atomic_pointer_get(head);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(head, node->next, node));
	__DMB();
    osEvent event = {.status = osEventSemaphore,.value ={.p = (void*)&mtx->count}};
	osEventWait(&event, osWaitForever);
	return (event.status == osEventTimeout)?thrd_timedout: thrd_success;
}
int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts)
{
	volatile void**head = (volatile void**)&cond->list;
	struct _list_mtx *node = g_slice_new(struct _list_mtx);
	node->mutex = mtx;
	int count = semaphore_enter(&mtx->count);
	mtx->count = 0; // lock
	do {// атомарно добавляем в список
		node->next = atomic_pointer_get(head);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(head, node->next, node));
	__DMB();
	// с насыщением!!!
	uint32_t millisec = div1M(ts->tv_nsec) + __USAT(ts->tv_sec,32-10)*1000;
    osEvent event = {.status = osEventSemaphore,.value ={.p = (void*)&mtx->count}};
	osEventWait(&event, millisec);
	return (event.status == osEventTimeout)?thrd_timedout: thrd_success;
}
