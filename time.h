#ifndef TIME_H
#define TIME_H
// \see C11 time.h
#include <stddef.h>
#include <signal.h>
#include <sys/_timespec.h>
#include <sys/_timeval.h>
#include <sys/_types.h>

/*
[C11]
The <time.h> header declares the structure tm, which includes at least the following members:

int    tm_sec   seconds [0..61]
int    tm_min   minutes [0,59]
int    tm_hour  hour [0,23]
int    tm_mday  day of month [1,31]
int    tm_mon   month of year [0,11]
int    tm_year  years since 1900
int    tm_wday  day of week [0,6] (Sunday = 0)
int    tm_yday  day of year [0,365]
int    tm_isdst daylight savings flag

Мы хотим использовать хранение в формате BCD- аппаратно зависимо и 

*/

struct tm {
	uint64_t tm_usec:10;//0-999 миллисекунды
	uint64_t tm_sec :7;	//0-59
	uint64_t tm_min :7;	//0-59
	uint64_t tm_hour:6;	//0-23
	uint64_t:1;
	uint64_t tm_isdst:1; // Daylight Saving Time flag
	
	uint64_t tm_wday:3; //0- 6 | 1-7
	uint64_t tm_mday:6;	//1-31 | 1-31
	uint64_t tm_mon :5;	//0-11 | 1-12
	uint64_t tm_year:9; //0-199  +1900

	uint64_t tm_yday:9; //0-365 bin
} __attribute__((packed));
// С11
#ifndef CLOCKS_PER_SEC
// 0 -- specifies that the "C standard" portability level
#if 1 //defined(_AEABI_PORTABILITY_LEVEL) && _AEABI_PORTABILITY_LEVEL!=0
extern const int __aeabi_CLOCKS_PER_SEC;
#define CLOCKS_PER_SEC (__aeabi_CLOCKS_PER_SEC)
#else
#define CLOCKS_PER_SEC (1000000)
#endif
#endif
// С11, C2x timespec base
#define TIME_MONOTONIC	1
#define TIME_UTC		2
#define TIME_ACTIVE		3
#define TIME_THREAD_ACTIVE		4
// С11 не поддержано в POSIX
int  timespec_get	(struct timespec *ts, int base);
int  timespec_getres(struct timespec *ts, int base);
static inline
void timespec_add	(struct timespec *ts, struct timespec *dt){
	ts->tv_sec += dt->tv_sec, ts->tv_nsec += dt->tv_nsec;
	if (ts->tv_nsec>=1000000000) {
		ts->tv_nsec-=1000000000;
		ts->tv_sec +=1;
	}
}


clock_t clock(void);
time_t time(time_t *timer);
time_t mktime(struct tm *timeptr); // заполняет поля wday и yday
struct tm* gmtime_r(const time_t *timer, struct tm*  result);
static inline struct tm* gmtime(const time_t *timer)// возвращает время в UTC
{
	static struct tm t;
	return gmtime_r(timer, &t);
}

struct tm* localtime(const time_t *timer); // возвращает локальное время с учетом часового пояса.
struct tm* localtime_r(const time_t *timer, struct tm*  result);
static inline struct tm* _localtime(const time_t *timer)
{
	static struct tm t;
	return localtime_r(timer, &t);
}

size_t     strftime(char *, size_t, const char *, const struct tm *);

// [POSIX], \sa thrd_sleep
int clock_getres   (clockid_t, struct timespec *);
int clock_gettime  (clockid_t, struct timespec *);
int clock_settime  (clockid_t, const struct timespec *);
int	clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp);
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
#define CLOCK_MONOTONIC TIME_MONOTONIC /*!< A nonsettable, monotonically increasing clock that
              measures time since some unspecified point in the past
              that does not change after system startup. */
#define CLOCK_REALTIME  TIME_UTC /*!< A settable system-wide real-time clock. */
#define CLOCK_PROCESS_CPUTIME_ID 	TIME_ACTIVE /*!< The identifier of the CPU-time clock associated with the process making a clock() or timer*() function call */
#define CLOCK_THREAD_CPUTIME_ID 	TIME_THREAD_ACTIVE

// [POSIX] поддержать
struct itimerspec {
	struct timespec  it_interval;	//!<  Timer period. 
	struct timespec  it_value;		//!<  Timer expiration. 
};
#define TIMER_ABSTIME 1
int        timer_create (clockid_t, struct sigevent *restrict, timer_t *restrict);
int        timer_delete (timer_t);
int        timer_getoverrun(timer_t);
int        timer_gettime(timer_t, struct itimerspec *);
int        timer_settime(timer_t, int, const struct itimerspec *restrict, struct itimerspec *restrict);

void       tzset(void);

extern int    daylight;
extern long   timezone;
extern char  *tzname[];

#endif//TIME_H

