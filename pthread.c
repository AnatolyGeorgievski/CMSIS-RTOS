/*! \file pthread.c -- POSIX Threads. реализация в составе R3v2 RTOS
	\author Anatoly Georgievskii <anatoly.georgievski@gmail.com>
	\date 2022-08-16
	\copyright 2022
 */
#include <atomic.h>		// Аппаратно-зависимые определения machine/atomic.h
#include <errno.h>		// С Library Arm EABI 
#include <unistd.h>  	// стандартные вызовы UNIX конфигурация задана набором определений _POSIX_*
#include <pthread.h> 	// POSIX Threads
#include <semaphore.h>	// POSIX Semaphores
#include <svc.h>		// переделать на sys/svccall.h
#include <sys/thread.h>
#include <sys/stdio.h>// dev
#include <stdlib.h>		// malloc

// определено в ERRNO
//enum {EOK, EBUSY, ETIMEDOUT, EINTR, EOWNERDEAD};

typedef struct _List List_t;
struct _List {
	struct _List * next;
};

typedef struct _thread osThread_t;

struct _mtx {
	volatile int count;
};
struct _cond {
	List_t* mtx;
};
/* 
Для взаимодействия с ОС используется ряд переменных расположенных в SHARED USER_RO памяти
Мы используем простую модель памяти, 

 */
volatile osThread_t * thread_current=NULL;// __attribute__((segment("shared")));
/*! выделение флагов, 
	\return индекс первого ненулевого флага, или -1 при невозможности выделения
 */
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
//#define atomic_list_push(, data) __extension__({ })
static void* atomic_list_pop(volatile void** ptr){
	List_t* item;
	do {
		item = atomic_pointer_get(ptr);
		if (item == (void*)0) break;
	} while (!atomic_pointer_compare_and_exchange(ptr, item, item->next));
	atomic_mb();
	return item;
}
static void* atomic_list_push(volatile void** ptr, void* data){
	List_t* item = (List_t*)data;
	List_t* next;
	do {
		next = atomic_pointer_get(ptr);
		if (next == (void*)~0) break;
		item->next = next;
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, next, item));
	return next;
}
static int osEventWait(int type, uint32_t value, uint32_t interval)
{
	int status = svc3(SVC_EVENT_WAIT, type, value, interval);
	pthread_t thr = pthread_self();
	status = thr->process.event.status;
	if (status & type) return 0;
	if (status & osEventTimeout) return ETIMEDOUT;
	return EINTR;
<<<<<<< HEAD
=======
}
static int osEventTimedWait(int type, uint32_t value, const struct timespec * restrict ts)
{
//	struct timespec *now;
//	clock_gettime(CLOCK_REALTIME, &now);
	uint32_t interval = (ts->tv_sec* 1000000U + ts->tv_nsec/1000) - clock();
	return osEventWait(type, value, interval);
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
}
static int osEventTimedWait(int type, uint32_t value, const struct timespec * restrict ts)
{
//	struct timespec *now;
//	clock_gettime(CLOCK_REALTIME, &now);
	uint32_t interval = (ts->tv_sec* 1000000U + ts->tv_nsec/1000) - clock();
	return osEventWait(type, value, interval);
}
/*!	\ingroup _posix

Чем отличаются мьютексы: - это невозможность разблокировать в чужом треде 
Unlock When Not Owner
Рекурсивные мьютексы подсчитывают число блокировок, 
это достигается за счет проверки владельца. 
Владелец может увеличивать и уменьшать счетчик блокировок.
Для реализации condition variables можно использовать только нормальные мьютексы. 

Спинлоки - это разделяемые блокировки в момент захвата владелец меняется. 
В нашей реализации - нет привязки к владельцу.

Реализация семафоров мьютексов и спинлоков основана на атомарных операциях счетчика семафора. 
Семафоры должны выделяться в памяти SHARED UNPRIVILEGED
Выделение семафоров на стеке позволяет обращаться к ним из планировщика, 
но не позволяет их использовать из других процессов. Функционал PSHARED 
- разделяемые между процессами не развит.
*/

/*! \defgroup TPS POSIX: Thread Execution Scheduling (TPS)
	\{ */
