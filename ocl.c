/*! \defgroup _ocl OpenCL 2.2 

*/
#include <stdint.h>
#include "atomic.h"
#include "semaphore.h"
#include "queue.h"
#include "cmsis_os.h"



typedef struct _MemPool MemPool_t;
struct _MemPool {
	uint32_t *base;
	uint32_t *flags;// флаги занятости 
	volatile int count; // semaphore
	const uint32_t size;// число элементов
	const uint8_t bits;// число бит при сдвиге, 2^n - размер в байтах
	
};
#define osMemPoolDef(name, type, num, bitn)  \
static type name##_pool_map[num]; \
static uint32_t name##_pool_flags[(num+31)>>5]={0}; \
static MemPool_t name##_pool = { \
	.base = (void*)name##_pool_map, .count = num, .size = num, .bits =bitn, \
	.flags = name##_pool_flags};


	
	
static uint32_t cl_event_flags[4];// 128 флагов, потому что отрицательные числа не флаги
static uint32_t cl_event_flags_ena[4];// 128 флагов, флаг выделения
static int cl_event_flag_count=128; // семафор для посчета свободных флагов

/*! \brief выделение флага из битовой строки 

	При выделении свободный бит атомарно выставляется в 1.
	Коллизия возникает если этот бит уже выставлен в ноль.
	
	\param str битовая строка, флаг 1 значит бит занят. 
	\return номер бита в битовой строке
*/
static uint32_t osEventFlagAlloc (uint32_t* str, int length)
{
	uint32_t i;
	do {
		for (i=0;i<length; i++) {
			uint32_t map  = atomic_load(&str[i]);
			while (~map) {// если заполнена полностью (все единицы) пропускаем
				uint32_t flag_id = __builtin_ctz(~map);// индекс флага
				uint32_t mask = (1UL<<(flag_id & 0x1F));
				map = atomic_fetch_or(&str[i], mask);
				if ((map & mask) == 0) {// была ли коллизия, бит может быть уже устанвлен
					return (flag_id + (i<<5));
				}
			}
		}
	} while(1);// может быть коллизия, тогда заново
	return ~0UL;
}
/*! \brief освобождает бит в строке
	\return если была коллизия (бит уже был чист) возвращает FALSE
*/
static int osEventFlagClear (uint32_t* str, uint32_t flag_id)
{
	uint32_t mask = (1UL<<(flag_id & 0x1F)); // очистить бит в маске
	uint32_t map = atomic_fetch_and(&str[flag_id>>5], ~mask); // бит атомарно чистим
	return ((map & mask) != 0); 
}
/*! \brief атомарно устанавливает бит в строке, если была коллизия возвращает FALSE */
static int osEventFlagSet (uint32_t* str, uint8_t flag_id)
{
	uint32_t mask = (1UL<<(flag_id & 0x1F)); // очистить бит в маске
	uint32_t map = atomic_fetch_or(&str[flag_id>>5], mask); // бит атомарно устанавливаем
	return ((map & mask) == 0); 
}


static void* osMemPoolAlloc(MemPool_t *pool, uint32_t timeout)
{
	uint32_t count = semaphore_enter(&pool->count);
	if (count==0) {
		osEvent evt = {.status=osEventSemaphore, .value={.p = (void*)&pool->count}};
		osEventWait(&evt, osWaitForever);
	}
	uint32_t idx = osEventFlagAlloc(pool->flags, pool->size);
	return pool->base+(idx<<pool->bits);
}
static void osMemPoolFree(MemPool_t *pool , void* block)
{
	uint32_t idx = (block - (void*)pool->base) >> pool->bits;//вычисляем индекс, номер флага
	(void) osEventFlagClear(pool->flags, idx);
	semaphore_leave(&pool->count);
}


typedef struct _CommandQueue CommandQueue_t;
struct _CommandQueue {
	Queue queue; // очередь команд
	volatile uint32_t thread_slots;// готовность треда
	void* slots[4];// общение происходит через слоты.
	MemPool_t *pool;
};



