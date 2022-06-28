/*!
    TODO ObjectClass tree_lookup Заменить на bsearch
*/

//#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include "atomic.h"
#include "r3_tree.h"
#include "r3_asn.h"
#include "r3_object.h"

//#include "bacnet.h"
#include "../bacnet/bacnet_error.h"
// уровни отладки
#define DBG3 0
extern char* bacnet_property_name(uint32_t );

#if 1
/// \todo Внимание мы определяем только ту часть которая тут используется, от этой структуры наследуется остальное
struct _DeviceInfo {
    struct _DeviceObject * device; // сам объект
	tree_t* device_objects;
};
#endif // 0


static Object_t* _object_lookup(DeviceInfo_t* devinfo, ObjectIdentifier_t object_identifier);
static Object_t* _object_new (ObjectIdentifier_t object_identifier);
static Object_t* _object_init(Object_t* object, ObjectIdentifier_t object_identifier, DeviceInfo_t*);
static int       _object_delete(DeviceInfo_t*devinfo, ObjectIdentifier_t object_identifier);

#define ASSERT(v) if(!(v)) return -1;
#define ERROR(errClass, errCode) ~((errClass<<16)|errCode)

static uint8_t __rbit8(uint8_t v) {
    v = (v & 0x55)<<1 | (v & 0xAA)>>1;
    v = (v & 0x33)<<2 | (v & 0xCC)>>2;
    v = (v & 0x0F)<<4 | (v & 0xF0)>>4;
    return v;
}
static void print_bits(char* title, uint8_t *buf, int bits)
{
    char s[bits+1];
    int i,n;
    for (i=0; i< bits>>3; i++){
        for (n=0; n<8; n++)
            s[i*8+n] = (buf[i] & (0x80>>n))?'T':'F';
        n=0;
    }
    for (n=0; n<(bits&0x7); n++)
        s[i*8+n] = (buf[i] & (0x80>>n))?'T':'F';
    s[i*8+n]='\0';
    printf("%s: %s\n", title, s);
}

//extern uint8_t * r3_pspec_encode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const ParamSpec_t* desc);
//extern uint8_t * r3_pspec_decode_param(uint8_t *buf,  void*ptr, const PropertySpec_t* desc);
extern void r3_pspec_free(void* data, const PropertySpec_t *desc, int pspec_length);
// перенести отсюда в место регистрации bacnet_object_device.c
uint32_t object_unique_id(uint32_t type)
{
    static volatile uint32_t count=0;
    return (type<<22) | /*++count;*/ atomic_fetch_add(&count,1);
}
/*
static const PropertySpec_t CreateObject_Request[]=  SEQUENCE {
[0] = {"object-specifier",  ASN_CLASS_CONTEXT| ASN_TYPE_CHOICE, CHOICE {
    [0] = {"object-type", ASN_TYPE_ENUMERATED}, // BACnetObjectType
    [1] = {"object-identifier", ASN_TYPE_OID}, // BACnetObjectIdentifier
    }},
[1] = {"list-of-initial-values", ASN_CLASS_SEQUENCE_OF|ASN_CLASS_CONSTRUCTIVE, BACnetPropertyValue, OPTIONAL}
};*/
#if 0
typedef struct _PropertyValue PropertyValue_t;
typedef struct _PropertyReference PropertyReference_t;
struct _PropertyValue {
    uint32_t/* enum _BACnetPropertyIdentifier */ property_identifier;
    uint16_t property_array_index;
    uint16_t priority;// RANGE(1, 16)
    void* property_value;
};
struct _PropertyReference {
    uint32_t/* enum _BACnetPropertyIdentifier */ property_identifier;
    uint16_t property_array_index;
};
#endif

// Общие свойства всех объектов
extern const PropertySpec_t CommonObject[2];

