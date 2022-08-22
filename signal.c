/* 


sigqueue */
#include <errno.h>
#include <signal.h>
#include "svc.h"
typedef 
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
	return svc3(SVC_SIGNAL, thr, sig, 0)
}
int raise(int sig)
{
	return svc3(SVC_SIGNAL, pthread_self(), sig, 0);
}
void abort(){
	svc3(SVC_SIGNAL, pthread_self(), SIGABRT, 0);
}
void exit(int status){
	svc3(SVC_SIGNAL, pthread_self(), SIGTERM, status);
}

/*! \brief отправить сигнал с параметром 

 */
int sigqueue(pid_t pid, int sig, const union sigval value){
	pthread_t * thr = pid;// найти как-то
	if (thr->sig_mask & (1<<sig))// signal blocked
		atomic_fetch_or(&thr->signals, (1<<sig));
	return 0;//svc3(SVC_SIGNAL, pid, sig, value);
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
	osEvent event = {.status = osEventSignal, .value={.s = *set}};
	osEventTimedWait(&event, ts);
	if (event.status & osEventSignal) {
		info->si_signo = SIGINT;// 
		info->si_code  = SI_QUEUE;
		info->si_value = (union sigval) event.value;
		return 0;
	} else if (event.status == osEventTimeout) {
		//return thrd_timeout;
		errno = EAGAIN;
	} else {
		errno = EINTR;
	}
	return -1;
}
// pthread_sigmask
int sigprocmask (int how, const sigset_t * restrict set, sigset_t * restrict oset)
{
	pthread_t *thr = pthread_self();
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