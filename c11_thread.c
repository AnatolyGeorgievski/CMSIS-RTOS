/*! [C11] Threads 

 \note Returning from func has the same behavior as invoking thrd_exit with the value returned from func.
*/
#include <threads.h>
#include <sys/thread.h>
//#include <stdlib.h>// malloc
#include <atomic.h>
#include <svc.h>
typedef struct _Event  osEvent_t;
typedef struct _thread osThread_t;
#define THREAD_ID(x) ((thrd_t)(x))
#define THREAD_PTR(x) ((osThread_t*)(x))

extern volatile osThread_t* thread_current;

static inline uint32_t _timespec_to_us(const struct timespec * ts){
	return ts->tv_sec*1000000 + ts->tv_nsec/1000;
}
static inline void _timespec_from_us(struct timespec * ts, uint32_t timestamp){
	ts->tv_sec = timestamp/CLOCKS_PER_SEC;
	ts->tv_nsec = (timestamp - ts->tv_sec*CLOCKS_PER_SEC)*(1000000000UL/CLOCKS_PER_SEC);
}
static void* atomic_list_push(volatile void** ptr, osThread_t* data){
	osThread_t* item = (osThread_t*)data;
	osThread_t* next;
	do {
		next = atomic_pointer_get(ptr);
		if (next == (void*)~0) break;
		item->next = next;
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, next, item));
	return next;
}
static int atomic_sig_alloc(sigset_t *flags) {
	volatile int* ptr = (volatile int*)flags;
	int value, idx;
	do {
		value = atomic_int_get(ptr);
		if (value==~0) return -1;
		idx = __builtin_ctz(~(value));
	}while(!atomic_int_compare_and_exchange(ptr, value, value | (1UL<<idx)));
	return idx;
}
static int atomic_sig_free(sigset_t *flags, int sig_no) {
	volatile int* ptr = (volatile int*)flags;
	sigset_t sig_mask = (1UL<<sig_no);
	return atomic_fetch_and(ptr, ~sig_mask) & sig_mask;
}
thrd_t thrd_current(void){
	return THREAD_ID(thread_current); // 
}
int  thrd_create(thrd_t *thr, thrd_start_t func, void *arg){
	osThread_t *thrd = THREAD_PTR(*thr);
	osThread_t* self = THREAD_PTR(thrd_current());
/*	if (thrd==NULL) {
		thrd = malloc(sizeof(osThread_t)); // выделяется в памяти процесса
		__builtin_bzero(thrd, sizeof(osThread_t));
	} */
	if (thrd==NULL) return thrd_nomem;
	thrd->parent = self;
	thrd->process.func = (void*(*)(void*))func;
	thrd->process.arg  = arg;
	thrd->process.event.status = osEventRunning;// запустить исполнение процесса
	if (thrd->next == NULL) {
		atomic_list_push((volatile void**)&self->next, thrd);
	}
	*thr = THREAD_ID(thrd);
	return thrd_success;
}
int  thrd_equal(thrd_t thr0, thrd_t thr1){
	return THREAD_PTR(thr0) == THREAD_PTR(thr1);
}
_Noreturn void thrd_exit(int res){
	thread_current->process.result = (void*)(intptr_t)res;
	svc1(SVC_EXIT, thread_current);
	while(1);
}
/*! блокировка треда может быть основана на мьютексе 
Тред который запросил thrd_join  ожидает освобождение ресурса, т.е семафор
Мы предполагаем, что только один тред может запросить thrd_join
 */
int  thrd_detach(thrd_t thr){
	osThread_t* thrd = THREAD_PTR(thr);
	thrd->parent = NULL;
	return thrd_success;
}
/*!
	
 */
int  thrd_join(thrd_t thr, int *res){
	osThread_t* thrd = THREAD_PTR(thr);
	osThread_t* self = THREAD_PTR(thrd_current());
	thrd->sig_no = atomic_sig_alloc(&self->sig_mask);
	thrd->parent = self;
	atomic_mb();// fence
	if(thrd->process.event.status!=osEventComplete) {
		svc3(SVC_EVENT_WAIT, osEventSignal, 1UL<<thrd->sig_no, osWaitForever);
		atomic_sig_free(&self->sig_mask, thrd->sig_no);
	}
	*res = (uintptr_t)thrd->process.result;
//	free(thrd); -- оставляем возможность использовать повторно?
	return thrd_success;
}
/*! 
	\return zero if the requested time has elapsed, −1 if it has been interrupted
	by a signal, or a negative value (which may also be −1) if it fails.
*/
int  thrd_sleep(const struct timespec *duration, struct timespec *remaining){
	uint32_t ts; 
	if (remaining) ts = clock();
	uint32_t interval = _timespec_to_us(duration);	// преобразует интервал в микросекунды
	int res = svc1(SVC_USLEEP, interval);
	if (remaining) {
		ts = clock() - ts;
		if (interval > ts){
			_timespec_from_us(remaining, interval - ts);// преобразует штамп времени в timespec
			return -1;
		} else {
			*remaining = (struct timespec){0,0};
		}
	}
	return 0;
}
void thrd_yield(void){
	svc(SVC_YIELD);
}
