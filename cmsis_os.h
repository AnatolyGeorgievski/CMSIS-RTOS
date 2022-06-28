/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.h template header file
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/

/*!
    \ingroup _system
    \defgroup _cmsis_os_h Header File Template: cmsis_os.h

The file \b cmsis_os.h is a template header file for a CMSIS-RTOS compliant Real-Time Operating System (RTOS).
Each RTOS that is compliant with CMSIS-RTOS shall provide a specific \b cmsis_os.h header file that represents
its implementation.

The file cmsis_os.h contains:
 - CMSIS-RTOS API function definitions
 - struct definitions for parameters and return types
 - status and priority values used by CMSIS-RTOS API functions
 - macros for defining threads and other kernel objects


<b>Name conventions and header file modifications</b>

All definitions are prefixed with \b os to give an unique name space for CMSIS-RTOS functions.
Definitions that are prefixed \b os_ are not used in the application code but local to this header file.
All definitions and functions that belong to a module are grouped and have a common prefix, i.e. \b osThread.

Definitions that are marked with <b>CAN BE CHANGED</b> can be adapted towards the needs of the actual CMSIS-RTOS implementation.
These definitions can be specific to the underlying RTOS kernel.

Definitions that are marked with <b>MUST REMAIN UNCHANGED</b> cannot be altered. Otherwise the CMSIS-RTOS implementation is no longer
compliant to the standard. Note that some functions are optional and need not to be provided by every CMSIS-RTOS implementation.


<b>Function calls from interrupt service routines</b>

The following CMSIS-RTOS functions can be called from threads and interrupt service routines (ISR):
  - \ref osSignalSet
  - \ref osSemaphoreRelease
  - \ref osPoolAlloc, \ref osPoolCAlloc, \ref osPoolFree
  - \ref osMessagePut, \ref osMessageGet
  - \ref osMailAlloc, \ref osMailCAlloc, \ref osMailGet, \ref osMailPut, \ref osMailFree

Functions that cannot be called from an ISR are verifying the interrupt status and return in case that they are called
from an ISR context the status code \b osErrorISR. In some implementations this condition might be caught using the HARD FAULT vector.

Some CMSIS-RTOS implementations support CMSIS-RTOS function calls from multiple ISR at the same time.
If this is impossible, the CMSIS-RTOS rejects calls by nested ISR functions with the status code \b osErrorISRRecursive.


<b>Define and reference object definitions</b>

With <b>\#define osObjectsExternal</b> objects are defined as external symbols. This allows to create a consistent header file
that is used throughout a project as shown below:

<i>Header File</i>
\code
#include <cmsis_os.h>                                         // CMSIS RTOS header file

// Thread definition
extern void thread_sample (void const *argument);             // function prototype
osThreadDef (thread_sample, osPriorityBelowNormal, 1, 100);

// Pool definition
osPoolDef(MyPool, 10, long);
\endcode


This header file defines all objects when included in a C/C++ source file. When <b>\#define osObjectsExternal</b> is
present before the header file, the objects are defined as external symbols. A single consistent header file can therefore be
used throughout the whole project.

<i>Example</i>
\code
#include "osObjects.h"     // Definition of the CMSIS-RTOS objects

\endcode

\code
#define osObjectExternal   // Objects will be defined as external symbols
#include "osObjects.h"     // Reference to the CMSIS-RTOS objects

\endcode

\{
*/

#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H
#ifndef _WIN32
#include "board.h"
#endif // _WIN32
/// \note MUST REMAIN UNCHANGED: \b osCMSIS identifies the CMSIS-RTOS API version.
#define osCMSIS           0x10002      ///< API version (main [31:16] .sub [15:0])

/// \note CAN BE CHANGED: \b osCMSIS_KERNEL identifies the underlying RTOS kernel and version number.
#define osCMSIS_KERNEL    0x10000	   ///< RTOS identification and version (main [31:16] .sub [15:0])

/// \note MUST REMAIN UNCHANGED: \b osKernelSystemId shall be consistent in every CMSIS-RTOS.
#define osKernelSystemId "R3 RTOS V22.06"   ///< RTOS identification string

