/*!
	у асинхронной очереди всегда один получатель, несколько отправителей.
 */

#include <cmsis_os.h>
#include "atomic.h"
#include "queue.h"
#include "r3stdlib.h"
typedef struct _Message Message_t;
struct _Message {
	List_t lst;
	uint32_t info;
};
struct os_messageQ_cb {
	Queue queue;
	osPoolId pool;
	osThreadId owner;
//	int refcount;// счетчик ссылок на объект
};
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id)
{
    osMessageQId mq = r3_mem_alloc(sizeof(struct os_messageQ_cb));
    queue_init(&mq->queue);
    mq->pool = osPoolCreate((const osPoolDef_t *)queue_def);
    mq->owner = thread_id;
    return mq;
}
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
    Message_t *msg;
    while ((msg = osPoolAlloc(queue_id->pool))==NULL) {
        osEvent event = {.status = (osEventSemaphore|osEventTimeout), .value={.p=&queue_id->pool}};
        osEventWait(&event, millisec);
        if (event.status == osEventTimeout) return osErrorTimeoutResource;
    }
	msg->info = info;
	queue_push(&queue_id->queue, &msg->lst);
	return osOK;
}
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec)
{
    osEvent event;
	if (queue_id->queue.head==NULL && queue_id->queue.tail==NULL) {
        event.status = (osEventMessage|osEventTimeout), event.value.p=&queue_id->queue.tail;
        osEventWait(&event, millisec);
        if (event.status == osEventTimeout) return event;
	}
    Message_t *msg = (void*) queue_pop(&queue_id->queue);
    event.status = osEventMessage;
    event.value.v = msg->info;
    osPoolFree(queue_id->pool, msg);
	return event;
}
