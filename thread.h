#ifndef THREAD_H
#define THREAD_H
#include <cmsis_os.h>
#include <time.h>
#include <threads.h>
//#include "queue.h"

#include <stdnoreturn.h>


//  typedef long clock_t;
#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
typedef uint32_t clock_t; // [C11] должен быть определен где-то в <time.h>
#endif // _CLOCK_T_DEFINED
#ifndef _SIG_ATOMIC_T_DEFINED
#define _SIG_ATOMIC_T_DEFINED
typedef volatile uint32_t sig_atomic_t; // [C11] должен быть определен где-то в <signal.h>
#endif // _SIG_ATOMIC_T_DEFINED

struct _Wait {
	uint32_t timestamp;	// когда началось ожидание
	uint32_t timeout;	// таймаут ожидания сигналов или 0
};
typedef struct _Process osProcess_t;
struct _Process {
	sig_atomic_t signals;
	osEvent event;
	struct {
		uint32_t timestamp;
		uint32_t timeout;
	} wait;
	int (*func)(void * arg);
	void* arg;
	int result;
	uint32_t pid;
};
struct _Service {
	struct _Process process;
	struct _Service * next;// список не должен присутствовать явным образом!!!
};

typedef struct os_thread_cb TCB;
struct os_thread_cb {
	void* sp;		//!< указатель стека, временно хранится пока тред неактивен
	TCB* next;		//!< указатель на следующий блок TCB или на себя, если в списке один элемент
	struct _Process process;
//	TCB* parent;    //!< кто его породил
    //osEvent* event; //!< NULL -- ожидание событиия \see [c11] struct sigevent \sa OpenCL
	int error_no;// ошибки см POSIX
	osPriority 		priority;	//!< приоритет исполнения
	const osThreadDef_t* def;
	tss_t tss;
};
//#define TCB_FROM_QUEUE(lst) ((TCB*)((void*)lst - __builtin_offsetof(TCB, queue_next)))
typedef int (*thrd_start_t)(void *);
#if 0 // все это определено в threads.h
typedef struct os_thread_cb *thrd_t;
enum {
	thrd_success =0,
	thrd_timedout,
	thrd_error,
	thrd_busy,
	thrd_nomem,
};

int  thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
thrd_t thrd_current(void);
int  thrd_detach(thrd_t thr);
int  thrd_equal(thrd_t thr0, thrd_t thr1);
_Noreturn void thrd_exit(int res);
int  thrd_join(thrd_t thr, int *res);
int  thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void thrd_yield(void);

typedef struct os_mutex_cb mtx_t;

int  mtx_init(mtx_t *mtx, int type);
int  mtx_lock(mtx_t *mtx);
void mtx_destroy(mtx_t *mtx);
int  mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts);
int  mtx_trylock(mtx_t *mtx);
int  mtx_unlock (mtx_t *mtx);

typedef struct os_cond_cb cnd_t;
int cnd_broadcast(cnd_t *cond);
void cnd_destroy(cnd_t *cond);
int cnd_init(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts);
int cnd_wait(cnd_t *cond, mtx_t *mtx);
#endif //0
#endif // THREAD_H