/// \note MUST REMAIN UNCHANGED: \b osFeature_xxx shall be consistent in every CMSIS-RTOS.
#define osFeature_MainThread   1       ///< main thread      1=main can be thread, 0=not available
#define osFeature_Modules      1       ///< R3 модули
#define osFeature_Services     1       ///< R3 сервисы, процессы
#define osFeature_OpenCL       0       ///< R3 модули
#define osFeature_ThreadCount  8       ///< R3 максимальное число тредов в системе
#define osFeature_ThreadPool   0       ///< R3 osThreadPool functions: 1=available, 0=not available
#define osFeature_Condition    0       ///< R3 osCond functions: 1=available, 0=not available
#define osFeature_AsyncQueue   1       ///< R3 osAsyncQueue functions: 1=available, 0=not available
#define osFeature_Pool         1       ///< Memory Pools:    1=available, 0=not available
#define osFeature_MailQ        0       ///< Mail Queues:     1=available, 0=not available
#define osFeature_MessageQ     0       ///< Message Queues:  1=available, 0=not available
#define osFeature_Signals      32      ///< maximum number of Signal Flags available per thread
#define osFeature_Semaphore    30      ///< maximum count for \ref osSemaphoreCreate function
#define osFeature_Wait         1       ///< osWait function: 1=available, 0=not available
#define osFeature_SysTick      1       ///< osKernelSysTick functions: 1=available, 0=not available

#define PRIVILEGED_FUNCTION __attribute__((section(".text.privileged")))

#include <stdint.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C"
{
#endif


// ==== Enumeration, structures, defines ====

/// Priority used for thread control.
/// \note MUST REMAIN UNCHANGED: \b osPriority shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;

/// Timeout value.
/// \note MUST REMAIN UNCHANGED: \b osWaitForever shall be consistent in every CMSIS-RTOS.
#define osWaitForever     0xFFFFFFFF     ///< wait forever timeout value

/// Status code values returned by CMSIS-RTOS functions.
/// \note MUST REMAIN UNCHANGED: \b osStatus shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osOK                    =     0,       ///< function completed; no error or event occurred.
  osEventComplete		  =     0,		 ///< R3 событие завершения команды, в параметрах события передается код завершения
  osEventRunning		  =     1,		 ///< R3 событие обрабатывается в очередь обработки
  osEventQueued			  =     2,		 ///< R3 событие поставлено в очередь обработки
//  osEventCondition		  =  0x7FFF0000,		 ///< R3 condition mask.
  osEventWaitAll          =  0x04,
  osEventSignal           =  0x08,       ///< function completed; signal event occurred.
  osEventMessage          =  0x10,       ///< function completed; message event occurred.
  osEventMail             =  0x20,       ///< function completed; mail event occurred.
  osEventTimeout          =  0x40,       ///< function completed; timeout occurred.

  osErrorParameter        =  0x80,       ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
  osErrorResource         =  0x81,       ///< resource not available: a specified resource was not available.
  osErrorTimeoutResource  =  0xC1,       ///< resource not available within given time: a specified resource was not available within the timeout period.
  osErrorISR              =  0x82,       ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
  osErrorISRRecursive     =  0x83,       ///< function called multiple times from ISR with same object.
  osErrorPriority         =  0x84,       ///< system cannot determine priority or thread has illegal priority.
  osErrorNoMemory         =  0x85,       ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorValue            =  0x86,       ///< value of a parameter is out of range.
  osErrorOS               =  0xFF,       ///< unspecified RTOS error: run-time error but no other error message fits.
  osEventSemaphore        =  0x100,		 ///< function completed; signal event occurred.
  osEventRecursive        =  0x200,		 ///< R3 mutex+recursive
  osEventAsyncQueue		  =  0x400,
  osEventService		  =  0x800,

  os_status_reserved      =  0x7FFFFFFF  ///< prevent from enum down-size compiler optimization.
} osStatus;
// Flags options (\ref osThreadFlagsWait and \ref osEventFlagsWait).
#define osFlagsWaitAny        0x00000000U ///< Wait for any flag (default).
#define osFlagsWaitAll        0x00000001U ///< Wait for all flags.
#define osFlagsNoClear        0x00000002U ///< Do not clear flags which have been specified to wait for.

