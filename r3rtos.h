/* r3rtos.h */
#ifndef R3RTOS_H
#define R3RTOS_H

#include "board.h"
#include "r3stdlib.h"
#undef system
/*! \ingroup timer_object
    \{
*/
typedef struct _TimerObject TimerObject; //microsecs timer
//! \brief тип функции-обработчика события от таймера
typedef void (*TimerCallback) (TimerObject* timer, void* arg);
//! \brief таймерный объект
struct _TimerObject {
	TimerObject *next;      //!< следующий таймер в списке
	unsigned int interval;  //!< интерфвал запуска в мкс
	int phase;              //!< фазовая задержка запуска обработчика, мкс
	unsigned long timestamp; //!<
	TimerCallback func;     //!< функция вызывается по таймеру всякий раз когда возникает переполнение таймера
	void* arg;              //!< аргумент функции обработчика
	unsigned int counter;   //!< счетчик срабатываний таймера, прибавляется каждый раз при запуске обработчика
	unsigned char run;      //!< состояние таймера 1- запущен, 0- пауза
};

TimerObject *  timer_init(TimerObject * tree, TimerObject * timer, unsigned int interval, int phase, TimerCallback callback, void* arg);
void timer_reset(TimerObject * timer, int phase);
void timer_close(TimerObject * timer);
void timer_work(TimerObject * timer);
//! \}
typedef struct _Service Service;
typedef void (*Service_cb)(void * data, void* arg);
struct _Service {
	unsigned short port;
	unsigned short enabled:1;
	unsigned short request_counter:15;
	Service_cb func;
	void *data; // optional private service data
	char *name; // name of the service
	Service * next;
};
//typedef struct _RunOnceTaskInfo RunOnceTaskInfo;

typedef struct _System tSystem;
struct _System {
	volatile int interval;
	volatile unsigned int timestamp;
	volatile unsigned int microsec;	// микросекунды, используются при вычислении задержек
#if !defined(BOARD_RTC)
	volatile unsigned int usec, sec;
#endif
	//RTC_date date;
	TimerObject* timer;
//	TimerObject* user_timer; // задачи запускаются в фоне, как бы по таймеру
//	RunOnceTaskInfo *taskqueue;
#ifdef BOARD_EMAC
	struct{
		unsigned int rx_frames;
		unsigned int tx_frames;
		unsigned int arp_packets;
		unsigned int icmp_packets;
		unsigned int udp_packets;
		unsigned int tcp_packets;
		unsigned int ipv6_packets;
		unsigned int err_packets;
		Service* udp_services;
		Service* tcp_services;
	}iface[1];
#endif
};
void server_init(Service** services, Service*new_service);//, unsigned short port, void * data, Service_cb func);
//extern int server_work(Service** services, unsigned short port, void  * arg);
//extern void server_scan(Service* services);
#undef system
extern tSystem system;
//--------------------------------------------------------------
//! \defgroup timeout_obj Измерение задержек, TimeoutObject
//! \brief Задержки вымеряются относительно системного таймера. Принцип работы прост.
//! При инициализации сохранятес текущее показание таймера. В при каждой следующей проверке
//! задержка вымеряется отностительно сохраненного значения.
//--------------------------------------------------------------
//! \ingroup timeout_obj
//! \struct _TimeoutObject
typedef struct _TimeoutObject TimeoutObject;
struct _TimeoutObject {
	unsigned int timestamp;	//!< сохраненное значение системного таймера
	unsigned int interval;	//!< интервал срабатывания таймера
};
//--------------------------------------------------------------
/*! \ingroup timeout_obj
    \brief Инициализация таймера. Происходит сохранение текущего системного времени.

    \param [in] timer ссылка на объект, для которого вызывается процедура инициализации
    \param [in] interval интервал срабатывания в микросекундах. Надо иметь ввиду частоту
    срабатывания системного таймера.
*/
static inline
void timeout_init(TimeoutObject * timer, unsigned int interval)
{
	timer->timestamp = system.microsec;//timestamp;
	timer->interval	 = interval;///R3_RTOS_PERIOD;
}
//--------------------------------------------------------------
//! \ingroup timeout_obj
//! \fn int timeout_check(TimeoutObject * timer)
//! \brief Проверка готовности таймера.
//! \param[in] timer ссылка на объект, для которого вызывается проверка
//! \return TRUE если заданный интервал вышел.

static inline
int timeout_check(TimeoutObject * timer)
{
	return ((system.microsec/*timestamp*/-timer->timestamp) >= timer->interval);
}
//! \ingroup timeout_obj
//! \brief Проверка интервала таймера
//!
//! Функция используется в тех случаях когда надо вымерять несколько задержек по одному таймеру
//! \param [in] timer ссылка на объект, для которого вызывается проверка
//! \param [in] interval тайм-аут
//! \return TRUE если заданный интервал вышел
static inline
int timeout_check_interval(TimeoutObject * timer, unsigned int interval)
{
	return ((system.microsec/*timestamp*/-timer->timestamp) >= interval);
}


typedef int (*WeakTaskCb)(void*); /// возвращает 0 если процесс завершен, если не 0 то таймер перезапускается
void schedule_timeout(const char* name, WeakTaskCb cb, unsigned int timeout, void* user_data);

/*
typedef struct _EMAC_iface EMAC_iface;
struct _EMAC_iface {
		unsigned char MAC_Addr[6];
		unsigned char IP_Addr[4];
		unsigned char IP_Mask[4];
		unsigned char GW_Addr[4];
		unsigned char PHY_Addr;
	};
*/

