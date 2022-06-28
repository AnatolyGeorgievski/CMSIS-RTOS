/*!
	у асинхронной очереди всегда один получатель, несколько отправителей.
 */

#include <cmsis_os.h>
#include "atomic.h"
#include "queue.h"
#include "r3stdlib.h"
typedef struct _Mail Mail_t;
struct _Mail {
	List_t lst;
	unsigned int data[0];
};
struct os_mailQ_cb {
	Queue queue;
	osPoolId pool;
	osThreadId owner;
//	int refcount;// счетчик ссылок на объект
};
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id)
{
    osMailQId mq = r3_mem_alloc(sizeof(struct os_mailQ_cb));
    queue_init(&mq->queue);
    mq->pool = osPoolCreate((const osPoolDef_t *)queue_def);
    mq->owner = thread_id;
    return mq;
}
void *osMailAlloc (osMailQId queue_id, uint32_t millisec)
{
    Mail_t *mail;
    while ((mail = osPoolAlloc(queue_id->pool))==NULL) {
        osEvent event = {.status = (osEventSemaphore|osEventTimeout), .value={.p=&queue_id->pool}};
        osEventWait(&event, millisec);
        if (event.status == osEventTimeout) return NULL;
    }
    return mail->data;
}
osStatus osMailPut (osMailQId queue_id, void *mail)
{
	queue_push(&queue_id->queue, &((Mail_t*)(mail - __builtin_offsetof(Mail_t, data)))->lst);
	return osOK;
}

osEvent osMailGet (osMailQId queue_id, uint32_t millisec)
{
    osEvent event;
	if (queue_id->queue.head==NULL && queue_id->queue.tail==NULL) {
        event.status = (osEventMail|osEventTimeout), event.value.p=&queue_id->queue.tail;
        osEventWait(&event, millisec);
        if (event.status == osEventTimeout) return event;
	}
    Mail_t *mail = (void*) queue_pop(&queue_id->queue);
    event.status = osEventMail;
    event.value.p = mail!=NULL?mail->data:NULL;
	return event;
}
osStatus osMailFree (osMailQId queue_id, void *mail)
{
    return osPoolFree(queue_id->pool, (Mail_t*)(mail - __builtin_offsetof(Mail_t, data)));
}

