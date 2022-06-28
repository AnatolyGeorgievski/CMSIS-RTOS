#include "board.h"
#include "svc.h"
#include "ident.h"
#include "pio.h"
#include <time.h>
#include <cmsis_os.h>
#include <stdio.h>
#define WEAK __attribute__ ((weak))

extern  void WEAK osTimerWork(uint32_t timestamp);
extern  void TRACE_Init();
extern  void WEAK osThreadInit();

static int system_running =0; 
static struct timespec system_uptime= {0};
static struct timespec UTC= {0};
static uint32_t system_ticks =0;

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
//    r3_slice_init();// выделение памяти слайсами
    r3_sys_ident();		// идентификация системы, одна и та же прошивка должна уметь адаптироваться под разный размер памяти или тип процессора
    r3_reset_ident();	// идентификация типа загрузки: по питанию, програмная перезагрузка или сбой
//	events_init();	// инициализация потока событий
    FLASH_LoadConfig();	// загрузить конфигурацию устройства
                // регистрация прерываний от ног, внешние события
    //r3_sys_init();	// инициализация системного таймера
	// переход из превилегированного режима в непревилегированный
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

 
 /*! \brief переключение контекстов задач

	\see osThreadYield
 */

//__attribute__((isr))
void  SysTick_Handler(void)
{
	static uint32_t system_microsec =0;//!< Время на переключение задачи

	system_ticks    += BOARD_RTC_PERIOD;
	// таймерные процессы
	osTimerWork(system_ticks);
    system_uptime.tv_nsec += BOARD_RTC_PERIOD*1000;// измеряется в микросекундах
    if (system_uptime.tv_nsec >= 1000000000)
    {
        system_uptime.tv_nsec -= 1000000000;
		system_uptime.tv_sec++;
	}
	system_microsec    += BOARD_RTC_PERIOD;
	if (system_microsec>=1000) {
		system_microsec -=1000;
		if (osThreadGetPriority(osThreadGetId()) < osPriorityRealtime) {
			 __YIELD();// запросить переключение задач по таймеру
		}
    }
}
/* увеличивается монотонно */
uint32_t HAL_GetTick()
{
    return system_ticks;
}
/*  функция clock() определена в С11 <time.h> с параметром 
см макрос CLOCKS_PER_SEC
*/
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000// (BOARD_MCK*1000000)
typedef uint32_t clock_t;
#endif
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
	volatile struct timespec *uptime = &system_uptime;
	register struct timespec t;
	do {
		*ts = *uptime;// атомарно?
	} while (ts->tv_sec != uptime->tv_sec);
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
time_t time(time_t* ref)
{
	if (ref!=NULL) {
		*ref = system_uptime.tv_sec;
	}
	return system_uptime.tv_sec;
//	return SysTick->LOAD - SysTick->VAL;
}
#if 0 //
int timespec_get(struct timespec *ts, int base) 
{
	*ts = system_uptime;// атомарно?
	if (base == TIME_UTC) {
		ts->tv_sec  += UTC.tv_sec;
		ts->tv_nsec += UTC.tv_nsec;
		if (ts->tv_nsec >= 1000000000) {// не реализуется, потому что таймерный процесс не прерывается.
			ts->tv_nsec -= 1000000000;
			ts->tv_sec ++;
		}
	}
	return base;
}
#endif
uint32_t osKernelSysTick (void)
{
	return system_ticks;
}

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
	debug("!");
	BOARD_SAFE_PINS;
//	char* str = "HardFault\r\n";
	debug(str);
	char s[20];
	snprintf(s, 20, "%08X\r\n", value);
	debug(s);
	while(1);
}
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
	_Fault("UsageFault\r\nCFSR", SCB->CFSR);
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