typedef int32_t cl_int;
typedef uint32_t cl_uint;
typedef uint8_t cl_event;
typedef CommandQueue_t* cl_command_queue;
/*! \brief коды состояния и коды флагов должны быть сопоставлены с кодами CMSIS RTOS */
enum {
	CL_SUCCESS=0,
	CL_SUBMITTED=0,
	CL_COMPLETE=1,
	CL_INVALID_VALUE=-1,
};
enum {// перенести в настройки ОС
	osEventWaitList   =3,
	osEventWaitListAny=3,
	osEventWaitListAll=2,
};

typedef struct _Command Command_t;
struct _Command {// по сути это таймер
	osEvent event;
	cl_event exec_event_id;
#if 1
	uint32_t timestamp;
	uint32_t interval;
#endif
	void* (*callback)(void*);
	void* args;
	Command_t * next;
};
typedef struct _CmdList CmdList_t;
struct _CmdList {
	List_t list;
	Command_t* cmd;
};

#if 0
/*! \brief поставить задачу в лист исполнения желаний */
void  osThreadEnqueue(volatile Command_t **queue_tail,  Command_t *cmd) 
{
	cmd->event.status = osEventQueued;
	Command_t *last_cmd;
	do {// добавление команды в очередь
		last_cmd = atomic_pointer_get(queue_tail);
		cmd->next = last_cmd;// скорее всего это NULL
		atomic_mb();// чтобы запись применилась до следующей команды
	} while (!atomic_pointer_compare_and_exchange(queue_tail, last_cmd, cmd));
	//atomic_mb();
}
#endif

