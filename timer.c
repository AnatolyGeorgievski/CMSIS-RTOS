/*! \ingroup _system
    \defgroup _timer Timer Management Functions
\todo добавить удаление таймера

    \{
 */
#include "board.h"
#include <cmsis_os.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic.h>

typedef struct os_timer_cb Timer_t;
struct os_timer_cb {
	Timer_t * next;
	uint32_t timestamp;
	uint32_t interval;
#if 0
	uint32_t count; // временно!
	int32_t jitter; // временно!
#endif
	void (*cb)(void * argument);
	void* data;
	int type;
};

static volatile Timer_t *timer_next=NULL;
static volatile Timer_t *timer_first=NULL; // первый таймер в очереди

/*! \brief создать таймерный объект
*/
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
	Timer_t *timer = malloc(sizeof(Timer_t));
	timer->timestamp = 0;
	timer->interval = 0;
#if 0
	timer->count=0; // счетчик срабатываний
	timer->jitter=0; // счетчик срабатываний
#endif
	timer->type = type;
	timer->cb = timer_def->ptimer;
	timer->data = argument;
	timer->next = NULL;

	return timer;
}
/*!	для обеспечения атомарного доступа к очереди можно сделать запрос через SVC
 */
osStatus osTimerStartMicroSec (osTimerId timer_id, uint32_t microsec, uint32_t delay)//uint32_t microsec, uint32_t delay)
{
	Timer_t * timer = timer_id;
	timer->interval = osKernelSysTickMicroSec(microsec);//osKernelSysTickMicroSec(microsec);
	timer->timestamp = osKernelSysTick();// - osKernelSysTickMicroSec(delay);
	volatile void** ptr = (volatile void**)&timer_first;
	Timer_t *next;
	do {
		next = atomic_pointer_get(ptr);
		timer->next = (next==NULL)? timer: next;// на себя циклим
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, next, timer));
	// добавить в очередь атомарно, в начало
	return osOK;
}
/* остановка таймера может быть запрошена из пользовательского треда, не может быть запрошена из обработчика прерывания
Status and Error Codes

    osOK: the specified timer has been stopped.
    osErrorISR: osTimerStop cannot be called from interrupt service routines.
    osErrorParameter: timer_id is incorrect.
    osErrorResource: the timer is not started.

 */
osStatus osTimerStop (osTimerId timer_id)
{
	// выкинуть из очереди
	Timer_t * timer = (Timer_t *) timer_first;
	while (timer){
		if (timer == timer_id) {
//			if (timer_prev) timer_prev->next = timer->next;
//			else timer_first = timer->next;
			timer->type = -1; // выключили
			return osOK;
		}
//		timer_prev = timer;
		timer = timer->next;
	}
	return osErrorResource;
}
osStatus osTimerDelete (osTimerId timer_id)
{
	return osErrorResource;
}
/*!	обработка очереди таймеров
	выполняется по системному таймеру SysTick
 */
void osTimerWork(uint32_t timestamp)
{
	Timer_t * timer = (Timer_t *) timer_next;
	if(timer!=NULL) {
		//uint32_t timestamp = osKernelSysTick();
		if ((uint32_t)(timestamp - timer->timestamp) >= timer->interval)
		{
			if (timer->type >= 0) {
				timer->cb(timer->data); // выполнить функцию таймера
				timer->timestamp+=timer->interval; // передвинуть по очереди дальше
#if 0
				if (timer->type == osTimerPeriodic) { // перезарядить в случае циклического таймера
#if 0
				uint32_t jitter = (timestamp - timer->timestamp) - timer->interval;
					if (jitter> timer->jitter) timer->jitter=jitter;
					timer->count++;
#endif
				} 
				else
#endif
				if (timer->type == osTimerOnce) { // выкинуть таймер из очереди
					timer->type = -1; // выключили
				} 
			}
		}
		timer_next = timer->next; // перейти к обработке следующего таймера
	}
	else
		timer_next = timer_first;
}
/*! \} */
#if 0
void osTimerDebug()
{
	Timer_t * timer = (Timer_t *)timer_first;
	int idx = 0;
	while (timer){
		printf("Timer %d: counts=%d jitter=%d\n", idx, timer->count, timer->jitter);
		idx++;
		timer = timer->next;
		if (timer==timer_first) break;
	}
}
#endif
