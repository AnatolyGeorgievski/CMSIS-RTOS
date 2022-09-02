/* MSG POSIX mqueue.c */
#include <unistd.h>
/*
[EBADF] The mqdes argument is not a valid message queue descriptor.
[EBUSY] A process is already registered for notification by the message queue.
The mq_notify( ) function may fail if:
[EINVAL] The notification argument is NULL and the process is currently not registered


*/
#if defined(_POSIX_MESSAGE_PASSING) && (_POSIX_MESSAGE_PASSING>0)
#include <mqueue.h>

typedef struct _MQueue osMQueue_t;
struct _MQueue {
	struct sigevent notification;
	const struct mq_attr * attr;
};
#define MQ_PTR(mqdes) (osMQueue_t*)(mqdes) //
#define MQ_DES(mq) (mqd_t)(mq)

#include <signal.h>

int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
//	svc3(SVC_NOTIFY, osEventMessageQ, mqdes, notification->sigev_notify_function);
	return 0;
}

int mq_setattr(mqd_t mdes, const struct mq_attr *restrict attr, struct mq_attr *restrict oattr)
{
	osMQueue_t *mq = MQ_PTR(mdes);
	mq->attr = attr;
	return 0;
}
/*
Теория event notify
определяется 
case SI_SIGNAL:// доставка через флаги сигналов треда, типично для обработки прерываний
	mask = atomic_fetch_or(process.signals,  1<<sig_no);
	if (mask & (1<<sig_no)){ // возникла коллизия
		// поставить в очередь, очередь обрабатывается в фоне
		sigqueue(pid, 
	}
case SI_QUEUE:// доставка через очередь, очередь общая для всех процессов
	(pid, )
case SI_MESGQ:// доставка через сообщения

case SI_TIMER:
	создает тред 
	if (thread не занят)
		thread = thrd_create(&thread, sigev_notify_function, sigev_value);
		osThreadNotify(thrd_t thread) -- запрашивает переключение задач, меняет очередность обработки
		


 */
void rpmsg_handler(uint32_t ept){
}
#if defined(_POSIX_THREAD_PROCESS_SHARED) && (_POSIX_THREAD_PROCESS_SHARED > 0)
#include <stdarg.h>
#include <fcntl.h>
#include <atomic.h>
#include <r3_slice.h>
#include "pshared.h"

static int mq_unique_id=0;
static const SharedClass_t mq_class = 
	{"rpm", DEV_MSG, sizeof(osMQueue_t), &mq_unique_id};

/*! 
	\sa \ref sem_open \куа shm_open
*/
mqd_t mq_open(const char *path, int oflag, ...)
{
	osMQueue_t* mq = shared_object_open(path, oflag, &mq_class);// рождается заблокированным и незримым
	if (mq!=NULL && (oflag & (O_CREAT))) {
		va_list  ap;
		va_start(ap, oflag);
		mode_t mode = va_arg(ap, mode_t);
		const struct mq_attr* attr = va_arg(ap, void*);
		va_end(ap);
		mq_setattr(mq, attr, NULL);
		shared_object_mode(mq, mode);// разрешить доступ и назначить права
	}
	return MQ_DES(mq);
}
int mq_close(mqd_t mdes) {
	return shared_object_close(MQ_PTR(mdes), &mq_class);
}
int mq_unlink(const char *path) {
	return shared_object_unlink(path, &mq_class);// уменьшает число ссылок и убивает, если обладает правом
}
#endif
#endif

