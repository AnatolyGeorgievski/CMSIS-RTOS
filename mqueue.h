#ifndef _MQUEUE_H_
#define _MQUEUE_H_
#include <sys/types.h>
#include <signal.h>// sigevent
//#include <sys/_timespec.h> //timespec
//! An mq_attr structure shall have at least the following fields:
typedef void* mqd_t;
struct timespec;

struct mq_attr {
	long mq_flags;		//!< Message queue flags.
	long mq_maxmsg;		//!< Maximum number of messages.
	long mq_msgsize;	//!< Maximum message size.
	long mq_curmsgs;	//!< Number of messages currently queued.
};

int 	mq_close(mqd_t);
int 	mq_getattr(mqd_t, struct mq_attr *);
int 	mq_notify(mqd_t, const struct sigevent *);
mqd_t 	mq_open(const char *, int, ...);
ssize_t mq_receive(mqd_t, char *, size_t, unsigned *);
int 	mq_send(mqd_t, const char *, size_t, unsigned);
int 	mq_setattr(mqd_t, const struct mq_attr *restrict, struct mq_attr *restrict);
ssize_t mq_timedreceive(mqd_t, char *restrict, size_t, unsigned *restrict, const struct timespec *restrict);
int 	mq_timedsend(mqd_t, const char *, size_t, unsigned, const struct timespec *);
int 	mq_unlink(const char *);
#endif//_MQUEUE_H_