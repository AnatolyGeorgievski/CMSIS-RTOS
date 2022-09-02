/* r3_object.h */
#ifndef R3_OBJECT
#define R3_OBJECT
#include <stdbool.h>
#include <stdint.h>
#include "r3_asn.h"

#ifndef BACnet_H
/*! Классификация объектов */
enum _BACnetObjectType {
ANALOG_INPUT 	= 0,
ANALOG_OUTPUT 	= 1,// commandable, сигнал обновления (флаг)
ANALOG_VALUE 	= 2,
BINARY_INPUT 	= 3,
BINARY_OUTPUT 	= 4,
BINARY_VALUE 	= 5,
CALENDAR 		= 6,
COMMAND 		= 7,// представляет последовательность команд Sequence_t
_DEVICE 		= 8,
EVENT_ENROLLMENT= 9,
_FILE 			= 10,
_GROUP 			= 11,
LOOP 			= 12,
MULTI_STATE_INPUT 	= 13,
MULTI_STATE_OUTPUT 	= 14,
_NOTIFICATION_CLASS = 15,
PROGRAM 			= 16,
SCHEDULE 			= 17,// значение переменной по расписнию и список свойств-объектов, которые перезаписываются
AVERAGING 			= 18,// любой цифровой фильтр с одним входом и одним выходом попадает в эту категорию.
MULTI_STATE_VALUE 	= 19,// не вижу разницы между MULTI_STATE_INPUT
TREND_LOG 			= 20,
LIFE_SAFETY_POINT 	= 21,
LIFE_SAFETY_ZONE 	= 22,
ACCUMULATOR 		= 23,// считает импульсы умножает на дробь N/M накопление ошибки округления
PULSE_CONVERTER 	= 24,// преобразует целое в вещественное
// набор по 2004 году
EVENT_LOG 			= 25,
GLOBAL_GROUP 		= 26,
TREND_LOG_MULTIPLE 	= 27,
LOAD_CONTROL 		= 28,
STRUCTURED_VIEW     = 29,

BITSTRING_VALUE 	= 39,
CHARACTERSTRING_VALUE = 40,
DATE_PATTERN_VALUE 	= 41,
DATE_VALUE 			= 42,
DATETIME_PATTERN_VALUE = 43,
DATETIME_VALUE 		= 44,
INTEGER_VALUE 		= 45,
LARGE_ANALOG_VALUE 	= 46,
OCTETSTRING_VALUE 	= 47,
POSITIVE_INTEGER_VALUE = 48,
TIME_PATTERN_VALUE 	= 49,
TIME_VALUE 			= 50,
NOTIFICATION_FORWARDER = 51,
ALERT_ENROLLMENT 	= 52,
CHANNEL 			= 53,
LIGHTING_OUTPUT 	= 54,
BINARY_LIGHTING_OUTPUT = 55,
NETWORK_PORT        = 56,
};
#endif // BACnet_H

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

enum _NodeType {
NodeType_unknown =(0),
NodeType_system =(1),
NodeType_network =(2),
NodeType_device =(3),
NodeType_organizational =(4),
NodeType_area =(5),
NodeType_equipment =(6),
NodeType_point =(7),
NodeType_collection =(8),
NodeType_property =(9),
NodeType_functional =(10),
NodeType_other =(11),
NodeType_subsystem =(12),
NodeType_building =(13),
NodeType_floor =(14),
NodeType_section =(15),
NodeType_module =(16),
NodeType_tree =(17),
NodeType_member =(18),
NodeType_protocol =(19),
NodeType_room =(20),
NodeType_zone =(21)
};
#if 0