int pthread_attr_getschedpolicy(const pthread_attr_t *restrict attr, int *restrict policy){
	*policy = attr->schedpolicy;
	return 0;
}
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy){
	attr->schedpolicy = policy;
	return 0;
}
int pthread_attr_setinheritsched(pthread_attr_t * attr, int inheritsched){
	attr->inheritsched = inheritsched;
	return 0;
}
int pthread_attr_getinheritsched(const pthread_attr_t *restrict attr,
								int *restrict inheritsched){
	*inheritsched = attr->inheritsched;
	return 0;
}
	//!\}
	
/*! \defgroup POSIX_THREADS_BASE POSIX: Base Threads
	\ingroup _posix
	\{
pthread_atfork( ), pthread_attr_destroy( ), pthread_attr_getdetachstate( ),
pthread_attr_getschedparam( ), pthread_attr_init( ), pthread_attr_setdetachstate( ),
pthread_attr_setschedparam( ), pthread_cancel( ), pthread_cleanup_pop( ), pthread_cleanup_push( ),
pthread_cond_broadcast( ), pthread_cond_destroy( ), pthread_cond_init( ), pthread_cond_signal( ),
pthread_cond_timedwait( ), pthread_cond_wait( ), pthread_condattr_destroy( ),
pthread_condattr_init( ), pthread_create( ), pthread_detach( ), pthread_equal( ), pthread_exit( ),
pthread_getspecific( ), pthread_join( ), pthread_key_create( ), pthread_key_delete( ), pthread_kill( ),
pthread_mutex_destroy( ), pthread_mutex_init( ), pthread_mutex_lock( ),
pthread_mutex_timedlock( ), pthread_mutex_trylock( ), pthread_mutex_unlock( ),
pthread_mutexattr_destroy( ), pthread_mutexattr_init( ), pthread_once( ), pthread_self( ),
pthread_setcancelstate( ), pthread_setcanceltype( ), pthread_setspecific( ), pthread_sigmask( ),
pthread_testcancel( ) */
int pthread_attr_init(pthread_attr_t * attr){
	*attr = (pthread_attr_t){0};
	return 0;
}
int pthread_attr_destroy(pthread_attr_t * attr){
	return 0;
}
int pthread_attr_setdetachstate(pthread_attr_t * attr, int detached){
	attr->detachstate = detached;
	return 0;
}
int pthread_attr_getdetachstate(const pthread_attr_t * attr, int *detachstate){
	*detachstate = attr->detachstate;
	return 0;
}
int pthread_attr_setschedparam(pthread_attr_t * attr, 
const struct sched_param *restrict schedparam){
	//attr->schedparam = *schedparam;
	return -1;
}
int pthread_attr_getschedparam(const pthread_attr_t *restrict attr, 
struct sched_param *restrict schedparam){
	//*schedparam = attr->schedparam;
	return -1;
}
pthread_t pthread_self(void){
	return (pthread_t)thread_current;//  переменная в кластере CPU может указывать на другой тред.
}

