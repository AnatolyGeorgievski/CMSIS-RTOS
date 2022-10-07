#ifndef _SYS_THREAD_H_
#define _SYS_THREAD_H_

//#define _SIG_ATOMIC_T_DEFINED
//typedef __SIG_ATOMIC_TYPE__ sig_atomic_t; // [C11] должен быть определен где-то в <signal.h>
#include <signal.h>
#include <sys/types.h>

struct _Event  {
  int32_t                 status;     ///< status code: event or error information
  union  {// \see union sigval POSIX
    uint32_t                    v;     ///< message as 32-bit value
    void                       *p;     ///< message or mail as void pointer
    sigset_t               signals;     ///< signal flags
  } value;                             ///< event value
};
struct _Process {
	volatile sig_atomic_t signals;// \sa POSIX sigset_t
	sigset_t sig_mask;//!< Блокировка сигналов пользователя \see POSIX \ref pthread_sigmask
	struct _Event event;
	struct _Wait {// \sa POSIX struct i
		uint32_t timestamp;
		uint32_t timeout;// interval
	} wait;
	void* (*func)(void * arg);
	void* arg;
	void* result;
//	pid_t pid;  уникальный идентификатор процесса
};
struct _sched { // \see  sched_param structure defined in <sched.h>
	int8_t priority;//!< Если не используется SS остается только атрибут priority 
	int8_t low_priority;
	int initial_budget;
	int repl_period;
};
#define BUDGET(t,prio) (516+(prio<<4))
struct _thread {
	void* sp;		//!< указатель стека, временно хранится пока тред не активен
	struct _thread* next;		//!< указатель на следующий блок TCB или на себя, если в списке один элемент
//	struct _thread* parent;		//!< указатель на родительский процесс
	struct _Process process;
	struct _sched   sched;
#if defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME>0)
	struct timespec CPU_time;
#endif
	int budget;
	int error_no;	//!< ошибки см POSIX errno.h
//	int8_t sig_no;		//!< Номер сигнала по случаю завершения
	int8_t priority;	//!< приоритет исполнения
	const void* attr;	//!< Атрибуты треда
	void* tss;	//!< Thread Specific Storage
};
enum {
	osEventComplete	=0, 
	osEventRunning 	=1, 
	osEventWaitAll	=0x02,
	osEventSignal   =0x04,
	osEventMail   	=0x08,
	osEventMessage	=0x10,
	osEventSemaphore=0x20, 
	osEventTimeout	=0x40, 
	osEventQueued	=0x100,
};
#ifndef osWaitForever
#define osWaitForever 0xFFFFFFFFU
#endif
/*
typedef enum {
  osOK                      =  0,thrd_success         ///< Operation completed successfully.
  osError                   = -1,thrd_error         ///< Unspecified RTOS error: run-time error but no other error message fits.
  osErrorTimeout            = -2,thrd_timedout  ETIMEDOUT       ///< Operation not completed within the timeout period.
  osErrorResource           = -3,         ///< Resource not available.
  osErrorParameter          = -4,         EIVAL///< Parameter error.
  osErrorNoMemory           = -5,thrd_nomem         ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorISR                = -6,         ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  osStatusReserved          = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osStatus_t;
*/
#endif