enum _ErrorClass {
DEVICE      = (0),
OBJECT      = (1),
PROPERTY    = (2),
RESOURCES   = (3),
SECURITY    = (4),
SERVICES    = (5),
VT          = (6),
COMMUNICATION=(7),
};
enum _DeviceError {
    OTHER,
    CONFIGURATION_IN_PROGRESS,
    DEVICE_BUSY,
    INCONSISTENT_CONFIGURATION,
    INTERNAL_ERROR,
    NOT_CONFIGURED,
    OPERATIONAL_PROBLEM,
};
enum _ObjectError {
//    OTHER,
    BUSY=1,
    DYNAMIC_CREATION_NOT_SUPPORTED,
    FILE_FULL,
    LOG_BUFFER_FULL,
    NO_ALARM_CONFIGURED,
    NO_OBJECTS_OF_SPECIFIED_TYPE,
    OBJECT_DELETION_NOT_PERMITTED,
    OBJECT_IDENTIFIER_ALREADY_EXISTS,
    OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED,
//    READ_ACCESS_DENIED,
    UNKNOWN_OBJECT,
    UNSUPPORTED_OBJECT_TYPE,
};
enum _PropertyError {
//    OTHER,
    CHARACTER_SET_NOT_SUPPORTED=1,
    DATATYPE_NOT_SUPPORTED,
    DUPLICATE_ENTRY,
    DUPLICATE_NAME,
    DUPLICATE_OBJECT_ID,
    INCONSISTENT_SELECTION_CRITERION,
    INVALID_ARRAY_INDEX,
    INVALID_DATATYPE,
    INVALID_VALUE_IN_THIS_STATE,
    LOGGED_VALUE_PURGED,
    NO_PROPERTY_SPECIFIED,
    NOT_CONFIGURED_FOR_TRIGGERED_LOGGING,
    NOT_COV_PROPERTY,
//    OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED,
    PROPERTY_IS_NOT_AN_ARRAY,
    READ_ACCESS_DENIED,
    UNKNOWN_PROPERTY,
    UNKNOWN_FILE_SIZE,
    VALUE_NOT_INITIALIZED,
    VALUE_OUT_OF_RANGE,
    VALUE_TOO_LONG,
    WRITE_ACCESS_DENIED,
};
enum _ResourceError {
//    OTHER,
    NO_SPACE_FOR_OBJECT,
    NO_SPACE_TO_ADD_LIST_ELEMENT,
    NO_SPACE_TO_WRITE_PROPERTY,
    OUT_OF_MEMORY
};
#endif
#define OBJECT_ID_MASK  0x3FFFFF // 22 бита
#define UNDEFINED       0x3FFFFF

#define OID(type, id) ((type<<22)|(id & OBJECT_ID_MASK))
#define DEFAULT_DEVICE  OID(_DEVICE, UNDEFINED)
typedef float REAL;
typedef uint32_t Time_t;//
typedef uint32_t Date_t;
typedef uint32_t ObjectIdentifier_t;
static inline uint32_t r3_object_type(ObjectIdentifier_t object_identifier) {
    return object_identifier>>22;
}

//#define BIT_STRING(size) uint32_t count; type* array; }
#define  LIST_OF(type) struct { List* list; }
#define ARRAY_OF(type) struct { uint32_t count; type* array; }
#define OFFSET(type, member) ((uintptr_t)&((type*)0)->member)

typedef struct _Object Object_t;
typedef struct _ObjectPropertyReference ObjectPropertyReference_t;
typedef struct _CommandableObject CommandableObject_t;
typedef struct _DateTime  DateTime_t;
typedef struct _OctetString OctetString_t;
typedef struct _BACnetAddress BACnetAddress_t;
typedef struct _BACnetRouterEntry BACnetRouterEntry_t;
typedef struct _NetworkPortObject NetworkPort_t;
typedef struct _DataLink DataLink_t;

/*
struct _BACnetAddress {
    uint16_t network_number;//!< A value of 0 indicates the local network
    uint8_t mac_len;        //!< A string of length 0 indicates a broadcast
    uint8_t status; // enum {available (0),busy (1),disconnected (2)}
    union {
        uint8_t*  p;// в форме строки
        uintptr_t u;// максимальное целое
        uint8_t b[sizeof(uintptr_t)];
    } mac_address;

};*/
typedef struct _BACnetAddressBinding    BACnetAddressBinding_t;
typedef struct _BACnetAddressBindingEx  BACnetAddressBindingEx_t;