// Flags errors (returned by osThreadFlagsXxxx and osEventFlagsXxxx).
#define osFlagsError          0x80000000U ///< Error indicator.
#define osFlagsErrorUnknown   0xFFFFFFFFU ///< osError (-1).
#define osFlagsErrorTimeout   0xFFFFFFFEU ///< osErrorTimeout (-2).
#define osFlagsErrorResource  0xFFFFFFFDU ///< osErrorResource (-3).
#define osFlagsErrorParameter 0xFFFFFFFCU ///< osErrorParameter (-4).
#define osFlagsErrorISR       0xFFFFFFFAU ///< osErrorISR (-6).


/// Timer type value for the timer definition.
/// \note MUST REMAIN UNCHANGED: \b os_timer_type shall be consistent in every CMSIS-RTOS.
typedef enum  {
  osTimerOnce             =     0,       ///< one-shot timer
  osTimerPeriodic         =     1        ///< repeating timer
} os_timer_type;

/// Entry point of a thread.
/// \note MUST REMAIN UNCHANGED: \b os_pthread shall be consistent in every CMSIS-RTOS.
typedef int (*os_pthread) (void *argument);

/// Entry point of a timer call back function.
/// \note MUST REMAIN UNCHANGED: \b os_ptimer shall be consistent in every CMSIS-RTOS.
typedef void (*os_ptimer) (void *argument);

// >>> the following data type definitions may shall adapted towards a specific RTOS

/// Thread ID identifies the thread (pointer to a thread control block).
/// \note CAN BE CHANGED: \b os_thread_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_thread_cb *osThreadId;

/// Timer ID identifies the timer (pointer to a timer control block).
/// \note CAN BE CHANGED: \b os_timer_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_timer_cb *osTimerId;

/// Mutex ID identifies the mutex (pointer to a mutex control block).
/// \note CAN BE CHANGED: \b os_mutex_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_mutex_cb *osMutexId;
//typedef struct os_mutex_cb osStaticMutex_t;
//#define osSTATIC_MUTEX_INIT {1}

/// Semaphore ID identifies the semaphore (pointer to a semaphore control block).
/// \note CAN BE CHANGED: \b os_semaphore_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_semaphore_cb *osSemaphoreId;

/// Pool ID identifies the memory pool (pointer to a memory pool control block).
/// \note CAN BE CHANGED: \b os_pool_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_cb *osPoolId;

/// Message ID identifies the message queue (pointer to a message queue control block).
/// \note CAN BE CHANGED: \b os_messageQ_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_messageQ_cb *osMessageQId;


/// Mail ID identifies the mail queue (pointer to a mail queue control block).
/// \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_cb *osMailQId;


/// Thread Definition structure contains startup information of a thread.
/// \note CAN BE CHANGED: \b os_thread_def is implementation specific in every CMSIS-RTOS.
typedef struct os_thread_def  {
  char*	name;
  os_pthread               pthread;    ///< start address of thread function
  osPriority             tpriority;    ///< initial thread priority
  uint32_t               instances;    ///< maximum number of instances of that thread function
  uint32_t               stacksize;    ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;

/// Timer Definition structure contains timer parameters.
/// \note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
typedef struct os_timer_def  {
  os_ptimer                 ptimer;    ///< start address of a timer function
} osTimerDef_t;

/// Mutex Definition structure contains setup information for a mutex.
/// \note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
typedef struct os_mutex_def  {
  void*                   dummy;    ///< dummy value.
} osMutexDef_t;

/// Semaphore Definition structure contains setup information for a semaphore.
/// \note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
typedef struct os_semaphore_def  {
	volatile int count;    ///< R3 значение семафора
} osSemaphoreDef_t;

/// Definition structure for memory block allocation.
/// \note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_def  {
  uint32_t                 pool_sz;    ///< number of items (elements) in the pool
  uint32_t                 item_sz;    ///< size of an item
  void                       *pool;    ///< pointer to memory for pool, можно выделять память под пул статически pool_sz*item_sz не используется
  void				          *map;
} osPoolDef_t;

