/*! tracer.c (c)	Geolab, 2007, 2008, 2009, 2010, 2014, 2015, 2016
	\author Anatoly Georgievsky
*/

/*!
	\ingroup _system
	\defgroup _tracer TRACER подсистема вывода отладочной информации
	\brief Используется для отображения и отладки работы системы. А также
	используется как стандартный поток вывода текстовой информации.
	Поток может быть перенаправлен в USB или терминал Telnet, может быть перенаправлен
	на последовательный порт.

	Трейсер представляет собой циклический буфер, в который валятся отладочные
	сообщения от системных служб. Назначение трейсера - журнал системы. Трейсер используется
	в отладочных целях.
	Всякое терминальное устройство может подключится и скопировать из буфера
	данные для отображения в своем терминальном устройстве.

	Задумана классификация сообщений по типу Syslog: критические, ошибочные, отладочные
	и пр. Возможно перенаправление сообщений в Syslog. Возможно отображение не всех
	сообщений, а только тех которые достаточно важны для отображения. Критерий отображения
	задается уровнем.

    \{
*/
#include "board.h"
#include "r3stdlib.h"

#include <stdarg.h>
#include <cmsis_os.h>
#include "trace.h"

#define TRACE_STR_SIZE 128 //!< максимальная длина одного сообщения

//!
static	unsigned char tx_buffer[TRACE_BUFFER_SIZE]; //!< циклический буфер
static volatile unsigned char *tracer_ptr = &tx_buffer[0]; //! указатель позиции записи в буфере

/*! Эта функция проходит инициализацию в первую очередь.
    Требование к функции выделения памяти - должно работать до процедуры
    инициализации выделения памяти.
*/
void TRACE_Init()
{
#if 0
	Tracer * tracer = mb_alloc(sizeof(Tracer));
	//tracer->log_level = 0;
	tracer_ptr = &tracer->tx_buffer[0];
	systracer = tracer;
#endif
}

/*! \brief регистрирует терминальное устройство */
void TRACE_terminal_init(Terminal* dev)
{
    dev->ptr = &tx_buffer[0];
}
/*! \brief Выполняет копирование системного буфера в буфер терминала */
int	TRACE_gets(Terminal* dev, unsigned char * buffer, int size)
{
    int idx = 0;
    unsigned char * src = dev->ptr;
    unsigned char * top = &tx_buffer[TRACE_BUFFER_SIZE];
    while (src != tracer_ptr && idx < size)
    {
        buffer[idx++] = *src++;
        if (src == top) src-= TRACE_BUFFER_SIZE;
    }
    dev->ptr = src;
    return idx;
}

static int trace_fputs(const char *str)
{
	const char* s = str;
	unsigned char* dst = (void*)tracer_ptr;//&trace->tx_buffer[trace->write_pos];
	unsigned char* top = &tx_buffer[TRACE_BUFFER_SIZE];
	while (*s){
        *dst++ = *s++;
        if (dst == top) dst -= TRACE_BUFFER_SIZE;
    }
    tracer_ptr = dst;
    return s - str;
}

/*!	\brief вывод одного симовла
    \return возвращает количество символов в записи
*/
int putchar(int ch)
{
	unsigned char* dst = (void*)tracer_ptr;//&trace->tx_buffer[trace->write_pos];
	unsigned char* top = &tx_buffer[TRACE_BUFFER_SIZE];
    *dst++ = ch;
    if (dst == top) dst -= TRACE_BUFFER_SIZE;
    tracer_ptr = dst;
	return ch;
}
#if 0
/*! \brief используется для записи форматированного в стиле printf сообщения в поток отладки
    \param[in] log_level - идентификатор уровня отладки DEBUG, INFO, CRITICAL, WARNING, ERROR и т.д.
    \param[in] format - формат
    */
void syslog(int log_level, const char *format, ...) {
	Tracer * trace = systracer;
	if (log_level < trace->log_level) return;

    va_list ap;
    char str[TRACE_STR_SIZE];

    va_start(ap, format);
    vsnprintf(str, TRACE_STR_SIZE-1, format, ap);
    va_end(ap);
    TRACE_puts(trace, str);
}
#endif

/*! \brief используется для записи текстовых сообщений форматированных в стиле printf
	\ingroup _libc POSIX_DEVICE_IO
*/
int printf(const char *format, ...)
{
    char str[TRACE_STR_SIZE];
    va_list ap;
    va_start(ap, format);
    vsnprintf(str, TRACE_STR_SIZE-1, format, ap);
    va_end(ap);
    return trace_fputs(str);
}
int vprintf(const char *format, va_list ap){
    char str[TRACE_STR_SIZE];
    vsnprintf(str, TRACE_STR_SIZE-1, format, ap);
    return trace_fputs(str);
}
/*! \brief используется для записи текстовых строк
*/
int puts(const char *str)
{
    return trace_fputs(str);
}

//! \}
