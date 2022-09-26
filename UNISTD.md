# Интерфейсы POSIX.1-2017 поддерживаемые в R3v2 (описание не завершено)
Поддержка разделена на субгруппы, представленные в стандарте \
[POSIX] E.1 Subprofiling Option Groups
Поддержка и функционал включаются/отключаются с использованием определений в заголовке <[unistd.h](unistd.h)>.
Реализация функций синхронизации основана на использовании атомарных не блокирующих операций с счетчиком семафора, см. <[semaphore.h](semaphore.h)>. 

|Сокр| Определение
|:--- |:--
|AIO| POSIX_ASYNCHRONOUS_IO (Asynchronous I/O)
|BAR| POSIX_BARRIERS (Barriers)
|CS | POSIX_CLOCK_SELECTION (Clock Selection)
|MF | POSIX_MAPPED_FILES (Memory Mapped Files)
|MPR| POSIX_MEMORY_PROTECTION (Memory Protection)
|MSG| POSIX_MESSAGE_PASSING (Message Passing)
|RWL| POSIX_READER_WRITER_LOCKS (Read-Write Locks)
|RTS| POSIX_REALTIME_SIGNALS (Realtime Signals Extension)
|SEM| POSIX_SEMAPHORES (Semaphores)
|SHM| POSIX_SHARED_MEMORY_OBJECTS (Shared Memory Objects)
|SIG| POSIX_SIGNALS (Signals)
|SIO| POSIX_SYNCHRONIZED_IO (Synchronized I/O)
|SPI| POSIX_SPIN_LOCKS (Spin Locks)
|THR| POSIX_THREADS (Threads)
|TCT| POSIX_THREAD_CPUTIME (CPU-time clocks)
|TMO| POSIX_TIMEOUTS (Timeouts)
|TMR| POSIX_TIMERS (Timers)

- THR Threads, в RTOS представлены треды C11 совместимые, планируется поддержка pthread, без атрибутов

|POSIX Threads | C11 threads.h |
|:--|:--|
|pthread_cond_broadcast( )  | cnd_broadcast
|pthread_cond_destroy( )    | cnd_destroy
|pthread_cond_init( )       | cnd_init
|pthread_cond_signal( )     | cnd_signal
|pthread_cond_timedwait( )  | cnd_timedwait
|pthread_cond_wait( )       | cnd_wait
|pthread_create( )          | thrd_create
|pthread_detach( )          | thrd_detach
|pthread_equal( )           | thrd_equal
|pthread_exit( )            | thrd_exit
|pthread_join( )            | thrd_join
|pthread_self( )            | thrd_current
|sched_yield( )             | thrd_yield
|nanosleep( )               | thrd_sleep
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

- **POSIX_THREADS_BASE**: Base Threads
```diff
- pthread_atfork( ), 
+ pthread_attr_destroy( ), pthread_attr_getdetachstate( ), pthread_attr_getschedparam( ), 
+ pthread_attr_init( ), pthread_attr_setdetachstate( ), pthread_attr_setschedparam( ), 
- pthread_cancel( ), pthread_cleanup_pop( ), pthread_cleanup_push( ),
+ pthread_cond_broadcast( ), pthread_cond_destroy( ), pthread_cond_init( ), pthread_cond_signal( ),
+ pthread_cond_timedwait( ), pthread_cond_wait( ), 
  pthread_condattr_destroy( ), pthread_condattr_init( ), 
+ pthread_create( ), pthread_detach( ), pthread_equal( ), pthread_exit( ), pthread_join( ), 
+ pthread_getspecific( ), pthread_setspecific( ), thread_key_create( ), pthread_key_delete( ),
+ pthread_mutex_destroy( ), pthread_mutex_init( ), pthread_mutex_lock( ),
+ pthread_mutex_timedlock( ), pthread_mutex_trylock( ), pthread_mutex_unlock( ),
  pthread_mutexattr_destroy( ), pthread_mutexattr_init( ), 
+ pthread_once( ), pthread_self( ), pthread_kill( ), pthread_sigmask( ),
- pthread_setcancelstate( ), pthread_setcanceltype( ), pthread_testcancel( )
```
- RTS **POSIX_REALTIME_SIGNALS**: Realtime Signals
```diff
+ sigqueue( ), sigtimedwait( ), sigwaitinfo( )
```