int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,
	void *(*start_routine)(void*), void *restrict arg)
{
	osThread_t* thr=NULL;
	if (thread) thr = *thread;
	if (thr==NULL) {
		thr = malloc(sizeof(osThread_t));// надо выделить shared
		__builtin_bzero(thr, sizeof(osThread_t));
	}
	thr->parent = (thread==NULL)? NULL: pthread_self();// detached state
	thr->process.func   = start_routine;
	thr->process.arg    = arg;
	thr->process.sig_mask = 0;
	if (thr->next == NULL) {
		pthread_t self = pthread_self();
		atomic_list_push((volatile void**)&self->next, thr);
	}
	thr->process.event.status = osEventRunning;
	if (thread) *thread = thr;
	return 0;
}
int pthread_detach(pthread_t thread) {
	thread->parent = NULL; // self
}
int pthread_join(pthread_t thread, void **value_ptr){
//	thread->parent = pthread_self();
	osEventWait(osEventSignal, 1<<thread->sig_no, osWaitForever);
	*value_ptr = thread->process.result;
	
	//удалить или использовать повторно
}
void pthread_exit(void *value_ptr)
{
//	thr = pthread_self();
//	thr->process.result = value_ptr;
	svc1(SVC_EXIT, value_ptr);
	while(1);
}
// It is advised that an application should not use a PTHREAD_MUTEX_RECURSIVE mutex with condition variables
int pthread_cond_broadcast(pthread_cond_t *cond)
{
	pthread_mutex_t *mtx;
	while ((mtx = atomic_list_pop((volatile void**)&cond->mtx))!=NULL)
		pthread_mutex_unlock(mtx);
	return 0;
}
int pthread_cond_signal(pthread_cond_t *cond)
{
	pthread_mutex_t* mtx = atomic_list_pop((volatile void**)&cond->mtx);
	if (mtx==NULL) return 0;
	return pthread_mutex_unlock(mtx);
}
int pthread_cond_destroy(pthread_cond_t *cond)
{
	void* ptr = atomic_exchange((void**)&cond->mtx, (void*)~0);
	pthread_mutex_t *mtx;
	while ((mtx = atomic_list_pop((volatile void**)&ptr))!=NULL) // атомарность не требуется
		pthread_mutex_unlock(mtx);
	return 0;
}
int pthread_cond_init(pthread_cond_t *restrict cond,
	const pthread_condattr_t *restrict attr)
{
	*(volatile void**)(&cond->mtx)=NULL;
	//cond->attr = attr;
	return 0;
}
int pthread_cond_timedwait(pthread_cond_t *restrict cond,
	pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	void* prev = atomic_list_push((volatile void**)&cond->mtx, mutex);
	if (prev==(void*)~0) return EOWNERDEAD;
	return osEventTimedWait(osEventSemaphore, (uint32_t)&mutex->count, abstime);
}
int pthread_cond_wait(pthread_cond_t *restrict cond,
	pthread_mutex_t *restrict mutex)
{
	void* prev = atomic_list_push((volatile void**)&cond->mtx, mutex);
	if (prev==(void*)~0) return EOWNERDEAD;
	return osEventWait(osEventSemaphore, (uint32_t)&mutex->count, osWaitForever);
}
int pthread_mutex_destroy(pthread_mutex_t *mtx){
	mtx->count = 0;
	return 0;
}
int pthread_mutex_init(pthread_mutex_t *restrict mtx, const pthread_mutexattr_t *restrict attr){
	mtx->count = 1;
	//mtx->attr = attr;
	return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mtx){
	int count = semaphore_enter(&mtx->count);
	if (count>0) return 0;
	return osEventWait(osEventSemaphore, (uint32_t)&mtx->count, osWaitForever);
}
int pthread_mutex_timedlock(pthread_mutex_t *restrict mtx, const struct timespec *restrict abstime){
	int count = semaphore_enter(&mtx->count);
	if (count>0) return 0;
	return osEventTimedWait(osEventSemaphore, (uint32_t)&mtx->count, abstime);
}
int pthread_mutex_trylock(pthread_mutex_t *mtx){
	int count = semaphore_enter(&mtx->count);
	return (count>0)? 0: EBUSY;
}
int pthread_mutex_unlock(pthread_mutex_t *mtx){
	semaphore_leave(&mtx->count);
	return 0;
}
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)){
	int count = atomic_fetch_or(once_control,1);
	if (count==0) init_routine();
}
int pthread_sigmask (int how, const sigset_t * restrict set, sigset_t * restrict oset)
{
	pthread_t thr = pthread_self();
	sigset_t msk;
	switch (how) {
	case SIG_SETMASK:
		msk = atomic_exchange(&thr->process.sig_mask, set[0]);
		break;
	case SIG_BLOCK:
		msk = atomic_fetch_or(&thr->process.sig_mask, set[0]);
		break;
	case SIG_UNBLOCK:
		msk = atomic_fetch_and(&thr->process.sig_mask, ~set[0]);
		break;
	}
	if (oset) *oset = msk;
	return 0;
}
<<<<<<< HEAD
//!\}
=======
#include <sys/select.h>
// Три вида сигналов в одной маске.

