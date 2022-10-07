#include "board.h"
#include "svc.h"
#include "ident.h"
#include "pio.h"
//#include "atomic.h"
#include <time.h>
#include <cmsis_os.h>
#include <stdio.h>
#define WEAK __attribute__ ((weak))

extern  void WEAK osTimerWork(uint32_t timestamp);
extern  void TRACE_Init();
extern  void WEAK osThreadInit();

static int system_running =0; 
static struct timespec system_uptime= {0};
static struct timespec UTC= {0};// смещение относительно системного времени

/*  функция clock() определена в С11 <time.h> с параметром 
см макрос CLOCKS_PER_SEC
*/
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000// (BOARD_MCK*1000000)
typedef uint32_t clock_t;
#endif
static clock_t system_ticks =0; /*!< системно-зависимый тип данных на 64 битной платформе может быть uint64_t 
	это надо учитывать при вызове и использовании clock()
*/

void SysTick_Handler(void) PRIVILEGED_FUNCTION;

extern void WEAK TRACE_Init();
extern void WEAK r3_mem_init();
//extern void WEAK r3_sys_ident();
//extern void WEAK r3_reset_ident();
extern void WEAK r3_slice_init();
extern void WEAK events_init();
extern void WEAK FLASH_LoadConfig();

/*! \brief инициализация ядра RTOS
 */
__attribute__ ((visibility ("default")))
osStatus osKernelInitialize (void)
{
	//system_running = 0;
	TRACE_Init();	// система сбора сообщений
	// инициализация системного таймера, используется для измерения времени при инициализации оборудования
    r3_mem_init();	// распределение динамической памяти
//    r3_slice_init();// выделение памяти слайсами -- инициализация не требуется
    r3_sys_ident();		// идентификация системы, одна и та же прошивка должна уметь адаптироваться под разный размер памяти или тип процессора
    r3_reset_ident();	// идентификация типа загрузки: по питанию, програмная перезагрузка или сбой
//	events_init();	// инициализация потока событий
    FLASH_LoadConfig();	// загрузить конфигурацию устройства
                // регистрация прерываний от ног, внешние события
    //r3_sys_init();	// инициализация системного таймера
	// переход из превилегированного режима в непревилегированный -- выполняется при начальной загрузке
	
	return osOK;
}
osStatus osKernelStart (void)
{
	if (system_running) return osOK;
    // регистрируем в системе обработчики системных вызовов
	osThreadInit();// инициализация тредов
	system_running = 1;
	return osOK;
}
/*! \brief определить статус ядра RTOS
 */
int32_t osKernelRunning(void)
{
	return system_running;
}
/*!  \brief переключение контекстов задач
 */
#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available

 
static inline 
uint64_t t64_bcd(uint64_t t64){
    if ((uint32_t)t64 > 999999999UL)
        t64+=(uint32_t)~999999999UL;
    return t64;
}

 /*! \brief переключение контекстов задач по системному таймеру

	\see osThreadYield
 */
//__attribute__((isr))
void  SysTick_Handler(void)
{
	static uint32_t system_microsec =0;//!< Время на переключение задачи

	system_ticks    += BOARD_RTC_PERIOD;
	// таймерные процессы
	osTimerWork(system_ticks);
	// обновить системное время clock_settime(
    system_uptime.tv_nsec += BOARD_RTC_PERIOD*1000;// измеряется в микросекундах
    if (system_uptime.tv_nsec >= 1000000000) {
        system_uptime.tv_nsec -= 1000000000;
		system_uptime.tv_sec++;
	}
	system_microsec    += BOARD_RTC_PERIOD;
#if 0
	if (current_thread->budget <= system_microsec) {
		current_thread->budget  = system_microsec = 0;
		current_thread->budget += BUDGET(current_thread->low_priority);
		__YIELD();// запросить переключение задач по таймеру
	}
#elif 0 // другой вариант
	if (--current_thread->budget <= 0) {
		current_thread->budget += BUDGET(current_thread->sched.low_priority);
		__YIELD();// запросить переключение задач по таймеру
	}
#endif
	if (system_microsec>=1000) {
		system_microsec-=1000;// rr_interval;
		if (osThreadGetPriority(osThreadGetId()) < osPriorityRealtime) {
			__YIELD();// запросить переключение задач по таймеру
		}
    }
}
//static inline
clock_t clock(void)
{
	return system_ticks;
}
/*! 

	\return A return value of 0 shall indicate that the call succeeded. A return value of -1 shall indicate that an error occurred, and errno shall be set to indicate the error.
*/

