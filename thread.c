/*!
    \defgroup _thrd С11 Thread management
	\ingroup _libc
	\brief Управление потоками
 */
/*!	\defgroup _thread Thread management
	\ingroup _system


	\see [AN298] Cortex-M4(F) Lazy Stacking and Context Switching
	\see [ARM DUI 0553A] Cortex™-M4 Devices Generic User Guide
	\see http://www.keil.com/pack/doc/CMSIS/General/html/index.html
	\see http://www.keil.com/pack/doc/cmsis_rtx/index.html
	\see http://www.tnkernel.com
	\see FreeRTOS Real Time Kernel (RTOS)
	\see http://developer.mbed.org/handbook/CMSIS-RTOS
	\see The Definitive Guide to ARM® Cortex®-M3 and Cortex®-M4 Processors
	\see https://github.com/ARM-software/CMSIS_5
	\see STM32 Cortex ®-M33 MCUs programming manual (PM0264)
\{
	Идеи:
	Идентификатор треда osThreadId == адрес TCB.
	Очередь задач мб дерево, мб кольцо, мб список односвязанный или двусвязный

	\todo надо реализовать приоритеты

	Есть возможность использовать два указателя стека PSP и MSP а также управлять. В нашей реализации PSP не используется, потому что реализация на одном стеке оказалась компактнее.

	Запуск первого треда и
	SCB->CCR |= SCB_CCR_STKALIGN_Msk; // enable double word stack alignment

	переход в main с MSP на PSP :
	__set_PSP(tcb[0]->sp);
	__set_CONTROL(0x3) no_fp, switch to psp, unprivileged
	__ISB();
	//task0();
	//SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;// запуск PendSV

	Запись элемента в очередь должна осуществляться атомарно. нужен примитив atomic_unlink(); и atomic_insert
	volatile void* ptr = tcb_list->next;
	do {
		tcb->next = __LDREX(tcb_list->next);
	} while (__STREX(tcb, tcb_list->next)==0);
	
	11.03.2020 Надо бы добавить конкуренцию тредам. Есть идея такая - задержка на переключение не должна превышать установленный таймаут. Каждая задача должна выполняться согласно приоритету. см. определение cooperative multitasking и preemptive multitasking
	
	09.05.2021 Надо бы очищать бит FPU->FPCCR &= ~(FPU_FPCCR_LSPACT_Msk); // активность FPU для вновь запускаемого треда.
	\see ARMv8-M Exception and interrupt handling Version 2.0
		For implementations without the Security Extension;
		EXC_RETURN Condition
		0xFFFFFFB0 Return to Handler mode.
		0xFFFFFFB8 Return to Thread mode using the main stack.
		0xFFFFFFBC Return to Thread mode using the process stack.	
		
Where synchronization is required between these separate cores, it can be provided through
message passing communication protocols, for example, the Multicore Communications
Association API (MCAPI). These can be implemented by using shared memory to pass data
packets and by the use of software-triggered interrupts to implement a so-called door-bell
mechanism.

The Multicore Communications API (MCAPI) is the first specification to be produced by the Multicore Association. MCAPI provides a standardized API for communication and synchronization between closely distributed (multiple cores on a chip and/or chips on a board) embedded systems. 

MCAPI provides three modes of communication: messages, packets, scalars
 */
#include "board.h"
#include <cmsis_os.h>
#include "svc.h"
#include "pio.h"
#include <sys/thread.h>
//#include <thread.h>
#include <threads.h>
#include <signal.h>
#include <errno.h>
//#include "atomic.h"
#include "semaphore.h"
#include "ident.h"// идентификация аппаратуры
#include "r3_slice.h"
//#include "r3stdlib.h"
//#include "r3rtos.h"
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>
#include <sys/limits.h>

//const int __aeabi_SIGTERM = 15;

#define PSP_THREAD_MODE 1

typedef struct _Event osEvent_t;
typedef struct _Event osEvent;
typedef struct _thread TCB;
typedef struct _thread osThread_t;
/// функция возвращает идентификатор треда по указателю
#define TCB_ID(t) ((void*)(t))
/// функция возвращает указатель по идентификатору треда или NULL в случае ошибки
#define TCB_PTR(t) ((TCB*)(t))
#define OS_SIGNAL_MASK ((~0UL)>>(32- osFeature_Signals))
//#define OS_EVENT_MASK (osEventSignal|osEventMessage|osEventMail|osEventTimeout)

#define OS_CCM_SLICE    0x400// 1024

extern void* _estack[]; // вершина стека, см в файле (CHIPX).ld
typedef struct _StackPool StackPool;
struct _StackPool {
//	void* base;
//	int item_bits;	// не более <=32
	volatile unsigned int map[1]; // не более 32 тредов
};
//osSemaphoreDef(thread_pool_count) = {osFeature_ThreadCount};// свобдных слотов для запуска тредов
#define STACK_POOL_BITS 10

static StackPool stack_pool = {/*.base=_estack*/{0}};
/*!	\brief выделение памяти под стек, число стеков ограничено

 */
