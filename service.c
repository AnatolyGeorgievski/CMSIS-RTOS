/*! \defgroup _services Прикладные сервисы

	\{
 */
#include "atomic.h"
#include <time.h>// clock
#include <sys/thread.h>
#include "r3_slice.h"
#include <stdlib.h>
//#include <stdio.h>
//#include "cmsis_os.h"

typedef struct _Event osEvent_t;
typedef struct _Process osProcess_t;
//typedef struct _Service osService_t;

typedef int (*osServiceFunc)(void*,void * arg);
typedef struct _list List_t;
struct _list {
	struct _list * next;
	void* data;
};
static List_t* services=NULL;

/*! \brief создать сервис

	функция может использовать статическое выделение памяти для сервиса

	\param svc - структура данных для заполнения или NULL если сервис создается динамически
*/
osProcess_t* osServiceCreate(osProcess_t *process, int(*func)(osProcess_t *process, void *), void* arg)
{
	if(process==NULL) process = malloc(sizeof(osProcess_t));
	process->signals = 0;
	process->func = (void*)func;// сервис
	process->arg  = arg;
	process->event.status = (osEventSignal);
	process->event.value.signals = ~0;// Any, любые сигналы ждем
	process->wait.timeout 	= 0;//osKernelSysTickMicroSec(RequestTimeout);
	process->wait.timestamp = clock();
//	svc->next = NULL;
	return process;
}
//static osService_t* current_service=NULL;
osEvent_t* osServiceGetEvent(osProcess_t *process)
{
	return &process->event;
}
uint32_t osServiceGetSignals(osProcess_t *process)
{
//	osService_t* svc = current_service;
	return process->signals;
}
/*
osService_t* osServiceGetId()
{
	return current_service;
}*/

/*! */
/*
int32_t* osServiceSignalRef(osService_t *svc, uint32_t signals)
{
	int32_t* ref = BB_ALIAS(&svc->process.signals, __builtin_ctz(signals));
	return ref;
}*/
uint32_t osServiceSignal(osProcess_t *process, uint32_t signals)
{
	return atomic_fetch_or(&process->signals, signals);
}
/*! \brief перемещает сервис ближе к раздаче */
void osServiceNotify(osProcess_t *svc)
{

}
/*! \brief активизировать сервис
	помещает сервис в список в очередь ожидания события
	\todo перейти на C11
*/
int osServiceRun   (osProcess_t *process)
{
//	svc->process.event.status |= (osEventRunning);
	List_t *entry = g_slice_alloc(sizeof(List_t));
	entry->data = process;
	volatile void** ptr = (volatile void**)&services;
	do {
		entry->next = atomic_pointer_get(ptr);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, entry->next, entry));
	return 0;
}
/*
static inline int osProcessTimerExpired(struct _Process *process)
{
	uint32_t system_timestamp = osKernelSysTick();// сделать функцию clock() для переносимости
	return ((uint32_t)(system_timestamp - process->wait.timestamp)>=process->wait.timeout);
}*/
int osServiceTimerExpired(osProcess_t* process, uint32_t timeout)
{
//	osService_t* svc = current_service;
	uint32_t system_timestamp = clock();
	return ((uint32_t)(system_timestamp - process->wait.timestamp)>=timeout);
}


void osServiceTimerStop  (osProcess_t *process)
{
	process->event.status &= ~(osEventTimeout);
}
void osServiceTimerStart (osProcess_t* process, uint32_t timeout)
{
//	osService_t* svc = current_service;
	process->wait.timeout 	= (timeout);
	process->wait.timestamp = clock();
	process->event.status  |= (osEventTimeout);
}
void osServiceTimerRestart (osProcess_t* process)
{
//	osService_t* svc = current_service;
	process->wait.timestamp += process->wait.timeout;//= osKernelSysTick();
	process->event.status   |= (osEventTimeout);
}

void osServiceTimerCreate (osProcess_t *process, uint32_t timeout)
{
	process->wait.timeout 	= (timeout);
	process->wait.timestamp = clock();
}


/*! функция сканирования сервисов

	по сути это такой же планировщик, только работает с другими объектами
 */
void osServiceWorkFlow()
{
	List_t * list = services;
	while (list) {
		osProcess_t * process = (osProcess_t *)list->data;
		// планировщик - функция обработки событий процесса
		osEvent_t *event = &process->event;
		if (event->status == osEventComplete)   {// ничего не ждем
			// event->status = osEventRunning;
		} else
		if (event->status & (osEventSignal)) 	{// ожидаем сигналы
			if (1) {//any
				if(process->signals!=0) {// Any
					event->value.signals = atomic_exchange(&process->signals, 0);
					event->status = osEventSignal|osEventRunning;
				}
			}
			else
			if ((process->signals & event->value.signals)==event->value.signals){// All
				event->value.signals &= atomic_fetch_and(&process->signals, ~event->value.signals);
				event->status = osEventSignal|osEventRunning;
			}
		}
		if (event->status & (osEventTimeout)) 	{// ожидаем таймаут
			if (osServiceTimerExpired(process, process->wait.timeout)) {
				//printf("ServiceTimeout %p\r\n", service);
				event->status = osEventTimeout|osEventRunning;
			}
		}
		if (event->status & osEventRunning){
			//current_service = service; // это нужно только чтобы изнутри находить свой идентификатор
			process->result = (void*)(intptr_t)((osServiceFunc)process->func)(process, process->arg);
			event->value.signals = 0;// очищаем
			if (process->result<0) {
                 event->status = osEventComplete;
                //*service_prev = service->next;/// выкинуть процесс из очереди
			} else
			{
                event->status = (event->status & (~osEventRunning))|osEventSignal;
			}
		}
		list = list->next;
	}
}

// \}