int clock_gettime(clockid_t clock_id, struct timespec *ts)
{
	volatile struct timespec *uptime;
	switch (clock_id){
#if defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME>0)
	//case TIME_THREAD_ACTIVE: ts->tv_nsec = current_thread->CPU_time + (CPU_clock() - arrival_time); break;
#endif
	default:
		uptime = &system_uptime;
	}
	do {
		*ts = *uptime;// атомарно?
	} while (ts->tv_sec != uptime->tv_sec);
	if (clock_id==TIME_UTC) {
		ts->tv_sec  += UTC.tv_sec;
		ts->tv_nsec += UTC.tv_nsec;
		if (ts->tv_nsec >= 1000000000) {
			ts->tv_nsec -= 1000000000;
			ts->tv_sec ++;
		}
	}
	return 0;
/*
	ptr = (&system_uptime.tv_nsec);
	do {
		//*ts = system_uptime;// атомарно?
		ts->tv_nsec = atomic_get(ptr);// прочитать с пометкой
		ts->tv_sec = (volatile )(&system_uptime.tv_sec);
	} while (!atomic_compare_and_exchange(ptr, ts->tv_nsec, ts->tv_nsec));// проверить что изменилось 
*/
}
/*! \brief function return the resolution of a specified clock.
 */
int clock_getres(clockid_t clock_id, struct timespec *res)
{
#if defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME>0)
	if(clock_id == TIME_THREAD_ACTIVE)
		*res =(struct timespec){.tv_nsec =1000/BOARD_MCK};
	else
#endif
		*res =(struct timespec){.tv_nsec =1000*BOARD_RTC_PERIOD};
	return 0;
}
/*
errno:
    [EINTR]
        The nanosleep() function was interrupted by a signal.
    [EINVAL]
        The rqtp argument specified a nanosecond value less than zero or greater than or equal to 1000 million.
	[ENOTSUP]
		The clock_id argument specifies a clock for which clock_nanosleep() is not supported, such as a CPU-time clock.
 */
/*! \brief Ускорение операций дления на константу */
static inline uint32_t div1000(uint32_t v) {
#if defined(__ARM_ARCH_8M_BASE__)
	return v/1000U;
#else
    return (v*0x83126E98ULL)>>41;
#endif
}
int clock_nanosleep(clockid_t clock_id, int flags,
       const struct timespec *rqtp, struct timespec *rmtp)
{
	if (rqtp->tv_sec!=0 || rqtp->tv_nsec>=1000000000 ) {
		// errno = EINVAL
		return -1;
	}
	svc1(SVC_USLEEP, div1000(rqtp->tv_nsec));
	return 0;
}
int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
	//! \todo наверное надо через SVC выставлять
	return 0;
}
time_t time(time_t* ref)
{
	uint32_t t = (system_uptime.tv_sec);// +UTC?
	if (ref!=NULL)
		*ref = t;
	return t;
//	return SysTick->LOAD - SysTick->VAL;
}
#if 0 //Это часть стандарта с11 в POSIX не поддержана
// Сравнение
int timespec_cmp(&timenow,ts2) {
	
}
int timespec_get(struct timespec *ts, int base) 
{
	volatile struct timespec *uptime = &system_uptime;
	do {
		*ts = *uptime;// атомарно?
	} while (ts->tv_sec != uptime->tv_sec);
	if (base == TIME_MONOTONIC){
		
	} else {
		struct timespec *d;
		switch (base) {
		case TIME_UTC: d = UTC; break;
		default: d = NULL; break;
		}
		if (d!=NULL) {
			ts->tv_sec  += d->tv_sec;
			ts->tv_nsec += d->tv_nsec;
			if (ts->tv_nsec >= 1000000000) {
				ts->tv_nsec -= 1000000000;
				ts->tv_sec ++;
			}
		}
	}
	return base;
}
int timespec_getres	(struct timespec *ts ,int base){
	if (base == TIME_THREAD_ACTIVE) {
		*ts = (struct timespec){.tv_nsec =5};
	} else
		*ts = (struct timespec){.tv_nsec =1000*BOARD_RTC_PERIOD};
	return base;
}
#endif
#if 0 // убрать
uint32_t osKernelSysTick (void)
{
	return system_ticks;
}
#endif
#endif