/// Definition structure for message queue.
/// \note CAN BE CHANGED: \b os_messageQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_def osMessageQDef_t;

/// Definition structure for mail queue.
/// \note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_def osMailQDef_t;

/// Event structure contains detailed information about an event.
/// \note MUST REMAIN UNCHANGED: \b os_event shall be consistent in every CMSIS-RTOS.
///       However the struct may be extended at the end.
typedef struct  {
  osStatus                 status;     ///< status code: event or error information
  union  {
    uint32_t                    v;     ///< message as 32-bit value
    void                       *p;     ///< message or mail as void pointer
    int32_t               signals;     ///< signal flags
  } value;                             ///< event value
#if 0 // R3 забить
  union  {
    osMailQId             mail_id;     ///< mail id obtained by \ref osMailCreate
    osMessageQId       message_id;     ///< message id obtained by \ref osMessageCreate
  } def;                               ///< event definition
#endif
} osEvent;

typedef struct _GCond           GCond;
struct _GCond
{
};

struct _DataUnit {
	uint8_t* buffer;
	uint16_t size;
	uint8_t status;
	uint8_t device_id;// MSTP
//	struct _DataUnit *next;
//	void*    destination;// нужно чтобы ждать
//	volatile int refcount;
};
//int DataUnitRef(struct _DataUnit * tr);
//int DataUnitUnref(struct _DataUnit * tr);

//  ==== Kernel Control Functions ====

/// Initialize the RTOS Kernel for creating objects.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
osStatus osKernelInitialize (void);

/// Start the RTOS Kernel.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
osStatus osKernelStart (void);

/// Check if the RTOS kernel is already started.
/// \note MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
/// \return 0 RTOS is not started, 1 RTOS is started.
int32_t osKernelRunning(void);

#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available

/// Get the RTOS kernel system timer counter
/// \note MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
/// \return RTOS kernel system timer as 32-bit value
uint32_t osKernelSysTick (void);


/// The RTOS kernel system timer frequency in Hz
/// \note Reflects the system timer setting and is typically defined in a configuration file.
#define osKernelSysTickFrequency (1000000)

/// Convert a microseconds value to a RTOS kernel system timer value.
/// \param         microsec     time value in microseconds.
/// \return time value normalized to the \ref osKernelSysTickFrequency
#ifndef osKernelSysTickMicroSec
#define osKernelSysTickMicroSec(microsec) (microsec)
#endif // osKernelSysTickMicroSec

#endif    // System Timer available
#if defined(__arm__) && (defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__))
static inline void debug (const char * str){
//  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
//      ((ITM->TER & 1UL               ) != 0UL)   )     /* ITM Port #0 enabled */
	while(1) {
		if (ITM->PORT[0].u32!=0){
			if (str[0]=='\0') break;
			ITM->PORT[0].u8 = *str++;
		}
	}
}
#else
	void debug (const char * str);
#endif // __arm__
void osEventWait (osEvent *event, uint32_t millisec);


//  ==== Thread Management ====

/// Create a Thread Definition with function, priority, and stack requirements.
/// \param         name         name of the thread function.
/// \param         priority     initial priority of the thread function.
/// \param         instances    number of possible thread instances.
/// \param         stacksz      stack size (in bytes) requirements for the thread function.
/// \note CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osThreadDef(name, priority, instances, stacksz)  \
extern const osThreadDef_t os_thread_def_##name
#else                            // define the object
#define osThreadDef(name, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (name), (priority), (instances), (stacksz)  }
#endif

/// Access a Thread definition.
/// \param         name          name of the thread definition object.
/// \note CAN BE CHANGED: The parameter to \b osThread shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osThread(name)  \
&os_thread_def_##name
/*! \brief Создать тред
	\param[in]	event	указатель на событие, которое заполняется статусом треда osEventRunning, osEventComplete
 */
