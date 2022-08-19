# Интерфейсы POSIX.1-2017 поддерживаемые в R3v2 (описание не завершено)
Поддержка разделена на субгруппы, представленные в стандарте \
[POSIX] E.1 Subprofiling Option Groups
Поддержка и функционал включаются/отключаются с использованием определений в заголовке <unistd.h>.
Реализация функций синхронизации основана на использовании атомарных не блокирующих операций с счетчиком семафора, см. <semaphore.h>. 

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
|                           | thrd_sleep
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

- POSIX_THREADS_BASE: Base Threads
```diff
- pthread_atfork( ), pthread_attr_destroy( ), pthread_attr_getdetachstate( ),
- pthread_attr_getschedparam( ), pthread_attr_init( ), pthread_attr_setdetachstate( ),
- pthread_attr_setschedparam( ), pthread_cancel( ), pthread_cleanup_pop( ), pthread_cleanup_push( ),
+ pthread_cond_broadcast( ), pthread_cond_destroy( ), pthread_cond_init( ), pthread_cond_signal( ),
+ pthread_cond_timedwait( ), pthread_cond_wait( ), 
- pthread_condattr_destroy( ), pthread_condattr_init( ), 
+ pthread_create( ), pthread_detach( ), pthread_equal( ), pthread_exit( ), pthread_join( ), 
+ pthread_getspecific( ), pthread_setspecific( ), thread_key_create( ), pthread_key_delete( ),
+ pthread_mutex_destroy( ), pthread_mutex_init( ), pthread_mutex_lock( ),
+ pthread_mutex_timedlock( ), pthread_mutex_trylock( ), pthread_mutex_unlock( ),
- pthread_mutexattr_destroy( ), pthread_mutexattr_init( ), 
+ pthread_once( ), pthread_self( ), pthread_kill( ), thread_sigmask( ),
- pthread_setcancelstate( ), pthread_setcanceltype( ), pthread_testcancel( )
```
- RTS Realtime Signals
```diff
+ sigqueue( ), sigtimedwait( ), sigwaitinfo( )
```

- SEM POSIX_SEMAPHORES: Semaphores:
```diff
- sem_close( ), sem_open( ), sem_unlink( ), 
+ sem_destroy( ),sem_getvalue( ),sem_init( ),sem_post( ),sem_timedwait( ), sem_trywait( ), sem_wait( )
```

- TMR POSIX_TIMERS: Timers: 
```diff
+ clock_getres( ), clock_gettime( ), clock_settime( ), nanosleep( ), 
+ timer_create( ), timer_delete( ), timer_getoverrun( ), timer_gettime( ), timer_settime( )
```
- POSIX_SPIN_LOCKS: Spin Locks
```diff
+ pthread_spin_destroy( ), pthread_spin_init( ), pthread_spin_lock( ), pthread_spin_trylock( ), pthread_spin_unlock( )
```
- POSIX_BARRIERS: Barriers
```diff
+ pthread_barrier_destroy( ), pthread_barrier_init( ), pthread_barrier_wait( ), 
- pthread_barrierattr( )
```
- POSIX_DYNAMIC_LINKING: Dynamic Linking
```diff
+ dlclose( ), dlerror( ), dlopen( ), dlsym( )
```
- **POSIX_SIGNALS**: Signals:
```diff
- abort( ), alarm( ), kill( ), pause( ), raise( ), 
+ sigaddset( ), sigdelset( ), sigemptyset( ), sigfillset( ), sigismember( ), 
+ sigprocmask( ), pthread_sigmask( ), pthread_kill( ),
- signal( ), sigaction( ), sigpending( ), sigsuspend( ), sigwait( )
```

- POSIX_DEVICE_IO: Device Input and Output: 
FD_CLR( ), FD_ISSET( ), FD_SET( ), FD_ZERO( ), clearerr( ), close( ), fclose( ), fdopen( ), feof( ),
ferror( ), fflush( ), fgetc( ), fgets( ), fileno( ), fopen( ), fprintf( ), fputc( ), fputs( ), fread( ), freopen( ),
fscanf( ), fwrite( ), getc( ), getchar( ), gets( ), open( ), perror( ), poll( ), printf( ), pread( ), pselect( ),
putc( ), putchar( ), puts( ), pwrite( ), read( ), scanf( ), select( ), setbuf( ), setvbuf( ), stderr, stdin,
stdout, ungetc( ), vfprintf( ), vfscanf( ), vprintf( ), vscanf( ), write( )

[POSIX.1-2017] IEEE Std 1003.1tm -2017 (Revision of IEEE Std 1003.1-2008)
The Open Group Standard Base Specfications, Issue 7
    https://pubs.opengroup.org/onlinepubs/9699919799/
  