/*! \todo привести устройство в нормальное состояние, 
	когда устройство не будет потеряно при зависании из-за того что пины находятся в активном состоянии.
	Ожидается выход через WatchDog
 */

#ifndef BOARD_SAFE_PINS
#warning "please define safe state"
#define BOARD_SAFE_PINS
#endif
_Noreturn static void _Fault(const char *str, uint32_t value)
{
//	debug("!");
	BOARD_SAFE_PINS;
//	char* str = "HardFault\r\n";
	debug(str);
	char s[20];
	snprintf(s, 20, "%08X\r\n", value);
	debug(s);
	while(1);
}
/* DFSR, Debug Fault Status Register
void DebugMon_Handler(){
	
}*/
#if defined(__ARM_ARCH_8M_BASE__)
_Noreturn void HardFault_Handler()
{
	_Fault("HardFault\r\n", 0);
}
#else // остальные пристуствуют всегда
_Noreturn void HardFault_Handler()
{
	_Fault("HardFault\r\nCFSR", SCB->CFSR);
/*	BOARD_SAFE_PINS;
	char* str = "HardFault\r\n";
	debug(str);
	char s[20];
	snprintf(s, 20, "CFSR %08X\r\n", SCB->CFSR);
	debug(s);*/
//	while(1);
}
_Noreturn void MemManage_Handler()
{
	_Fault("MemManage\r\nMMFAR", SCB->MMFAR);
/*	BOARD_SAFE_PINS;
	char* str = "MemManage\r\n";
	debug(str);
	char s[20];
	snprintf(s, 20, "MMFAR %08X\r\n", SCB->MMFAR);
	debug(s);*/
}
_Noreturn void UsageFault_Handler()
{
#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8_1M_MAIN__)
	_Fault("UsageFault\r\nUFSR", SCB->UFSR);// This register is RES0 if the Main Extension is not implemented
#else
	_Fault("UsageFault\r\nSFSR", SCB->CFSR);
#endif
/* SCB_CFSR_DIVBYZERO_Msk: Divide by Zero
/*	BOARD_SAFE_PINS;
	const char* str = "UsageFault\r\n";
	debug(str); */
/*	char s[20];
	snprintf(s, 20, "CFSR %08X\r\n", SCB->CFSR);
	debug(s);
	if (SCB->CFSR & SCB_CFSR_DIVBYZERO_Msk)debug("Divide by Zero\r\n");
	if (SCB->CFSR & SCB_CFSR_UNALIGNED_Msk)debug("Unalignaed\r\n");*/
	//while(1);
}
_Noreturn void BusFault_Handler()
{
	_Fault("BusFault\r\nBFAR", SCB->BFAR);
/*	BOARD_SAFE_PINS;
	char* str = "BusFault\r\n";
	debug(str);
	char s[20];
	snprintf(s, 20, "BFAR %08X\r\n", SCB->BFAR);
	debug(s); */
	//while(1);
}
#endif