/* базвый класс для объектов */
struct _Object {
    ObjectIdentifier_t object_identifier; //!< 22 бита, порядковый номер, 10 старших бит- тип устройства
    char* object_name;  //!< Уникальное имя объекта, имя устройства должно быть уникально на сети
    ARRAY_OF(const PropertySpec_t) property_list;
    // Description
    // Profile_Name
    // Profile_Location
    // Tags
};
struct _DateRange {
    Date_t start_date;
    Date_t end_date;
};
struct _DateTime {
    Date_t date;
    Time_t time;
};
struct _DeviceObjectReference {
    uint32_t device_identifier;
    ObjectIdentifier_t object_identifier;
};
struct _DeviceObjectPropertyReference {
    uint32_t device_identifier;
    ObjectIdentifier_t object_identifier;
    uint16_t property_identifier;//
    uint16_t property_array_index;
};
typedef int (*Commandable_cb)(Object_t* object, uint32_t property_identifier, void* user_data);
struct _Commandable {
    Commandable_cb cb;
	void * user_data;
};

struct _CommandableObject {
    struct _Object instance;
    struct _Commandable commandable;
    REAL present_value;

};
enum _StatusFlags {
	STATUS_FLAG_IN_ALARM,
	STATUS_FLAG_FAULT,
	STATUS_FLAG_OVERRIDDEN,
	STATUS_FLAG_OUT_OF_SERVICE
};


struct _AnalogInputObject {
    struct _Object instance;
    REAL present_value;
    uint16_t    units;
    volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
};
struct _AnalogValueObject {
    struct _Object 		instance;
	struct _Commandable commandable;
    REAL present_value;
    uint16_t    units;
    volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
// ограничения в интерфейсе
    REAL relinquish_default;
	REAL min_pres_value, max_pres_value, resolution;


};
struct _AnalogOutputObject{
    struct _Object instance;
	struct _Commandable commandable;
    REAL present_value;
	uint16_t    units;
    volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
    REAL relinquish_default;
	REAL min_pres_value, max_pres_value, resolution;
};
struct _BinaryInputObject {
    struct _Object instance;
    uint8_t present_value;
    uint8_t polarity;
};
struct _BinaryValueObject {
    struct _Object instance;
	struct _Commandable commandable;
    uint8_t present_value;
    uint8_t polarity;
};
struct _BinaryOutputObject{
    struct _Object instance;
	struct _Commandable commandable;
    uint8_t present_value;
    uint8_t polarity;
    uint8_t relinquish_default;
};
struct _LightingOutputObject {
    struct _Object instance;
    REAL present_value;
	volatile uint8_t status_flags;// BIT_STRING(BACnetStatusFlags)
    // in_process;
};
union _CalendarEntry {
    struct _DateRange range;
    Date_t date;
};
struct _CalendarObject{// 12.9 Calendar Object Type
    struct _Object instance;
    bool present_value;
    LIST_OF(struct _CalendarEntry) date_list;
};
struct _CommandObject{// 12.10 Command Object Type
    struct _Object instance;
	struct _Commandable commandable;
    uint32_t present_value;
    bool in_process;
    bool all_writes_successful;
    ARRAY_OF(struct _ActionList) action;
};
struct _DeviceObject{// 12.11 Device Object Type
    struct _Object instance;
    enum _DeviceStatus {
        OPERATIONAL, OPERATIONAL_READ_ONLY, DOWNLOAD_REQUIRED,
        DOWNLOAD_IN_PROGRESS, NON_OPERATIONAL, BACKUP_IN_PROGRESS
    } system_status;
	const char* vendor_name;
	const char* model_name;
	const char* serial_number;      //!< Серийный номер устройства или MAC адрес сетевого интрефейса eth0
	const char* firmware_revision;  //!< в это поле попадает версия сборки при загрузке, поменять нельзя, только после перезагрузки
	const char* application_software_version;

	char* location;                 //!< ОПЦИЯ размещение устройства
	char* description;              //!< ОПЦИЯ описание устройства