/*! \brief дождаться завершения очереди команд, все команды загружены в слоты

issues all previously queued OpenCL commands in command_queue_to the device associated with_command_queue.
clFlush only guarantees that all queued commands to command_queue will eventually be submitted to the appropriate
device. There is no guarantee that they will be complete after clFlush returns.
	\returns CL_SUCCESS if the function call was executed successfully. Otherwise, it returns one of the following errors:
• CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
• CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
• CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.
*/
cl_int clFlush(cl_command_queue command_queue)
{
	return CL_SUCCESS;
}
/*! \brief дождаться завешнения всех команд, загруженных в слоты

blocks until all previously queued OpenCL commands in command_queue are issued to the associated device and have
completed. clFinish does not return until all previously queued commands in command_queue have been processed and
completed. clFinish is also a synchronization point.
	\returns CL_SUCCESS if the function call was executed successfully. Otherwise, it returns one of the following errors:
• CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
• CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
• CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.

*/
cl_int clFinish(cl_command_queue command_queue)
{
	return CL_SUCCESS;
}
/*! \brief обработка событий по мотивам OpenCL 

*/
/*! \brief запуск команд из пользовательского треда 
	\fn cl_int clEnqueueNativeKernel(cl_command_queue command_queue,
					void (CL_CALLBACK *user_func)(void *),
					void *args,
					size_t cb_args,
					cl_uint num_mem_objects,
					const cl_mem *mem_list,
					const void **args_mem_loc,
					cl_uint num_events_in_wait_list,
					const cl_event *event_wait_list,
					cl_event *event)
enqueues a command to execute a native C/C++ function not compiled using the OpenCL compiler.

*/
cl_int clEnqueueNativeKernel(cl_command_queue command_queue, Command_t* cmd,
			cl_uint wait_list_length, const cl_event *wait_list,  cl_event* exec_event_id)
{
	//используем семафор для ожидания и ограничения числа флагов
	int32_t count = semaphore_enter(&cl_event_flag_count);
	if (count==0) {// ждем пока будет возможность писать 
		osEvent evt = {.status = osEventSemaphore, .value={.p=&cl_event_flag_count}};
		osEventWait(&evt, osWaitForever);
	}
	uint8_t event_id = osEventFlagAlloc(cl_event_flags_ena, 1);// ждем выделения
	osEventFlagClear(cl_event_flags, event_id);// начальное состояние флага, не выставлен
	
	cmd->exec_event_id = event_id;
	cmd->event.value.p = (void*)wait_list;
	cmd->event.status |= /* osEventWaitList | */(wait_list_length<<16);// таймер и флаг Any могут быть предустановлены
	if (exec_event_id) *exec_event_id = event_id;
	// записать команду в очередь
	//osThreadEnqueue (&command_queue->tail, cmd);
	CmdList_t * list = osMemPoolAlloc(command_queue->pool, osWaitForever);
	list->cmd = cmd;
	queue_push(&command_queue->queue, &list->list); 
	return CL_SUCCESS;
}
/*! \fn cl_event clCreateUserEvent(cl_context context, cl_int *errcode_ret)
	\brief 
	\return returns a valid non-zero event object and errcode_ret_is set to CL_SUCCESS if the user event object is created successfully. Otherwise, it returns a NULL value with one of the following error values returned in _errcode_ret:
	-# CL_INVALID_CONTEXT if _context_is not a valid context.
	-# CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
	-# CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.

	The execution status of the user event object created is set to CL_SUBMITTED.

*/
cl_event clCreateUserEvent(CommandQueue_t* command_queue, cl_int *errcode_ret)
{
	//используем семафор для ожидания и ограничения числа флагов
	int32_t count = semaphore_enter(&cl_event_flag_count);
	if (count==0) {// ждем пока будет возможность писать 
		osEvent evt = {.status = osEventSemaphore, .value={.p=&cl_event_flag_count}};
		osEventWait(&evt, osWaitForever);
	}
	uint8_t event_id = osEventFlagAlloc(cl_event_flags_ena, 1);
	osEventFlagClear(cl_event_flags, event_id);
	
	if(errcode_ret) *errcode_ret = CL_SUCCESS;
	return event_id;
}
/*!	\fn cl_int clSetUserEventStatus(cl_event event, cl_int execution_status)
	\brief sets the execution status of a user event object 

	\param event is a user event object created using \ref clCreateUserEvent.
	\param execution_status specifies the new execution status to be set and can be \b CL_COMPLETE or a negative integer value to indicate an error. A negative integer value causes all enqueued commands that wait on this user event to be terminated.
	\b clSetUserEventStatus can only be called once to change the execution status of event.
	\return CL_SUCCESS if the function was executed successfully. Otherwise, it returns one of the following errors:
	• CL_INVALID_EVENT if event is not a valid user event object.
	• CL_INVALID_VALUE if the execution_status is not \b CL_COMPLETE or a negative integer value.
	• CL_INVALID_OPERATION if the execution_status for event has already been changed by a previous call to clSetUserEventStatus.
	• CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
	• CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.

	\note If there are enqueued commands with user events in the event_wait_list argument of clEnqueue commands, the user must ensure that the status of these user events being waited on are set using clSetUserEventStatus before any OpenCL
	APIs that release OpenCL objects except for event objects are called; otherwise the behavior is undefined.

*/
cl_int clSetUserEventStatus(cl_event event, cl_int execution_status)
{
	if (execution_status == CL_COMPLETE)
		return osEventFlagSet(cl_event_flags, event);
	else
		return CL_INVALID_VALUE;
}
cl_int clGetUserEventStatus(cl_event event)
{
	return osEventFlagGet(cl_event_flags, event)?CL_COMPLETE: CL_SUBMITTED;
}

