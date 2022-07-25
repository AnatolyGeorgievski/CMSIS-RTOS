#ifndef R3V2PROTOCOL_H
#define R3V2PROTOCOL_H

/*! \ingroup _r3_cmd

    \{
*/

//! r3 bus states
//!<code>
#define R3_ERR_OK	  	0x00    //0<<5
#define R3_ERR_SEG      0x20    //1<<5
//                      0x40    //2<<5
//                      0x60    //3<<5
#define R3_ERR_NAK      0x80    //4<<5 
#define R3_ERR_ERROR    0x80    //4<<5 
#define R3_ERR_LINE		0xA0    //5<<5
#define R3_ERR_TIMEOUT	0xC0    //6<<5
#define R3_ERR_BUSY		0xC0
#define R3_ERR_CRC		0xE0    //7<<5
#define R3_ERR_ABORT	0xE0    //7<<5
//!</code>

/*!	\brief Маска выделения команд протокола R3.

	Для формирования команды используются таршие 3 бита в байте. */
#define R3_CMD_MASK 	0xE0

/*!	\brief Маска выделения адреса устройства на линии RS-485.

	Для формирования адреса устройства на линии используются
	младшие  5 бит. Адрес может быть от 0 - 31. Адрес 0 используется при
	обращении к мастеру шины и для формирования широковещательных запросов.

	Адрес 31 может быть задействован в процедуре динамического назначения адресов и
	автоматической конфигурации.
*/
#define R3_ADDRESS_MASK	0x1F
/*! \brief Команда запроса идентификатора (PING)

    Команда используется для распознавания устройства и детектирования устройства на линии.
    При опросе устройства через USB, в качестве отклика возвращается строка содержащая
    информацию о модели устройства и версии программного обеспечения.

    При опросе устройства на линии сбора данных в ответ на команду PING возвращается статус устройства
    длина отклика - один байт.
*/
#define R3_CMD_ID 		0x00
/*! \brief Запрос пакетной передачи данных. Запрос применятеся при работе с УСД

    В качестве отклика возвращается пакета данных с результатами опроса всех устройств на линии.
*/
#define R3_CMD_GET_BULK	0x20
/*! \brief Запрос результата преобразования

    При помощи команды GET произаводится периодический опрос всех устройств на линии.
*/
#define R3_CMD_GET		0x40
/*! \brief Команда синхронизации SYNC

    Команда используется для синхронизации датчиков на линии. По команде каждый датчик должен
    сбросить результат последнего измерения в буфер отсылки и начать очередное измерение.
*/
#define R3_CMD_SYNC 	0x60
/*! \brief Команда FLASH введена для обновления микрокода (firmware) датчика

    Процедура обновления микрокода датчика описана в приложени к спецификации протокола R3.
*/
#define R3_CMD_FLASH	0x80
/*! \brief Команда инициализации датчика, INIT

    Инициализация датчика производися до начала цикла опроса.
*/
#define R3_CMD_INIT		0xA0
/*! \brief Расширенные команды управления, SNMP

*/
#define R3_CMD_SNMP 	0xC0
/*! \brief Вывод журнала сообщений системы, TRACE

Используется при работе на шине USB для вывода текстовых сообщений.
Команды TRACE используются только в процессе отладки системы, однако
на базе команд может TRACE быть построен журнал сообщений, с перенаправлением в Syslog.
*/
#define R3_CMD_TRACE 	0xE0
//! \}

/*! \ingroup _r3_snmp
    \{
*/
// Все команды чтения имеют признак 1 в старшем разряде
// команда чтения от команды записи отличается на 0x80+


#define R3_SNMP_ID				0x00
#define R3_SNMP_GET_ID			0x80
// команды разрешения работы устройств на шине
#define R3_SNMP_SET_ENABLE 		0x02
#define R3_SNMP_GET_ENABLE 		0x82
// конфигурация 
#define R3_SNMP_SAVE_CONFIG		0x03
#define R3_SNMP_SHOW_CONFIG     0x83
// запуск основной деятельности по сбору данных
#define R3_SNMP_RUN				0x04
// календарь
#define R3_SNMP_GET_TIMESTAMP	0x85
// календарь
#define R3_SNMP_SET_DATE		0x06
#define R3_SNMP_GET_DATE		0x86

/* Low-level pin control
  следом идут номер порта (0..2) и номер ноги */