typedef struct _AlarmsState AlarmsState;
struct _AlarmsState {
    volatile unsigned long sync_count; //!< счетчик импульсов от датчика пути, импульсы переводятся в метры
	volatile unsigned long delta_sync;
	volatile unsigned long alarm_count; //!< счетчик импульсов от будильника
	volatile unsigned long delta_alarm;
	volatile unsigned long timer_count; //!< счетчик сихронизации от внутреннего таймера
};
/*
typedef struct _DeviceState sDeviceState;

struct _DeviceState {
//	unsigned char IsR3ScanRunning:1;
//	THeliumDevStatus helium_dev_sta[R3_NUM_ADDRESS];
	volatile unsigned long sync_count; //!< счетчик импульсов от датчика пути, импульсы переводятся в метры
	volatile unsigned long delta_sync;
	volatile unsigned long alarm_count; //!< счетчик импульсов от будильника
	volatile unsigned long delta_alarm;
	volatile unsigned long timer_count; //!< счетчик сихронизации от внутреннего таймера
#ifdef R3_BUS
	R3BusState r3bus[R3_CH_NUM];
#endif
};
extern sDeviceState DeviceState;
*/
/*
typedef struct _DeviceHeliumConfig sDeviceHeliumConfig;
struct _DeviceHeliumConfig {
	float BigConstant;			//2
	float RootConstant;			//3
	float TempConstant;			//4 Для перевода в температуру
	float Ch2Constant;			//5 Для перевода в давление на втором канале
//	unsigned char Enabled:1;
};

*/
/*
typedef struct _DeviceConfig sDeviceConfig;

#ifndef AT91C_IFLASH_PAGE_SIZE
#define AT91C_IFLASH_PAGE_SIZE 512
#endif

struct _DeviceConfig {
	unsigned int ID;
#ifdef EMAC
	EMAC_iface iface[1];
	struct{
		unsigned char server[4];
		unsigned int log_level;
	}syslog;
	struct{
		unsigned char server[4];
		unsigned int timeout;
	}ntp;
#endif
#ifdef R3_BUS
	R3bus r3bus[R3_CH_NUM];
#endif
	unsigned int r3bus_autorun;
	enum {SYNC_EXT=0, SYNC_TIMER=1, SYNC_RTC=2} r3bus_sync_mode;
	unsigned int alarm_sync_divider;
//	char hostname[32];   //Пока нафиг не нужно
//	sDeviceHeliumConfig helium_config[R3_NUM_ADDRESS];

} __attribute__((aligned(AT91C_IFLASH_PAGE_SIZE)));

extern sDeviceConfig DeviceConfig __attribute__((section("CONFIG")));
*/
void R3BUS_calculate(unsigned char *rx_scan_buffer);
/*
int LCDMode(unsigned char NumDevice,unsigned char mode);
//void LCDASCII (unsigned char NumDevice,char * string,unsigned char size);
void LCD_show_pause();
void LCD_show_run();
void LCD_show_init();
void FramBackup_init();
//void FramBackup(void* arg);
*/
void r3_sys_init();//real-time os
int r3_sys_soft_reset(unsigned char* buffer, int*size);

//extern void FLASH_load_config();
//extern void FLASH_save_config();
//extern void r3_boot_image();
//extern void r3_boot_in_ram();


typedef struct _PortIrqCounter PortIrqCounter;
struct _PortIrqCounter {
    unsigned int mask;
    volatile unsigned int counter;
    volatile unsigned int pio_state;
    PortIrqCounter* next;
};
void R3_port_irq_init();

#if !defined(STM32F10X_MD) && !defined(STM32F10X_CL) 
//void R3_port_irq_append(PortIrqCounter* counter, const Pin* pin);
#endif

typedef void (*SignalCall)( void*);
void signal_emit(unsigned int signal_id);
void signal_cb  (unsigned int signal_id, SignalCall, void*);
// сигналы
#define MAC_LINK_UP 1
#define MAC_LINK_DOWN 2

#define BLOCK_MEDIA_SHIFT 9
#define BLOCK_MEDIA_UNIT (1 << BLOCK_MEDIA_SHIFT)
typedef struct _BlockMedia BlockMedia;
struct _BlockMedia {
    void*   data;
    int     (*init) (void*);
    void*   (*read) (void* fp,       void* buffer, uint32_t block, uint32_t size);
    int     (*send) (void* fp, const void* buffer, uint32_t block, uint32_t size);
    int     (*over) (void* fp, const void* buffer, uint32_t block, uint32_t offset, uint32_t size);
    int     (*unref)(void* fp, void* buffer);
    int     (*check)(void* fp, uint32_t segment);
    int     (*erase)(void* fp, uint32_t segment);
	int 	(*append)(void* fp, uint32_t offset, const void* buffer, uint32_t  len);

//    int     (*attach)(void* fp);
//    int     (*detach)(void* fp);

    uint32_t    num_blocks;
    int         page_size_log2;     //!< размер страницы в блоках  (sec_per_clus_log2), структурная единица
    int         segment_size_log2;  //!< размер сегмента в блоках, единица стирания данных
    const char* name;//!< имя носителя
};
/*! \brief This function provides accurate delay (in microseconds)
	возможно стоит реализовать переключение задач
 */
//static inline void usleep(uint32_t delay)
#endif /* R3RTOS_H */