/*! \brief waits on the host thread for commands identified by event objects in event_list to complete. 
	
	A command is considered complete if its execution status is \b CL_COMPLETE or a negative value. The events specified in event_list act as synchronization points.

	\return \b CL_SUCCESS if the execution status of all events in 'event_list' is \b CL_COMPLETE. Otherwise,
	it returns one of the following errors:
	-# • CL_INVALID_VALUE if num_events is zero or 'event_list' is NULL.
	-# • CL_INVALID_CONTEXT if events specified in 'event_list' do not belong to the same context.
	-# • CL_INVALID_EVENT if event objects specified in 'event_list' are not valid event objects.
	-# • CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the execution status of any of the events in event_list 
	is a negative integer value.
	-# • CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
	-# • CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.
*/
cl_int clWaitForEvents(cl_uint num_events, const cl_event *event_list)
{
#if 0
	cl_uint i;
	for (i=0; i< num_events; i++){
		cl_event event = event_list[i];
		if (!osEventFlagTest(cl_event_flags, event)) {// если нет такого флага, ждем когда появится
			osEvent evt={status=osEventFlags| ((event&0x1F)<<16), value={.p=&cl_event_flags[event>>5]}};
			osEventWait(&evt, osWaitForever);
		}
	}
#endif
	return CL_SUCCESS;
}
/*! \fn cl_int clReleaseEvent(cl_event event)
	\brief decrements the event reference count.
	\returns CL_SUCCESS if the function is executed successfully. Otherwise, it returns one of the following errors:
• CL_INVALID_EVENT if event is not a valid event object.
• CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by the OpenCL implementation on the device.
• CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources required by the OpenCL implementation on the host.

The event object is deleted once the reference count becomes zero, the specific command identified by this event has
completed (or terminated) and there are no commands in the command-queues of a context that require a wait for this
event to complete. Using this function to release a reference that was not obtained by creating the object or by calling
clRetainEvent causes undefined behavior.

	\note Developers should be careful when releasing their last reference count on events created by clCreateUserEvent that
have not yet been set to status of CL_COMPLETE or an error. If the user event was used in the event_wait_list argument
passed to a clEnqueue API or another application host thread is waiting for it in clWaitForEvents, those commands and
host threads will continue to wait for the event status to reach CL_COMPLETE or error, even after the application has
released the object. Since in this scenario the application has released its last reference count to the user event, it would
be in principle no longer valid for the application to change the status of the event to unblock all the other machinery. As a
result the waiting tasks will wait forever, and associated events, cl_mem objects, command queues and contexts are likely
to leak. In-order command queues caught up in this deadlock may cease to do any work.

*/
cl_int clReleaseEvent(cl_event event)
{
	osEventFlagClear(cl_event_flags_ena, event);
	(void) atomic_fetch_add(&cl_event_flag_count, 1);// освободили ресурс, увеличили счетчик свободных флагов
	return CL_SUCCESS;
}
/*! \brief проверка источника события 
	похожий разбор производится в планировщике
*/
static uint32_t clTestEvents(osEvent * event)
{
	if (event == NULL) return osEventComplete;
	uint32_t event_type = event->status;
	if (event_type & osEventTimeout) {
		
		return osEventTimeout;
	}
	uint8_t wait_list_length = event_type>>16;
	uint8_t wait_list_first  = event_type>>24;
	uint8_t i;
	cl_event *wait_list = event->value.p;
	if (event_type & osEventWaitListAny) {
		for (i=wait_list_first; i<wait_list_length; i++) {
			cl_event event_id = wait_list[i];
			uint32_t mask = (1UL<<(event_id&0x1F));
			if ((cl_event_flags[event_id>>5] & mask)!=0) { // флаг выставлен выход
				return osEventComplete;
			}
		}
		return osEventWaitListAny;
	} 
	else {// osEventWaitListAll
		for (i=wait_list_first; i<wait_list_length; i++) {
			cl_event event_id = wait_list[i];
			uint32_t mask = (1UL<<(event_id&0x1F));
			if ((cl_event_flags[event_id>>5] & mask)==0) { // флаг не выставлен
				return osEventWaitListAll | (i<<24);
			}
		}
		return osEventComplete;
	}
}
void cl_command_thread (void *slot) {
	osEvent evt = {.status=osEventAsyncQueue};
	while (1){// анализировать указатель
		evt.value.p = slot;
		osEventWait(&evt, osWaitForever);
		if (evt.status!=osEventAsyncQueue) break;
		Command_t* cmd = evt.value.p;
		// может стек восстановить?
			// если есть условие ожидания, то задачу следует отложить в иную очередь
			// не ждем начальный статус может быть ожидание, таймаут
			if (cmd->callback) // 
				res = cmd->callback(cmd->args);
			osEventFlagSet(cl_event_flags, cmd->exec_event_id);
			//osEventFlagSet(cl_event_flags, cmd->exec_event_id); освободить слот
			// что делаем со списком задач?
	}
}
#include "segment.h"
#define COMMAND_LIST(mod_name, cmd_init, cmd_list) CommandSpec_t mod_name  SEGMENT_ATTR(CmdList) \
    = { /*.name = #mod_name, */\
        .init = cmd_init, \
        .cmdlist = cmd_list }
