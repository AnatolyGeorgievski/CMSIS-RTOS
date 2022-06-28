/*! \ingroup _system
    \defgroup _thread Thread management


	\see [AN298] Cortex-M4(F) Lazy Stacking and Context Switching
	\see [ARM DUI 0553A] Cortex™-M4 Devices Generic User Guide
	\see http://www.keil.com/pack/doc/CMSIS/General/html/index.html
	\see http://www.keil.com/pack/doc/cmsis_rtx/index.html
	\see http://www.tnkernel.com
	\see FreeRTOS Real Time Kernel (RTOS)
	\see http://developer.mbed.org/handbook/CMSIS-RTOS
	\see The Definitive Guide to ARM® Cortex®-M3 and Cortex®-M4 Processors
	\see https://github.com/ARM-software/CMSIS_5
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
	while(1);// нет возврата

	Запись элемента в очередь должна осуществляться атомарно. нужен примитив atomic_unlink(); и atomic_insert
	volatile void* ptr = tcb_list->next;
	do {
		tcb->next = __LDREX(tcb_list->next);
	} while (__STREX(tcb, tcb_list->next)==0);
	
	11.03.2020 Надо бы добавить конкуренцию тредам. Есть идея такая - задержка на переключение не должна превышать установленный таймаут. Каждая задача должна выполняться согласно приоритету. см. определение cooperative multitasking и preemptive multitasking
	
	09.05.2021 Надо бы очищать бит FPU->FPCCR &= ~(FPU_FPCCR_LSPACT_Msk); // активность FPU для вновь запускаемого треда.
	
 */
#include "board.h"
#include <cmsis_os.h>
#include "svc.h"
#include "pio.h"
#include "thread.h"
#include <threads.h>
//#include "atomic.h"
#include "semaphore.h"
#include "ident.h"// идентификация аппаратуры
#include "r3_slice.h"
//#include "r3stdlib.h"
//#include "r3rtos.h"
#include <stdlib.h>
#include <stdnoreturn.h>


#define PSP_THREAD_MODE 1


/// функция возвращает идентификатор треда по указателю
#define TCB_ID(t) (t)
/// функция возвращает указатель по идентификатору треда или NULL в случае ошибки
#define TCB_PTR(t) (t)
#define OS_SIGNAL_MASK ((~0UL)>>(32- osFeature_Signals))
//#define OS_EVENT_MASK (osEventSignal|osEventMessage|osEventMail|osEventTimeout)

#define OS_CCM_SLICE    0x400// 1024

extern void* _estack[]; // вершина стека, см в файле (CHIPX).ld
typedef struct _StackPool StackPool;
struct _StackPool {
	void* base;
//	int item_bits;	// не более <=32
	volatile unsigned int map[1]; // не более 32 тредов
};
//osSemaphoreDef(thread_pool_count) = {osFeature_ThreadCount};// свобдных слотов для запуска тредов
#define STACK_POOL_BITS 10

static StackPool stack_pool = {.base=_estack};
/*!	\brief выделение памяти под стек, число стеков ограничено

 */
void* stack_pool_base = _estack;
static void *osStackAlloc (int size)
{
	StackPool* pool = &stack_pool;
	volatile int *ptr = &pool->map[0];
	register unsigned int idx, value;
	do {
		value = atomic_int_get(ptr);
		idx = __builtin_ctz(~value);
	} while (!atomic_int_compare_and_exchange(ptr, value, value| (1UL<<idx)));
	uint8_t* data=  (uint8_t*)(stack_pool_base - 0x400)- (idx << STACK_POOL_BITS);
//	printf("osStackAlloc: %08X idx:%d\n", data, idx);
//	printf("osStackDebug: %08X\n", pool->map[0]);
	return data;
}

