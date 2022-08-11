# Интерфейсы POSIX.1-2017 поддерживаемые в R3v2 (описание не завершено)
Поддержка разделена на субгруппы, представленные в стандарте \
[POSIX] E.1 Subprofiling Option Groups


|Сокр| Определение
|:--- |:--
|BAR| _POSIX_BARRIERS (Barriers)
|CS | _POSIX_CLOCK_SELECTION (Clock Selection)
|MF | _POSIX_MAPPED_FILES (Memory Mapped Files)
|MPR| _POSIX_MEMORY_PROTECTION (Memory Protection)
|RTS| _POSIX_REALTIME_SIGNALS (Realtime Signals Extension)
|SEM| _POSIX_SEMAPHORES (Semaphores)
|  | _POSIX_SIGNALS (Signals)
|SPI| _POSIX_SPIN_LOCKS (Spin Locks)
|THR| _POSIX_THREADS (Threads)
|TMR| _POSIX_TIMERS (Timers)

В настоящее время поддерживаются группы
- SEM Semaphores, поддержан функционал кроме pshared, не поддержаны вызовы sem_open и sem_unlink.
- RTS Realtime Signals
  sigqueue( ), sigtimedwait( ), sigwaitinfo( )
- THR Threads, в RTOS представлены треды C11 совместимые, планируется поддержка pthread, без атрибутов

|POSIX Threads | C11 threads.h |
|:--|:--|
|pthread_cond_broadcast( )  | cnd_broadcast
|pthread_cond_destroy( )    | cnd_destroy
|pthread_cond_init( )       | cnd_init
|pthread_cond_signal( )     | cnd_signal
|pthread_cond_timedwait( )  | cnd_timedwait
|pthread_cond_wait( )       | cnd_wait
|pthread_create( )          | thr_create
|pthread_detach( )          | thr_detach
|pthread_equal( )           | thr_equal
|pthread_exit( )            | thr_exit
|pthread_join( )            | thr_join
|pthread_kill( )            |
|pthread_self( )            | thr_current
|sched_yield( )             | thrd_yield
|pthread_getspecific( )     | tss_get
|pthread_setspecific( )     | tss_set
|pthread_key_create( )      | tss_create
|pthread_key_delete( )      | tss_delete 
|pthread_mutex_destroy( )   | mtx_destroy
|pthread_mutex_init( )      | mtx_init
|pthread_mutex_lock( )      | mtx_lock
|pthread_mutex_timedlock( ) | mtx_timedlock 
|pthread_mutex_trylock( )   | mtx_trylock
|pthread_mutex_unlock( )    | mtx_unlock
|pthread_once( )            | call_once

Жаль, что при внедрении в стандарт C11 упустили семафоры. Мне бы хотелось чтобы в C11-С2x были представлены и семафоры в том же стиле.
- SEM POSIX Semaphores:
sem_close( ),
sem_destroy( ),
sem_getvalue( ),
sem_init( ),
sem_open( ),
sem_post( ),
sem_timedwait( ), 
sem_trywait( ), 
sem_unlink( ), 
sem_wait( )
- TMR Timers: clock_getres( ), clock_gettime( ), clock_settime( ), nanosleep( ), timer_create( ), timer_delete( ),
timer_getoverrun( ), timer_gettime( ), timer_settime( )
- 

  [POSIX.1-2017] IEEE Std 1003.1tm -2017 (Revision of IEEE Std 1003.1-2008)
  The Open Group Standard Base Specfications, Issue 7
    https://pubs.opengroup.org/onlinepubs/9699919799/
  