static void* stack_pool_base = _estack - 0x400;// Уже выделен под MSP
static void *osStackAlloc (int size)
{
	StackPool* pool = &stack_pool;
	volatile int *ptr = &pool->map[0];
	register unsigned int idx, value;
	do {
		value = atomic_int_get(ptr);
		idx = __builtin_ctz(~value);
	} while (!atomic_int_compare_and_exchange(ptr, value, value| (1UL<<idx)));
	uint8_t* data=  (uint8_t*)(stack_pool_base)- (idx << STACK_POOL_BITS);
//	printf("osStackAlloc: %08X idx:%d\n", data, idx);
//	printf("osStackDebug: %08X\n", pool->map[0]);
	return data;
}

static void osStackFree (void* stk, int size)
{
	
	StackPool* pool = &stack_pool;
	int idx = ((uint8_t*)(stack_pool_base) - (uint8_t*)stk) >> STACK_POOL_BITS;// проверить правильность выделения
//	printf("osStackFree: %08X idx:%d\r\n", stk, idx);
	(void) atomic_fetch_and(&pool->map[0], ~(1UL<<idx));
}
void osStackDebug()
{
	StackPool* pool = &stack_pool;
    printf("osStackDebug: %08X\n", pool->map[0]);
}
// функции могут вызваться только из SVC
static void svc_handler_yield   (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_signal  (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_usleep  (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_kill	(unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_event_wait(unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_notify  (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;

static TCB main_thread = {.sp = NULL, .next=&main_thread, 
	.process.event.status = osEventRunning,
	.process.signals = 0, /*.errno=0,*/.priority=osPriorityNormal};
// Эти переменные должны быть доступны на чтение, но не на изменение

TCB *current_thread = &main_thread; //!< указатель на дескриптор треда
static TCB *tcb_list = &main_thread; //!< указатель на дескриптор треда. Первый элемент списка это процесс "main", он считается привелегированным и никогда не выкидывается из списка

/*! \brief задать функцию обработки прерывния по событию
	
 */
static void svc_handler_notify(unsigned int *frame, int svc_number)
{
	
}
static void svc_handler_signal(unsigned int *frame, int svc_number)
{
	TCB* thr = TCB_PTR(frame[0]);
	int sig = (int)frame[1];
	union sigval value = (union sigval)(int)frame[2];
	sigset_t set = atomic_fetch_or(&thr->process.event.status , 1<<value.sival_int);
	if(set & (1U<<value.sival_int)) {
	// если сигнал не обработан то вернуть EAGAIN
		frame[0] = -1;
		current_thread->error_no = EAGAIN;
		return;
	}
	//
	__YIELD();// пробуем передать управление
}
/*! \brief выполнить переключение задач на выходе из ISR

 */
static void svc_handler_yield(unsigned int *frame, int svc_number)
{
    __YIELD();
}
/*! \brief убить тред
    \param [in] frame указатель на стек, запроса, регистры r0-r3 расположены в стеке
        - [0] идентификатор треда \ref osThreadId
    \param [in] svc_number номер запроса SVC
    \todo разрешить убивать чужие процессы родительскому процессу.
 */
static void svc_handler_kill(unsigned int *frame, int svc_number)
{    // убить тред
    TCB* thr = TCB_PTR(frame[0]);
//	debug("-KILL\r\n");
    if (thr==NULL) return;
	if (thr->process.event.status & osEventService) 
	{// это служба ей не положено
		uint32_t* sp = atomic_exchange(&thr->sp, NULL);
		osStackFree(sp, 0/*thr->attr->stacksize*/);//thr->def->stacksize
		__YIELD();
		return;
	}
    //if (current_thread != thread->parent) return;
    TCB* tcb = tcb_list;
	volatile void** ptr = (volatile void**)&tcb->next;
	do {
		while ((tcb = (TCB*)*(ptr))!=thr) { // найти предыдущий элемент очереди
			ptr = (volatile void**)&tcb->next;
		}
		*ptr = thr->next;
	} while(0);//!atomic_pointer_compare_and_exchange(ptr, tcb, thr->next));
    //*ptr = thr->next;// если не изменилось содержимое ptr !!
	{// если задача имеет размерность N запустить дубликат, с новым индексом instance
		thr->process.event.status = osEventComplete; // исполнение завершено!  Complete=0, Running=1, Waiting, Idle
	}
    osStackFree(thr->sp, 0/*thr->def->stacksize*/);
    if (thr == current_thread){ // запросить переключение задач на выходе из процедуры
        __YIELD();
    }
}

/*! \brief выполнить задержку в микросекундах */
static void svc_handler_usleep(unsigned int *frame, int svc_number)
{
	uint32_t interval = frame[0];// delay;
	osEvent* event = (void*)frame[1];// status;
//	debug("-SLEEP\r\n");
	if (interval < 1000) {
		uint32_t timestamp = clock();
		while ((uint32_t)(clock()-timestamp) >= osKernelSysTickMicroSec(interval));
	}
	else 
	{
		TCB* tcb = TCB_PTR(current_thread);
		tcb->process.wait.timeout  = osKernelSysTickMicroSec(interval);
		tcb->process.wait.timestamp=clock();
		tcb->process.event.status = osEventTimeout;
		__YIELD();
	}
}
/*! \brief выполнить задержку в микросекундах */
static void svc_handler_event_wait(unsigned int *frame, int svc_number)
{
	//osEvent_t* event = (osEvent_t*)frame[0];// status;
	uint32_t status  = (uint32_t)frame[0];// status;
	void* arg  = (void*)frame[1];// si_value arg;
	uint32_t interval = frame[2];// delay;
	TCB* tcb = TCB_PTR(current_thread);
	if (interval!=osWaitForever) {// не работает проверка при умножении *1000
		status |= osEventTimeout;
		tcb->process.wait.timeout  = osKernelSysTickMicroSec(interval);// todo выразить в микросекундах
		tcb->process.wait.timestamp= clock();
	}
	// event->status &=~osEventRunning;// когда ждем бит(статус) не выставлен
	tcb->process.event = (osEvent_t){.status = status, .value.p = arg};
	__YIELD();
}
/*! \brief запуск системного таймера и ядра системы.
 */
osStatus osThreadInit(void)
{
    // регистрируем в системе обработчики системных вызовов
    svc_handler_callback(svc_handler_event_wait,SVC_EVENT_WAIT);
    svc_handler_callback(svc_handler_usleep,SVC_USLEEP);
    svc_handler_callback(svc_handler_yield, SVC_YIELD);
    svc_handler_callback(svc_handler_kill,  SVC_EXIT);// exit
//    svc_handler_callback(svc_handler_kill,  SVC_EXEC);

#if 0 //def PSP_THREAD_MODE // переключились PSP стек -- это происходит в начальной загрузке
	uint32_t* sp = osStackAlloc(0x400);// Выделяем стек для Handler mode
	uint32_t msp = __get_MSP();
	__set_PSP(msp);
	__set_CONTROL(2);
	__ISB();
	__set_MSP((uint32_t)sp);
#endif
	// разрешить переключение контекста
	NVIC_SetPriority(SVCall_IRQn, (1<<__NVIC_PRIO_BITS) - 1); // самый низкий приоритет
	NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1); // самый низкий приоритет
	// разрешить прерывания на переключение контекста
	return osOK;
}


#define C11_THRD

/*!	\breif Создать контекст и запустить процесс
	\ingroup _thrd _libc
	\param thrd - идентификатор треда или NULL
	\param func - функция процесса
	\param arg - аргумент функции процесса
 */
int thrd_create(thrd_t *thrd, thrd_start_t func, void *arg)
{
	TCB* thr = NULL;
	if (thrd) thr = TCB_PTR(*thrd);
	// способ выделения блоков памяти под треды требует Shared в кластере
	if (thr==NULL) thr = malloc(sizeof(struct _thread));
	thr->priority = osPriorityNormal;
	//thr->errno = 0;
	thr->sp = NULL;// стек не выделяется, пока задача не запущена
	thr->process.signals = 0; // нет сигналов
	thr->process.event = (osEvent_t){.status=osEventRunning, };
	thr->process.arg  = arg;
	thr->process.func =  (void* (*)(void *))func;
	thr->process.sig_mask = 0;// POSIX Signals
	thr->error_no = 0;
	thr->tss=NULL;	// инициализация Thread Specific Storage
//	thr->cond=NULL; // инициализация Condition используется для ожидания завершения 
	if(thrd) *thrd = TCB_ID(thr);// 
	else {// Detached
		
	}
		
// атомарно запихиваем в очередь на исполнение
	volatile void** ptr= (volatile void**)&(current_thread->next);
	do {// атомарно записываем элемент в список задач (есть страх что current->next изменится, current - это мой он не изменится пока я не вышел.
		thr->next = atomic_pointer_get(ptr);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, thr->next, thr));
	return thrd_success;
}
/*!	\brief Удалить задачу из списка активных задач
	\ingroup _thrd _libc
	\param args - это то что возвращается из треда
	\return нет возврата
	
	\note For every thread-specific storage key which was created with a non-null destructor and for which
the value is non-null, \b thrd_exit sets the value associated with the key to a null pointer value and
then invokes the destructor with its previous value. 
The order in which destructors are invoked is unspecified
*/
_Noreturn void thrd_exit(int res)
{
	TCB* self = current_thread;
#if 0
	extern void  tss_destroy(tss_t tss);
	if (self->tss)
		tss_destroy(self->tss);
#endif
	self->process.result = (void*)(intptr_t)res;
    svc3(SVC_SIGNAL, self, SIGTERM, 0);
	while(1);
}
/*!	\ingroup _thrd _libc
*/
thrd_t thrd_current(void)
{ 
	return TCB_ID(current_thread); 
}
/*!	\ingroup _thrd _libc
*/
int thrd_joint(thrd_t thr, int *res)
{
#if 0
	mtx_t *mutex=NULL;
	cnd_wait((cnd_t *)&thr->cond, mutex);
#endif
	return thrd_error;
}
/*!	\ingroup _thrd _libc
*/
int thrd_detach(thrd_t thr) 
{
#if 0
	cnd_destroy((cnd_t *)&thr->cond);
#endif
	return thrd_success;
}
/*!	\ingroup _thrd _libc
*/
void thrd_yield(void)
{
	svc(SVC_YIELD);
}
/*!	\ingroup _thrd _libc
*/
int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{// see clock_nanosleep и nanosleep
	clock_t ts;
	if (remaining) {
		ts = clock();
	}

	int32_t interval = (unsigned long)duration->tv_nsec/1000; // ошибка в 2.4%
	
	svc3(SVC_EVENT_WAIT, 0, 0, interval);
	if (remaining) {
		interval -= (clock() - ts);
		if (interval>0)
			*remaining = (struct timespec){.tv_nsec = interval*1000}; // время до конца интервала
		else 
			*remaining = (struct timespec){0};
	}
	osEvent_t* event = &current_thread->process.event;
	return event->status & osEventTimeout?-1:0;
}
/*! c11 */
/*
_Noreturn void exit (int err) {
	current_thread->errno = err;
    (void) osThreadTerminate(current_thread);
}*/
/*! \brief В довершение процесса выполняется указанная функция 
	\ingroup _libc
 */
int atexit(void (*func)(void))
{
	uint32_t* stk = (uint32_t*)TCB_PTR(current_thread) + (OS_CCM_SLICE>>2);
	stk[-3] = (uint32_t)func;// подложить в стек функцию завершения
}
/*! \brief Код ошибки 
	\ingroup _libc _aeabi
	\headerfile <errno.h>
 */
volatile int* __aeabi_errno_addr() 
{
	return &current_thread->error_no;
}

/*!	\brief Создать процесс
    \note функция может запускаться до инициализации ядра.
	
	Приложения: 
	1) Надо организовать последовательное исполнение по цепочке, перед исполнение треда может быть вставлено событие
	event - событие, которое заполняется системой, при изменении состояния . 
	Можно ожидать это событие функцией osEventWait();
	
 */
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *args)
{
//	semaphore_enter();
//  osThreadId tcb 
//	uint32_t* sp = 
//	tcb = r3_mem_alloc_aligned(thread_def->stacksize, 8);
//	tcb->stacksize = thread_def->stacksize;
//	uint32_t* sp = (uint32_t*)tcb;
//	sp += thread_def->stacksize>>2;
//	printf("ThCreate %08X - %08X\n", (uint32_t)tcb, (uint32_t)sp);
	
//	uint32_t* sp = NULL;//osStackAlloc(thread_def->stacksize);

	osThreadId thr = NULL;
	thrd_create(&thr, thread_def->pthread, args);
	TCB_PTR(thr)->priority= thread_def->tpriority;
	return thr;
}

/*!	\brief Удалить задачу из списка активных задач
	\param args - это то что возвращается из треда
	\return нет возврата
*/
void  osThreadExit();

__attribute__((noinline)) osStatus osThreadTerminate (osThreadId thread_id);
/*!	\brief Удалить задачу из списка активных задач

	\todo надо запретить одновременное исполнение операции osThreadTerminate, в случае
	\note нельзя вызывать функцию для себя

	\return Status and Error Codes
    - \b osOK: the specified thread has been successfully terminated.
    - \b osErrorParameter: thread_id is incorrect.
    - \b osErrorResource: thread_id refers to a thread that is not an active thread.
    - \b osErrorISR: osThreadTerminate cannot be called from interrupt service routines.


 */
osStatus osThreadTerminate (osThreadId thread_id)
{
	TCB* tcb = TCB_PTR(thread_id);
    svc3(SVC_SIGNAL, TCB_ID(tcb), SIGTERM, 0);// заменить на TCB_PID - идентификатор процесса
    return osOK;
}
/*! \brief запросить идентификатор активного треда
 */
osThreadId osThreadGetId (void)
{
	return TCB_ID(current_thread);
}


/*! \brief установить приоритет треда
 */
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
	TCB* tcb = TCB_PTR(thread_id);
	if (tcb==NULL) return osErrorResource;
	tcb->priority = priority;
	return osOK;
}
/*! \brief запросить приоритет треда
 */
osPriority osThreadGetPriority (osThreadId thread_id)
{
	TCB* tcb = TCB_PTR(thread_id);
	if (tcb==NULL) return osPriorityError;
	return tcb->priority;
}
#if 0
__attribute__((noinline)) osStatus osThreadYield (void);
/*! \brief запросить переключение задач

    вызов выполняется из треда, для вызова из ISR следует использовать \b __YIELD()
 */
osStatus osThreadYield (void)
{
    svc(SVC_YIELD);
	return osOK;
}
#endif
/*! \} */


/*! эта функция предназначена для запуска из прерывания чтобы сократить задержку на обработку 
\todo сделать глобальную переменную osThreadId osThreadCurrent = current_thread;
\todo сделать функцию макросом
 */
osStatus osThreadNotify(osThreadId thread_id) PRIVILEGED_FUNCTION;
osStatus osThreadNotify(osThreadId thread_id)
{
	// \TODO вставить в очередь с учетом приоритета
	TCB* tcb = TCB_PTR(thread_id);
	if ((tcb) != current_thread) {// если всего два треда, то не надо мутить 
/*
		volatile void** ptr = &current_thread->next;
		thread_id->next = atomic_pointer_exchange(ptr, thread_id->next);
		do {// атомарно записываем элемент в список задач
			thread_id->next = atomic_pointer_get(ptr);
			atomic_mb();
		} while (!atomic_pointer_compare_and_exchange(ptr, thread_id->next, tcb));
		__YIELD();// вызвать переключение задач на выходе из прерывания
		*/
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;// вызов PendSV
	} else {
		SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;// запретить вызов PendSV
	}
//	tcb->pending++;
//	tcb->budget += BUDGET(tcb->priority);
	return osOK;	
}
#if 1
int osThreadErrno (osThreadId thr, int err_no)
{
	return atomic_exchange(&TCB_PTR(thr)->error_no, err_no);
}
#endif

void  PendSV_Handler(void) PRIVILEGED_FUNCTION;
__attribute__((naked)) void PendSV_Handler();
/*! \brief переключение контекстов задач
	\ingroup _system

Cortex-M3 Cortex-M4 Cortex-M4F Cortex-M7 Cortex-M23 Cortex-M33
 */
void PendSV_Handler()
{
#ifndef PSP_THREAD_MODE
# if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    __asm volatile(//  при входе сохранены регистры R0-R3,R12,LR,PC,xPSR
		"tst       lr, #0x10\n\t"                     // if FPU context is active
		"it        eq\n\t"                            // we should save its registers
		"vpusheq  {S16-S31}\n\t"
		"push     {r4-r11,lr}\n\t"
		"mov r0,sp\n\t"
		// r0 указывает на стек процесса, на который переключились, lr содержит его EXC_RETURN 
		"bl osThreadScheduler\n\t"
		"mov sp, r0\n\t"
		"pop {r4-r11, lr}\n\t"
		"tst    lr, #0x10\n\t"
		"it     eq\n\t"                            // Restore VFP S16..S31
		"vpopeq {S16-S31}\n\t"
		"bx	lr\n\t" // выход из обработчика
	:::"memory");	
# elif defined(__ARM_ARCH_8M_BASE__)
    __asm volatile(//  при входе сохранены регистры R0-R3,R12,LR,PC,xPSR
		"push     {r4-r7,lr}\n\t"
		"mov r4,r8\n\t"
		"mov r5,r9\n\t"
		"mov r6,r10\n\t"
		"mov r7,r11\n\t"
		"push     {r4-r7}\n\t"
		"bl osThreadScheduler\n\t"
		"pop {r4-r7}\n\t"
		"mov r11,r7\n\t"
		"mov r10,r6\n\t"
		"mov r9,r5\n\t"
		"mov r8,r4\n\t"
		"pop {r4-r7, pc}\n\t"
	:::"memory");
# else
    __asm volatile(//  при входе сохранены регистры R0-R3,R12,LR,PC,xPSR
		"push     {r4-r11,lr}\n\t"
		"mov r0,sp\n\t" // r0 указывает на стек процесса, на который переключились, lr содержит его EXC_RETURN 
		"bl osThreadScheduler\n\t"
		"mov sp, r0\n\t"
		"pop {r4-r11, pc}\n\t"
	:::"memory");	
# endif
#else
#  if defined(__ARM_ARCH_8M_BASE__)
    __asm volatile(//  при входе сохранены регистры R0-R3,R12,LR,PC,xPSR
		"mrs      r0, psp\n\t"
		"mov      r3, lr\n\t"
		"sub      r0,r0,#36\n\t"
		"stmia    r0!, {r4-r7}\n\t"             // сохраняем на стеке процесса
        "mov      r4,r8\n\t"
        "mov      r5,r9\n\t"
        "mov      r6,r10\n\t"
        "mov      r7,r11\n\t"
		"stmia    r0!,{r4-r7}\n\t"	             // Save R8..R11
		"stmia    r0!,{r3}\n\t"             // сохраняем LR на стеке процесса
		"sub      r0,r0,#36\n\t"
		"bl   osThreadScheduler\n\t"
		"add      r0,r0,#16\n\t"
		"ldmia    r0!,{r4-r7}\n\t"               // Restore R8..R11
		"ldmia    r0!,{r3}\n\t"               // Restore LR
        "mov      r8,r4\n\t"
        "mov      r9,r5\n\t"
        "mov      r10,r6\n\t"
        "mov      r11,r7\n\t"
		"msr      psp, r0\n\t"
        "sub      r0,r0,#36\n\t"                 // Adjust address
		"ldmia    r0!,{r4-r7}\n\t"            // Restore R4..R7
		"bx		  r3\n\t" // выход из обработчика
	:::"memory");	
#  elif defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8_1M_MAIN__)
    __asm volatile(//  при входе сохранены регистры R0-R3,R12,LR,PC,xPSR
		"mrs       r0, psp\n\t"
		"tst       lr, #0x10\n\t"                     // if FPU context is active
		"it        eq\n\t"                            // we should save its registers
		"vstmdbeq  r0!, {S16-S31}\n\t"                // on the process stack
		"stmdb     r0!, {r4-r11,lr}\n\t"             // сохраняем на стеке процесса
		"bl osThreadScheduler\n\t"
		"ldmia 	r0!, {r4-r11,lr}\n\t"
		"tst    lr, #0x10\n\t"
		"it     eq\n\t"                            // Restore VFP S16..S31
		"vldmiaeq r0!, {S16-S31}\n\t"                // on the process stack
		"msr	psp, r0\n\t"
		//"orr lr,#4\n\t"
		"bx		lr\n\t" // выход из обработчика
	:::"memory");
#  else
	#error "Need Context Swith to PSP Thread mode"
#  endif
#endif
}

// Event Queue
/* Данная реализация представляет собой рчередь FIFO 
	Для EDF нужна очередь с приоритетами
 */
typedef struct {
	int signo;
	osThreadId thread_id;
	clock_t activation_time;
} sig_data_t;
typedef struct {
		volatile int count;// начальное значение _POSIX_SIGQUEUE_MAX
		uint16_t wr_pos;
		uint16_t rd_pos;
		uint32_t  ready[1];
		sig_data_t  buf[_POSIX_SIGQUEUE_MAX];// 
} event_queue_t;
static event_queue_t signal_queue= {.count=0};
static inline uint32_t CPU_clock(){
    return DWT->CYCCNT;
}

int sig_queue_put(event_queue_t* q, osThreadId thread_id, int signo){
    if (!semaphore_enter(&q->count)){// FIFO переполнено
        return -1;
    }
	int n = atomic_fetch_add(&q->wr_pos, 1) & 0x1F;
	sig_data_t * sig = &q->buf[n];
	sig->signo = signo;
	sig->thread_id = thread_id;
	sig->activation_time = CPU_clock();
	atomic_fetch_or(&q->ready[0], 1u<<n);
	return n;
}
static inline void* sig_queue_try(event_queue_t* q){
	int n = q->rd_pos & 0x1F;
	int flag = atomic_fetch_and(&q->ready[0], ~(1u<<n));
	return (flag & (1u<<n))? &q->buf[n]: NULL;
}
static inline int sig_queue_next(event_queue_t* q){
	int count = atomic_fetch_add(&q->count, 1);
	q->rd_pos++;
	return count;
}

static volatile int pid_signals = 0;// в отдельный сегмент

/*! \brief Планировщик процессов 
	\ingroup _system
	\param [in] stk указатель на стек процесса
	\return указатель на стек процесса который будет исполняться после переключения задач.
	

Планировщик вызывается по случаю слишком долго исполняется отдельный процесс, по времени. При вызове функции Yield()
Или по событию. 

В данной версии ОС ожидание процесса - это флаги событий и семафоры.
Ожидание Мьютексов, Семафоров, ожидание очередей собщений, пулов памяти реализовано на семафорах. Семафоры и флаги событий отданы в приложение и реализованы на атомарных операциях. 

Синхронизация приложений по готовности - это ожидание Condition на своем мьютексе.
 */
unsigned int * osThreadScheduler(unsigned int * stk)
{
	current_thread->sp = stk;// вынести эту операцию в PendSV
	//current_thread->CPU_time += execution_time;
	TCB* thr = NULL;
#if 1
// Сигналы в очереди должны быть организованы по приоритетам
// Приоритет EDF, Erlier deadline First
	sig_data_t *sig;
	while ((sig = sig_queue_try(&signal_queue))!=NULL) {
		thr = TCB_PTR(sig->thread_id);
		uint32_t sig_mask = 1u<<sig->signo;
		if (thr==NULL) {// сигнал адресован процессу
#if defined(_POSIX_SIGNALS) && (_POSIX_SIGNALS>0)
			sig_mask &= atomic_fetch_or(&pid_signals, sig_mask);
			if (sig_mask) break; // не доставлен
#endif
		} else {
#if defined(_POSIX_THREAD_SPORADIC_SERVER) && (_POSIX_THREAD_SPORADIC_SERVER>0)
			// Deffered Service/Sporadic Service
			if ((uint32_t)(CPU_clock() - sig->activation_time) < thr->sched.repl_period){
				break;// рано, сигнал не доставлен
			}
#endif
			sig_mask &= atomic_fetch_or(&thr->process.signals, sig_mask);
			if (sig_mask) {// сигнал не обработан
				break;
			} else {// начислить бюджет, событие доставлено
				// atomic_fetch_sub(&thr->sig_pending, 1);// уменьшили число не обработаных событий 
				thr->priority= thr->sched.priority;
					thr->budget = BUDGET(thr, thr->priority);
				if (thr->budget > thr->sched.initial_budget)// ограничено
					thr->budget = thr->sched.initial_budget;
				if (thr->next==NULL) 
					thr->next = current_thread;// возврат к предыдущему
				//if((thr->process.event.status & (osEventRunning))==0)
					thr->process.event.status |= (osEventSignal|osEventRunning);
			}
		}
		sig_queue_next(&signal_queue);
		if (thr) break;
	}
	if (thr==NULL)
#endif
	// планировщик задач
	do {
		/* чтобы можно было запускать несколько планировщиков, это должна быть служба, 
		контекст привязан к ядру для которого выполняется планирование, доступ к списку задач - атомарный
		*/
#if 0
		// по круговому списку задач, всегда в списке одна задача "main"
		volatile void** ptr = (volatile void** )&current_thread;// общая очередь для кластера ядер
		do {
			thr = atomic_pointer_get(ptr);// __LDREX
		} while (!atomic_pointer_compare_and_exchange(ptr, thr, thr->next));// __STREX
		thr = thr->next;
#else
		thr = current_thread = current_thread->next; 
#endif
        osEvent_t *event = &thr->process.event;
        if (event->status & osEventRunning) break;
		
        if (event->status & (osEventSignal)) 	{// ожидаем сигналы
//			volatile int* ptr = event->value.p;
//			uint32_t signals = *ptr & event->value.signals;
#if 0 //defined(_POSIX_REALTIME_SIGNALS) && (_POSIX_REALTIME_SIGNALS>0)
// доставка сигналов в тред через очередь сигналов
			if (pid_signals & thr->process.sig_mask) {// доставка сигналов 
				uint32_t sig_mask = ~thr->process.signals & thr->process.sig_mask;
				sig_mask &= atomic_fetch_and(&pid_signals, ~sig_mask);
				if (sig_mask) {
					atomic_fetch_or(&thr->process.signals, sig_mask);
				}
			}
#endif
			// 31.08.2022 
			uint32_t signals = thr->process.signals & event->value.signals;
			// uint32_t signals = thr->process.signals & thr->process.sig_mask; - хочу так
            if (signals){
				//if ((event->status & osEventWaitAll)==0 || (signals==event->value.signals))
				{
					event->value.signals =(signals); // это может лишняя операция
					event->status = osEventSignal|osEventRunning;
					break;
				}
            }
        } 
		else
        if (event->status & (osEventSemaphore)) {// выделение памяти из пула, семафоры и мьютексы
            volatile int* ptr = event->value.p;
			int count = semaphore_enter(ptr);
			if (count > 0) { // счетчик семафора до входа
				event->value.v = count;// это может лишняя операция
				event->status = osEventSemaphore|osEventRunning;
				break;
			}
        } 
skip_event:
        if (event->status & (osEventTimeout)) 	{// ожидаем таймаут, можем использовать DWT->CYCCOUNT
			uint32_t system_timestamp = clock();
            if ((uint32_t)(system_timestamp - thr->process.wait.timestamp)>=thr->process.wait.timeout){
                // 01.02.2018 event->value.v = system_timestamp - thr->wait.timestamp; // возвращаем задержку
                event->status = osEventTimeout|osEventRunning;
                break;
            }
        }
	} while (1);//(thr->wait.events & thr->events)==0);// условие выхода из ожидания: событие настало
	atomic_free();// Clear Exclusive Monitor -- эта операция аппаратно зависима
	if (thr->sp==NULL) {//стек не выделен!
		unsigned int *sp = osStackAlloc(0x400/*thr->stacksize*/);
		// надо ли резервировать место для S0-15, FPSCR 
			// инициализация стека R0-R3,R12,LR,PC,xPSR
		*(--sp) = (uint32_t)0x01000000;		// xPSR Thumb state
		*(--sp) = (uint32_t)thr->process.func;//|1;		// PC start function |1 -Thumb mode
		*(--sp) = (uint32_t)osThreadExit;//|1;	// LR stop function |1 -Thumb mode
		*(--sp) = 0; //R12
		*(--sp) = 0; //R3
		*(--sp) = 0; //R2
		*(--sp) = 0; //(uint32_t)&thr->process.event; //R1
		*(--sp) = (uint32_t)thr->process.arg; //R0
#ifdef PSP_THREAD_MODE
# if defined(__ARM_ARCH_8M_BASE__) 
		*(--sp) = (uint32_t)0xFFFFFFBC; //PSP Thread no-FPU, Cortex-M23/33 wo security extention
# else
		*(--sp) = (uint32_t)EXC_RETURN_THREAD_PSP;// 0xFFFFFFFD PSP Thread no-FPU, 0xFFFFFFE9 - MSP
# endif
#else
		*(--sp) = (uint32_t)EXC_RETURN_THREAD_MSP; // 0xFFFFFFF9 MSP Thread no-FPU, 0xFFFFFFE9 - MSP w-FPU
#endif
		sp-=8;// резервируем место на R4-R11 в любой последовательности
		thr->sp = sp;
#if 0 // Был раньше вариант быстрого выхода
		__asm volatile (
#if 0 // использовать PSP
		"msr psp, %0\n\t"
		"mov lr, #0xFFFFFFFD\n\t"// это магия! - non-floating-point state, PSP
#else // рабочий вариант
		"mov sp, %0\n\t"
		"mov lr, #0xFFFFFFF9\n\t"// это магия! - non-floating-point state, MSP
#endif
		"bx lr\n\t"
		::"r"(sp):"memory");
#endif
	}
	return thr->sp;
/* Из документации на Cortex-M3/M4
0xFFFFFFF1 
	Return to Handler mode, exception return uses non-floating-point state from MSP and execution uses MSP after return.
0xFFFFFFF9 
	Return to Thread mode, exception return uses non-floating-point state from MSP and execution uses MSP after return.
0xFFFFFFFD 
	Return to Thread mode, exception return uses non-floating-point state from PSP and execution uses PSP after return.
0xFFFFFFE1 
	Return to Handler mode, exception return uses floating-point-state from MSP and execution uses MSP after return.
0xFFFFFFE9 
	Return to Thread mode, exception return uses floating-point state from MSP and execution uses MSP after return.
0xFFFFFFED 
	Return to Thread mode, exception return uses floating-point state from PSP and execution uses PSP after return.

Из документации на Cortex-M0
0xFFFFFFF1	Return to Handler mode.
	Exception return gets state from the main stack.
	Execution uses MSP after return.
0xFFFFFFF9	Return to Thread mode.
	Exception return gets state from MSP.
	Execution uses MSP after return.
0xFFFFFFFD	Return to Thread mode.
	Exception return gets state from PSP.
	Execution uses PSP after return.
	
Из документации на Cortex-M23/33
For implementations without the Security Extension;
0xFFFFFFB0 	Return to Handler mode
0xFFFFFFB8 	Return to Thread mode using the main stack.
0xFFFFFFBC 	Return to Thread mode using the process stack

*/
}


/*!
    \brief Wait for Signal, Message, Mail, or Timeout.

 */
__attribute__((noinline)) void osEventWait (osEvent_t *event, uint32_t interval);
/*! \todo сделать время в микросекундах */
void osEventWait (osEvent_t *event, uint32_t interval)
{
	svc3(SVC_EVENT_WAIT, event->status, event->value.v, interval*1000);
	*event = current_thread->process.event;// вот это копирование можно исключить!!
	//return (event->status & osEventTimeout)?thrd_timedout:thrd_success;
}
#if 0//!defined (__ARM_ARCH_8M_BASE__)
/* Единственное место где выполняется преобразование timespec в интервал */
osStatus osEventTimedWait (osEvent *event, const struct timespec* restrict ts)
{
	uint32_t interval = (ts->tv_sec*1000000UL + ts->tv_nsec/1000) - clock();// надо оптимизировать и исключить отрицательные значения интервала
	svc3(SVC_EVENT_WAIT, event->status, event->value.v, interval);
	*event = current_thread->process.event;// вот это копирование можно исключить!!
	return (event->status & osEventTimeout)?thrd_timedout:thrd_success;
}
#endif
/*! \} */

//  ==== Signal Management ====
#if (defined(osFeature_Signals) && (osFeature_Signals!=0))
/*!	\defgroup _Signals RTOS: Signal Management
	\ingroup _rtos
	\{
 */
int32_t osSignalSet (osThreadId thread_id, int32_t signals)
{
	// если тред не найден return 0x80000000;
	//if (thread_id==NULL) return 0x80000000;
	return atomic_fetch_or((volatile int *)&TCB_PTR(thread_id)->process.signals, (signals & OS_SIGNAL_MASK));
}
int32_t osSignalClear (osThreadId thread_id, int32_t signals)
{
	// если тред не найден return 0x80000000;
	//if (thread_id==NULL) return 0x80000000;
	return atomic_fetch_and((volatile int *)&TCB_PTR(thread_id)->process.signals, ~(signals & OS_SIGNAL_MASK));
}
#if 0// забанили
/*!  \brief создает ссылку через bit-band alias на сигнал 
	\param signal - номер сигнала
 */
int32_t* osSignalRef (osThreadId thread_id, int32_t signal)
{
#if defined(__ARM_ARCH_7EM__)
	int32_t* ref = BB_ALIAS(&thread_id->process.signals, signal);
#else
	int32_t* ref = (int32_t*)signal;
#endif
		//printf("SET %08X -> %08X\n", (uint32_t)ref, (uint32_t)&thread_id->signals);
	return ref;
}
#endif
/*! \} */
#endif
//  ==== Signal Management ====
#if (defined(osFeature_ThreadFlags) && (osFeature_ThreadFlags > 0))
/*!	\defgroup _ThreadFlags RTOSv2: Thread Flags
	\ingroup _rtos2
	\{ */
uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
//	if (flags==0) flags = ~0;
	svc3(SVC_EVENT_WAIT, osEventSignal|options, flags, timeout*1000);
	osEvent_t *event = &current_thread->process.event;
	return (event->status & osEventTimeout)?osFlagsErrorTimeout: event->value.signals;
}
uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
	// if (thread_id==NULL) return 0x80000000; эту проверку надо куда-то убрать. В коде ей не место
	return atomic_fetch_or((volatile int *)&TCB_PTR(thread_id)->process.signals, (flags & OS_SIGNAL_MASK));
}
uint32_t osThreadFlagsClear(osThreadId_t thread_id, uint32_t flags)
{
	// if (thread_id==NULL) return 0x80000000; эту проверку надо куда-то убрать. В коде ей не место
	return atomic_fetch_and((volatile int *)&TCB_PTR(thread_id)->process.signals, ~(flags & OS_SIGNAL_MASK));
}
uint32_t osThreadFlagsGet(osThreadId_t thread_id)
{
	return (TCB_PTR(thread_id)->process.signals & OS_SIGNAL_MASK);
}
/*! \} */
#endif