static void osStackFree (void* stk, int size)
{
	
	StackPool* pool = &stack_pool;
	int idx = ((uint8_t*)(stack_pool_base - 0x400) - (uint8_t*)stk) >> STACK_POOL_BITS;// проверить правильность выделения
//	printf("osStackFree: %08X idx:%d\r\n", stk, idx);
	(void) atomic_fetch_and(&pool->map[0], ~(1UL<<idx));
}
void osStackDebug()
{
	StackPool* pool = &stack_pool;
    printf("osStackDebug: %08X\n", pool->map[0]);
}

static void svc_handler_yield   (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_exit    (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_usleep  (unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_kill	(unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;
static void svc_handler_event_wait(unsigned int *frame, int svc_number) PRIVILEGED_FUNCTION;

static TCB main_thread = {.sp = NULL, .next=&main_thread, 
	.process.event.status = osEventRunning,
	.process.signals = 0, /*.errno=0,*/.priority=osPriorityNormal};
static TCB *current_thread = &main_thread; //!< указатель на дескриптор треда
static TCB *tcb_list = &main_thread; //!< указатель на дескриптор треда. Первый элемент списка это процесс "main", он считается привелегированным и никогда не выкидывается из списка

#if 0
static unsigned char IP_Addr[4] = {192,168,2,144};
#define ARP_REQUEST			0x0001

void task1(void const* data)
{
	//if () printf("Once %d ", (unsigned int)data);
	int64_t count=0;
	while (count<8){
//		if ((count&0xFFFFFFULL) == 0)
        {
//            printf("T1 %d\n", HAL_GetTick());
            osDelay(2000);
            printf("=T1 %lu\n", HAL_GetTick());
			int res = arp_request(/*iface, */__REV16(ARP_REQUEST), IP_Addr, NULL);

			if (res != 0) {
				printf("=T1 err\n");
			}
		}
		count++;
	}
}
//osThreadDef(task1,osPriorityNormal,0,OS_CCM_SLICE);
#endif // test

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
    osThreadId thr = (void*)frame[0];
//	debug("-KILL\r\n");
    if (thr==NULL) return;
	if (thr->process.event.status & osEventService) 
	{// это служба ей не положено
		uint32_t* sp = atomic_exchange(&thr->sp, NULL);
		osStackFree(sp, thr->def->stacksize);
		__YIELD();
		return;
	}
    //if (current_thread != thread->parent) return;
    osThreadId tcb = tcb_list;
	volatile void** ptr = (volatile void**)&tcb->next;
	do {
		while ((tcb = (osThreadId)*(ptr))!=thr) { // найти предыдущий элемент очереди
			ptr = (volatile void**)&tcb->next;
		}
		*ptr = thr->next;
	} while(0);//!atomic_pointer_compare_and_exchange(ptr, tcb, thr->next));
    //*ptr = thr->next;// если не изменилось содержимое ptr !!
	{// если задача имеет размерность N запустить дубликат, с новым индексом instance
		thr->process.event.status = osEventComplete; // исполнение завершено!  Complete=0, Running=1, Waiting, Idle
	}
    osStackFree(thr->sp, thr->def->stacksize);
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
		uint32_t timestamp = osKernelSysTick();
		while ((uint32_t)(osKernelSysTick()-timestamp) >= osKernelSysTickMicroSec(interval));
	}
	else 
	{
		TCB* tcb = TCB_PTR(current_thread);
		tcb->process.wait.timeout  = osKernelSysTickMicroSec(interval);
		tcb->process.wait.timestamp=osKernelSysTick();
		tcb->process.event.status = osEventTimeout;
		__YIELD();
	}
}
/*! \brief выполнить задержку в микросекундах */
static void svc_handler_event_wait(unsigned int *frame, int svc_number)
{
	osEvent* event = (void*)frame[0];// status;
	uint32_t interval = frame[1];// delay;
	TCB* tcb = TCB_PTR(current_thread);
	if (interval!=osWaitForever) {
		event->status |= osEventTimeout;
		tcb->process.wait.timeout  = osKernelSysTickMicroSec(interval*1000);
		tcb->process.wait.timestamp= osKernelSysTick();
	}
	// event->status &=~osEventRunning;// когда ждем бит(статус) не выставлен
	tcb->process.event = *event;
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
    svc_handler_callback(svc_handler_kill,  SVC_KILL);
//    svc_handler_callback(svc_handler_kill,  SVC_EXEC);

#if 0 //def PSP_THREAD_MODE // переключились PSP стек
	uint32_t* sp = osStackAlloc(0x400);// Выделяем стек для Handler mode
	uint32_t msp = __get_MSP();
	__set_PSP(msp);
	__set_CONTROL(2);
	__ISB();
	__set_MSP((uint32_t)sp);
#endif
	// разрешить переключение контекста
	NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1); // самый низкий приоритет
	// разрешить прерывания на переключение контекста
	return osOK;
}


#define C11_THRD

#if 0
static inline void* osAsyncQueueWait(volatile void** ptr, uint32_t millisec)
{
	osEvent evt = {.status=osEventAsyncQueue, .value={.p=ptr}};// ждем когда этот указатель будет не нуль
	osEventWait(&evt, millisec);
	return evt.value.p;
}
/*! таймеры треда  используются для служб */ 
uint32_t osProcessTimerStart(osProcessId process, uint32_t timeout)
{
	process->wait.timeout   = osKernelSysTickMicroSec(timeout);
	process->wait.timestamp = osKernelSysTick();
	return atomic_fetch_or (&process->event.status,  osEventTimeout);
}
uint32_t osProcessTimerStop(osProcessId process)
{
	return atomic_fetch_and(&process->event.status, ~osEventTimeout);
}
/* используется для службой чтобы настраивать свои события */
osEvent* osProcessGetEvent(osProcessId thr)
{
	return &thr->event;
}
#endif
// C11
int thrd_create(thrd_t *thrd, thrd_start_t func, void *arg)
{
	thrd_t thr = NULL;
	if (thrd) thr = *thrd;
	if (thr==NULL) thr = malloc(sizeof(struct os_thread_cb));
	thr->priority = osPriorityNormal;
	//thr->errno = 0;
	thr->sp = NULL;
	thr->process.signals = 0; // нет сигналов
	thr->process.event = (osEvent){.status=osEventRunning, };
	thr->process.arg  = arg;
	thr->process.func = func;
	thr->error_no = 0;
	thr->tss=NULL;
	volatile void** ptr= (volatile void**)&(current_thread->next);
	do {// атомарно записываем элемент в список задач (есть страх что current->next изменится, current - это мой он не изменится пока я не вышел)
		thr->next = atomic_pointer_get(ptr);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, thr->next, thr));
	if(thrd) *thrd = thr;
	return thrd_success;
}
/*!	\brief Удалить задачу из списка активных задач
	\param args - это то что возвращается из треда
	\return нет возврата
	
	\note For every thread-specific storage key which was created with a non-null destructor and for which
the value is non-null, \b thrd_exit sets the value associated with the key to a null pointer value and
then invokes the destructor with its previous value. 
The order in which destructors are invoked is unspecified
*/
_Noreturn void thrd_exit(int res)
{
extern void  tss_destroy(tss_t tss);
	
#if 0
	if (current_thread->tss)
		tss_destroy(current_thread->tss);
#endif
	current_thread->process.result = res;
    __ASM volatile ("ldr r0, %[tcb]"::[tcb]"m"(current_thread):"r0");
    svc(SVC_KILL);
	while(1);
}
thrd_t thrd_current(void)
{ 
	return current_thread; 
}
int thrd_joint(thrd_t thr, int *res)
{
	// найти тред и ждать его завершения
	// evt= {.status=osEventComplete, .value={.p=&thr->exec}};// ждем когда в ноль обратится статус события
	// osEventWait(evt)
	//if (res) *res = thr->errno;
	// free(thr);
}
int thrd_detach(thrd_t thr) 
{
	return thrd_success;
}
void thrd_yield(void)
{
	svc(SVC_YIELD);
}
/*! c11 */
/*
_Noreturn void exit (int err) {
	current_thread->errno = err;
    (void) osThreadTerminate(current_thread);
}*/
int atexit(void (*func)(void))
{
	uint32_t* stk = (uint32_t*)TCB_PTR(current_thread) + (OS_CCM_SLICE>>2);
	stk[-3] = (uint32_t)func;// подложить в стек функцию завершения
}
/*! \brief expands to a modifiable lvalue 
#include <errno.h>
#define errno (*_errno())
*/
int* __errno()
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
	
	uint32_t* sp = NULL;//osStackAlloc(thread_def->stacksize);

	osThreadId tcb = NULL;
	thrd_create(&tcb, thread_def->pthread, args);
	tcb->priority= thread_def->tpriority;
	return tcb;
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
    svc(SVC_KILL);
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
__attribute__((noinline)) osStatus osThreadYield (void);
/*! \brief запросить переключение задач

    вызов выполняется из треда, для вызова из ISR следует использовать \b __YIELD()
 */
