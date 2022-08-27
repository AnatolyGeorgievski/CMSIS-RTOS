/* MSG POSIX mqueue.c */
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>
/*
[EBADF] The mqdes argument is not a valid message queue descriptor.
[EBUSY] A process is already registered for notification by the message queue.
The mq_notify( ) function may fail if:
[EINVAL] The notification argument is NULL and the process is currently not registered


*/
typedef struct _MQueue osMQueue_t;
struct _MQueue {
	struct sigevent notification;
};
#define MQ_PTR(mqdes) (osMQueue_t*)(mqdes)
int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
	osMQueue_t *mq = MQ_PTR(mqdes);
	mq->notification = *notification;
}

