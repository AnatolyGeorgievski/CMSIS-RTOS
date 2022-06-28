/*! */
#ifndef R3_ASN_H
#define R3_ASN_H
#include <stdint.h>
#include "r3_slice.h"
// флаги для таблиц описания данных 
#define RD (1<<10)
#define RO (1<<10)
#define WR (2<<10)
#define RW (3<<10)
#define EX (4<<10) // командное свойство, исполнение, на операцию записи

#define LIST     (1<<9)
#define ARRAY    (1<<8)
#define OPTIONAL (1<<15)
#define SIZE(n)  (n) // кодирование размера данных
#define ASN_CONTEXT(n) (((n)<<4)|0x08)
// Коды классификации при кодировании
#define ASN_CLASS_APPLICATION    0x0
#define ASN_CLASS_CONTEXT        0x8
#define ASN_CLASS_CONSTRUCTIVE   0x6

#define ASN_CLASS_SEQUENCE_OF    0x5 // кодирование списка (последовательности однотипных элементов)
#define ASN_CLASS_SEQUENCE       0x6
#define ASN_CLASS_ARRAY          0x7
#define ASN_CLASS_OPENING        0x6
#define ASN_CLASS_CLOSING        0x7
#define ASN_CLASS_MASK           0x7

#define ASN_TYPE_NULL	         0x00
#define ASN_TYPE_BOOLEAN         0x10
#define ASN_TYPE_UNSIGNED        0x20
#define ASN_TYPE_INTEGER         0x30
#define ASN_TYPE_REAL	         0x40
#define ASN_TYPE_DOUBLE	         0x50
#define ASN_TYPE_OCTETS    		 0x60
#define ASN_TYPE_STRING    		 0x70
#define ASN_TYPE_BIT_STRING		 0x80
#define ASN_TYPE_ENUMERATED		 0x90
#define ASN_TYPE_DATE			 0xA0
#define ASN_TYPE_TIME			 0xB0
#define ASN_TYPE_OID             0xC0

#define ASN_TYPE_CHOICE 0xD0// псевдо тип не используется в протоколе
#define ASN_TYPE_ANY    0xE0// псевдо тип не используется в протоколе

//typedef struct _ObjectDesc ObjectDesc;
typedef struct _PropertySpec PropertySpec_t;

#define PROPERTY_ARRAY_SIZE(ps) ((ps)->size)
struct _PropertySpec {
    uint16_t prop_id;   // идентификатор свойства 22 бита или 32
    uint16_t asn_type;  // кодирование типа
    uint16_t offset;    // смещение
    uint16_t size;  // размер чего списка или
    uint16_t flags;
    union {
        const struct _PropertySpec *ref;
        uint32_t default_value;
    };
};
/*
typedef struct _ParamSpec ParamSpec_t;
struct _ParamSpec {
    uint16_t asn_type;
    uint16_t offset;
    uint16_t flags;
    uint16_t size;
    const struct _ParamSpec *ref;
};*/
struct _AnyValue {
    union {
        int32_t  i32;
        uint32_t u32;
        float    f32;
        void*    ptr;
        char*    str;
        uint8_t* bin;
    } value;
/*
	uint16_t length;
    uint8_t type_id;
    uint8_t context_id;
	*/
};
typedef struct _List List;
struct _List {
    struct _List * next;
    uint8_t data[0];//!< данные записываются следом
};
#define  List_new(type) (g_slice_alloc0 (sizeof (type)+sizeof(List)))
#define  List_free(type, mem) (g_slice_free1 (sizeof(type)+sizeof(List), mem))
typedef int (*List_compare) (void* , void*);
/* \param next указывает на элемент который подменяется в результате head->next или */
static inline void List_insert(List** next, List* elem) {
	elem->next = *next;
	*next = elem;
}
/* см выше
static inline void List_prepend(List** head, List* elem) {
	elem->next = *head;
	*head = elem;
} */
static inline void List_append(List** prev, List* elem) {
    while (*prev!=NULL){
        prev = &(*prev)->next;
    }
    elem->next = *prev;
    *prev = elem;
}
static inline void List_insert_sorted(List** prev, List* elem, List_compare cmpx) {
    while (*prev!=NULL && cmpx((*prev)->data, elem->data)<0){
        prev = &(*prev)->next;
    }
    elem->next = *prev;
    *prev = elem;
}

typedef struct _Object Object_t;
typedef struct _DeviceObject Device_t;
typedef struct _DeviceInfo DeviceInfo_t;
typedef struct _DeviceObjectClass DeviceObjectClass;
struct _DeviceObjectClass {
    const PropertySpec_t* pspec;//!< таблица свойств объекта
    uint16_t pspec_length;//!< размер таблицы свойств (число строк)
    uint16_t size;//!< размер объекта (в байтах)
    Object_t* (*ctor)(Object_t*, DeviceInfo_t*);//!< конструктор
    //char* name;
};
const DeviceObjectClass* object_classes_lookup(uint8_t type_id);
// Описывает базовые принципы кодирования 
uint8_t* r3_asn_next(uint8_t * buf);
uint8_t* r3_encode_tag (uint8_t *buf, uint16_t tag, size_t len);
uint8_t* r3_decode_tag (uint8_t *buf, uint16_t* node_tag, size_t* node_length);

uint8_t* r3_asn_encode_tag   (uint8_t *buf, uint8_t asn_type, size_t len);
uint8_t* r3_asn_encode_u32   (uint8_t * buf, uint8_t asn_type, uint32_t value);
uint8_t* r3_asn_encode_unsigned(uint8_t *buf, uint8_t tag, uint32_t value);
uint8_t* r3_asn_encode_signed(uint8_t *buf, uint8_t asn_type, int32_t value);
uint8_t* r3_asn_encode_real  (uint8_t *buf, uint8_t asn_type, float  fvalue);
uint8_t* r3_asn_encode_string(uint8_t *buf, uint8_t tag, const char* str, size_t len);
uint8_t* r3_asn_encode_octets(uint8_t *buf, uint8_t asn_type, const uint8_t* str, size_t len);

uint8_t* r3_asn_decode_length(uint8_t *buf, size_t *len);
uint8_t* r3_asn_decode_signed(uint8_t *buf, int32_t *value);
#endif//R3_ASN_H