#define R3_SNMP_SET_PIN             0xB0
#define R3_SNMP_CLEAR_PIN           0xB1


#define R3_SNMP_MEMORY_TEST             0x27
#define R3_SNMP_SLICE_TEST              0x29

#define R3_SNMP_BUS_TEST                0x2A    //!< диагностика устройств на линии RS-485
#define R3_SNMP_BUS_CSR                 0x2B    //!< профиль опроса линии
#define R3_SNMP_SET_BUS_CONFIG          0x2C    //!< конфигурация интерфейса: адрес на шине, частоту, период опроса, режим синхронизации
#define R3_SNMP_GET_BUS_CONFIG          0xAC
#define R3_SNMP_SET_BUS_SPEED           0x2D    //!<Огрызок от R3_SNMP_SET_BUS_CONFIG, установка скорости линии
// установить конфигурацию устройства,  C008 BUS_ID DEV_ID RX_SIZE
#define R3_SNMP_SET_DEVICE_CONF	0x08
// опросить конфигурацию устройства,    C088 BUS_ID DEV_ID
#define R3_SNMP_GET_DEVICE_CONF	0x88
// опросить конфигурацию канала         C089 BUS_ID, возвращает все размеры, 0-255
#define R3_SNMP_SET_SIZES		0x09
#define R3_SNMP_GET_SIZES		0x89
#define R3_SNMP_SET_SYNC_MODE	0x0A
#define R3_SNMP_GET_SYNC_MODE	0x8A


#define R3_SNMP_SOFT_RESET		0x70
#define R3_SNMP_SHOW_VERSION    0x71	//!< Показать версию системы Trace
#define R3_SNMP_GET_VERSION    	0xF1	//!< Возвращает строку -- версию системы
#define R3_SNMP_GET_R3	    	0xF2	//!< Возвращает маску команд
#define R3_SNMP_SHOW_MODULES    0x72    //!< Выводит список активных модулей
#define R3_SNMP_SHOW_ARP_TABLE  0x73
#define R3_SNMP_SHOW_MIBS       0x74
#define R3_SNMP_SET_USB_SERIAL  0x75 //!< устанавливает серийный номер для USB контроллера
#define R3_SNMP_SHOW_R3_BUS     0x76 //!< Показать список устройств на шине и их размеры, C07600
#define R3_SNMP_SHOW_R3         0x77 //!< Показать список всех зарегистрированных команд R3-SNMP

#define R3_SNMP_SHOW_SPI        0x78 //!< показать список всех устройств SPI
#define R3_SNMP_SHOW_TASKS      0x79 //!< показать список всех отложенных заданий

#define R3_SNMP_CACHE_ENABLE    0x7A //!< включить кеш

#define R3_SNMP_TRACE			0xFE//!< Запросить фрагмент журнала
#define R3_SNMP_SET_REG			0x7F
#define R3_SNMP_GET_REG			0xFF

//State Machine
#define R3_SNMP_FSM_PRINT 0x0D
#define R3_SNMP_FSM_SET 0x0E

//! \}
#define R3_BUFFER_NUM 1		// буферы отсылки для USB
#define R3_BUFFER_SIZE 64   // размер буфера USB
/*! правила форматирования слов */
#define R3_INT32(buf) ((uint32_t)(buf[0]) + (uint32_t)(buf[1]<<8) + (uint32_t)(buf[2]<<16) + (uint32_t)(buf[3]<<24))
#define R3_UINT16(buf) ((uint16_t)(((uint8_t*)buf)[0]) + (uint16_t)(((uint8_t*)buf)[1]<<8))
#define R3_INT8(buf)  ((uint32_t)(buf[0]))

/// регистрация команд протокола R3
typedef struct _R3cmdInfo R3cmdInfo;
struct _R3cmdInfo
{
    uint8_t code;   //!< код команды
    uint8_t level;  //!< уровень привилегий
//    const char* name;   //!< имя команды
    int (*cb)(unsigned char* buffer, int* size);   //!< обработка команды на буфере
};
#define R3CMD(key, callback) const R3cmdInfo key##_key __attribute__((used, aligned(4), section("r3_cmd_callbacks"))) \
    = { .code = key, .cb = callback }

	int R3_cmd_snmp_recv(uint8_t code, uint8_t * buffer, int length);
	int R3_cmd_snmp_recv2(uint8_t code, uint8_t * buffer, int *length);

#endif //R3V2PROTOCOL_H
