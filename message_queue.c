/*! \brief Message Queue */
#include <cmsis_os2.h>
#include "semaphore.h"
#include <stdlib.h>
#include <string.h>
typedef struct _osMessageQueue osMessageQueue_t;
typedef struct _PriorityList PriorityList_t;
struct _PriorityList {
	PriorityList_t* next;
	uint8_t prio;
	uint8_t data[0];
};
struct _osMessageQueue {
    volatile int count;// счетчик семафора
	osMemoryPoolId_t pool_id;
	uint32_t msg_size;
    osPriorityList_t *tail;
    osPriorityList_t *head;
//    uint16_t block_size;
};
/*
	\param [in]	msg_count	maximum number of messages in queue.
	\param [in]	msg_size	maximum message size in bytes.
	\param [in]	attr	message queue attributes; NULL: default values.

 */
osMessageQueueId_t osMessageQueueNew 	( 	uint32_t  	msg_count,
		uint32_t  	msg_size,
		const osMessageQueueAttr_t *  	attr
	)
{
    osMessageQueue_t * queue = malloc(sizeof(osMessageQueue_t));
	queue->pool_id = osMemoryPoolNew(msg_count, msg_size + sizeof(osPriorityList_t),
                                    (const osMemoryPoolAttr_t*)attr);
	semaphore_init(&queue->count, msg_size);
	queue->head=NULL;
	queue->tail=NULL;
	queue->msg_size = msg_size;
	return queue;
}
osStatus_t osMessageQueueGet 	( 	osMessageQueueId_t  	mq_id,
		void *  	msg_ptr,
		uint8_t *  	msg_prio,
		uint32_t  	timeout
	)
{
    osMessageQueue_t* queue = mq_id;
	int count = semaphore_enter(&queue->count);// ожидаем сообщение
	if(count==0) {
		osEvent event = {.status = osEventSemaphore, .value={.p = (void*)&queue->count}};
		osEventWait(&event, timeout);
		if (event.status == osEventTimeout){
			return osErrorTimeout;
		}
	}
	{
		PriorityList_t* msg = (osPriorityList_t*)atomic_pointer_exchange(&queue->head, NULL);// снимаем шляпу
		// если используются приоритеты, следует использовать сортировку
		while(msg){
			PriorityList_t *msg_next = msg->next;
            PriorityList_t *list;
			PriorityList_t **next = &queue->head;
			while ((list = *next)!=NULL && list->prio >= msg->prio) {
				next = &list->next;
			}
			msg->next = *next;// вставка после указанного
			*next = msg;
			msg = msg_next;
		}

		msg = queue->head;
		queue->head = msg->next;
		memcpy(msg_ptr, msg->data, queue->msg_size);
		*msg_prio = msg->prio;
		osMemoryPoolFree(queue->pool_id, msg);
	}
	return osOK;
}
osStatus_t osMessageQueuePut 	( 	osMessageQueueId_t  	mq_id,
		const void *  	msg_ptr,
		uint8_t  	msg_prio,
		uint32_t  	timeout
	)
{
	osStatus_t res;
	osPriorityList_t* msg;
	osMessageQueue_t* queue = mq_id;
	msg = osMemoryPoolAlloc(queue->pool_id, timeout);
	if(msg != NULL) {
		memcpy(msg->data, msg_ptr, queue->msg_size);// копирование можно было исключить
		msg->prio = msg_prio;
		// атомарно подменяем вершину стека
		volatile void**ptr = (volatile void**)&queue->tail;
		do {
			msg->next = atomic_pointer_get(ptr);
			atomic_mb();
		} while (!atomic_pointer_compare_and_exchange(ptr, msg->next, msg));
		semaphore_leave(&queue->count);// увеличиваем число сообщений в очереди
		res = osOK;
	} else
		res = osErrorTimeout;
	return res;
}