typedef struct _CommandSpec CommandSpec_t;
struct _CommandSpec {
	void (*init)(void* database);// фнкция настройки взаимодействует с конфигурацией устройсва и создает аргумент для команд
	Command_t* cmdlist; // список команд
};
// маркеры начала и конца сегмента
extern CommandSpec_t __start_CmdList[];
extern CommandSpec_t __stop_CmdList[];


osMemPoolDef(cmd_queue, CmdList_t, 32, 3);// 32 элемента по 2^3 байт

static CommandQueue_t command_queue;
CommandQueue_t* cl_command_queue_init(void* database)
{
	
	// инициализировать MemPool
	command_queue.pool = &cmd_queue_pool;
	command_queue.thread_slots = 0;
	command_queue.slots[0] = NULL;
	command_queue.slots[1] = NULL;
	command_queue.slots[2] = NULL;
	command_queue.slots[3] = NULL;
	// инициализировать AsyncQueue
	
	Queue * queue = &command_queue.queue;
	queue_init(queue);
	//void _init1()
	{
		CommandSpec_t * cmdspec = __start_CmdList;
		while (cmdspec != __stop_CmdList)
		{
			if (cmdspec->init != NULL) // бывают
			{   
				(void) cmdspec->init(/* 'void *' или 'BACnetObject' */database);// кто делает аргумент для команды?
			}
			// поставить на конвейер, 
			CmdList_t * list = osMemPoolAlloc(command_queue.pool, osWaitForever);
			list->cmd = cmdspec->cmdlist;
			queue_push(queue, &list->list);
			cmdspec++;
		}
	}

	// запустить слоты для тредов
	
	return &command_queue;
}
/*! \brief обработка очереди команд запускается периодически по таймеру или в фоновом процессе */
void cl_command_queue_process (CommandQueue_t* command_queue)
{
	Queue * queue = &command_queue->queue;
	while (command_queue->thread_slots != 0) 
	{//есть свободный слот для запуска команд, слоты очищает планировщик
		List_t * list = queue_pop(queue);
		Command_t* cmd = ((CmdList_t*)list)->cmd;
		if (cmd==NULL) break;
		while (cmd) {// по списку команд в очереди
		// два типа событий - таймаут или готовность списка ожидания
			uint32_t event_status = clTestEvents(&cmd->event);
			if(event_status == osEventComplete)
			{// дошли до конца списка или таймер сработал
				// запустить команду или ожидать свободный тайм слот
				//osMemPoolFree(command_queue->pool, list);// освободить элемент очереди
				if (cmd->callback==NULL) {// барьер для синхронизации (маркер)
					osEventFlagSet(cl_event_flags, cmd->exec_event_id);// выставляет флаг 
					cmd = cmd->next;
					continue;// продолжить планирование
				}
				int slot_id = __builtin_ctz(command_queue->thread_slots);// номер флага выставленного 
				atomic_fetch_and(&command_queue->thread_slots, ~(1UL<<slot_id));// заняли слот
				command_queue->slots[slot_id] = cmd;// передать слоту команду, это вызывает автоматический запуск
				// если высокий приоритет, то можно торкнуть переключение команд, чтобы следующим в очереди был он
			} 
			else {// не готов
#if 0 
				if ((event_status & (osEventWaitListAll|osEventWaitListAny))== osEventWaitListAll) {
					cmd->event.status = (event_status & 0xFF000000UL) | (cmd->event.status & 0x00FFFFFFUL);
				}
#endif
				((CmdList_t*)list)->cmd = cmd;
				queue_push(queue, list); // вернуть команду в конец очереди
			}
			cmd = cmd->next;
		}
	}// нет слотов для выполнения команд
}

