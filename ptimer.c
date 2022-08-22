// posix timer

#include <atomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <svc.h>// sys/svccall.h

extern void osThreadNotify(void*);
extern sigset_t osSignalSet(void*, sigset_t);
extern void* osThreadGetId();

typedef struct _process osProcess_t;
struct _process {
	sigset_t signals;// pending signals
	struct _event {
		uint32_t status;
		union {
			void* 	 p;
			sigset_t signals;
		} value;
	} event;
	struct {
		clock_t timestamp;
		clock_t interval;
	} wait;
};

#define PROCESS_PTR(x) ((osProcess_t*)(x))

typedef struct os_timer_cb Timer_t;
struct os_timer_cb {
	Timer_t * next;
	uint32_t timestamp;
	uint32_t interval;
	uint32_t overrun;
//	struct itimerspec ts;
// \see struct sigevent
	int          sigev_notify;               /* Notification type */
	union sigval sigev_value;
	void (*sigev_notify_function)(union sigval argument);
	void* owner;
};
static volatile Timer_t *timer_current = NULL;
#define TIMER_PTR(x) ((Timer_t*)(x))
#define TIMER_ID(x) ((timer_t)(x))
static inline void* timer_next(volatile void** ptr){
	Timer_t* item;
	do {
		item = atomic_pointer_get(ptr);
		if (item==NULL){
			atomic_free();
			break;
		}
	} while (!atomic_pointer_compare_and_exchange(ptr, item, item->next));
	atomic_mb();
	return item;
}
int  timer_create (clockid_t clock_id, struct sigevent *restrict event, timer_t *restrict timer_id)
{
	Timer_t *tim = NULL;
	if(timer_id) tim = TIMER_PTR(*timer_id);
	if(tim==NULL) tim = malloc(sizeof(Timer_t));
	__builtin_bzero(tim, sizeof(Timer_t));
	if (event->sigev_notify == SIGEV_THREAD)
		tim->sigev_notify_function = event->sigev_notify_function;
	tim->sigev_value = event->sigev_value;
//	tim->parent = osThreadGetId();
	if (timer_id) *timer_id = TIMER_ID(tim);	// преобразует тип в идентификатор 
	return 0;
}
int  timer_delete (timer_t timer_id){
	Timer_t * tim = TIMER_PTR(timer_id);
	tim->interval = 0;
	free(tim);
	return 0;
}
int  timer_getoverrun(timer_t timer_id){
	Timer_t * tim = TIMER_PTR(timer_id);
	return tim->overrun;
}
#define M1 1000000UL
static inline uint32_t _timespec_to_us(const struct timespec * ts)
{
	return ts->tv_sec*M1 + ts->tv_nsec/1000;
}
static inline void _timespec_from_us(struct timespec * ts, uint32_t interval)
{
	if (interval < M1)// CLOCKS_PER_SEC
		ts->tv_sec  = 0;
	else {
		ts->tv_sec  = interval/M1;
		interval -= ts->tv_sec*M1;
	}
	ts->tv_nsec = interval*M1;
}
/*! 
Вызов timer_gettime() возвращает время до следующего срабатывания таймера timerid и интервал в буфер curr_value. Оставшееся время до следующего срабатывания возвращается в curr_value->it_value; это всегда относительное значение, независимо от того, указывался ли флаг TIMER_ABSTIME при включении таймера. Если значение curr_value->it_value равно нулю, то таймер в данный момент выключен. Интервал таймера возвращается в curr_value->it_interval. Если значение curr_value->it_interval равно нулю, то это «одноразовый» таймер. 
 */
int  timer_gettime(timer_t timer_id, struct itimerspec * value){
	Timer_t * tim = TIMER_PTR(timer_id);
	clock_t timestamp = clock();
	// Если значение curr_value->it_value равно нулю, то таймер в данный момент выключен.
	// если значение меньше нуля?
	_timespec_from_us(&value->it_value,    tim->interval - (timestamp - tim->timestamp));
	_timespec_from_us(&value->it_interval, tim->interval);
	return 0;
}
/*! 
	\param flags
		\arg TIMER_ABSTIME если 
 */
int  timer_settime(timer_t timer_id, int flags, const struct itimerspec *restrict value,
	struct itimerspec *restrict ovalue)
{
	Timer_t * tim  = TIMER_PTR(timer_id);
	tim->interval  = _timespec_to_us(&value->it_interval);
	tim->timestamp = _timespec_to_us(&value->it_value);
	if (flags != TIMER_ABSTIME)
		tim->timestamp += clock();
	