osThreadId osThreadCreateEx (osThreadId thread, const osThreadDef_t *thread_def, void *args/* , osEvent* event*/);

/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
//static inline
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument);
/*{
	return osThreadCreateEx(thread_def, argument, NULL);
}*/

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
osThreadId osThreadGetId (void);

/// Terminate execution of a thread and remove it from Active Threads.
/// \param[in]     thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
osStatus osThreadTerminate (osThreadId thread_id);
/// Terminate execution of the current thread and remove it from Active Threads.
_Noreturn void thrd_exit(int res);
static inline void osThreadExit() {thrd_exit(0);}

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
osStatus osThreadYield (void);
/// Change priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);

/// Get current priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return current priority value of the thread function.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
osPriority osThreadGetPriority (osThreadId thread_id);


osStatus osThreadNotify(osThreadId thread_id);
/// \deprecated изменить значение ошибки для указанного треда, атомарно читает содержимое
int osThreadErrno(osThreadId thread_id, int errno);// I2C __attribute__((deprecated));
/*!	\brief ждать завершения процесса
	\param[in]	event 	используется событие определенное при запуске \b osThreadCreateEx()
	\param[in]	millisec -- ожидание ограничено по времени, задается  в милесекундах. Значение 0 - не ограничено.
	\return статус завершения или код ошибки. Возможные значения статуса: \b osEventComplete == \b osOK, \b osEventTimeout,
	или код ошибки 0x80 - 0xFF, выставлен бит \b osErrorParameter.
 */
static inline osStatus osThreadJoin (osEvent* event, uint32_t millisec)
{
	if (event==NULL) return osErrorParameter;
	osEventWait (event, millisec);// выход происходит по (event->status === osEventComplete)
	return event->status;
}
/* Предложение по развитию системы:
добавить функции для потока
*/
typedef struct os_thread_pool_cb *osThreadPoolId;
typedef struct os_event_cb *osEventId; // полностью приватное определение
typedef struct os_thread_pool_def  {
	uint16_t    instances;    ///< количество резервируемых тайм слотов(стеков) для параллельного исполнения тредов
	uint16_t	queue_length;	///< максимальная длина очереди команд
	osEventId* events;
} osThreadPoolDef_t;
#define osThreadPoolDef(name, count, length) \
	static volatile int os_thpool_eventgroups_##name[length]; \
	const osThreadPoolDef os_thpool_def_##name = {count, length};

//!	\brief выделить, резервировать несколько стеков для обработки
osThreadPoolId osThreadPoolCreate(const osThreadPoolDef_t*);
/*! \brief
	\param[in]	event - условие по которому производится запуск
	\return		событие по которому можно отслеживать готовность треда, не требует удаления
 */
osEvent* osThreadPoolEnqueue(osThreadPoolId , const osThreadDef_t*,  void* argument, osEvent* event);
osStatus  osEventGroupAppend(osEvent*, osEvent*);

//typedef struct os_cond_cb *osCondId;
typedef uint32_t osCondId;
typedef struct os_cond_def  {
	uint16_t flag_idx;
} osCondDef_t;
osCondId osCondCreate(const osCondDef_t *);
osStatus osCondClear (osCondId);
osStatus osCondSignal(osCondId);
osStatus osCondDelete(osCondId);
osStatus osMutexCondWait (osMutexId, osCondId, uint32_t millisec);
int osOnceInitEnter(volatile int * flag);
void osOnceInitLeave(volatile int * flag, int value);
/*
Есть еще два кандидата:
BitLock
RWLock
*/


//  ==== Generic Wait Functions ====
/*! Добавить событие в группу */
/*
osEvent event_list[4];
osKernel(,,&event_list[3]);
osEventGroupWait()

static inline osStatus osEventGroupWait (const osEvent* event_group, uint32_t num_events, uint32_t millisec)
{
    osEvent event = {.status = osEventGroup, .value.p = event_group};
	osEventWait(&event, millisec);
	return event.status;
}
*/
/// Wait for Timeout (Time Delay).
/// \param[in]     millisec      time delay value
/// \return status code that indicates the execution status of the function.
static inline osStatus osDelay (uint32_t millisec)
{
    osEvent event = {.status = osEventTimeout};
	osEventWait(&event, millisec);
	return osOK;
}
/*! \brief нижний уровень для функций osWait и osSignalWait */