int __popcountsi2 (uint32_t n) {
	uint8_t pop[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
	uint32_t count =0;
	while (n) {
		count += pop[n&0xF];
		n>>=4;
	}
	return count;
}
int pselect(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timespec *restrict timeout,
       const sigset_t *restrict sigmask)
{
	sigset_t signals = readfds->fds_bits[0]|(writefds->fds_bits[0]<<1)|(errorfds->fds_bits[0]<<2);
	pthread_t thr = pthread_self();
	sigset_t omask = atomic_exchange(&thr->process.sig_mask, sigmask[0]);// блокировать сигналы пользователя
	uint32_t interval = (uint32_t)((timeout->tv_sec*1000000U)+ timeout->tv_nsec/1000U);
	osEventWait(osEventSignal, signals, interval);
	int event_type = thr->process.event.status;
	if (event_type & osEventSignal) {
		signals     &= atomic_fetch_and(&thr->process.signals, ~signals);
		readfds ->fds_bits[0] &= signals;
		writefds->fds_bits[0] &= signals>>1;
		errorfds->fds_bits[0] &= signals>>2;
		thr->process.sig_mask = omask;// восстановить сигналы
		return __builtin_popcount(signals); 
	}
	thr->process.sig_mask = omask;// восстановить сигналы
	return (event_type & osEventTimeout)?0: -1;
}
int select(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timeval *restrict ts)
{
	
	sigset_t signals = readfds->fds_bits[0]|(writefds->fds_bits[0]<<1)|(errorfds->fds_bits[0]<<2);
	uint32_t interval = ts->tv_sec* 1000000U + ts->tv_usec;
	osEventWait(osEventSignal, signals, interval);
	pthread_t thr = pthread_self();
	int event_type = thr->process.event.status;
	if (event_type & osEventSignal) {
		signals  &= atomic_fetch_and(&thr->process.signals, ~signals);
		readfds ->fds_bits[0] &= signals;
		writefds->fds_bits[0] &= signals>>1;
		errorfds->fds_bits[0] &= signals>>2;
		return __builtin_popcount(signals); 
	}
	return (event_type & osEventTimeout)?0: -1;
}
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#if 0
int socket(int domain, int type, int protocol) {
	int fd = _flags_alloc();
	return fd;
}
int bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	
}
int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len){
	int fd = _flags_alloc();
	return -1;
}
int connect(int socket, const struct sockaddr *address, socklen_t address_len){
	int fd = _flags_alloc();
	return -1;
}
ssize_t send(int socket, const void *buffer, size_t length, int flags)
{
	Object_t *obj = devices[socket];
	if (obj->type == SOCKET) {
		dev->send(dev->hdl, buffer, length);
	}
	return -1;
}
ssize_t recv(int socket, void *buffer, size_t length, int flags){
	Object_t *obj = devices[socket];
	if (obj->type == SOCKET) {
		
	}
	return -1;
}
#endif
#if defined(_POSIX_SEMAPHORES) && (_POSIX_SEMAPHORES > 0)
/* SEM _POSIX_SEMAPHORES (Semaphores) */
/*! \defgroup POSIX_SEMAPHORES POSIX: Semaphores (SEM)
sem_close( ), sem_destroy( ), sem_getvalue( ), sem_init( ), sem_open( ), sem_post( ),
sem_timedwait( ), sem_trywait( ), sem_unlink( ), sem_wait( )

 \{ */
