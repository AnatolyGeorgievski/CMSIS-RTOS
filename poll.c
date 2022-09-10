#include <unistd.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/thread.h>
#include <atomic.h>
#include <svc.h>

#define FDS_SIG(fds, i) (1UL << fds[i].fd)
/*! \brief 
 */
int poll(struct pollfd *fds, nfds_t count, int timeout) {
	int i;
	uint32_t signals;
	uint32_t fds_bits[4];
	sigset_t sig_mask = 0;
	for (i=0; i< count; i++) {
		sig_mask |= fds[i].events << fds[i].fd;
/*		if (fds[i].events & POLLIN)
			sig_mask |= 1UL<<fds[i].fd;
		if (fds[i].events & POLLOUT)
			sig_mask |= 2UL<<fds[i].fd;
		if (fds[i].events & POLLERR)
			sig_mask |= 4UL<<fds[i].fd; */
	}

	pthread_t self = pthread_self();
	signals = self->process.signals;
	if ((signals & sig_mask) == 0) {
		svc3(SVC_EVENT_WAIT, osEventSignal, sig_mask, timeout);
		if (self->process.event.status & osEventTimeout) 
			return 0;
	}
// если дескрипторы выделяются c шагом через 3 шт
	signals  = atomic_fetch_and(&self->process.signals, ~sig_mask);
	signals &= sig_mask;
// если дескрипторы выделяются подряд, то sigset_t состоит из 3 элементов
	int res = 0;
	for (i=0; i< count; i++){
		fds->revents = 0x7 & (signals >> fds[i].fd);// in out err
		if (fds->revents) res++;
	}
	return res;
}