#if (defined (osFeature_Services)  &&  (osFeature_Services != 0))
typedef struct _Process osService_t;// оставляем для совместимости, надо вычистить код
typedef struct _Process osProcess_t;
typedef struct _osSignalRef osSignalRef_t;

void*           osServiceInit  (uint32_t process_id);
osStatus      	osServiceRunAs (osProcess_t *svc, void*);
osStatus        osServiceRun   (osProcess_t *svc);
osStatus        osServiceBind  (osProcess_t *svc, int socket, uint32_t mask);// нужна маска какие сигналы разрешить (read, write, err...)
osProcess_t*    osServiceCreate(osProcess_t *svc, int(*func)(osProcess_t *, void *), void* arg);
osEvent*        osServiceGetEvent(osProcess_t *svc);
uint32_t        osServiceGetSignals(osProcess_t *svc);
void*        	osServiceSetData(osProcess_t *svc, void* arg);
uint32_t        osServiceSignal (osProcess_t *svc, uint32_t signal);
osSignalRef_t   osServiceSignalRef(osProcess_t *svc, uint32_t signals);
void osServiceTimerStart   (osProcess_t* svc, uint32_t Timeout);
void osServiceTimerStop    (osProcess_t *svc);
void osServiceTimerRestart (osProcess_t* svc);
int  osServiceTimerExpired (osProcess_t *svc, uint32_t timeout);

#endif
#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0))     // Generic Wait available

/// Wait for Signal, Message, Mail, or Timeout.
/// \param[in] millisec          timeout value or 0 in case of no time-out
/// \return event that contains signal, message, or mail information or error code.
/// \note MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
static inline osEvent osWait (uint32_t millisec)
{
    osEvent event = {.status = (osEventSignal/*|osEventSemaphore|osEventMessage|osEventMail*/|osEventTimeout)};
	osEventWait(&event, millisec);
	return  event;
}

#endif  // Generic Wait available


//  ==== Timer Management Functions ====
/// Define a Timer object.
/// \param         name          name of the timer object.
/// \param         function      name of the timer call back function.
/// \note CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function)  \
extern const osTimerDef_t os_timer_def_##name
#else                            // define the object
#define osTimerDef(name, function)  \
const osTimerDef_t os_timer_def_##name = \
{ (function) }
#endif

/// Access a Timer definition.
/// \param         name          name of the timer object.
/// \note CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osTimer(name) \
&os_timer_def_##name

/// Create a timer.
/// \param[in]     timer_def     timer object referenced with \ref osTimer.
/// \param[in]     type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
/// \param[in]     argument      argument to the timer call back function.
/// \return timer ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument);

/// Start or restart a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \param[in]     millisec      time delay value of the timer.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
osStatus osTimerStartMicroSec (osTimerId timer_id, uint32_t microsec, uint32_t delay);
static inline osStatus osTimerStart (osTimerId timer_id, uint32_t millisec){
	return osTimerStartMicroSec(timer_id, millisec*1000, 0);
}

/// Stop the timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
osStatus osTimerStop (osTimerId timer_id);

/// Delete a timer that was created by \ref osTimerCreate.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osTimerDelete shall be consistent in every CMSIS-RTOS.
osStatus osTimerDelete (osTimerId timer_id);


//  ==== Signal Management ====

/// Set the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that should be set.
/// \return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
int32_t osSignalSet (osThreadId thread_id, int32_t signals);
static inline
int32_t osSignalGet (osThreadId thread_id) {	return osSignalSet(thread_id, 0);  }

/// Clear the specified Signal Flags of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     signals       specifies the signal flags of the thread that shall be cleared.
/// \return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSignalClear shall be consistent in every CMSIS-RTOS.
int32_t osSignalClear (osThreadId thread_id, int32_t signals);

/// Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
/// \param[in]     signals       wait until all specified signal flags set or 0 for any single signal flag.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return event flag information or error code.
/// \note MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
static inline osEvent osSignalWait (int32_t signals, uint32_t millisec)
{
	if (signals==0) signals = ~0;
    osEvent event = {.status = osEventSignal,.value ={.signals = signals}};
	osEventWait(&event, millisec);//signals, (millisec), (osEventSignal|osEventTimeout));
	return  event;
}
/*! Возвращает ссылку на бит в области bit-band */
//int32_t* osSignalRef (osThreadId thread_id, int32_t signal); -- убрать deprecated
//  ==== Mutex Management ====

/// Define a Mutex.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name)  \
extern const osMutexDef_t os_mutex_def_##name
#else                            // define the object
#define osMutexDef(name)  \
static void* os_mutex_id_##name; \
const osMutexDef_t os_mutex_def_##name = { &os_mutex_id_##name }
#endif

/// Access a Mutex definition.
/// \param         name          name of the mutex object.
/// \note CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMutex(name)  \
&os_mutex_def_##name

/// Create and Initialize a Mutex object.
/// \param[in]     mutex_def     mutex definition referenced with \ref osMutex.
/// \return mutex ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
osMutexId osMutexCreate (const osMutexDef_t *mutex_def);

/// Wait until a Mutex becomes available.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec);

/// Release a Mutex that was obtained by \ref osMutexWait.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
osStatus osMutexRelease (osMutexId mutex_id);

/// Delete a Mutex that was created by \ref osMutexCreate.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
osStatus osMutexDelete (osMutexId mutex_id);


//  ==== Semaphore Management Functions ====

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))     // Semaphore available

/// Define a Semaphore object.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osSemaphoreDef(name)  \
extern osSemaphoreDef_t os_semaphore_def_##name
#else                            // define the object
#define osSemaphoreDef(name)  \
osSemaphoreDef_t os_semaphore_def_##name
#endif

/// Access a Semaphore definition.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphore(name)  \
&os_semaphore_def_##name

/// Create and Initialize a Semaphore object used for managing resources.
/// \param[in]     semaphore_def semaphore definition referenced with \ref osSemaphore.
/// \param[in]     count         number of available resources.
/// \return semaphore ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);

/// Wait until a Semaphore token becomes available.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return number of available tokens, or -1 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);

/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);

/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id);

#endif     // Semaphore available


//  ==== Memory Pool Management Functions ====

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  // Memory Pool Management available

/// \brief Define a Memory Pool.
/// \param         name          name of the memory pool.
/// \param         no            maximum number of blocks (objects) in the memory pool.
/// \param         type          data type of a single block (object).
/// \note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osPoolDef(name, no, type)   \
extern const osPoolDef_t os_pool_def_##name
#else                            // define the object
#define osPoolDef(name, no, type)   \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type)}
/// \brief Define a Static Memory Pool.
/// \param         name          name of the memory pool.
/// \param         size          максимальный размер памяти выделенный для пула
/// \param         slice         размер блока памяти
/// \param         base          начальный адрес пула
/// \note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osPoolStaticDef(name, size, slice, base)   \
static unsigned int os_pool_map_##name[(size)/(slice*sizeof(unsigned int)*8) + 4];\
const osPoolDef_t os_pool_def_##name = \
{ (size/slice), (slice), (base), os_pool_map_##name }
#endif

/// \brief Access a Memory Pool definition.
/// \param         name          name of the memory pool
/// \note CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osPool(name) \
&os_pool_def_##name

/// Create and Initialize a memory pool.
/// \param[in]     pool_def      memory pool definition referenced with \ref osPool.
/// \return memory pool ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
osPoolId osPoolCreate (const osPoolDef_t *pool_def);

/// Allocate a memory block from a memory pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
/// \note MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
void *osPoolAlloc (osPoolId pool_id);

/// Allocate a memory block from a memory pool and set memory block to zero.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \return address of the allocated memory block or NULL in case of no memory available.
/// \note MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
void *osPoolCAlloc (osPoolId pool_id);

/// Return an allocated memory block back to a specific memory pool.
/// \param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
/// \param[in]     block         address of the allocated memory block that is returned to the memory pool.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
osStatus osPoolFree (osPoolId pool_id, void *block);

#endif   // Memory Pool Management available


//  ==== Message Queue Management Functions ====

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0))     // Message Queues available

