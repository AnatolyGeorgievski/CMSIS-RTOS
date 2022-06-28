/*! \brief C11 threads */
#ifndef MACHINE_THREADS_H
#define MACHINE_THREADS_H
#include <stdint.h>
typedef struct os_cond_cb cnd_t;
typedef volatile int once_flag;
typedef struct os_mutex_cb mtx_t;
typedef struct os_tss * tss_t;
typedef struct os_thread_cb *thrd_t;


#define __BEGIN_DECLS
#define __requires_exclusive(a)
#define __locks_exclusive(a)
#define __unlocks(a)
#define __trylocks_exclusive(a,b)
#define __requires_unlocked(a)
#define __END_DECLS
#endif // MACHINE_THREADS_H