    uint16_t vendor_identifier;
    uint16_t max_apdu_length_accepted;
    enum _Segmentation /* ::= ENUMERATED */{
        SEGMENTED_BOTH =(0),
        SEGMENTED_TRANSMIT =(1),
        SEGMENTED_RECEIVE =(2),
        NO_SEGMENTATION =(3)
    } segmentation_supported;

	const /* BIT_STRING(127) */ uint32_t protocol_object_types_supported[4];
	/* BIT_STRING(64) */  uint32_t protocol_services_supported[2];


	uint32_t database_revision;
    Time_t 	local_time;
    Date_t 	local_date;
    DateTime_t last_restore_time;//!< Значение должно сохраняться во флеш

	uint8_t last_restart_reason;
    ARRAY_OF(ObjectIdentifier_t) object_list;
    ARRAY_OF(ObjectIdentifier_t) structured_object_list;//!< Содержит представление системы в форме дерева -- файловая система
    ARRAY_OF(ObjectIdentifier_t) configuration_files;
    LIST_OF(BACnetAddressBinding_t) device_address_binding;
    LIST_OF(BACnetCOVSubscription_t) active_cov_subscriptions;

    uint16_t backup_failure_timeout;
    uint16_t backup_preparation_time;
    uint16_t restore_preparation_time;
    uint16_t restore_completion_time;
    enum _BackupState /* ::= ENUMERATED */ {
        BACKUP_STATE_IDLE 		=(0),
        PREPARING_FOR_BACKUP    =(1),
        PREPARING_FOR_RESTORE   =(2),
        PERFORMING_A_BACKUP     =(3),
        PERFORMING_A_RESTORE    =(4),
        BACKUP_FAILURE          =(5),
        RESTORE_FAILURE         =(6)
    } backup_and_restore_state;
//    struct _TimeStamp last_restore_time;
// приватные
//    tree_t* objects;// списко объектов принадлежащих устройству в системе
    NetworkPort_t* network_ports;
};
struct _FileObject{// 12.13 File Object Type
    struct _Object instance;
    uint32_t file_size;
    char *file_type;// bin, cfg...
    DateTime_t modification_date;
    bool archive;// выставляем в TRUE когда надо записать файл во флеш
    bool read_only;
    //enum BACnetFileAccessMethod
    void *fp;
//    int32_t start_position;
//    size_t   data_len;
};
struct _GroupObject{// 12.14 Group Object Type
    struct _Object instance;
    LIST_OF(struct _ReadAccessSpecification) list_of_group_members;
    LIST_OF(struct _ReadAccessResult) present_value;
};
struct _LoopObject{// 12.17 Loop Object Type -- внешнее представление регулятора
    struct _Object instance;
    REAL present_value;
	uint32_t update_interval; //  interval in milliseconds at which the loop algorithm updates the output
	int (*update)(struct _LoopObject*);
};
struct _NotificationClassObject {// 12.21 Notification Class Object Type
	struct _Object instance;
	uint32_t notification_class;
	// 12.21.8 Recipient_List
	ARRAY_OF(uint16_t) property_list;
};
struct _TimeValue {
	Time_t time;// время кодируется секундами часы(5):минуты(7):сек(7):сотые(7), старшие биты можно использовать на фалги
	struct _AnyValue value;// ON/OFF
};
// 12.22 Program Object Type
// 12.23 Pulse Converter Object Type
struct _ScheduleObject{// 12.24 Schedule Object Type
    struct _Object instance;
	struct _AnyValue  present_value;
	struct _AnyValue  schedule_default;
	struct _DateRange effective_period;
	struct _TimeValue* weekly_schedule[7];
//	ARRAY_OF(struct _SpecialEvent) exception_schedule;
};
struct _TrendLogObject{// 12.25 Trend Log Object Type
	struct _Object instance;
	struct _DateTime start_time;
	struct _DateTime stop_time;
	uint32_t log_interval; // specifies the periodic interval in hundredths of seconds
	uint32_t buffer_size;
	struct _DeviceObjectPropertyReference log_device_object_property;
	LIST_OF(struct _LogRecord) log_buffer;
};
// 12.26 Access Door Object Type
struct _AccessRule {
	// флаги: enable, time-range-specifier(specified=0|always=1), location-specifier (0|all=1)

/* BACnetDeviceObjectPropertyReference, references a
property that can be evaluated to TRUE or FALSE, which defines whether the
rule is valid (TRUE) or not (FALSE). .. unspecified if it contains 4194303  */
	struct _DeviceObjectReference time_range;
/* Location This optional field, of type BACnetDeviceObjectReference, refers to the Access
Point or Access Zone that this access rule is valid for.
The Location reference shall be considered unspecified if it contains 4194303 */
	struct _DeviceObjectReference location;
//	struct _TimeRange time_range;
};