/// \brief Create a Message Queue Definition.
/// \param         name          name of the queue.
/// \param         queue_sz      maximum number of messages in the queue.
/// \param         type          data type of a single message element (for debugger).
/// \note CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMessageQDef(name, queue_sz, type)   \
extern const osMessageQDef_t os_messageQ_def_##name
#else                            // define the object
#define osMessageQDef(name, queue_sz, type)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type)  }
#endif

/// \brief Access a Message Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMessageQ(name) \
&os_messageQ_def_##name

/// Create and Initialize a Message Queue.
/// \param[in]     queue_def     queue definition referenced with \ref osMessageQ.
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return message queue ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id);

/// Put a Message to a Queue.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     info          message information.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);

/// Get a Message or Wait for a Message from a Queue.
/// \param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return event information that includes status code.
/// \note MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);

#endif     // Message Queues available
//osStatus osAsyncQueueInit(osAsyncQueueId queue_id);
typedef struct os_AsyncQ_cb {
    void* head;
    void* tail;
} osAsyncQueue_t;
static inline int      osAsyncQueueEmpty(osAsyncQueue_t* queue_id){
    return queue_id->head==NULL && queue_id->tail==NULL;
}
void     osAsyncQueuePut (osAsyncQueue_t* queue_id, void* data);
void*    osAsyncQueueGet (osAsyncQueue_t* queue_id);
void     osAsyncQueueInit(osAsyncQueue_t* queue_id);
//osEvent  osAsyncQueueGet (osAsyncQueueId queue_id, uint32_t millisec);

//  ==== Mail Queue Management Functions ====

#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))     // Mail Queues available

/// \brief Create a Mail Queue Definition.
/// \param         name          name of the queue
/// \param         queue_sz      maximum number of messages in queue
/// \param         type          data type of a single message element
/// \note CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#if defined (osObjectsExternal)  // object is external
#define osMailQDef(name, queue_sz, type) \
extern const osMailQDef_t os_mailQ_def_##name
#else                            // define the object
#define osMailQDef(name, queue_sz, type) \
const osMailQDef_t os_mailQ_def_##name =  \
{ (queue_sz), sizeof (type) }
#endif

/// \brief Access a Mail Queue Definition.
/// \param         name          name of the queue
/// \note CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osMailQ(name)  \
&os_mailQ_def_##name

/// Create and Initialize mail queue.
/// \param[in]     queue_def     reference to the mail queue definition obtain with \ref osMailQ
/// \param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
/// \return mail queue ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id);

/// \brief Allocate a memory block from a mail.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
void *osMailAlloc (osMailQId queue_id, uint32_t millisec);

/// Allocate a memory block from a mail and set memory block to zero.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return pointer to memory block that can be filled with mail or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osMailCAlloc shall be consistent in every CMSIS-RTOS.
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec);

/// \brief Put a mail to a queue.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
osStatus osMailPut (osMailQId queue_id, void *mail);

/// \brief Get a mail from a queue.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out
/// \return event that contains mail information or error code.
/// \note MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
osEvent osMailGet (osMailQId queue_id, uint32_t millisec);

/// \brief Free a memory block from a mail.
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \param[in]     mail          pointer to the memory block that was obtained with \ref osMailGet.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
osStatus osMailFree (osMailQId queue_id, void *mail);

#endif  // Mail Queues available

// выделение памяти блоками большого размера
//void* mb_alloc(uint32_t size);
//void  mb_free(void* ptr, uint32_t size);

#ifdef  __cplusplus
}
#endif
//! \}
#endif  // _CMSIS_OS_H
