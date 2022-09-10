#include <unistd.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/thread.h>
#include <atomic.h>
#include <svc.h>

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
/*! \addgroup POSIX_DEVICE_IO
	\{*/

int pselect(int nfds, fd_set *restrict read_fds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timespec *restrict timeout,
       const sigset_t *restrict sigmask)
{
	sigset_t signals = 0;
	if (read_fds) signals |= read_fds->fds_bits[0];
	if (writefds) signals |= writefds->fds_bits[0]<<1;
	if (errorfds) signals |= errorfds->fds_bits[0]<<2;

	pthread_t thr = pthread_self();
	sigset_t omask = atomic_exchange(&thr->process.sig_mask, sigmask[0]);// блокировать сигналы пользователя
	uint32_t interval = (uint32_t)((timeout->tv_sec*1000000U)+ timeout->tv_nsec/1000U);
	svc3(SVC_EVENT_WAIT, osEventSignal, signals, interval);
	int event_type = thr->process.event.status;
	if (event_type & osEventSignal) {
		signals     &= atomic_fetch_and(&thr->process.signals, ~signals);
		if (read_fds) read_fds->fds_bits[0] &= signals;
		if (writefds) writefds->fds_bits[0] &= signals>>1;
		if (errorfds) errorfds->fds_bits[0] &= signals>>2;
		thr->process.sig_mask = omask;// восстановить сигналы
		return __builtin_popcount(signals); 
	}
	thr->process.sig_mask = omask;// восстановить сигналы
	return (event_type & osEventTimeout)?0: -1;
}
int select(int nfds, fd_set *restrict read_fds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timeval *restrict ts)
{
	// если дескрипторы выделяются через 3 шт
	sigset_t signals = 0;
	if (read_fds) signals |= read_fds->fds_bits[0];
	if (writefds) signals |= writefds->fds_bits[0]<<1;
	if (errorfds) signals |= errorfds->fds_bits[0]<<2;
	
	// если дескрипторы выделяются подряд
	//sigset_t signals[3] = {read_fds->fds_bits[0], writefds->fds_bits[0], errorfds->fds_bits[0]};
	uint32_t interval = ts->tv_sec* 1000000U + ts->tv_usec;
	svc3(SVC_EVENT_WAIT, osEventSignal, signals, interval);
	pthread_t thr = pthread_self();
	int event_type = thr->process.event.status;
	if (event_type & osEventSignal) {
		signals = atomic_fetch_and(&thr->process.signals, ~signals);
		// если дескрипторы выделяются через 3 шт
		if (read_fds) read_fds->fds_bits[0] &= signals;
		if (writefds) writefds->fds_bits[0] &= signals>>1;
		if (errorfds) errorfds->fds_bits[0] &= signals>>2;
		
		return __builtin_popcount(signals); 
	}
	return (event_type & osEventTimeout)?0: -1;
}
	//!\}