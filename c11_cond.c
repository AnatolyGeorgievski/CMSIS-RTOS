//#include <cmsis_os2.h>
#include <threads.h>
#include <sys/thread.h>
#include <svc.h>

typedef struct _thread osThread_t;
#define THREAD_PTR(x) ((osThread_t*)(x))

/*!	\defgroup _cond_ C11 Condition Variables
	\ingroup _system _libc

	\brief Функции управлениея некондицией процессов. 
	
	Некондиция представляет собой спсоб управления синхронизацией процессов, 
	когда один процесс управляет оживлением множества некондиционных процессов. 
	Процесс может, заблокировать себя используя собственную блокировку (мьютекс) и впасть в некондиционное состояние
	идентифицируемое, как абстрактная некондиция. 
	Кондиция - это оживление некондиционных процессов - переменная типа \b Cond_t, пожет иметь только идентификатор или адрес, 
	который служит идентификатором, без ассоциированных данных. 
	
	Выход из некондиции происходит либо по истечение заданного интервала времени, либо по сигналу 
	от процесса выполняющего управление некондицией. Некондиция самоблокированных процессов скапливается 
	в форме списка. Список некондиции связанных с идентификатором кондиции существует неявно и обрабатывается операционной системой.
	
	Методы управлениея некондицией включают возможность оживить и заставить работать один процесс из списка некондиции или все сразу.
	
	С точки зрения поведения процессов в системе, те что самоблокировались и ожидают кондиции, по сути ждут либо освобождения какого-то ресурса, 
	но тогда следует использовать ожидание семафора, либо ждут наступления какого-то события. 

	\{
*/
/*! \see ARM® Synchronization Primitives */
#include "semaphore.h"
#include "r3_slice.h"
struct _list_mtx {
	struct _list_mtx *next;
	mtx_t *mutex;
};
struct os_mutex_cb {
	volatile int count;
};
/*! \brief Инициализация некондиции

The \b cnd_init function creates a condition variable. If it succeeds it sets the variable pointed to by
cond to a value that uniquely identifies the newly created condition variable. A thread that calls
\b cnd_wait on a newly created condition variable will block.
	\return	\b thrd_success on success, or \b thrd_nomem if no memory could be
allocated for the newly created condition, or \b thrd_error if the request could not be honored.
 */
int cnd_init(cnd_t *cond)
{
	*(volatile void**)cond=NULL;
	return thrd_success;
}
/*! \brief удаление некондиции

The \b cnd_destroy function releases all resources used by the condition variable pointed to by cond.
The \b cnd_destroy function requires that no threads be blocked waiting for the condition variable
pointed to by cond.
*/
void cnd_destroy(cnd_t *cond)
{
	/* struct _list_mtx */
	volatile void**head = (volatile void**)cond;
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
			semaphore_leave(&node->mutex->count);// mtx_unlock(node->mutex);
		}
		g_slice_free(struct _list_mtx, node);
	}
	return;
}
/*! \brief Оповещение о кондиции одного процесса

The \b cnd_signal function unblocks one of the threads that are blocked on the condition variable
pointed to by cond at the time of the call. If no threads are blocked on the condition variable at the
time of the call, the function does nothing and returns success.
	\return \b thrd_success on success or \b thrd_error if the request could not be honored.
 */
int cnd_signal(cnd_t *cond)
{
	volatile void** head = (volatile void**)cond;
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
		semaphore_leave(&node->mutex->count);// mtx_unlock(node->mutex);
	}
	g_slice_free(struct _list_mtx, node);
	return thrd_success;
}
/*! \brief Оповестить все процессы в сипске о наступлении события

The cnd_broadcast function unblocks all of the threads that are blocked on the condition variable
pointed to by cond at the time of the call. If no threads are blocked on the condition variable pointed
to by cond at the time of the call, the function does nothing.
	\return \b thrd_success on success, or \b thrd_error if the request could not be honored
 */
int cnd_broadcast(cnd_t *cond)
{
	volatile void** head = (volatile void**)cond;
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
			semaphore_leave(&node->mutex->count);// mtx_unlock(node->mutex);
		}
		g_slice_free(struct _list_mtx, node);
	}
	return thrd_success;
}
/*! \brief Ускорение операций дления на константу */
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
/*! \brief Ожидать состояние кондиции 
	\param [in] cond - переммнная/идентификатор группы некондиции
	\param [in] mtx - собственная блокировка, мьютекс должна быть заблокирована до выфзова
	\param [in] ts - штам времени, абсолютное значение в TIME_UTC

Процесс совершает акт само блокировки и переходит в некондиционное состояние на неопределенное время, 
помещается в список некондиции, связанный с идентификатором группы некондиции \b cond.

Рекомендуемый способ вызова:
timespec_get(&ts, TIME_UTC);
ts.tv_nsec += Nanosec(timeout);
if (ts.tv_nsec>=1`000`000`000) {
	ts.tv_sec  += ts.tv_nsec / 1`000`000`000;
	ts.tv_nsec  = ts.tv_nsec % 1`000`000`000;
}

cnd_timedwait(cond, mtx, &ts);

The \b cnd_timedwait function atomically unlocks the mutex pointed to by \b mtx and
endeavors to block until the condition variable pointed to by \b cond is signaled by a call to
\b cnd_signal or to \b cnd_broadcast, or until after the TIME_UTC-based calendar
time pointed to by \b ts.

The cnd_timedwait function requires that the mutex pointed to by \b mtx be locked by the calling thread.	
 */
int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts)
{
	volatile void**head = (volatile void**)cond;
	struct _list_mtx *node = g_slice_new(struct _list_mtx);
	node->mutex = mtx;
	//int count = semaphore_enter(&mtx->count);
	mtx->count = 0; // lock
	do {// атомарно добавляем в список
		node->next = atomic_pointer_get(head);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(head, node->next, node));
	__DMB();
	// с насыщением!!!
	struct timespec dt;
	timespec_get(&dt, TIME_UTC);// UTC based Мы тут используем функцию из C11 как и треды
	dt.tv_sec = ts->tv_sec - dt.tv_sec;
	dt.tv_nsec = ts->tv_nsec - dt.tv_nsec;
	if (dt.tv_nsec<0) {
		dt.tv_nsec+= 1000000000UL;
		dt.tv_sec ++;
	}
	uint32_t interval = (dt.tv_sec*1000000 + dt.tv_nsec/1000);
	svc3(SVC_EVENT_WAIT, osEventSemaphore, (void*)&mtx->count, interval);
	thrd_t thr = thrd_current();
	int status = THREAD_PTR(thr)->process.event.status;
	return (status & osEventSemaphore)?thrd_success:thrd_timedout;
	
//    osEvent_t event = {.status = osEventSemaphore,.value ={.p = (void*)&mtx->count}};
//	return osEventTimedWait(&event, ts);// абсолютное время
}
int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
	return cnd_timedwait(cond, mtx, NULL);
}


	//! \}