- SEM **POSIX_SEMAPHORES**: Semaphores:
```diff
+ sem_close( ), sem_open( ), sem_unlink( ), 
+ sem_destroy( ),sem_getvalue( ),sem_init( ),sem_post( ),sem_timedwait( ), sem_trywait( ), sem_wait( )
```

- MSG **POSIX_MESSAGE_PASSING**: Message Passing
```diff
+ mq_close( ), mq_open( ), mq_unlink( ),
+ mq_getattr( ), mq_notify( ), mq_setattr( ), 
+ mq_receive( ), mq_send( ), mq_timedreceive( ), mq_timedsend( )
```

- TMR **POSIX_TIMERS**: Timers 
```diff
+ clock_getres( ), clock_gettime( ), clock_settime( ), nanosleep( ), 
+ timer_create( ), timer_delete( ), timer_getoverrun( ), timer_gettime( ), timer_settime( )
```

- **POSIX_RW_LOCKS**: Reader Writer Locks
```diff
+ pthread_rwlock_destroy( ), pthread_rwlock_init( ), pthread_rwlock_rdlock( ),
+ pthread_rwlock_timedrdlock( ), pthread_rwlock_timedwrlock( ), pthread_rwlock_tryrdlock( ),
+ pthread_rwlock_trywrlock( ), pthread_rwlock_unlock( ), pthread_rwlock_wrlock( ),
- pthread_rwlockattr_destroy( ), pthread_rwlockattr_init( ), 
- pthread_rwlockattr_getpshared( ), pthread_rwlockattr_setpshared( )
```

- **POSIX_SPIN_LOCKS**: Spin Locks
```diff
+ pthread_spin_destroy( ), pthread_spin_init( ), pthread_spin_lock( ), pthread_spin_trylock( ), pthread_spin_unlock( )
```

- **POSIX_BARRIERS**: Barriers
```diff
+ pthread_barrier_destroy( ), pthread_barrier_init( ), pthread_barrier_wait( ), 
  pthread_barrierattr( )
```

- **POSIX_DYNAMIC_LINKING**: Dynamic Linking
```diff
+ dlclose( ), dlerror( ), dlopen( ), dlsym( )
```
- **POSIX_SIGNALS**: Signals
```diff
- abort( ), alarm( ), kill( ), pause( ), raise( ), 
+ sigaddset( ), sigdelset( ), sigemptyset( ), sigfillset( ), sigismember( ), 
+ sigprocmask( ), pthread_sigmask( ), pthread_kill( ),
- signal( ), sigaction( ), sigpending( ), sigsuspend( ), sigwait( )
```

- **POSIX_DEVICE_IO**: Device Input and Output
```diff
+ FD_CLR( ), FD_ISSET( ), FD_SET( ), FD_ZERO( ), 
+ clearerr( ), close( ), open( ), read( ), write( )
fdopen( ), feof( ), fflush( ), 
ferror( ), fgetc( ), fgets( ), 
+ fopen( ), fprintf( ), fputc( ), fputs( ), fread( ), freopen( ), fileno( ), 
fscanf( ), fwrite( ), getc( ), getchar( ), gets( ), perror( ), 
+ poll( ), printf( ), pread( ), pselect( ), putchar( ), puts( ), 
putc( ), scanf( ), select( ), setbuf( ), setvbuf( ), stderr, stdin, stdout, 
ungetc( ), vfprintf( ), vfscanf( ), vprintf( ), vscanf( ), 
```