struct _sem {
	volatile int count;
};
int	 sem_destroy(sem_t *sem){
	sem->count = -1;
	__DSB();
}
int  sem_getvalue(sem_t *restrict sem, int *restrict value){
	*value =  sem->count;
	return 0;
}
int  sem_init(sem_t * sem, int pshared, unsigned value){
	sem->count = value;
	__DSB();
	return 0;
}
int  sem_post(sem_t * sem){
	return semaphore_leave(&sem->count);
}
int  sem_timedwait(sem_t * sem, const struct timespec *restrict ts){
	int count = semaphore_enter(&sem->count);
	if (count >0) return 0;
	return osEventTimedWait(osEventSemaphore, (uint32_t)&sem->count, ts);
}
int  sem_trywait(sem_t * sem){
	int count = semaphore_enter(&sem->count);
	return (count >0)? 0: EBUSY;
}
int  sem_wait(sem_t * sem){
	int count = semaphore_enter(&sem->count);
	if (count >0) return 0;
	return osEventWait(osEventSemaphore, (uint32_t)&sem->count, osWaitForever);
}
#if defined(_POSIX_THREAD_PROCESS_SHARED) && (_POSIX_THREAD_PROCESS_SHARED > 0)
#include <stdarg.h>
#include <fcntl.h>
int sem_close(sem_t* sem) {
<<<<<<< HEAD
	Device_t *dev = (Device_t *)sem -1;
	dtree_unref(dev);// уменьшает nlink
	return 0;
}
int sem_unlink(const char* path) {
	Device_t *dev = dtree_path(NULL, path, &path);
	if (dev==NULL || path!=NULL) return -1;
	dtree_unref(dev);
	return 0;
}
sem_t* sem_open(const char* path, int oflags, ...)
{
//	sem_t *sem = shared_object_open(path, oflags, DEV_SEM);// рождается заблокированным и незримым
	const char* name;
	Device_t *dev = dtree_path(NULL, path, &name);// рождается заблокированным и незримым
	if (dev ==NULL) return NULL; 
	if (name!=NULL && (oflags & (O_CREAT))) {
		va_list  ap;
		va_start(ap, oflags);
		mode_t mode = va_arg(ap, int);//mode_t);
		uint32_t value = va_arg(ap, uint32_t);
		va_end(ap);
		dev = dtree_mknodat(dev, name, mode, DEV_SEM);
		sem_t *sem = (sem_t*)(dev+1);
		sem_init(sem, 1, value);
	}
	return (sem_t*)(dev+1);
}
#endif
 //!\}
=======
	return shared_object_close(sem);
}
int sem_unlink(const char* path) {
	return shared_object_unlink(path);// уменьшает число ссылок и убивает
}
sem_t* sem_open(const char* path, int oflags, ...)
{
	sem_t *sem = shared_object_open(path, oflags/*, DEV_SEM */);// рождается заблокированным и незримым
	if (sem!=NULL && (oflags & (O_CREAT))) {
		va_list  ap;
		va_start(ap, oflags);
		mode_t mode = va_arg(ap, mode_t);
		uint32_t value = va_arg(ap, uint32_t);
		va_end(ap);
		sem_init(sem, 1, value);
		shared_object_mode(sem, mode);// разрешить доступ и назначить права
	}
	return sem;
}
#endif
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif
#if defined(_POSIX_SPIN_LOCKS) && (_POSIX_SPIN_LOCKS > 0)
/* SPI _POSIX_SPIN_LOCKS (Spin Locks) */
/*! \defgroup POSIX_SPIN_LOCKS POSIX: Spin Locks 
	\ingroup _posix
	\{ */
typedef struct _lock pthread_spinlock_t;
struct _lock {
	volatile int count;
};

int   pthread_spin_destroy(pthread_spinlock_t *lock){
	lock->count = -1;
	return 0;
}
int   pthread_spin_init(pthread_spinlock_t *lock, int pshared){
	semaphore_init(&lock->count, 1);
	return 0;
}
int   pthread_spin_lock(pthread_spinlock_t *lock){
	int count = semaphore_enter(&lock->count);
	if (count >0) return 0;
	return osEventWait(osEventSemaphore, (uint32_t)&lock->count, osWaitForever);
}
int   pthread_spin_trylock(pthread_spinlock_t *lock){
	int count = semaphore_enter(&lock->count);
	return (count>0)?0: EBUSY;
}
int   pthread_spin_unlock(pthread_spinlock_t *lock) {
	semaphore_leave(&lock->count);
#if defined(_POSIX_THREAD_PROCESS_SHARED) && (_POSIX_THREAD_PROCESS_SHARED>0)
// 	hw_semaphore_unlock(&lock->hw, pid_t);
#endif
	return 0;
}
//! \}
#endif
#if defined(_POSIX_READER_WRITER_LOCKS)   && (_POSIX_READER_WRITER_LOCKS   > 0)
/*! \defgroup POSIX_READER_WRITER_LOCKS POSIX: Reader Writer Locks 
	\ingroup _posix
	\{ */