struct _EventLogRecord {
	struct _DateTime timestamp;
	ObjectIdentifier_t event_object_identifier; // источник
	uint8_t * notification;// запись на диске?? в формате  ConfirmedEventNotification-Request
	//CHOICE_OF(union _event_log_datum) log_datum;
};
struct _EventLogObject{// 12.27 Event Log Object Type
	struct _Object instance;
	struct _DateTime start_time;
	struct _DateTime stop_time;
	uint32_t buffer_size;
	uint32_t record_count;
	uint32_t total_record_count;
	LIST_OF(struct _EventLogRecord) log_buffer;
};
// 12.28 Load Control Object Type
struct _StructuredViewObject{// 12.29 Structured View Object Type
    struct _Object instance;
    enum _NodeType node_type;
    struct _DeviceObjectReference represens;
    ARRAY_OF(struct _DeviceObjectReference) subordinate_list;
};

struct _NetworkPortObject       {// 12.56 Network Port Object Type
    struct _Object instance;
    uint16_t network_number;
    uint16_t APDU_length;
    uint32_t MAC_address[2];
    REAL Link_Speed;
    char* network_interface_name;

    uint8_t /* enum _BACnetNetworkType */ network_type;
    // параметры применимы к порту MS/TP
    uint8_t max_master;// см Nmax_master
    uint8_t max_info_frames;// \sa Mmax_info_frames
/* \see datalink */
    uint8_t ip_address[4];
    uint8_t ip_subnet_mask[4];
    uint8_t ip_default_gateway[4];
    uint16_t BACnet_IP_UDP_Port;

    bool slave_proxy_enable;
    LIST_OF(BACnetAddressBindingEx_t) manual_slave_address_binding;// расширенная привязка
    bool auto_slave_discovery;
    LIST_OF(BACnetAddressBinding_t) slave_address_binding;
#if 1 //defined(B_RTR) // BACnet Router (B-RTR) \see ANNEX A
    LIST_OF(BACnetRouterEntry_t)    routing_table;// Routing_Table BACnetLIST of BACnetRouterEntry
#endif // defined
// приватные
    DataLink_t* datalink;
    struct _NetworkPortObject * next;
};
struct _StringValueObject{// 12.37 CharacterString Value Object Type
	struct _Object instance;
	struct _Commandable commandable;
	char* present_value;
	volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
};
struct _DateTimeValueObject{// 12.38 DateTime Value Object Type
	struct _Object instance;
	struct _Commandable commandable;
	struct _DateTime present_value;
//    struct _DateTime relinquish_default;
	volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
};
struct _TimeValueObject{// 12.42 Time Value Object Type
	struct _Object instance;
	struct _Commandable commandable;
	Time_t present_value;
    Time_t relinquish_default;
	volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
};
struct _DateValueObject{// 12.45 Date Value Object Type
	struct _Object instance;
	struct _Commandable commandable;
	Date_t present_value;
//    Date_t relinquish_default;
	volatile uint8_t status_flags; // BIT_STRING(BACnetStatusFlags)
};

struct _ObjectPropertyReference {
    uint32_t object_identifier;
    uint32_t/* enum _BACnetPropertyIdentifier */ property_identifier;// используются 22 бита
    uint16_t property_array_index;// можно оставить только 10 бит, тогда уложится в 32 бита
    uint16_t context_id;// индекс в таблице свойств объекта
    const PropertySpec_t * pspec;// таблица свойств объекта

//	Object_t* object;// Ссылка на объект
//    void* data;// вычисленное значение Можно его отсюда исключить
//	struct _Commandable *commandable;
};