osStatus osThreadYield (void)
{
    svc(SVC_YIELD);
	return osOK;
}
/*! \} */


/*! эта функция предназначена для запуска из прерывания чтобы сократить задержку на обработку 
\todo сделать глобальную переменную osThreadId osThreadCurrent = current_thread;
\todo сделать функцию макросом
 */
osStatus osThreadNotify(osThreadId thread_id) PRIVILEGED_FUNCTION;
osStatus osThreadNotify(osThreadId thread_id)
{
	// \TODO вставить в очередь с учетом приоритета
	if (thread_id != current_thread) {// если всего два треда, то не надо мутить 
		// исключить из списка задач
		
/*
		volatile void** ptr = &current_thread->next;
		thread_id->next = atomic_pointer_exchange(ptr, thread_id->next);
		do {// атомарно записываем элемент в список задач
			thread_id->next = atomic_pointer_get(ptr);
			atomic_mb();
		} while (!atomic_pointer_compare_and_exchange(ptr, thread_id->next, tcb));
		__YIELD();// вызвать переключение задач на выходе из прерывания
		*/
	}
	return osOK;	
}
#if 1
int osThreadErrno (osThreadId thread_id, int err_no)
{
	return atomic_exchange(&thread_id->error_no, err_no);
}
#endif

void  PendSV_Handler(void) PRIVILEGED_FUNCTION;
__attribute__((naked)) void PendSV_Handler();
/*!  \brief переключение контекстов задач
Cortex-M3
Cortex-M4
Cortex-M4F
Cortex-M7



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
#  elif defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__)
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
unsigned int * osThreadScheduler(unsigned int * stk)
{
	current_thread->sp = stk;
	// планировщик задач
	do {
		current_thread = current_thread->next; // по круговому списку задач, всегда в списке одна задача "main"
        osEvent *event = &current_thread->process.event;
        if (event->status & osEventRunning) break;
		
        if (event->status & (osEventSignal)) 	{// ожидаем сигналы
			uint32_t signals = current_thread->process.signals & event->value.signals;
            if (signals){
				//if ((event->status & osEventWaitAll)==0 || (signals==event->value.signals))
				{
					event->value.signals =(signals);
					event->status = osEventSignal|osEventRunning;
					break;
				}
            }
        } 
#if (defined(osFeature_AsyncQueue) && (osFeature_AsyncQueue!=0))
		else
        if (event->status & (osEventAsyncQueue)){// очереди, по поведению похож на Mutex, похож на сигналы треда
			void* ptr = atomic_pointer_exchange((void**)event->value.p, NULL);
            if (ptr != NULL) { // есть объект в очереди
				event->value.p= ptr;// возвращаем указатель на объект из очереди
                event->status = osEventAsyncQueue;
                break;
            }
        }
#endif
		else
        if (event->status & (osEventSemaphore)) {// выделение памяти из пула, семафоры и мьютексы
            volatile int* ptr = event->value.p;
			int count = semaphore_enter(ptr);
			if (count > 0) { // счетчик семафора до входа
				event->value.v = count;
				event->status = osEventSemaphore|osEventRunning;
				break;
			}
        }
skip_event:
        if (event->status & (osEventTimeout)) 	{// ожидаем таймаут, можем использовать DWT->CYCCOUNT
			uint32_t system_timestamp = osKernelSysTick();
            if ((uint32_t)(system_timestamp - current_thread->process.wait.timestamp)>=current_thread->process.wait.timeout){
                // 01.02.2018 event->value.v = system_timestamp - current_thread->wait.timestamp; // возвращаем задержку
                event->status = osEventTimeout|osEventRunning;
                break;
            }
        }
	} while (1);//(current_thread->wait.events & current_thread->events)==0);// условие выхода из ожидания: событие настало
//	current_thread->event = NULL; // автоматически выключаем
	atomic_free();// Clear Exclusive Monitor
	if (current_thread->sp==NULL) {//стек не выделен!
		unsigned int *sp = osStackAlloc(0x400/*current_thread->def->stacksize*/);
		// надо ли резервировать место для S0-15, FPSCR 
			// инициализация стека R0-R3,R12,LR,PC,xPSR
		*(--sp) = (uint32_t)0x01000000;		// xPSR Thumb state
		*(--sp) = (uint32_t)current_thread->process.func;//|1;		// PC start function |1 -Thumb mode
		*(--sp) = (uint32_t)osThreadExit;//|1;	// LR stop function |1 -Thumb mode
		*(--sp) = 0; //R12
		*(--sp) = 0; //R3
		*(--sp) = 0; //R2
		*(--sp) = 0; //(uint32_t)&current_thread->process.event; //R1
		*(--sp) = (uint32_t)current_thread->process.arg; //R0
