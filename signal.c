/* 

POSIX_SIGNALS: Signals
abort( ), alarm( ), kill( ), pause( ), raise( ), sigaction( ), sigaddset( ), sigdelset( ), sigemptyset( ),
sigfillset( ), sigismember( ), signal( ), sigpending( ), sigprocmask( ), sigsuspend( ), sigwait( )
sigqueue */
#include <stdint.h>
#include <atomic.h>
#include <sys/thread.h>
#include <errno.h>

#include <stdlib.h>// abort, raise
#include <unistd.h>// alarm, kill, pause
#include <signal.h>
#include "svc.h"

typedef struct _thread osThread_t;
typedef void* pthread_t;
extern volatile osThread_t* thread_current;
#define THREAD_PTR(x) ((osThread_t*)(x))
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
	return svc3(SVC_SIGNAL, thread_current, SIGALARM, seconds);
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
	osProc_t * proc = PROC_PTR(pid);// найти как-то объект по идентификатору
	if (proc==NULL) self->error_no = ESRCH, return -1;
//	if (proc->sig_mask & (1<<signo)) self->error_no = EINVAL, return -1;
	int count = semaphore_enter(&proc->sig_queue_sem);//счетчик ресурсов
	if (count == 0) self->error_no = EAGAIN, return -1;
	int idx = atomic_fetch_add(&proc->sig_queue.wrpos, 1) & (_POSIX_SIGQUEUE_MAX-1);// индекс FIFO
	//proc->sig_queue.data[idx] = value;
	siginfo_t * msg = &proc->sig_queue.data[idx];// взять буфер в 

	msg->si_code  = SI_QUEUE;
	msg->si_signo = signo;
	msg->si_value = value;

	proc->send(dev, msg, sizeof(info)); // выставить бит.

	sigset_t mask = (1UL << value->sival_int);
	if (thr->sig_mask & mask){// signal blocked for delivery
		if ( atomic_fetch_or(&thr->process.signals, mask) & mask) {
			// добавить в очередь или вернуть ...
			return -1;
		}
	} else {// применить стандартный способ обработки сигналов
		__aeabi_SIG_DFL(sig);
	}
	return 0;//svc3(SVC_SIGNAL, pid, sig, value);
}
typedef int (*rpmsg_rx_cb_t)(struct rpmsg_device *, void *, int, void *, uint32_t);
int sigqueue_rx_cb(struct rpmsg_device * dev, void *msg, int mlen, void *priv, uint32_t saddr);
{
	siginfo_t *info= msg;
	// для сигналов пользователя типа kill 
}
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
	case SIGABRT:
	case SIGTERM:
	case SIGCHLD:// завершение дочернего процесса
	case SIGINT :// привлечь внимание
	}
}
void __aeabi_SIG_ERR(int sig) {// завершить процесс
}
/*! есть некоторая особенность реализации 
	Предполагается что pid_t указывает на родительский процесс, 
	таким образом можно ожидать сигналы в родительском процессе.
	В OS Solaris есть вариант ожидания в треде.. 
	В POSIX тредах треды наследуют маску. маску можно установить \ref sigprocmask
	
    sigemptyset(&signal_mask);
    sigaddset  (&signal_mask, SIGINT);
    sigaddset  (&signal_mask, SIGTERM);
    rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);	
	
*/
int sigtimedwait (const sigset_t *set, siginfo_t *info, const struct timespec *ts)
{
	//
//	osEvent_t event = {.status = osEventSignal, .value={.s = *set}};
	uint32_t interval = (ts->tv_sec*1000000+ ts->tv_nsec/1000) - clock();
	svc3(SVC_EVENT_WAIT, osEventSignal, set[0], interval);
	int status = self->process.event.status;
	if (status & osEventSignal) {
		info->si_signo = SIGINT;// 
		info->si_code  = SI_QUEUE;
		info->si_value = (union sigval) event.value;
		return 0;
	} else 
	if (event.status & osEventMessage) {
		info->si_signo = SIGINT;// 
		info->si_code  = SI_MESGQ;
		info->si_value = (union sigval) event.value;
	} else 
	if (event.status & osEventTimeout) {
		//return thrd_timeout;
		self->error_no = EAGAIN;
	} else {
		self->error_no = EINTR;
	}
	return -1;
}
// pthread_sigmask
int sigprocmask (int how, const sigset_t * restrict set, sigset_t * restrict oset)
{
	osThread_t *thr = thread_current;
	sigset_t msk;
	switch (how) {
	case SIG_SETMASK:
		msk = atomic_exchange(&thr->sig_mask, set);
		break;
	case SIG_BLOCK:
		msk = atomic_fetch_or(&thr->sig_mask, set);
		break;
	case SIG_UNBLOCK:
		msk = atomic_fetch_and(&thr->sig_mask, ~set);
		break;
	}
	if (oset) *oset = msk;
	return 0;
}