typedef struct _COV_value COV_value_t;
struct _COV_value {
	union {
		float f;
		int32_t i;
	};
	void* process_identifier;
};


//static uint8_t * _pspec_encode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const PropertySpec_t* desc);
//
#if 0
#include <time.h>
static inline uint32_t bcd2bin_time(uint32_t v)
{
	return (v&0x0F0F0F00) - ((v&0xF0F0F000)>>4)*6;
}

/*! \brief Преобразовать локальное представление даты в формат BACnet

	Date values shall be encoded in the contents octets as four binary integers. The first contents octet shall represent the year minus
1900; the second octet shall represent the month, with January = 1; the third octet shall represent the day of the month; and the
fourth octet shall represent the day of the week, with Monday = 1. A value of X'FF' = D'255' in any of the four octets shall
indicate that the corresponding value is unspecified and shall be considered a wildcard when matching dates. If all four octets =
X'FF', the corresponding date may be interpreted as "any" or "don't care."

*/
static inline Date_t date_encode_mask(const struct tm* tv, Date_t mask)
{
	union {
		struct {// представление времени в формате BACnet
			uint32_t wday:8;
			uint32_t mday:8;
			uint32_t mon :8;
			uint32_t year:8;
		};
		Date_t u32;
	} t;
	t.wday = tv->tm_wday;
	t.mday = tv->tm_mday;
	t.mon  = tv->tm_mon;
	t.year = tv->tm_year;
	t.u32 = bcd2bin_time(t.u32);
	t.year+=100;
	return t.u32 | mask;
}
/*! \brief Преобразовать локальное представление времени в формат BACnet

	Time values shall be encoded in the contents octets as four binary integers. The first contents octet shall represent the hour, in
the 24-hour system (1 P.M. = D'13'); the second octet shall represent the minute of the hour; the third octet shall represent the
second of the minute; and the fourth octet shall represent the fractional part of the second in hundredths of a second. A value of
X'FF' = D'255' in any of the four octets shall indicate that the corresponding value is unspecified and shall be considered a
wildcard when matching times. If all four octets = X'FF', the corresponding time may be interpreted as "any" or "don't care."
*/
static inline Time_t time_encode_mask(const struct tm* tv, Time_t mask)
{
	union {
		struct {// представление времени в формате BACnet
			uint32_t hundredths:8;
			uint32_t sec:8;
			uint32_t min:8;
			uint32_t hours:8;
		};
		Time_t u32;
	} t;
	t.hundredths = tv->tm_usec/10;
	t.sec = (tv->tm_sec);
	t.min = (tv->tm_min);
	t.hours = (tv->tm_hour);
	t.u32 = bcd2bin_time(t.u32);
	return t.u32 | mask;
}
#endif
void* r3cmd_get_property(DeviceInfo_t* devinfo, ObjectIdentifier_t object_identifier, uint32_t property_identifier, uint16_t property_array_index);
Object_t* r3cmd_object_new(DeviceInfo_t* devinfo, ObjectIdentifier_t object_identifier);
int r3cmd_object_init   (DeviceInfo_t* devinfo, Object_t* object, ObjectIdentifier_t object_identifier);
int r3cmd_object_create (DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t *response_buffer);
int r3cmd_object_delete (DeviceInfo_t* devinfo, uint8_t * buffer, size_t length);
//int r3cmd_object_confirm(tree_t** device_tree, ObjectIdentifier_t object_identifier);
uint8_t * r3cmd_property_encode (uint8_t * buf, Object_t *object, uint32_t prop_id);

int r3cmd_read_property (DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t *response_buffer);
int r3cmd_read_property_multiple(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t * response_buffer);
int r3cmd_read_property_multiple_cnf(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t * response_buffer);
int r3cmd_write_property(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, int hint_create_objects);
int r3cmd_write_property_multiple(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t * response_buffer);

//void r3_object_init(uint32_t oid, const Object_t* object);
#endif //R3_OBJECT
