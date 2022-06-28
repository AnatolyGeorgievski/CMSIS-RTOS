

mutex_wait()
if(atomic_int_add(1)!=0) wait(&ptr, mask, );

mutex_unlock()
atomic_int_add(~0);

static inline void
__mutex_fastpath_lock(atomic_t *count, void (*fail_fn)(atomic_t *))
{
        int __ex_flag, __res;

        __asm__ (

                "ldrex  %0, [%2]        \n\t"
                "sub    %0, %0, #1      \n\t"
                "strex  %1, %0, [%2]    "

                : "=&r" (__res), "=&r" (__ex_flag)
                : "r" (&(count)->counter)
                : "cc","memory" );

        __res |= __ex_flag;
        if (unlikely(__res != 0))
                fail_fn(count);
}
static inline void
__mutex_fastpath_unlock(atomic_t *count, void (*fail_fn)(atomic_t *))
{
	int __ex_flag, __res, __orig;
	__asm__ (
 78 
 79                 "ldrex  %0, [%3]        \n\t"
 80                 "add    %1, %0, #1      \n\t"
 81                 "strex  %2, %1, [%3]    "
 82 
 83                 : "=&r" (__orig), "=&r" (__res), "=&r" (__ex_flag)
 84                 : "r" (&(count)->counter)
 85                 : "cc","memory" );
 86 
 87         __orig |= __ex_flag;
 88         if (unlikely(__orig != 0))
 89                 fail_fn(count);
 90 }

void __sched mutex_unlock(struct mutex *lock)
{
        /*
         * The unlocking fastpath is the 0->1 transition from 'locked'
         * into 'unlocked' state:
         */
...
__mutex_fastpath_unlock(&lock->count, __mutex_unlock_slowpath);
}

void __sched mutex_lock(struct mutex *lock)
{
//        might_sleep(); // для отладки
        /*
         * The locking fastpath is the 1->0 transition from
         * 'unlocked' into 'locked' state.
         */
        __mutex_fastpath_lock(&lock->count, __mutex_lock_slowpath);
        mutex_set_owner(lock);
}
