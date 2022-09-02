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
<<<<<<< HEAD
	sigset_t sig_mask;//!< Блокировка сигналов пользователя \see POSIX \ref pthread_sigmask
	struct _Event event;
	struct _Wait {// \sa POSIX struct i
=======
	struct _Event event;
	struct {// \sa POSIX struct i
>>>>>>> 519f8ee62cb3b565a00b6a8405ac0d6476ca0071
		uint32_t timestamp;
		uint32_t timeout;// interval
	} wait;
	void* (*func)(void * arg);
	void* arg;
	void* result;
//	pid_t pid;  уникальный идентификатор процесса
};
struct _thread {
	void* sp;		//!< указатель стека, временно хранится пока тред неактивен
	struct _thread* next;		//!< указатель на следующий блок TCB или на себя, если в списке один элемент
	struct _thread* parent;		//!< указатель на родительский процесс
	struct _Process process;
	int error_no;	//!< ошибки см POSIX errno.h
	int8_t sig_no;		//!< Номер сигнала по случаю завершения
	int8_t priority;	//!< приоритет исполнения
	const void* attr;	//!< Атрибуты треда
	void* tss;	//!< Thread Specific Storage
<<<<<<< HEAD
=======
	sigset_t sig_mask;//!< Блокировка сигналов пользователя \see POSIX \ref pthread_sigmask
>>>>>>> 519f8ee62cb3b565a00b6a8405ac0d6476ca0071
};
enum {
	osEventComplete	=0, 
	osEventRunning 	=1, 
<<<<<<< HEAD
	osEventWaitAll	=0x02,
	osEventSignal   =0x04,
	osEventMail   	=0x08,
	osEventMessage	=0x10,
	osEventSemaphore=0x20, 
	osEventTimeout	=0x40, 
	osEventQueued	=0x100,
=======
	osEventQueued	=2,
	osEventWaitAll	=0x04,
	osEventSignal   =0x08,
	osEventMessage	=0x10,
	osEventSemaphore=0x20, 
	osEventTimeout	=0x40, 
>>>>>>> 519f8ee62cb3b565a00b6a8405ac0d6476ca0071
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