#ifdef PSP_THREAD_MODE
# if defined(__ARM_ARCH_8M_BASE__) 
		*(--sp) = (uint32_t)0xFFFFFFBC; //PSP Thread no-FPU, Cortex-M23/33 wo security extention
# else
		*(--sp) = (uint32_t)0xFFFFFFFD; //PSP Thread no-FPU, 0xFFFFFFE9 - MSP
# endif
#else
		*(--sp) = (uint32_t)0xFFFFFFF9; //MSP Thread no-FPU, 0xFFFFFFE9 - MSP
#endif
		sp-=8;// R4-R11 в любой последовательности
		current_thread->sp = sp;
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
	return current_thread->sp;
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
__attribute__((noinline)) void osEventWait (osEvent *event, uint32_t interval);
/*! \todo сделать время в микросекундах */
void osEventWait (osEvent *event, uint32_t interval)
{
	svc(SVC_EVENT_WAIT);
	*event = current_thread->process.event;// вот это копирование можно исключить!!
}
/*! \} */

//  ==== Signal Management ====
#if (defined(osFeature_Signals) && (osFeature_Signals!=0))
/*! \ingroup _system
    \defgroup _signals Signal Management
    \{
 */
int32_t osSignalSet (osThreadId thread_id, int32_t signals)
{
	// если тред не найден return 0x80000000;
	if (thread_id==NULL) return 0x80000000;
	return atomic_fetch_or((volatile int *)&thread_id->process.signals, (signals & OS_SIGNAL_MASK));
}
int32_t osSignalClear (osThreadId thread_id, int32_t signals)
{
	// если тред не найден return 0x80000000;
	if (thread_id==NULL) return 0x80000000;
	return atomic_fetch_and((volatile int *)&thread_id->process.signals, ~(signals & OS_SIGNAL_MASK));
}
#if 0
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
#if (defined(osFeature_ThreadFlags) && (osFeature_ThreadFlags!=0))
uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
//	if (flags==0) flags = ~0;
    osEvent event = {.status = osEventSignal|options,.value ={.signals = flags}};
	osEventWait(&event, timeout);//signals, (millisec), (osEventSignal|osEventTimeout));
	return (event.status & osEventTimeout)?osFlagsErrorTimeout: event.value.signals;
}
#endif