/*! \return индекс в таблице свойств объекта object_class->pspec[i] */
static const int r3_property_find(const PropertySpec_t* pspec, uint32_t pspec_length, uint16_t property_identifier)
{
    uint32_t i;
    for (i=0; i<pspec_length; i++){
        if (pspec[i].prop_id == property_identifier) {
            return i;
        }
    }
    return -1;
}
static uint32_t decode_oid(uint8_t ** buffer)
{
    uint8_t* buf = *buffer;
    *buffer+=5;
    return __builtin_bswap32(*(uint32_t*)&buf[1]);
}
//__attribute__((noinline))
static uint32_t decode_unsigned(uint8_t ** buffer)
{
    uint8_t* buf = *buffer;
    int size = *buf++ & 0x7;
    *buffer=buf+size;
    uint32_t value=__builtin_bswap32(*(uint32_t*)buf);
    switch (size){
    case 3: value>>= 8; break;
    case 2: value>>=16; break;
    case 1: value>>=24; break;
    case 0: value  = 0; break;
    default: break;
    }
    return value;
}
/// TODO заменить на функции из r3asn
//static inline
static uint8_t* encode_unsigned(uint8_t * buf, uint8_t asn_type, uint32_t value)
{
    int size = value!=0? 4-(__builtin_clz(value)>>3) : 0;
    *buf++= asn_type | size;
    switch (size){
    case 4: *(uint32_t*)buf=__builtin_bswap32(value);buf+=4; break;
    case 3: *buf++=value>>16;
    case 2: *buf++=value>>8;
    case 1: *buf++=value;
    default: break;
    }
    return buf;
}
static uint8_t* encode_bool(uint8_t * buf, uint8_t asn_type, bool value)
{
	if(value) {
		*buf++ = asn_type|1;
		*buf++ = 0x01;
	} else
		*buf++ = asn_type;
	return buf;
}
static uint8_t* encode_32(uint8_t * buf, uint8_t asn_type, uint32_t value)
{
    *buf++ = asn_type|4;
    *(uint32_t*)buf = __builtin_bswap32(value); buf+=4;
    return buf;
}
static uint8_t* encode_error(uint8_t * buf, enum _BACnetErrorClass error_class, enum _BACnetErrorCode error_code)
{
    *buf++ = ASN_TYPE_ENUMERATED|1;
    *buf++ = error_class;
    *buf++ = ASN_TYPE_ENUMERATED|1;
    *buf++ = error_code;
    return buf;
}
static uint8_t* decode_error(uint8_t *buf, int *err) {
    if(!(*buf++ == (ASN_TYPE_ENUMERATED|1))) return buf;
    uint32_t error_class = *buf++;
    if(!(*buf++ == (ASN_TYPE_ENUMERATED|1))) return buf;
    uint32_t error_code = *buf++;
    if (err) *err = ERROR(error_class, error_code);
    return buf;
}
extern uint8_t* r3_pspec_decode(uint8_t* buf, void* data, const PropertySpec_t* desc, int desc_length);
/// FIX r3_pspec_decode_param - заменить на эту, в файле r3_asn
static uint8_t * _pspec_decode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const PropertySpec_t* pspec)
{
	if (0) printf("_pspec_decode_param context_id = %d, %p\r\n", context_id, ptr);
    const PropertySpec_t* desc = pspec+context_id;
	uint16_t node_tag;
	size_t   node_length;
	buf = r3_decode_tag(buf, &node_tag, &node_length);
	if (ptr==NULL) {
        buf+=node_length;
	} else
    if ((desc->asn_type&ASN_CLASS_MASK)==ASN_CLASS_SEQUENCE) {// SEQUENCE
        buf = r3_pspec_decode (buf, ptr, desc->ref, desc->size);
		if ((buf[0] & ASN_CLASS_MASK)==ASN_CLASS_CLOSING)
			buf = r3_decode_tag(buf, &node_tag, &node_length);
    } else
    if ((desc->asn_type&ASN_CLASS_MASK)==ASN_CLASS_SEQUENCE_OF) {// SEQUENCE OF
		List * prev = *(List**)ptr;
		if (prev) while(prev->next) prev = prev->next; // в конец списка добавляем
		int size = (desc->asn_type>>8) + sizeof(List*);
		while((node_tag & ASN_CLASS_MASK)!=ASN_CLASS_CLOSING) {
			List* elem = g_slice_alloc(size);
			elem->next = NULL;
			buf = r3_pspec_decode(buf, elem->data, desc->ref, desc->size);
			if(prev) prev->next = elem;
			else *(List**)ptr = elem;
			prev = elem;
			node_tag = buf[0];
		}
		buf = r3_decode_tag(buf, &node_tag, &node_length);// закрывающий тег конструктивного типа
    } else
	{ // не конструктивно
		switch (desc->asn_type&0xF0) {
        case ASN_TYPE_ANY: // 0xE0
            *(void**)ptr = buf;
            break;
		case ASN_TYPE_NULL: break;  // 0x00
		case ASN_TYPE_CHOICE: break;
		case ASN_TYPE_BOOLEAN:{     // 0x10
			if (node_tag & ASN_CLASS_CONTEXT) {
				*(bool *)ptr = (*buf!=0)?true:false;
			} else {
				*(bool *)ptr = (node_tag & 1)?true:false;
				node_length = 0;
			}
		} break;
		case ASN_TYPE_ENUMERATED:   // 0x90
		case ASN_TYPE_UNSIGNED: {   // 0x20
			uint32_t value;
			switch (node_length){// способ записи определен как BE
			case 0: value = 0; break;
			case 1: value = buf[0]; break;
			case 2: value = (uint32_t)buf[0]<<8  | (uint32_t)buf[1]; break;
			case 3: value = (uint32_t)buf[0]<<16 | (uint32_t)buf[1]<<8 | (uint32_t)buf[2]; break;
			case 4: value = __builtin_bswap32(*(uint32_t*)buf); break;
			default:value = 0; break;
			}
			if (DBG3) printf(" === Unsigned: %u\n", value);
			switch (desc->asn_type&0x07) {// способ записи может быть LE или BE
			case 1: *(uint8_t *)ptr = value; break;
			case 2: *(uint16_t*)ptr = value; break;
			default:// если не указано явно, то 4 байта
			case 4: *(uint32_t*)ptr = value; break;
			}
		} break;
		case ASN_TYPE_INTEGER: {    // 0x30
			int32_t value;
			switch (node_length){// способ записи определен как BE
			case 0: value = 0; break;
			case 1: value = (int8_t)buf[0]; break;
			case 2: value = (int16_t)__builtin_bswap16(*(uint16_t*)buf); break;
			case 3: value = (int32_t)buf[0]<<16 | (uint32_t)buf[1]<<8 | (uint32_t)buf[2]; break;
			case 4: value = (int32_t)__builtin_bswap32(*(uint32_t*)buf); break;
			default:value = 0; break;
			}
			value = __builtin_bswap32(*(uint32_t*)buf);
			if (DBG3) printf(" === Integer: %d\n", value);
			switch (desc->asn_type&0x07) {// способ записи может быть LE или BE
			case 1: *(int8_t *)ptr = value; break;
			case 2: *(int16_t*)ptr = value; break;
			default:// если не указано явно, то 4 байта
			case 4: *(int32_t*)ptr = value; break;
			}
		} break;
		case ASN_TYPE_DOUBLE: {     // 0x50
			if (node_length==8)
				*(uint64_t*)ptr = __builtin_bswap64(*(uint64_t*)buf);
			break;
		} break;
		case ASN_TYPE_OCTETS: {     // 0x60
			uint8_t *octets = ptr;
			int size = (desc->size < node_length? desc->size : node_length);
			int i;
			for (i=0;i<size; i++) {
				octets[i] = buf[i];
			}
			size = desc->size;
			for (;i<size; i++) {
				octets[i] = 0;
			}
		} break;
		case ASN_TYPE_STRING: {     // 0x70
			// если строка в словаре, кварк?
			// строка в указателе?
			// если строка только для чтения, инициализируется из флеш памяти (SEGMENT(buf)==iFLASH) то установим ссылку
			if (node_length==0) {
				*(char**)ptr = NULL;
			} else {
				uint8_t *str = malloc(node_length);//*(uint8_t**)ptr;// сылка на строку
				// в первом байте содержится кодировка 00- UTF-8
				__builtin_memcpy(str, buf+1, node_length-1);
				str[node_length-1]='\0';
                if (DBG3) printf(" === String: '%s'\n", str);
				str = (uint8_t*)atomic_pointer_exchange((void**)ptr, str);
				if (str!=NULL/* (SEGMENT(str)!=iFLASH */) {
					//printf(" == free string = %p\r\n", str);
					free(str);
				}
			}
		} break;
		case ASN_TYPE_BIT_STRING: { // 0x80
			if (node_length>0) {// битовая строка кодируется как число бит в последнем октете
			/*  The initial octet shall encode, as an unsigned binary integer, the number of unused bits in the final subsequent octet.
				The number of unused bits shall be in the range zero to seven, inclusive. */
				uint8_t* bits = ptr;
				uint8_t mask = 0xFF<<buf[0];
				int size = (desc->size < node_length? desc->size:node_length-1);
                char str [12];
                if (1) print_bits(" === BitString", buf+1, size*8 - buf[0]);
				int i;
				for (i=0; i<size-1; i++) {
					bits[i] = __rbit8(buf[i+1]);
				}
				bits[i] = __rbit8(buf[i+1] & mask); // Undefined bits shall be zero
				size = desc->size;
				for (++i;i<size; i++) {
					bits[i] = 0;
				}
			}
		} break;
		case ASN_TYPE_REAL: {       // 0x40
			uint32_t value = __builtin_bswap32(*(uint32_t*)buf);
			*(uint32_t*)ptr = value;
			float fv = *(float*)&value;
			if (DBG3) printf(" === Real: %d.%u\r\n", (int)fv, (int)(fv*10 - (int)(fv)*10));
		}	break;
		case ASN_TYPE_DATE:         // 0xA0
		case ASN_TYPE_TIME:         // 0xB0
		case ASN_TYPE_OID :         // 0xC0
		default: // REAL DATE TIME OID
			if (node_length==4) {
                uint32_t value = __builtin_bswap32(*(uint32_t*)buf);
				*(uint32_t*)ptr = value;
				if (DBG3) printf(" == value = %08x\r\n", value);
			} else {
			    if (DBG3) printf(" == type = %02X, size = %d\r\n", desc->asn_type, (int)node_length);
			}
			break;
		}
		buf+=node_length;
    }
    return buf;
}

/*! \brief выполнить кодирование базового типа
 */