	if (tim->next==NULL) {
		volatile void** ptr = (volatile void**)&timer_current;
		Timer_t *next;
		do {
			next = atomic_pointer_get(ptr);
			tim->next = (next==NULL)? tim: next;// на себя циклим
			atomic_mb();
		} while (!atomic_pointer_compare_and_exchange(ptr, next, tim));		
	}
	return 0;
}
/*! \brief системная функция, запускается по системному таймеру 
	\param timestamp - абсолютное время выражено в микросекундах получено из TIME_MONOTONIC
\todo унифицировать функцию osTimerWork
 */
void osTimerWork2(uint32_t timestamp)
{
	Timer_t * timer = timer_next((volatile void **)&timer_current);
	if (timer!=NULL && timer->timestamp!=0) {
		if ((int32_t)(timestamp - timer->timestamp)>= 0) 
		{
			if (timer->sigev_notify_function)// SIGEV_THREAD
				timer->sigev_notify_function(timer->sigev_value);
			else  
			{// SIGEV_SIGNAL
				sigset_t flag = 1UL<<timer->sigev_value.sival_int;
				sigset_t mask = osSignalSet(timer->owner,  1UL<<timer->sigev_value.sival_int);
				if (mask & flag) timer->overrun++;
				// если процесс ждет сигнал, то надо передать ему управление
				osThreadNotify(timer->owner);
			}// есть вариант отсылки сигнала SIGEV_NONE
			if (timer->interval) {
				timer->timestamp += timer->interval; // время следующего срабатывания. 
				// переместить таймер в очереди
			} else {// удалить таймер
				timer->next = NULL;
			}
			//if (timer->sigev_notify & osTimerOnce) timer_free();// удалить таймер
		}
	}
}
#if 0
static void svc_handler_sleep(uint32_t microsec) {// завершение треда
	thr->status = osEventTimeout; // остановить, атомарно
	thr->wait.timestamp = clock();
	thr->wait.interval  = microsec;
	__YIELD();
}
static void svc_handler_exit (thrd_t thr, int res) {// завершение треда
//	if (thr->context) -- освободить стек
//		mem_pool_free(thr->context);
//	if (thr->tss) tss_ -- удалить переменные треда
	thr->result = res;// userspace
	thr->status = osEventComplete;
	//sigqueue(thr->pid, SIGCHLD, thr);
	int count = semaphore_leave (thr->sem);// освободить того кто ждет.
	if (count!=0) {// detached
		
	}
	__YIELD();
}

typedef struct _process osProcess_t;
struct _process {
	sigset_t signals;// pending signals
	struct _event {
		uint32_t status;
		union {
			void* 	 p;
			sigset_t signals;
		} value;
	} event;
	struct {
		clock_t timestamp;
		clock_t interval;
	} wait;
}


volatile osThread_t *thread_current;
volatile osThread_t *thread_next;

 atomic_list_next(arg){
	volatile void** ptr = arg;
	List* thr;
	do {
		thr = atomic_pointer_get(ptr);
	} while(!atomic_pointer_compare_and_exchange(ptr, thr, thr->next));
	if (thr)
	thread_next = thr; 
	if (thr->next = status )
}
 atomic_list_push(arg, thr){
	volatile void** ptr = arg;
	do {
		thr->next = atomic_pointer_get(ptr);
		atomic_mb();
	} while(!atomic_pointer_compare_and_exchange(ptr, thr->next, thr));
	thr; 
}


/* Результат планирования - pid который следует запустить следующим */
void osThreadScheduler(cpu_id)
{
	osProcess_t *proc = NULL;
	while ((proc = process_next(&prev))!=NULL) { // сделать атомарную выборку
		osEvent *event = &proc->event;
		if (event->status & osEventRunning) break;
		if (event->status & (osEventSignal)) 	{// ожидаем сигналы
			sigset_t signals = proc->signals & event->value.signals;// сделать атомарную выборку
            if (signals){
				event->status = osEventSignal|osEventRunning;
				break;
            }
        } else
		if (event->status & (osEventSemaphore)) {
            volatile int* ptr = event->value.p;
			int count = semaphore_enter(ptr);
			// семафоры должны создаваться в shared memory и видны планировщику
			if (count > 0) { // счетчик семафора до входа
				event->status = osEventSemaphore|osEventRunning;
				break;
			}
		}
		if (event->status & (osEventTimeout)) {
            if ((clock_t)(clock() - proc->wait.timestamp) >= proc->wait.timeout){
                event->status = osEventTimeout|osEventRunning;
                break;
			}
		}
	}
}

