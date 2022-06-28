#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

/*! \ingroup _tracer
Уровни значимости системных сообщений:
-	0 Emergency: system is unusable
-	1 Alert: action must be taken immediately
-	2 Critical: critical conditions
-	3 Error: error conditions
-	4 Warning: warning conditions
-	5 Notice: normal but significant condition
-	6 Informational: informational messages
-	7 Debug: debug-level messages

    \{
  */
#define TRACE_EMERGENCY	0       //!< клиника, спасать поздно
#define TRACE_ALERT 	1       //!< красная тревога, пора перезагружаться
#define TRACE_CRITICAL 	2       //!< ошибка системная, типа ахтунг, места не хватило
#define TRACE_ERROR 	3       //!< ошибка приложения
#define TRACE_WARNING	4       //!< ошибка поправимая
#define TRACE_NOTE 		5       //!< повод отослать уведомление например по SNMP-trap
#define TRACE_INFO 		6       //!< повод распечатать информационное сообщение в терминале
#define TRACE_DEBUG 	7       //!< используются на этапе отладки протоколов

#ifndef TRACE_BUFFER_SIZE
#define TRACE_BUFFER_SIZE 2048  //!< размер буфера журанала сообщений
#endif
#define TRACE_BUFFER_MASK	 (TRACE_BUFFER_SIZE-1)

//! Макросы используются или планируются к использованию
//! в качестве маски источника отладочных сообщений, чтобы можно было находу отключать отладку
//! по признаку источника или перенаправлять в отдельный журнал сообщений как это сделано в
//! glib

#define TRACE_NONE		0x00
#define TRACE_DBGU		0x01
#define TRACE_UART		0x02
#define TRACE_CAN		0x04
#define TRACE_NET		0x08
#define TRACE_FLASH		0x10
#define TRACE_R3LOG		0x20

//! \}

// Attention: TRACE_UDP нигде не объявлен
#define TRACE_DEV TRACE_UDP             //!< устройство по умолчанию, куда ориентировать уведомления

typedef struct _Terminal Terminal;
/// \ingroup _tracer
/// \brief Привязка терминального устройства
struct _Terminal {
    unsigned char * ptr;    //!< указатель на позицию чтения из буфера терминала
    unsigned short width;   //!< ширина окна терминала в символах и
    unsigned short height;  //!< высота окна в символах
};

int	TRACE_gets(Terminal* dev, unsigned char * buffer, int size);
void TRACE_terminal_init(Terminal* dev);
void TRACE_log(int log_level, const char *format, ...) __attribute__ ((format (printf,2,3)));
//int	TRACE_copy(unsigned char * buffer, int size);

#endif // TRACE_H_INCLUDED