/*! Для получения доступа на чтение или запись используется один счетчик числа блокировок.
	Единая переменная позволяет выполнить операции атомарно. 
	Блокировка возникает при обращении на чтение ReadLock.  
	Число читателей расчитывается в отрицательных числах. 
	Если число читателей равно нулю, можно получить доступ на запись. 
	При этом читатели блокируются, значение счетчика 0 не позволяет получить доступ на чтение. 
	Если все читатели вышли, писатель может получить доступ на изменение. 
	Писатель может ожидать освобождение ресурса. 

	Читатель не может ждать, надо придумать другой метод. 
	Например Yield в цикле, передать упраление писателю.
	Мы предполагаем использовать методы RWLock для работы с системным реестром (директорией). 
*/

struct _rwlock {
	volatile int count;// число читателей
//	const pthread_rwlockattr_t* attr;
};

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
	rwlock->count = 0;
	return 0;
}
int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attr){
	rwlock->count = 1;
//	rwlock->attr  = attr;
	return 0;
}
/*!	\brief 
	\param 
	\return  shall return zero if the lock for writing on the read-write
lock object referenced by rwlock is acquired. Otherwise, an error number shall be returned to
indicate the error
		\arg EBUSY The read-write lock could not be acquired for writing because it was already
		locked for reading or writing.
		
 */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	volatile int* ptr = &rwlock->count;
	int count;
	do {// если число читателей нуль, заблокировать чтение. 
		count = atomic_int_get(ptr);
		if (count <= 0) 
			return EBUSY;
	}while(!atomic_int_compare_and_exchange(ptr, count, 0));
	return 0; // count 1 - получен доступ
}
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	volatile int* ptr = &rwlock->count;
	int count;
	do {// если число читателей нуль count=1, заблокировать чтение. 
		count = atomic_int_get(ptr);
		if (count <= 0) 
			return osEventWait(osEventSemaphore, (uintptr_t)&rwlock->count, osWaitForever);// Уменьшает на единицу, если больше 0
	}while(!atomic_int_compare_and_exchange(ptr, count, 0));
	return 0; // 1 - получен доступ
}
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	volatile int* ptr = &rwlock->count;
	int count;
	do { // увеличить число читателей
		count = atomic_int_get(ptr);
		if (count==0) // заблокировано для читателей и для писателей
			return EBUSY;
	}while(!atomic_int_compare_and_exchange(ptr, count, count-2));
	return 0;
}
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	volatile int* ptr = &rwlock->count;
	register int count;
	do {// уменьшить число читателей или сбросить блокировку чтения
		count = atomic_int_get(ptr);
	}while(!atomic_int_compare_and_exchange(ptr, count, count==0?1:count+2));
	return 0;
}
	//! \}
#endif
#if defined(_POSIX_BARRIERS)   && (_POSIX_BARRIERS   > 0)
/*! \defgroup POSIX_BARRIERS POSIX: Barriers (BAR) 
	\ingroup _posix
	\{ */
/*! 
Синхронизация множества тредов с использованием барьеров исполнения
Мы используем счетчик ресурсов, когда он достигает заданного числа, 
разрешаем проход.  
 */
struct _barrier {
	volatile int count;
	int threshold;
	sem_t sem;
};

int   pthread_barrier_destroy(pthread_barrier_t *barrier){
	return 0;
}
int   pthread_barrier_init(pthread_barrier_t *restrict barrier,
          const pthread_barrierattr_t *restrict attr, unsigned count)
{
//	assert(count>0);
	barrier->count = 0;
	barrier->threshold = count-1;
	sem_init(&barrier->sem,0,0);
	return 0;
}
int   pthread_barrier_wait(pthread_barrier_t *barrier)
{
	int count = atomic_fetch_add(&barrier->count, 1);// увеличили число ждунов
	if (count < barrier->threshold){
		sem_wait(&barrier->sem);
	} else {
		barrier->sem.count = barrier->threshold;
		atomic_mb();// записать в память
	}
	count = atomic_fetch_sub(&barrier->count,1);// уменьшили
	return count==1?PTHREAD_BARRIER_SERIAL_THREAD:0;
}
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) {
	return 0;
}
// TSH 
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict attr, int *restrict pshared){
	return 0;
}
int pthread_barrierattr_init(pthread_barrierattr_t *attr){
	return 0;
}
// TSH 
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared){
	return 0;
}
	//!\}
#endif