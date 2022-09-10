#include <stdint.h>
#include <atomic.h>
#include <sys/thread.h>
#include <pthread.h>
#include <errno.h>

#include <stdlib.h>// abort, raise
#include <unistd.h>// alarm, kill, pause
#include <signal.h>
#include "svc.h"
#include "semaphore.h"

typedef struct _thread osThread_t;

extern volatile osThread_t* thread_current;
#define THREAD_PTR(x) ((osThread_t*)(x))
<<<<<<< HEAD
#if 1/*! \defgroup POSIX_SIGNALS POSIX: Signals
	\ingroup _posix
abort( ), alarm( ), kill( ), pause( ), raise( ), sigaction( ), sigaddset( ), sigdelset( ), sigemptyset( ),
sigfillset( ), sigismember( ), signal( ), sigpending( ), sigprocmask( ), sigsuspend( ), sigwait( )
	\{ */

=======
#if 0
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
/*! 


[EINVAL]
    The value of the sig argument is an invalid or unsupported signal number.
[EPERM]
    The process does not have permission to send the signal to any receiving process.
[ESRCH]
    No process or process group can be found corresponding to that specified by pid. 
*/
int kill(pid_t pid, int sig)
{
	return svc3(SVC_SIGNAL, pid, sig, 0);
}
int pthread_kill(pthread_t thr, int sig)
{
	return svc3(SVC_SIGNAL, THREAD_PTR(thr), sig, 0);
}
unsigned alarm(unsigned seconds)
{
//	return svc3(SVC_SIGNAL, thread_current, SIGALARM, seconds);
}
int raise(int sig)
{
	return svc3(SVC_SIGNAL, thread_current, sig, 0);
}
void abort(){
	(void) svc3(SVC_SIGNAL, thread_current, SIGABRT, 0);
}
void exit(int status){
	thread_current->process.result = (void*)(intptr_t)status;
	(void) svc3(SVC_EXIT, thread_current, status);
}

int pause(void) {/* suspend the thread until a signal is received */
	svc3(SVC_EVENT_WAIT, osEventSignal, ~0, osWaitForever);
	if (thrd->process.event.status & osEventSignal) return 0;
	thrd->error_no = EINTR;
	return -1;
}
int signal(int sig, void (*func)(int))
{
	
}
<<<<<<< HEAD
	//!\}
=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif

typedef struct _fifo osSigQueue_t;
struct _fifo {
	volatile int rdlock;// число свободных слотов на запись
	volatile int wrlock;// число свободных слотов на запись
	volatile int wrpos;// много писателей
	int rdpos;// один читатель
	uint32_t  ready[_POSIX_SIGQUEUE_MAX/32];
	siginfo_t info [_POSIX_SIGQUEUE_MAX];
};
#define SIGQUEUE_PTR(x) (osSigQueue_t*)(x)
#define ERR(e) -(e)
<<<<<<< HEAD

/*! \defgroup POSIX_REALTIME_SIGNALS POSIX: Realtime Signals
	\ingroup _posix
sigqueue( ), sigtimedwait( ), sigwaitinfo( )
	\{ */

=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
/*! \brief отправить сигнал с параметром 

[EAGAIN]
    No resources are available to queue the signal. The process has already queued {SIGQUEUE_MAX} signals that are still pending at the receiver(s), or a system-wide resource limit has been exceeded.
[EINVAL]
    The value of the signo argument is an invalid or unsupported signal number.
[EPERM]
    The process does not have appropriate privileges to send the signal to the receiving process.
[ESRCH]
    The process pid does not exist. 
 */
int sigqueue(pid_t pid, int signo, const union sigval value)
{
	osSigQueue_t * sig_queue = SIGQUEUE_PTR(pid);// найти как-то объект по идентификатору
	if (sig_queue==NULL) return ERR(ESRCH);
	
	int count = semaphore_enter(&sig_queue->wrlock);//счетчик ресурсов
	if (count == 0) return ERR(EAGAIN);
	int idx = atomic_fetch_add(&sig_queue->wrpos, 1) & (_POSIX_SIGQUEUE_MAX-1);// индекс FIFO
	// заполнить структуру siginfo
	siginfo_t *info = &sig_queue->info[idx];
	info->si_code  = SI_QUEUE;
	info->si_signo = signo;
	info->si_value = value;
	//info->pid = pid;
	atomic_fetch_or(&sig_queue->ready[idx/32], 1UL<<(idx%32));
	semaphore_leave(&sig_queue->rdlock);
	return 0;
}
static void* signal_queue_init(osSigQueue_t* sig_queue)
{
	int i;
	for (i=0; i< _POSIX_SIGQUEUE_MAX/32; i++) sig_queue->ready[i] = 0;
	sig_queue->wrpos = 0;
	sig_queue->rdpos = 0;
	semaphore_init(&sig_queue->wrlock, _POSIX_SIGQUEUE_MAX);
	return sig_queue;
}
static int signal_queue_get(osSigQueue_t* sig_queue, siginfo_t *info)
{
	int idx  = sig_queue->rdpos & (_POSIX_SIGQUEUE_MAX-1);
	volatile uint32_t *ptr = &sig_queue->ready[idx/32];
	unsigned int mask = 1U<<(idx%32);
	if (*ptr & mask);
	{
		*info = sig_queue->info[idx];
		// сигнал доставлен
		atomic_fetch_and (ptr, ~mask);
		semaphore_leave(&sig_queue->wrlock);
		sig_queue->rdpos++;
		return 1;
	}
	return 0;
	
}
#if 0
static void signal_queue_scan(void* data, siginfo_t *info)
{
	osSigQueue_t* sig_queue = data;
	int idx  = sig_queue->rdpos & (_POSIX_SIGQUEUE_MAX-1);
	volatile uint32_t *ptr = &sig_queue->ready[idx/32];
	unsigned int mask = 1U<<(idx%32);
	if (*ptr & mask){
		siginfo_t *info = &sig_queue->info[idx];
		
		
		
		// найти процесс которому адресованы сигналы
		osProcess_t * process = PROCESS_PTR(info->pid);
		// выбрать способ доставки:
		unsigned int sig_mask = 1U<<info->si_signo;
		if (process->event.status & osEventRunning) {
			// пропустить
		} else
		if (process->sig_mask & sig_mask) { // сигнал блокирован для доставки
			process->event.value = info->si_value;
			unsigned int sig_set = atomic_fetch_or (process->signals, sig_mask); 
			if (sig_set & sig_mask) {// сигнал не был доставлен
			} else {// сигнал доставлен
				atomic_fetch_and (&ptr, ~mask);
				semaphore_leave(&sig_queue->count);
				sig_queue->rdpos++;
			}
		} else {// обработка системных и пользовательских сигналов по умолчанию
			
		}
	}
}

typedef int (*rpmsg_rx_cb_t)(struct rpmsg_device *, void *, int, void *, uint32_t);
int sigqueue_rx_cb(struct rpmsg_device * dev, void *msg, int mlen, void *priv, uint32_t saddr)
{
	siginfo_t *info= msg;
	// для сигналов пользователя типа kill 
}
#endif
// переместить в линкерный скрипт загрузчика
const int __aeabi_SIGABRT = SIGABRT;
const int __aeabi_SIGFPE  = SIGFPE;
const int __aeabi_SIGILL  = SIGILL;
const int __aeabi_SIGINT  = SIGINT;
const int __aeabi_SIGSEGV = SIGSEGV;
const int __aeabi_SIGTERM = SIGTERM;
void __aeabi_SIG_IGN (int sig)
{
	switch (sig){
	default: break;
	}
}
void __aeabi_SIG_DFL (int sig)
{
	switch (sig){
//	case SIGABRT:
	case SIGTERM:
//	case SIGCHLD:// завершение дочернего процесса
	case SIGINT :// привлечь внимание
		break;
	}
}
void __aeabi_SIG_ERR(int sig) {// завершить процесс
}
/*!	\brief 

 есть некоторая особенность реализации 
	Предполагается что pid_t указывает на родительский процесс, 
	таким образом можно ожидать сигналы в родительском процессе.
	В OS Solaris есть вариант ожидания в треде.. 
	В POSIX тредах треды наследуют маску. маску можно установить \ref sigprocmask
	
    sigemptyset(&signal_mask);
    sigaddset  (&signal_mask, SIGINT);
    sigaddset  (&signal_mask, SIGTERM);
    rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);	
	
	\return Upon successful completion (that is, one of the signals specified by set is pending or is
generated) sigwaitinfo( ) and sigtimedwait( ) shall return the selected signal number. Otherwise,
the function shall return a value of −1 and set errno to indicate the error.
*/
int sigtimedwait (const sigset_t *set, siginfo_t *info, const struct timespec *ts)
{
	osSigQueue_t* sig_queue=NULL;
	osThread_t* self = (osThread_t*)thread_current;
	uint32_t mask = (self->process.signals & set[0]);
	if (mask){
		int signo = __builtin_ctz(mask);
		atomic_fetch_and (&self->process.signals, ~(1UL<<signo));// clear
		info->si_code  = SI_USER;
		info->si_signo = signo;
		info->si_value.sival_ptr = self->process.event.value.p;
		return signo;
	} else 
	if (signal_queue_get(sig_queue, info)){
		return 0;
	}
	
	uint32_t interval = (ts==NULL)? osWaitForever: (ts->tv_sec*1000000+ ts->tv_nsec/1000) - clock();
	svc3(SVC_EVENT_WAIT, osEventSignal, set[0], interval);
	int status = self->process.event.status;
	if (status & osEventSignal) {
		int signo = __builtin_ctz(self->process.signals);
		atomic_fetch_and (&self->process.signals, ~(1UL<<signo));// clear
		info->si_signo = signo;
		info->si_code  = SI_USER;//__builtin_ctz(status & 0x1E);
		info->si_value.sival_ptr = self->process.event.value.p;
		return signo;
	} else 
	if (status & osEventTimeout) {
		//return thrd_timeout;
		return -EAGAIN;
	} else {// ожидание прервано
		return -EINTR;
	}
	return 0;
}
// pthread_sigmask
int pthread_sigmask (int how, const sigset_t * restrict set, sigset_t * restrict oset)
{
	osThread_t *thr = (osThread_t *)thread_current;
	sigset_t msk=0;
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
	/*! \} */