#include <unistd.h>
//static inline 
long sysconf(int name)
{
	long v;
	switch (name){
case _SC_ADVISORY_INFO	: v = _POSIX_ADVISORY_INFO; 	break;
case _SC_BARRIERS		: v = _POSIX_BARRIERS; 	break;
case _SC_ASYNCHRONOUS_IO: v = _POSIX_ASYNCHRONOUS_IO; 	break;
case _SC_CLOCK_SELECTION: v = _POSIX_CLOCK_SELECTION; 	break;
case _SC_CPUTIME		: v = _POSIX_CPUTIME; 	break;
case _SC_FSYNC			: v = _POSIX_FSYNC; 	break;
case _SC_IPV6			: v = _POSIX_IPV6; 	break;
case _SC_JOB_CONTROL	: v = _POSIX_JOB_CONTROL; 	break;
case _SC_MAPPED_FILES	: v = _POSIX_MAPPED_FILES; 	break;
case _SC_MEMLOCK		: v = _POSIX_MEMLOCK; 	break;
case _SC_MEMLOCK_RANGE	: v = _POSIX_MEMLOCK_RANGE; 	break;
case _SC_MEMORY_PROTECTION	: v = _POSIX_MEMORY_PROTECTION; 	break;
case _SC_MESSAGE_PASSING	: v = _POSIX_MESSAGE_PASSING; 	break;
case _SC_MONOTONIC_CLOCK	: v = _POSIX_MONOTONIC_CLOCK; 	break;
case _SC_PRIORITIZED_IO		: v = _POSIX_PRIORITIZED_IO; 	break;
case _SC_PRIORITY_SCHEDULING: v = _POSIX_PRIORITY_SCHEDULING; 	break;
case _SC_RAW_SOCKETS		: v = _POSIX_RAW_SOCKETS; 	break;
case _SC_READER_WRITER_LOCKS: v = _POSIX_READER_WRITER_LOCKS; 	break;
case _SC_REALTIME_SIGNALS	: v = _POSIX_REALTIME_SIGNALS; 	break;
case _SC_REGEXP		: v = _POSIX_REGEXP; 	break;
case _SC_SAVED_IDS	: v = _POSIX_SAVED_IDS; 	break;
case _SC_SEMAPHORES	: v = _POSIX_SEMAPHORES; 	break;
case _SC_SHARED_MEMORY_OBJECTS	: v = _POSIX_SHARED_MEMORY_OBJECTS; 	break;
case _SC_SHELL	: v = _POSIX_SHELL; 	break;
case _SC_SPAWN	: v = _POSIX_SPAWN; 	break;
case _SC_SPIN_LOCKS			: v = _POSIX_SPIN_LOCKS; 	break;
case _SC_SPORADIC_SERVER	: v = _POSIX_SPORADIC_SERVER; 	break;
//case _SC_SS_REPL_MAX		: v = _POSIX_SS_REPL_MAX; 	break;
case _SC_SYNCHRONIZED_IO		: v = _POSIX_SYNCHRONIZED_IO; 	break;
case _SC_THREAD_ATTR_STACKADDR	: v = _POSIX_THREAD_ATTR_STACKADDR; 	break;
case _SC_THREAD_ATTR_STACKSIZE	: v = _POSIX_THREAD_ATTR_STACKSIZE; 	break;
case _SC_THREAD_CPUTIME	: v = _POSIX_THREAD_CPUTIME; 	break;
case _SC_THREAD_PRIO_INHERIT	: v = _POSIX_THREAD_PRIO_INHERIT; 	break;
case _SC_THREAD_PRIO_PROTECT	: v = _POSIX_THREAD_PRIO_PROTECT; 	break;
case _SC_THREAD_PRIORITY_SCHEDULING	: v = _POSIX_THREAD_PRIORITY_SCHEDULING; 	break;
case _SC_THREAD_PROCESS_SHARED	: v = _POSIX_THREAD_PROCESS_SHARED; 	break;
//case _SC_THREAD_ROBUST_PRIO_INHERIT	: v = _POSIX_THREAD_ROBUST_PRIO_INHERIT; 	break;
//case _SC_THREAD_ROBUST_PRIO_PROTECT	: v = _POSIX_THREAD_ROBUST_PRIO_PROTECT; 	break;
case _SC_THREAD_SAFE_FUNCTIONS	: v = _POSIX_THREAD_SAFE_FUNCTIONS; 	break;
//case _SC_THREAD_SPORADIC_SERVER	: v = _POSIX_THREAD_SPORADIC_SERVER; 	break;
case _SC_THREADS	: v = _POSIX_THREADS; 	break;
case _SC_TIMEOUTS	: v = _POSIX_TIMEOUTS; 	break;
case _SC_TIMERS		: v = _POSIX_TIMERS; 	break;
case _SC_TRACE		: v = _POSIX_TRACE; 	break;
case _SC_TRACE_EVENT_FILTER		: v = _POSIX_TRACE_EVENT_FILTER; 	break;
//case _SC_TRACE_EVENT_NAME_MAX	: v = _POSIX_TRACE_EVENT_NAME_MAX; 	break;
case _SC_TRACE_INHERIT	: v = _POSIX_TRACE_INHERIT; 	break;
case _SC_TRACE_LOG		: v = _POSIX_TRACE_LOG; 	break;
//case _SC_TRACE_NAME_MAX	: v = _POSIX_TRACE_NAME_MAX; 	break;
//case _SC_TRACE_SYS_MAX	: v = _POSIX_TRACE_SYS_MAX; 	break;
//case _SC_TRACE_USER_EVENT_MAX	: v = _POSIX_TRACE_USER_EVENT_MAX; 	break;
case _SC_TYPED_MEMORY_OBJECTS	: v = _POSIX_TYPED_MEMORY_OBJECTS; 	break;
case _SC_VERSION			: v = _POSIX_VERSION; 	break;
case _SC_V7_ILP32_OFF32		: v = _POSIX_V7_ILP32_OFF32; 	break;
case _SC_V7_ILP32_OFFBIG	: v = _POSIX_V7_ILP32_OFFBIG; 	break;
case _SC_V7_LP64_OFF64		: v = _POSIX_V7_LP64_OFF64; 	break;
case _SC_V7_LPBIG_OFFBIG	: v = _POSIX_V7_LPBIG_OFFBIG; 	break;
	default: v = -1; 
		errno = EINVAL;
		break;
	}
	return v;//sys_config[name];
}