static uint8_t * _pspec_encode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const PropertySpec_t* desc)
{

//    printf ("_pspec_encode_param %d\n", desc[context_id].prop_id);
    uint16_t asn_type = desc[context_id].asn_type;
    uint16_t node_tag = asn_type;
    if (node_tag & ASN_CLASS_CONTEXT) {
        node_tag = (context_id>=0xF)?(context_id<<8)|0xF8: (context_id<<4)|0x08;
    }
#if 0
    if ((desc->asn_type&ASN_CLASS_MASK)==ASN_CLASS_SEQUENCE) {// SEQUENCE
        if (node_tag & ASN_CLASS_CONTEXT)
            buf = r3_encode_constructed(buf, node_tag|ASN_CLASS_OPENING);
        buf = r3_pspec_encode (buf, ptr, desc->ref, desc->size);
        if (node_tag & ASN_CLASS_CONTEXT)
            buf = r3_encode_constructed(buf, node_tag|ASN_CLASS_CLOSING);
    } else
    if ((desc->asn_type&ASN_CLASS_MASK)==ASN_CLASS_SEQUENCE_OF) {// SEQUENCE OF
        if (node_tag & ASN_CLASS_CONTEXT)
            buf = r3_encode_constructed(buf, node_tag|ASN_CLASS_OPENING);
        List* list = *(List**) ptr;
        while (list){
            buf = r3_pspec_encode (buf, list->data, desc->ref, desc->size);
            list = list->next;
        }
        if (node_tag & ASN_CLASS_CONTEXT)
            buf = r3_encode_constructed(buf, node_tag|ASN_CLASS_CLOSING);
    } else
#endif
    {// не конструктивно
        size_t node_length = asn_type & 0x7;
        union {
            int32_t i;
            uint32_t u;
            void* ptr;
        } value = {.ptr=0};
        switch(asn_type & 0xF0) {
        case ASN_TYPE_NULL:
            node_length = 0;
            break;
        case ASN_TYPE_BOOLEAN: {// 0x10
            value.u = *(bool*)ptr;
            node_length = ((node_tag&ASN_CLASS_CONTEXT) /*|| node->value.b*/)? 1: 0;
        } break;
        case ASN_TYPE_ENUMERATED:// 0x90
        case ASN_TYPE_UNSIGNED:// 0x20
            if (node_length==0) node_length = desc[context_id].size;
            switch (node_length){
            case 1: value.u = *(uint8_t *)ptr;  break;
            case 2: value.u = *(uint16_t*)ptr;  break;
            default:
            case 4: value.u = *(uint32_t*)ptr;  break;
            }
            node_length = value.u!=0? 4-(__builtin_clz(value.u)>>3) : 0;
            break;
        case ASN_TYPE_INTEGER: {// 0x30
            switch (node_length){
            case 1: value.i = *(int8_t *)ptr;  break;
            case 2: value.i = *(int16_t*)ptr;  break;
            default:
            case 4: value.i = *(int32_t*)ptr;  break;
            }
            if (value.i<0) {
                node_length = ~value.u!=0? 4-((__builtin_clz(~value.u)-1)>>3) : 1;
            } else {
                node_length = value.u!=0? 4-((__builtin_clz(value.u)-1)>>3) : 0;
            }
        } break;
        case ASN_TYPE_OCTETS: //0x60
			if (node_length==0) node_length = desc[context_id].size;
			uint8_t *s=*(uint8_t**) ptr;
            while (node_length!=0 && s[node_length-1]!=0) node_length--;
            break;
        case ASN_TYPE_STRING: {//0x70
            node_length = 0;//strlen(*(char**)ptr)+1;
            char* s=*(char**)ptr;
            if (s) {
                while(s[node_length++]!='\0');
                //node_length++;// ноль копируем?
//                printf (" == String: %s\n", s);
                ptr = *(char**)ptr;
            } else {
                printf ("String String\n");
                ptr = "Unknown";
                node_length=8;
            }
        } break;
        case ASN_TYPE_BIT_STRING: { //0x80
            if(1)printf (" === BitString: ");

            if (node_length==0) node_length = desc[context_id].size;
            uint8_t* bits=(uint8_t*)ptr+node_length;
            while (node_length) {
                if (*--bits) break;
                node_length--;
            }
            node_length++;
        } break;
        default:
        case ASN_TYPE_OID:  // 0xE0
        case ASN_TYPE_TIME: // 0xB0
        case ASN_TYPE_DATE: // 0xA0
            node_length = 4;
            break;
        case ASN_TYPE_REAL: // 0x40
            node_length = 4;
            break;
        case ASN_TYPE_DOUBLE:// 0x50
            node_length = 8;
            break;
        }
// кодировать тег
        buf = r3_encode_tag(buf, node_tag, node_length);
// кодировать данные

        switch(asn_type & 0xF0) {
        case ASN_TYPE_OCTETS:
            __builtin_memcpy(&buf[0], ptr, node_length);
            buf+=node_length;
            break;
        case ASN_TYPE_STRING:
            buf[0] = 0;// encoding UTF-8
            __builtin_memcpy(&buf[1], ptr, node_length-1);
            buf+=node_length;
            break;
        case ASN_TYPE_BIT_STRING: {
            uint8_t *bits = ptr;
            *buf++  = __builtin_clz(bits[node_length-2])-24;// = unused_bits 0..7;
            int i;
            for (i=0; i< node_length-1; i++) {
                *buf++= __rbit8(bits[i]);
            }
//            __builtin_memcpy(&buf[1], ptr, node_length-1);
//            buf+=node_length;
        } break;
        default:
            while (node_length>=4) {
                node_length-=4;
                *(uint32_t*)buf= __builtin_bswap32(*(uint32_t*)(ptr+node_length)); buf+=4;
            }
            if (node_length>=2) {
                node_length-=2;
                *(uint16_t*)buf = __builtin_bswap16(*(uint16_t*)(ptr+node_length)); buf+=2;//value.u>>16;
            }
            if (node_length>0) {
                *buf++ = *(uint8_t*)ptr;
            }
            break;
        }
    }
    return buf;
}
/// TODO регистрация классов? Конструктор
/*! \brief выполнить регистрацию статических объектов

*/
int r3cmd_object_init (DeviceInfo_t* devinfo, Object_t* object, uint32_t object_identifier)
{
    _object_init(object, object_identifier, devinfo);
	//object->object_identifier = object_identifier;
    //object->instance.object_type= object_identifier>>22;
    //if (object->instance.object_name==NULL) object->instance.object_name = "Unknown";
    if (object->property_list.array==NULL) {
        //_Exit(480);
        //return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);

        const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
        if (object_class==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
        object->property_list.array = object_class->pspec;
        object->property_list.count = object_class->pspec_length;
    }
    return 0;
}
/*! \brief варианты использования

    заполнить поля по описанию из json, xml, bacnet
    вызвать конструктор класса
*/
Object_t* r3cmd_object_new(DeviceInfo_t* devinfo, ObjectIdentifier_t object_identifier)
{
    Object_t* object = _object_new(object_identifier);
    return _object_init(object, object_identifier, devinfo);
}
#if 0
void*  r3_object_property_get_value(tree_t** object, uint32_t property_identifier)
{
	context_id = r3_property_find(object, sizeof(CommonObject)/sizeof(const PropertySpec_t), property_identifier);

    const PropertySpec_t *pspec = object->property_list.array;
    uint32_t pspec_length       = object->property_list.count;
    if (pspec==NULL) {
        const DeviceObjectClass *object_class = object_classes_lookup(type_id);
        if (object_class==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
        pspec = object_class->pspec;
        pspec_length = object_class->pspec_length;
    }

    uint32_t prop_id = pref->property_identifier;
//    printf("%s: prop_id =%d\n", __FUNCTION__, prop_id);
    int i;
    for (i=0; i< pspec_length; i++){
        if (pspec[i].prop_id == prop_id) break;
    }
    if (i==pspec_length){
        pspec = CommonObject;
        pspec_length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
        for (i=0; i< pspec_length; i++){
            if (pspec[i].prop_id == prop_id) break;
        }
        if (i==pspec_length){
//            printf("%s: ERROR(PROPERTY, UNKNOWN_PROPERTY)\n", __FUNCTION__, prop_id, pspec_length);
            return ERROR(PROPERTY, UNKNOWN_PROPERTY);
        }
    }
	return object + pspec[context_id].offset;
}
#endif
/*! \brief создать и выполнить регистрацию объекта */
int r3cmd_object_create(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t* response_buffer)
{
    uint8_t *buf = buffer;
    uint32_t object_identifier = 0x3FFFFF;
    ASSERT(*buf++ == 0x0E);// choice
    if ((*buf & 0xF8)  == 0x08){// enumerated
        object_identifier = decode_unsigned(&buf);
        object_identifier = (object_identifier<<22)|0x3FFFFF;
    } else {// OID
        ASSERT(*buf == 0x1C);
        object_identifier = decode_oid(&buf);
    }
    ASSERT(*buf++ == 0x0F);
/*
    const DeviceObjectClass *object_class = object_classes_lookup(type_id);
    if (object_class==NULL)
        return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);

    if ((object_identifier&OBJECT_ID_MASK)==UNDEFINED)
         object_identifier = object_unique_id(type_id);
    Object_t* object = malloc(object_class->size);
    __builtin_bzero(object, object_class->size);
*/
    Object_t* object = _object_new(object_identifier);
    if (object==NULL || object->property_list.array==NULL) {
        return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
    }
    if ((object_identifier& OBJECT_ID_MASK)==UNDEFINED) {
        object->object_identifier = object_identifier = object_unique_id(object_identifier>>22);
    }
    _object_init(object, object_identifier, devinfo);
    if(*buf == 0x1E) {// PD Opening Tag 1 (List Of Initial Values)
        buf++;// 0x1F
        while (*buf   != 0x1F){
            ASSERT((*buf & 0xF8) == 0x08);// SD Context Tag 0 (Property Identifier, L=1)
            uint32_t property_identifier = decode_unsigned(&buf);
            const PropertySpec_t * pspec = object->property_list.array;
            uint16_t pspec_length = object->property_list.count;
            int i;
            for (i=0; i<pspec_length; i++) {
                if (pspec[i].prop_id == property_identifier) break;
            }
            if (i==pspec_length) {
                pspec = CommonObject;
                pspec_length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
                for (i=0; i< pspec_length; i++){
                    if (pspec[i].prop_id == property_identifier) break;
                }
            }
            void* data;
            if (i!=pspec_length)
                data = (void*)object + pspec[i].offset;
            else
                data = NULL;
            ASSERT(*buf++ == 0x2E);// PD Opening Tag 2 (Value)
            buf = _pspec_decode_param(buf, data, i, pspec);
            ASSERT(*buf++ == 0x2F);

        }
        buf++;// 0x1F
    }
    buf = encode_32(response_buffer, ASN_TYPE_OID|4, object_identifier);
    return buf - response_buffer;
}
#if 0
/* сформировать упакованный вид */
static void serialize(tree_t* device_tree, uint32_t object_identifier, void* user_data)
{
    uint8_t *buf = *(uint8_t**)user_data;
    buf = encode_32(buf, 0x0C, object_identifier);
	void *object = tree_lookup(device_tree, object_identifier);
	if (object != NULL) {
        uint16_t type_id = object_identifier>>22;
        const DeviceObjectClass *object_class = object_classes_lookup(type_id);
        const PropertySpec_t* pspec = object_class->pspec;
        if (pspec) {
            *buf++ = 0x1E;
            int i;
            for (i=0;i< object_class->pspec_length;i++){
                void *data = object+pspec[i].offset;
                //buf = r3_encode_primitive(buf, &pspec[i].prop_id, CONTEXT(1)|ASN_TYPE_ENUMERATED, 2);
                buf = encode_unsigned(buf, 0x28, pspec[i].prop_id);
                *buf++=0x4E;
/*
                if ((pspec[i].asn_type & ASN_CLASS_MASK) == ASN_CLASS_ARRAY) {// ARRAY
                    uint32_t size = pspec[i].size;// откуда взять размер элемента массива?
                    if(size==0) size = 4;
                    uint32_t length = PROPERTY_ARRAY_SIZE(pspec[i]);//pspec[i].asn_type>>8;// & 0xFF;
                    int n;
                    for(n=0;n<length;n++){//
                        buf = r3_pspec_encode_param(buf, data+n*size, pspec[i].ref);
                    }
                } else */
                {
                    buf = r3_pspec_encode_param(buf, data, pspec[i].ref);// OID, REAL, UNSIGNED,INTEGER
                }
                *buf++=0x4F;
            }
            *buf++ = 0x1F;
        }
	}
	*(uint8_t**)user_data = buf;
}

// ReinitializeDevice WARMSTART, COLDSTART, ACTIVATE_CHANGES - записать изменения во флеш и применить отложенные нстройки сетевых интерфейсов,
int r3_device_tree_backup(uint8_t * buffer, int length)
{// записать изменения во флеш и применить отложенные нстройки сетевых интерфейсов
    tree_notify(device_tree, serialize, &buffer);
    return 0;
}
#endif // 0
/*! \brief удалить объект из реестра */
int r3cmd_object_delete(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length)
{
    uint8_t *buf = buffer;
    ASSERT(*buf == 0x0C);
    uint32_t object_identifier = decode_oid(&buf);
    return _object_delete(devinfo, object_identifier);
}

/*! \brief используется как примитив для кодирования данных */
uint8_t * r3cmd_property_encode (uint8_t * buf, Object_t *object, uint32_t prop_id)
{
    const PropertySpec_t *pspec = object->property_list.array;
    uint32_t pspec_length       = object->property_list.count;
    int i;
    for (i=0; i< pspec_length; i++){
        if (pspec[i].prop_id == prop_id) break;
    }
    if (i==pspec_length){
        pspec = CommonObject;
        pspec_length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
        for (i=0; i< pspec_length; i++){
            if (pspec[i].prop_id == prop_id) break;
        }
        if (i==pspec_length){
//            printf("%s: ERROR(PROPERTY, UNKNOWN_PROPERTY)\n", __FUNCTION__, prop_id, pspec_length);
            return buf;//ERROR(PROPERTY, UNKNOWN_PROPERTY);
        }
    }
    void* data = (void*)object + pspec[i].offset;
    buf = _pspec_encode_param(buf,  data, i, pspec);
    return buf;
}
/*!

    uint16_t asn_type;
    uint16_t offset;
    uint16_t flags;
    uint16_t size;
    \param hint разрешить создание нового объекта, если объект не создан
*/
static int r3cmd_property_reference (DeviceInfo_t* devinfo, ObjectPropertyReference_t *pref, Object_t **object_ref,  int hint)
{
    Object_t* object = _object_lookup(devinfo, pref->object_identifier);
    if (object==NULL) {
        if (!hint)
            return ERROR(OBJECT, UNKNOWN_OBJECT);
		//printf(" == %s:%d -- new object\r\n", __FUNCTION__, __LINE__);

        object = _object_new(pref->object_identifier);
        if (object==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
            /// \todo переделать в пользьзу devinfo? поменят местами аргументы
        _object_init(object, pref->object_identifier, devinfo);
/*
        int result = r3cmd_object_init(devinfo, object, pref->object_identifier);
        if (result!=0)
            return result; */
//        printf("r3cmd_property_reference: ERROR(OBJECT, UNKNOWN_OBJECT)=0x%08X\n", pref->object_identifier);
    }
    const PropertySpec_t *pspec = object->property_list.array;
    uint32_t pspec_length       = object->property_list.count;
    if (pspec==NULL) {
        const DeviceObjectClass *object_class = object_classes_lookup(pref->object_identifier>>22);
        if (object_class==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
        pspec = object_class->pspec;
        pspec_length = object_class->pspec_length;
    }

    uint32_t prop_id = pref->property_identifier;
//    printf("%s: prop_id =%d\n", __FUNCTION__, prop_id);
    int i;
    for (i=0; i< pspec_length; i++){
        if (pspec[i].prop_id == prop_id) break;
    }
    if (i==pspec_length){
        pspec = CommonObject;
        pspec_length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
        for (i=0; i< pspec_length; i++){
            if (pspec[i].prop_id == prop_id) break;
        }
        if (i==pspec_length){
//            printf("%s: ERROR(PROPERTY, UNKNOWN_PROPERTY)\n", __FUNCTION__, prop_id, pspec_length);
            return ERROR(PROPERTY, UNKNOWN_PROPERTY);
        }
    }
    pref->context_id = i;// номер свойства по списку, чтобы быстро находить
    pref->pspec = pspec;//.ref;
//    pref->data   = data;
	*object_ref = object;
/* TODO
	if (pref->property_array_index>=PROPERTY_ARRAY_SIZE(&pspec[i]))
		return ERROR(PROPERTY, INVALID_ARRAY_INDEX);
*/
	/*
    if ((pspec->flags & EX)!=0) {// применить hint Commandable
		pref->commandable = &((CommandableObject_t*) object)->commandable;
	} else
		pref->commandable = NULL;
*/
    return 0;
}
static inline void* _property_get(Object_t * object, const PropertySpec_t *  pspec)
{
    return (void*)(object) + pspec->offset;
}
/*! Функции для работы с файловой системой */
/*! \brief функция записи данных в файл (журнал)

если секция 1(1E-1F) отсуствует, транзакция расценивается как object-delete, иначе object-modify. Если объект не найден, то object-create
Псевдо код потоковой записи во флеш память.
    asn_encode(CONTEXT_TYPE(0), ObjectIdntifier); -- кодируется тип и номер объекта(число 32 бита)
    asn_begin (CONTEXT_TYPE(1)); -- описание параметров
        asn_encode(CONTEXT_TYPE(0), param->PropertyIdentifier);
        if (is_array(param))  {// если запись ведется по элементам списка или массива
            asn_encode(CONTEXT_TYPE(1), param->ArrayIndex);
        }
        if (is_default(param->value)) {// значение по умолчанию
            asn_begin (CONTEXT_TYPE(2));
                asn_encode(CONTEXT_TYPE(0), param->value);
            asn_end   (CONTEXT_TYPE(2));
        }
    asn_end   (CONTEXT_TYPE(1));
    asn_encode(CONTEXT_TYPE(3), crc32);// от блока данных (0) и (1)
*/
extern uint32_t bacnet_crc32k(uint8_t * data, size_t length);
uint8_t * r3cmd_object_store (uint8_t *buf, void* object, uint32_t object_id, const PropertySpec_t *pspec, uint32_t pspec_len)
{
    uint8_t *block = buf;

    buf = r3_asn_encode_u32(buf, ASN_CONTEXT(0), object_id);// это надо делать снаружи
    if (object==NULL){// объект удален!
        return buf;
    }
    *buf++ = 0x1E;
    int i;
    for (i=0; i< pspec_len; i++){
        buf = r3_asn_encode_unsigned(buf, ASN_CONTEXT(0), pspec[i].prop_id);
        if (0){// ARRAY_ELEMENT?
            buf = r3_asn_encode_unsigned(buf, ASN_CONTEXT(1), 0);
        }
        *buf++ = 0x2E;
         buf = _pspec_encode_param(buf, object + pspec[i].offset, i, pspec);
        *buf++ = 0x2F;
    }
    *buf++ = 0x1F;
#if 0
    // может быть вынести функцию за пределы блока, блока применяется только если указана CRC или CMAC/HMAC
    uint32_t crc32 = bacnet_crc32k(block, buf-block);
    *buf++ = 0x3C;// CRC32
    *(uint32_t*)buf = crc32;
#endif
    return buf;
}
/*! \brief читает с носителя
    \todo не используется, отладить
*/
uint8_t * r3cmd_object_load (uint8_t *buf, void* object, const PropertySpec_t *pspec, size_t pspec_len)
{
    // объект читаем снаружи
    int i=0;// номер свойства в таблице, при записи некоторые свойства могут быть пропущены, тогда им присваиваются значения по умолчанию
    if (*buf != 0x1E) return buf;
    buf++;
    while (*buf != 0x1F) {
        uint32_t prop_id = 85;//PRESENT_VALUE
        uint16_t array_idx=0;
        if ((*buf & 0xF8) == ASN_CONTEXT(0)) {
            buf++;
            prop_id  = decode_unsigned(&buf);
        }
        if ((*buf & 0xF8) == ASN_CONTEXT(1)) {
            buf++;
            array_idx= decode_unsigned(&buf);
        }
        for (; i<pspec_len;i++) {
            if (pspec[i].prop_id==prop_id) break;
        }
        if (i<pspec_len && *buf == 0x2E) {
            buf++;
            void* data = _property_get(object, pspec + i);
            buf = _pspec_decode_param (buf, data, i, pspec);
            if (!(*buf == 0x2F)) break;// FIX не уверен что это правильный выход
        } else {
            buf = r3_asn_next(buf);// пропустить текущую запись
        }
    }
    buf++;
    return buf;
}
#if 0
/*! \brief функция загрузки журнала
    \return возвращаем начало блока, который не был разобран.
*/
uint8_t * r3cmd_load (DeviceInfo_t* devinfo, uint8_t *buf, size_t length)
{
    uint8_t* block= buf;
    uint32_t object_identifier;
    uint8_t  code;// = *buf;
    while (((code = *buf)&0xF)!=0xF)
    {
        switch (code ){
        case 0x0C: {// 0x18 object_ID Context(0)|4
            object_identifier = decode_oid(&buf);
            uint16_t type_id = object_identifier>>22;
            const DeviceObjectClass *object_class = object_classes_lookup(type_id);
			if (object_class==NULL)
				return block;//ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);

            Object_t* object = tree_lookup(devinfo->device_objects, object_identifier);
            if (object==NULL) {// создать объект
                object = malloc(object_class->size);
                __builtin_bzero(object, object_class->size);
                //r3cmd_object_init (devinfo, object, object_identifier);
				_object_init(object, object_identifier, devinfo);
				//object->object_identifier = object_identifier;
				//object->instance.object_type= object_identifier>>22;
				//if (object->instance.object_name==NULL) object->instance.object_name = "Unknown";
				object->property_list.array = object_class->pspec;
				object->property_list.count = object_class->pspec_length;
            } else {// обновить значения параметров для существующего объекта
            }
            buf = r3cmd_object_load(buf, object, object_class->pspec, object_class->pspec_length);
        } break;
        case 0x3C: {// CRC32K Context(3)|4
            buf++;
            uint32_t crc32 = bacnet_crc32k(block, buf-block);
            if (crc32 != *(uint32_t*)buf) return block;
            buf+=4;
            block = buf;
        } break;
        default:
            buf = r3_asn_next(buf);// пропустить непонятную запись
            break;
        }
    }
    return block;
}
#endif

/*! \brief возвращает ссылку на элемент данных
    используется только в dt_model -
*/
void* r3cmd_get_property(DeviceInfo_t* devinfo, uint32_t object_identifier, uint32_t property_identifier, uint16_t property_array_index)
{
    ObjectPropertyReference_t pref = {.object_identifier=object_identifier, .property_identifier = property_identifier, .property_array_index = property_array_index};
	Object_t * object=NULL;
    int result = r3cmd_property_reference (devinfo, &pref, &object, 0);
    if (result) {
        printf("-> Result: NF\n");
        return NULL;
    }
	return _property_get(object, pref.pspec + pref.context_id);
}
/*! \brief обработка запроса на чтение свойства объекта */
int r3cmd_read_property(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t *response_buffer)
{
    uint8_t* buf = buffer;
    ObjectPropertyReference_t pref;
//	printf("==> r3cmd_read_property\r\n");
    ASSERT(*buf==0x0C);
    pref.object_identifier = decode_oid(&buf);
//	printf("==> OID: %08X\r\n", pref.object_identifier);
    ASSERT((*buf & 0xF8) == 0x18);
    pref.property_identifier = decode_unsigned(&buf);
//	printf("==> OID: %04X\r\n", pref.property_identifier);
    if ((*buf & 0xF8) == 0x28){
        pref.property_array_index = decode_unsigned(&buf);
    } else {
        pref.property_array_index = 0xFFFF;
    }
//    uint8_t *buf = r3_pspec_decode(buffer, &pref, DEF(ObjectPropertyReference));
	Object_t * object=NULL;
    int result = r3cmd_property_reference (devinfo, &pref, &object, 0);
    if (result) {
        printf("-> Result: NF\n");
        return result;
    }
    /* формирование ответа, дописываем */
#if 0
    if(pref.pspec!=NULL && (pref.pspec->flags & RD)==0)
        return ERROR(PROPERTY, READ_ACCESS_DENIED);
#endif // 0
//    printf("%s:Property ref\n", __FUNCTION__);
    __builtin_memcpy(response_buffer, buffer, buf-buffer);
    uint8_t *rsp = response_buffer+(buf-buffer);
    *rsp++=0x3E;
	void* data = _property_get(object, pref.pspec + pref.context_id);

    if (pref.pspec[pref.context_id].flags & LIST) {
//        printf("r3cmd_read_property LIST\n");
        LIST_OF(void) *list_of = data;
        List * list = list_of->list;
        while (list) {
            rsp = _pspec_encode_param(rsp, list->data, pref.context_id, pref.pspec);
            list = list->next;
        }
    } else
    if (pref.pspec[pref.context_id].flags & ARRAY) {
        /// вариант упаковки массивов по ссылке на массив
//        printf("r3cmd_read_property ARRAY\n");
        ARRAY_OF(void) *array_of = data;
        int count = array_of->count;
        data = array_of->array;
        /// \todo другой вариант упаковки массивов в структуре
        if (pref.property_array_index == 0xFFFF) {
            int i;
            for (i=0; i < count;i++, data += pref.pspec[pref.context_id].size) {
                rsp = _pspec_encode_param(rsp, data, pref.context_id, pref.pspec);
            }
        } else
        if (pref.property_array_index == 0) {
            rsp = r3_asn_encode_u32(rsp, ASN_TYPE_UNSIGNED, count);
        } else {
            data += pref.pspec->size*(pref.property_array_index-1);
            rsp = _pspec_encode_param(rsp, data, pref.context_id, pref.pspec);
        }
    } else {
        rsp = _pspec_encode_param(rsp, data, pref.context_id, pref.pspec);
    }
    *rsp++=0x3F;
    if (1) printf("%s: Param encode done\n", __FUNCTION__);
    return rsp - response_buffer;
}
/*! \brief подтверждает создание динамического объекта на удаленной машине для назначения ему идентификатора
    Объект имеет идентификатор с номером UNDEFINED=0x3FFFFF, пока не полуено подтверждение от удаленного устройства

    Это действие можно выполнить иначе. Можно оставить элемент дерева с value=NULL

    tree_replace(&device_tree, key1, key2);
 */
#if 0
int r3cmd_object_confirm(tree_t** device_tree, uint32_t object_identifier)
{
    uint16_t type_id = object_identifier >> 22;
    // удалить данные не удаляя элемент дерева
    Object_t* object = tree_replace_data(*device_tree, object_identifier | UNDEFINED, NULL);
    if (object==NULL) {
        return -1;
    }
    object->object_identifier = object_identifier;
    tree_t* leaf = g_slice_alloc(sizeof(tree_t));
    tree_init(leaf, object_identifier, object);
    leaf = tree_insert_tree(device_tree, leaf);/// вставить атомарно, не нарушая структуру дерева
    if (leaf!=NULL) {// объект с таким именем уже существует, удалить его
        // todo удалить и забыть если идет замена. value1 != value2
        if (leaf->value != object) {
            // пометить на удаление..
        }
        g_slice_free(tree_t, leaf);
    }
    return 0;

}
#endif // 0
/*! \brief обработка запроса на изменение свойства объекта

ReadProperty-ACK ::= SEQUENCE {
    object-identifier       [0] BACnetObjectIdentifier,
    property-identifier     [1] BACnetPropertyIdentifier,
    property-array-index    [2] Unsigned OPTIONAL, -- used only with array datatype
        -- if omitted with an array the entire array is referenced
    property-value          [3] ABSTRACT-SYNTAX.&Type
}
*/
int r3cmd_write_property(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, int hint)
{

    uint8_t* buf = buffer;
    ObjectPropertyReference_t pref;
    if(0)printf(" == %s:%d\r\n", __FUNCTION__, __LINE__);
    ASSERT(*buf==0x0C);
    pref.object_identifier = decode_oid(&buf);
    ASSERT((*buf & 0xF8) == 0x18);
    pref.property_identifier = decode_unsigned(&buf);
    if ((*buf & 0xF8) == 0x28){
        pref.property_array_index = decode_unsigned(&buf);
    }
	Object_t* object=NULL;
    int result = r3cmd_property_reference (devinfo, &pref, &object,  hint);
    if (result) {
        return result;
    }
#if 0
    if ((pref.pspec->flags & WR)==0)
        return ERROR(PROPERTY, WRITE_ACCESS_DENIED);
#endif
    if(0)printf(" == %s:%d\r\n", __FUNCTION__, __LINE__);
    ASSERT(*buf++==0x3E);

    if (DBG3) printf(" == %s:%d property='%s'(%d)\n", __FUNCTION__, __LINE__, bacnet_property_name(pref.property_identifier), pref.property_identifier);
	void* data = _property_get(object, pref.pspec + pref.context_id);
    buf = _pspec_decode_param(buf, data, pref.context_id, pref.pspec);
///    buf = r3_pspec_decode_param(buf, pref.data, pref.pspec->ref);
    if(0)printf(" == %s:%d Decode done!!!\n", __FUNCTION__, __LINE__);
    ASSERT(*buf++==0x3F);// r3_encode_context(3, ASN_CLASS_CLOSING);
    if(0)printf(" == %s:%d Done !!!\n", __FUNCTION__, __LINE__);


/*
    uint8_t priority = 0;
    if((*buf&0xF8) == 0x48){
        priority = decode_unsigned(&buf);
    } */
    // дописать update object->notify(, priority);
//    if (pref.commandable != NULL) {// применить hint Commandable
	if ( pref.pspec[pref.context_id].flags & EX) {
		printf("Commandable property\r\n");
		struct _CommandableObject * commandable = (void*)object;
		if(commandable!=NULL && commandable->commandable.cb!=NULL){
			printf("Commandable property update\r\n");
			(void)commandable->commandable.cb(object, pref.property_identifier, commandable->commandable.user_data);
		}
	//TODO	(pref.commandable->cb)(pref->object, pref.commandable->user_data);
	} else {
		printf(" == obj=%p context_id=%d\r\n",object,  pref.context_id);
	}
    return 0;//buf-buffer;
}
/*! \brief обработка запроса на чтение свойств объекта
WritePropertyMultiple-Request ::= SEQUENCE {
    list-of-write-access-specifications SEQUENCE OF WriteAccessSpecification
}
ReadAccessSpecification ::= SEQUENCE {
    object-identifier [0] BACnetObjectIdentifier,
    list-of-property-references [1] SEQUENCE OF BACnetPropertyReference
}
\see F.3.7 Encoding for Example E.3.7 - ReadPropertyMultiple Service
*/
int r3cmd_read_property_multiple(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t *response_buffer)
{
    uint8_t *buf = buffer;
    uint8_t *rsp = response_buffer;
    //while (buf-buffer<length)
    if (1) {
        ASSERT(*buf == 0x0C);
        uint32_t object_identifier = decode_oid(&buf);

        Object_t* object =_object_lookup(devinfo, object_identifier);
        if (object==NULL)
            return ERROR(OBJECT, UNKNOWN_OBJECT);
        const PropertySpec_t * pspec = object->property_list.array;
        uint16_t pspec_length = object->property_list.count;
        if (pspec == NULL) {/// выкинуть отсюда, все объекты должны рождаться с классами
            const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
            if (object_class==NULL)
                return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
            pspec = object_class->pspec;
            pspec_length = object_class->pspec_length;
        }
//printf("%s: obj_id=%08X\n", __FUNCTION__, object_identifier);
        ASSERT(*buf++ == 0x1E);
//        buf = r3_decode_constructed(buf, ASN_CLASS_OPENING);
        rsp = encode_32(rsp, ASN_CONTEXT(0), object_identifier);
        *rsp++ = 0x1E;
        const PropertySpec_t * InObject = pspec;
        while (*buf != 0x1F) {
            uint8_t *hdr = buf;// начало заголовка
            ASSERT((*buf & 0xF8) == 0x08);
            uint32_t property_identifier = decode_unsigned(&buf);
            //printf("%s: - prop_id=%d\n", __FUNCTION__, property_identifier);
            rsp = encode_unsigned(rsp,  ASN_CONTEXT(2), property_identifier);
            uint32_t property_array_index = ~0;
            if (*buf!=0x1F && (*buf & 0xF8) == 0x18){// array_index -- if omitted with an array the entire array is referenced
                property_array_index =decode_unsigned(&buf);
                rsp = encode_unsigned(rsp,  ASN_CONTEXT(3), property_array_index);
            }
            pspec = InObject;
            uint16_t context_id = r3_property_find(pspec, pspec_length, property_identifier);
            if (context_id==0xFFFF){
                pspec = CommonObject;
                uint32_t _length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
                context_id = r3_property_find(pspec, _length, property_identifier);
            }

            if (context_id==0xFFFF){
                *rsp++ = 0x5E;
                rsp = encode_error(rsp, PROPERTY, UNKNOWN_PROPERTY);
                *rsp++ = 0x5F;
            } else {
                void* data = (void*)object + pspec[context_id].offset;
                /// \todo обработка индекса если массив
                *rsp++ = 0x4E;
                 rsp = _pspec_encode_param(rsp, data, context_id, pspec);
                 //rsp = r3_pspec_encode_param (rsp, data, context_id, pspec[context_id].ref);/// \todo контекстный идентификатор откуда берется?
                *rsp++ = 0x4F;
            }
        }
        *rsp++ = 0x1F;
        buf++;//ASSERT(*buf++ == 0x1F);
    }
    return rsp-response_buffer;
}
/*! \brief обработка запроса на изменение свойств объекта

WriteAccessSpecification ::= SEQUENCE {
    object-identifier  [0] BACnetObjectIdentifier,
    list-of-properties [1] SEQUENCE OF BACnetPropertyValue
}
*/
int r3cmd_write_property_multiple(DeviceInfo_t* devinfo, uint8_t * buffer, size_t length, uint8_t * response_buffer)
{
    uint8_t *buf = buffer;
    while (buf-buffer<length) {
        ASSERT(*buf == 0x0C);
        uint32_t object_identifier = decode_oid(&buf);
        //r3_decode_primitive(buf, &object_identifier, 0, ASN_TYPE_OID);

        Object_t * object = _object_lookup(devinfo, object_identifier);

        if (object==NULL)
            return ERROR(OBJECT, UNKNOWN_OBJECT);
        const PropertySpec_t * pspec = object->property_list.array;
        uint16_t pspec_length = object->property_list.count;
        if (pspec == NULL) {
            const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
            if (object_class==NULL)
                return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
            pspec = object_class->pspec;
            pspec_length = object_class->pspec_length;
        }

        ASSERT(*buf++ == 0x1E);
        while (*buf != 0x1F) {/// \todo проверять length
            ASSERT((*buf & 0xF8) == 0x08);
            uint32_t property_identifier = decode_unsigned(&buf);
            uint16_t context_id = r3_property_find(pspec, pspec_length, property_identifier);
            if ((*buf & 0xF8) == 0x18){// array_index -- if omitted with an array the entire array is referenced
                uint32_t property_array_index =decode_unsigned(&buf);
            }
            if(DBG3)printf(" == Property='%s'(%d)\n", /* __FUNCTION__, __LINE__, */bacnet_property_name(property_identifier), property_identifier);
            ASSERT(*buf++ == 0x2E);// property-value
            void* data = _property_get(object, pspec + context_id);
            buf = _pspec_decode_param(buf, data, context_id, pspec);
//            void* data = object + pspec[context_id].offset;
//            buf = r3_pspec_decode_param(buf, data, /* context_id, */pspec[context_id].ref);
            ASSERT(*buf++ == 0x2F);// priority -- used only when property is commandable
            uint8_t priority = 0;
            if ((*buf & 0xF8) == 0x38) {
                priority = decode_unsigned(&buf);
            }
			if ( pspec[context_id].flags & EX) {
				//printf("Commandable property\r\n");
				struct _CommandableObject * commandable = (void*)object;
				if(commandable!=NULL && commandable->commandable.cb!=NULL){
					printf("Commandable property update\r\n");
					(void)commandable->commandable.cb(object, property_identifier, commandable->commandable.user_data);
				}
			} else {
				printf(" == obj=%p context_id=%d\r\n",object,  context_id);
			}
        }
        /// опционально мы добавляем CRC32 или CMAC
        buf++;// == 0x1F
    }
    return 0;
}
/*! \brief обработка запроса на изменение свойств объекта

WriteAccessSpecification ::= SEQUENCE {
    object-identifier  [0] BACnetObjectIdentifier,
    list-of-properties [1] SEQUENCE OF BACnetPropertyValue
}
*/
int r3cmd_read_property_multiple_cnf(DeviceInfo_t *devinfo, uint8_t * buffer, size_t length, uint8_t * response_buffer)
{
    uint8_t *buf = buffer;
    while (buf-buffer<length) {
        ASSERT(*buf == 0x0C);
        uint32_t object_identifier = decode_oid(&buf);
        //r3_decode_primitive(buf, &object_identifier, 0, ASN_TYPE_OID);

        Object_t * object = _object_lookup(devinfo, object_identifier);
        if (object==NULL)
            return ERROR(OBJECT, UNKNOWN_OBJECT);
        const PropertySpec_t * pspec = object->property_list.array;
        uint16_t pspec_length = object->property_list.count;
        if (pspec == NULL) {
            const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
            if (object_class==NULL)
                return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
            pspec = object_class->pspec;
            pspec_length = object_class->pspec_length;
        }

        ASSERT(*buf++ == 0x1E);
        while (*buf != 0x1F) {/// \todo ограничить по другому признаку, length
            ASSERT((*buf & 0xF8) == ASN_CONTEXT(2));
            uint32_t property_identifier = decode_unsigned(&buf);
            const PropertySpec_t * _pspec;
            uint16_t context_id = r3_property_find(pspec, pspec_length, property_identifier);
            if (context_id==0xFFFF){
                _pspec = CommonObject;
                uint32_t _length = sizeof(CommonObject)/sizeof(const PropertySpec_t);
                context_id = r3_property_find(_pspec, _length, property_identifier);
            } else {
                _pspec = pspec;
            }

            if ((*buf & 0xF8) == ASN_CONTEXT(3)){// array_index -- if omitted with an array the entire array is referenced
                uint32_t property_array_index =decode_unsigned(&buf);
            }
            if (DBG3) printf(" == Property='%s'(%d)\n", /* __FUNCTION__, __LINE__, */bacnet_property_name(property_identifier), property_identifier);
            ASSERT(context_id!=0xFFFF);
            if (*buf == 0x4E) {
                buf++;
                void* data = _property_get(object, _pspec + context_id);
                buf = _pspec_decode_param(buf, data, context_id, _pspec);
    //            void* data = object + pspec[context_id].offset;
    //            buf = r3_pspec_decode_param(buf, data, /* context_id, */pspec[context_id].ref);
                ASSERT(*buf++ == 0x4F);
            } else {
                ASSERT(*buf++ == 0x5E);
                int error;
                buf = decode_error(buf, &error);
                ASSERT(*buf++ == 0x5F);
            }

        }
        buf++;// == 0x1F
    }
    return 0;
}
int r3cmd_add_list_element (DeviceInfo_t* devinfo, uint8_t* buffer, size_t length)
{
    uint8_t *buf = buffer;
    ObjectPropertyReference_t pref;
    ASSERT(*buf == 0x0C);// SD Context Tag 0 (Object Identifier, L=4)
    pref.object_identifier = decode_oid(&buf);
    ASSERT((*buf & 0xF8) == 0x18);//SD Context Tag 1 (Property Identifier, L=1)
    pref.property_identifier = decode_unsigned(&buf);
    if ((*buf & 0xF8) == 0x28) {
        pref.property_array_index = decode_unsigned(&buf);
    }
	Object_t* object=NULL;
    int result = r3cmd_property_reference (devinfo, &pref, &object, 0);
    if (result) return result;
// проверить явлется ли листом
	void* data = (void*)object + pref.pspec[pref.context_id].offset;
    List* list=*(List**)data;
    if (list) while (list->next) list=list->next;

    ASSERT(*buf++ == 0x3E);// PD Opening Tag 3 (List Of Elements)
// проверить явлется ли SEQUENCE OF
    while (*buf != 0x3F) {
        List* elem = g_slice_alloc(pref.pspec->size+sizeof(List*));
        elem->next = NULL;
///        buf = r3_pspec_decode_param(buf, elem->data, pref.pspec);
        if(list) list->next= elem;
        else *(List**)data =elem;
        list=elem;
    }
    ASSERT(*buf++ == 0x3F);
    return 0;// Simple-Ack
}
#if 0
// сравнение двух элементов списка
static int r3_param_compare(void *a, void* b, const PropertySpec_t* pspec)
{
    int result =0;
    switch (pspec->asn_type&0xF0) {
    default:
        switch(pspec->asn_type&0xF){
        case 1:
            result = *(uint8_t *)a == *(uint8_t *)b;
            break;
        case 2:
            result = *(uint16_t*)a == *(uint16_t*)b;
            break;
        default:
            result = *(uint32_t*)a == *(uint32_t*)b;
            break;
        }
        break;
    }
    return result;
}
#endif // 0
int r3cmd_remove_list_element (DeviceInfo_t* devinfo, uint8_t* buffer, size_t length)
{
    uint8_t *buf = buffer;
    ObjectPropertyReference_t pref;
    ASSERT(*buf == 0x0C);// SD Context Tag 0 (Object Identifier, L=4)
    pref.object_identifier = decode_oid(&buf);
    ASSERT((*buf & 0xF8) == 0x18);//SD Context Tag 1 (Property Identifier, L=1)
    pref.property_identifier = decode_unsigned(&buf);
    if ((*buf & 0xF8) == 0x28) {
        pref.property_array_index = decode_unsigned(&buf);
    } else pref.property_array_index = 0xFFFF;
	Object_t *object = NULL;
    int result = r3cmd_property_reference (devinfo, &pref, &object, 0);
    if (result) return result;
// проверить явлется ли листом
    ASSERT(*buf++ == 0x3E);
//    buf = r3_decode_tag(buf, &node_tag, &node_length);
    while (*buf != 0x3F) {
        //void* elem_data = g_slice_alloc(pspec->size);
///        buf = r3_pspec_decode_param(buf, elem_data, pref.pspec);// исключить копирование строк
		void* data = _property_get(object, pref.pspec + pref.context_id);
        List* list = *(List**)data;
        List* prev = NULL;
        while (list) {
///            if(r3_param_compare(list->data, elem_data, pref.pspec))
            {// если элементы сравнимы, удалить элемент списка
                if(prev) prev->next = list->next;
                else *(List**)data = list->next;
                r3_pspec_free(list->data, pref.pspec->ref, pref.pspec->size);// освободить
                g_slice_free1(pref.pspec->size+sizeof(List), list);
                break;
            }
            prev = list;
            list=list->next;
        }
    }
    buf++;// == 0x3F);
//    buf = r3_decode_tag(buf, &node_tag, &node_length);
    return result;
}
/// TODO добавить readRange и writeGroup
static Object_t * _object_lookup(DeviceInfo_t* devinfo, uint32_t object_identifier)
{
    Object_t * object = NULL;
    if (object_identifier==DEFAULT_DEVICE)
        object = (Object_t*)devinfo->device;
    else if (devinfo->device_objects)
        object = tree_lookup(devinfo->device_objects, object_identifier);
    return object;
/*
    if (object==NULL) return ERROR(OBJECT, UNKNOWN_OBJECT);
    const PropertySpec_t * pspec = object->property_list.array;
    uint16_t pspec_length = object->property_list.count;
    if (pspec == NULL) {
        const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
        if (object_class==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
        pspec = object_class->pspec;
        pspec_length = object_class->pspec_length;
    }
    *object_ref = object;
    */
}
static Object_t* _object_init(Object_t* object, uint32_t object_identifier, DeviceInfo_t *devinfo)
{
    object->object_identifier = object_identifier;
    tree_t *leaf = g_slice_alloc(sizeof(tree_t));
	tree_init(leaf, object_identifier, object);
	if (devinfo->device_objects==NULL)
        devinfo->device_objects = leaf;
	else
        tree_insert_tree(&devinfo->device_objects, leaf);
    return object;
}
static Object_t* _object_new(uint32_t object_identifier)
{
    const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
    if (object_class==NULL)
        return NULL;

    Object_t* object = malloc(object_class->size);
    __builtin_bzero(object, object_class->size);
    object->object_identifier = object_identifier;
    object->property_list.array = object_class->pspec;
    object->property_list.count = object_class->pspec_length;

    return object;
}
static int _object_delete(DeviceInfo_t* devinfo, uint32_t object_identifier)
{
	tree_t *leaf = tree_remove(&devinfo->device_objects, object_identifier);
	if (leaf != NULL) {
/*        const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
        if (object_class==NULL)
            return ERROR(OBJECT, UNSUPPORTED_OBJECT_TYPE);
*/
        Object_t* object = leaf->value;
        if (object) {
            const PropertySpec_t* pspec = object->property_list.array;
            int pspec_length = object->property_list.count;

            r3_pspec_free(object, pspec, pspec_length);
            free(object); leaf->value=NULL;
        }
        g_slice_free1(sizeof(tree_t), leaf);
		return 0;
	}
	return ERROR(OBJECT, UNKNOWN_OBJECT);
}