void mtx_destroy(mtx_t *mtx){
	mtx->count = 0;
}
int mtx_init(mtx_t *mtx, int type){
	mtx->count = 1;
	return thrd_success;
}
int mtx_lock(mtx_t *mtx){
	int count = semaphore_enter(&mtx->count);
	if (count>0) return thrd_success;
	return osEventWait(osEventSemaphore, &mtx->count);
}
int mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts){
	int count = semaphore_enter(&mtx->count);
	if (count>0) return thrd_success;
	return osEventTimedWait(osEventSemaphore, &mtx->count, ts);
}
int mtx_trylock(mtx_t *mtx){
	int count = semaphore_enter(&mtx->count);
	return (count>0)? thrd_success: thrd_busy;
}
int mtx_unlock(mtx_t *mtx){
	semaphore_leave(&mtx->count);
	return thrd_success;
}
int  cnd_broadcast(cnd_t *cond){
	mtx_t *mtx;
	while ((mtx = atomic_list_pop(cond))!=NULL)
		mtx_unlock(mtx);
	return thrd_success;
}
void cnd_destroy(cnd_t *cond){
	mtx_t *mtx;
	while ((mtx = atomic_list_pop(cond))!=NULL)
		mtx_unlock(mtx);
}
int cnd_init(cnd_t *cond){
	*(volatile void**)cond=NULL;
	return thrd_success;
}
int cnd_signal(cnd_t *cond){
	mtx_t* mtx = atomic_list_pop(cond);
	if (mtx==NULL) return thrd_success;
	return mtx_unlock(mtx);
}
int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts){
	atomic_list_push(cond, mtx);
	return osEventTimedWait(osEventSemaphore, &mtx->count, ts);
}
int cnd_wait(cnd_t *cond, mtx_t *mtx){
	atomic_list_push(cond, mtx);
	return osEventWait(osEventSemaphore, &mtx->count);
}

/*!
Ожидаем завершения множества тредов
Один процесс может представлять флаг для нескольких
 */
// выделение флагов
	osEventWait(osEventSemaphore, &flags->sem);// ждем доступность флага
do{
	flag_id = __builtin_ctz(~map);// индекс флага
	map |= (1<<flag_id);
}
	flag_usage[flag_id]++; 
// назначение флагов при запуске треда
	thr->atexit() = ;
// завершение процесса 
	atomic_fetch_or(&parent->signals, 1<<thr->sig_no);// по умолчанию SIGCHLD
// ожидание 
	osEventWait(osEventSignalAny, map);
	// дождались завершения одного из процессов

Просматриваем очередь вперед.
	mask |= 1<<flag_id;
	osEventWait(osEventSignalAll, mask); // все сигналы из маски
	// не все флаги ждем, есть забытые флаги не использованные
// освобождение флагов
	while (mask) {
		flag_id = __builtin_ctz(mask);// индекс флага
		if((--flag_usage[flag_id])==0){
			flags->map ^= 1<<flag_id;
			flags->sem++;// semaphore_leave(flags->sem);//
		}
		mask ^= 1<<flag_id;
	}
/*! ThreadPool */
	sem_init(pool->slots, N);
// запуск 
	sem_wait(slot);
	thrd_create(thr, pool->func, data);
	thrd_detach(thr);
// При завершении загружается следующий блок данных
atexit:// этот путь быстрее, эффективнее
	// число одновременных процессов
	work = queue_next(pool->queue);
	thr->arg  = work->data;
	work->res = thr->func(work->data);
// чередь с множеством 
	// чтение
	do {
		head = atomic_get(ptr);
		if (head==NULL) {
			atomic_clear();
			tail= ptr; atomic_get(&queue->tail);
			if (tail == )
			break;
		}
	} while (!cas(ptr, head, head->next)); // без изменения
	if (head->next==NULL)
//запись в очередь
	ptr = tail;
	do {
		while (item = atomic_pointer_get(ptr)!=NULL) 
			*ptr = &item->next;
	} while (!cas(ptr, NULL, next)); // без изменения
	
#endif