- **POSIX_NETWORKING**: Networking
```diff
+ accept( ), bind( ), connect( ), 
endhostent( ), endnetent( ), endprotoent( ), endservent( ),
freeaddrinfo( ), gai_strerror( ), getaddrinfo( ), gethostent( ), gethostname( ), getnameinfo( ),
getnetbyaddr( ), getnetbyname( ), getnetent( ), getpeername( ), getprotobyname( ),
getprotobynumber( ), getprotoent( ), getservbyname( ), getservbyport( ), getservent( ),
getsockname( ), getsockopt( ), htonl( ), htons( ), if_freenameindex( ), if_indextoname( ),
if_nameindex( ), if_nametoindex( ), inet_addr( ), inet_ntoa( ), inet_ntop( ), inet_pton( ), listen( ),
+ ntohl( ), ntohs( ), recv( ), recvfrom( ), recvmsg( ), send( ), sendmsg( ), sendto( ), 
sethostent( ), setnetent( ), setprotoent( ), setservent( ), 
+ setsockopt( ), shutdown( ), socket( ), 
sockatmark( ), socketpair( )
```

- **POSIX_FILE_SYSTEM**: File System
```diff
+ access( ), chdir( ), closedir( ), creat( ), fchdir( ), fpathconf( ), fstat( ), link( ),
+ mkdir( ), mkstemp( ), opendir( ), pathconf( ), remove( ), rename( ), rmdir( ),
+ stat( ), tmpfile( ), truncate( ), unlink(), 
- fstatvfs( ), getcwd( ), readdir( ), rewinddir( ), statvfs( ), tmpnam( ), utime( )
```

- **POSIX_FILE_SYSTEM_R**: Thread-Safe File System
```diff
+ readdir_r( )
```
- **POSIX_FILE_SYSTEM_FD**: File System File Descriptor Routines
```diff
+ faccessat( ), fdopendir( ), fstatat( ), linkat( ), mkdirat( ), openat( ), 
+ renameat( ), unlinkat( ), utimensat( )
```

- **POSIX_FILE_ATTRIBUTES**: File Attributes
```diff
+ chmod( ), chown( ), fchmod( ), fchown( ), umask( )
```

- **POSIX_FILE_ATTRIBUTES_FD**: File Attributes File Descriptor Routines
```diff
+ fchmodat( ), fchownat( )
```

- **POSIX_FILE_LOCKING**: Thread-Safe Stdio Locking
```diff
  flockfile( ), ftrylockfile( ), funlockfile( ), 
  getc_unlocked( ), getchar_unlocked( ), 
  putc_unlocked( ), putchar_unlocked( )
```

- **POSIX_MAPPED_FILES**: Memory Mapped Files
```diff
+ mmap( ), munmap( )
```

- **POSIX_MEMORY_PROTECTION**: Memory Protection
```diff
mprotect( )
```

- **POSIX_SINGLE_PROCESS**: Single Process
```diff
- confstr( ), setenv( ), unsetenv( ), 
+ environ, errno, getenv( ), sysconf( ), uname( )
```

- **POSIX_THREAD_SAFE_FUNCTIONS**: Thread-Safe Functions
On POSIX-conforming systems, the symbolic constant \_POSIX_THREAD_SAFE_FUNCTIONS is
always defined. Therefore, the following functions are always supported:
```diff
- asctime_r( ) ctime_r( )
  flockfile( ) ftrylockfile( ) funlockfile( )
- getc_unlocked( ) getchar_unlocked( )
  getgrgid_r( ) getgrnam_r( ) getpwnam_r( ) getpwuid_r( ) 
+ gmtime_r( ) localtime_r( )
+ putc_unlocked( ) putchar_unlocked( )
  rand_r( ) strerror_r( ) 
+ readdir_r( ) strtok_r( )
```

[POSIX.1-2017] IEEE Std 1003.1tm -2017 (Revision of IEEE Std 1003.1-2008)
The Open Group Standard Base Specfications, Issue 7
    https://pubs.opengroup.org/onlinepubs/9699919799/
  
