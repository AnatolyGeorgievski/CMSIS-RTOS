#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <malloc.h>
#include <stddef.h>
#include "r3_asn.h"

//#include <glib.h>
#include "r3_slice.h"
#include "atomic.h"



uint8_t* r3_asn_decode_length(uint8_t *buf, size_t *node_length)
{
    uint32_t len = *buf++ & ASN_CLASS_MASK;
    if (len == 0x5) {
        len = *buf++;
        if (len == 254) {
            len = (buf[0]<<8) | buf[1]; buf+=2;
        } else
        if (len == 255) {
            len = *(uint32_t *) buf;
            len = __builtin_bswap32(len);//ntoh() (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
            buf+=4;
        }
    }
	*node_length = len;
	return buf;
}
/*! разобрать последовательнсоть данных
    выходные параметры тип данных и длина
*/
uint8_t * r3_decode_tag (uint8_t *buf, uint16_t* node_tag, size_t* node_length)
{
	size_t len;
	uint16_t tag = *buf++;
	if ((tag & 0xF0) == 0xF0)
		tag |= (uint16_t)(*buf++)<<8;
	if ((tag&ASN_CLASS_CONSTRUCTIVE) == ASN_CLASS_CONSTRUCTIVE) {// открывающий или закрывающий тег
		len = 0;
	} else {
		len = tag & ASN_CLASS_MASK;
        if (len == 0x5) {
            len = *buf++;
            if (len == 254) {
                len = (buf[0]<<8) | buf[1]; buf+=2;
            } else
            if (len == 255) {
                len = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
                buf+=4;
            }
        }
	}
	*node_tag = tag;
	*node_length = len;

	return buf;
}
/*!
1. старшие четыре бита 0xF0 node->tag обязаны содержать указание на основной тип данных. Если код больше 0xC(14) то записывается как (code<<8)|0xF0, если меньше, то записывается как (code<<4);
2. если тип контекстный то поле node->type_id содержит контекстный идентификатор, иначе тот же тип данных.
 */
uint8_t * r3_encode_tag (uint8_t *buf, uint16_t tag, size_t len)
{
	if (len >= 0x05) {
	    tag |= 0x05;
	} else {
        tag |= len;
	}
    *buf++ = tag;
	if (tag>>8) {
        *buf++ = tag>>8;
	}
	if (len >= 0x5) {
		if (len>0xFFFF) {// эта ветка не реализуется потому что node->length = 16бит
            *buf++ = 0xFF;//255
            *buf++ = len>>24;
            *buf++ = len>>16;
            *buf++ = len>> 8;
		} else
		if (len>0xFF){
		    *buf++ = 0xFE;//
            *buf++ = len>> 8;
		}
		*buf++ = len;
	}

	return buf;
}
/*! \brief прропустить запись при декодировании */
uint8_t * r3_asn_next(uint8_t * buf)
{
    if ((*buf & 0xF) == ASN_CLASS_CLOSING) {// ничего не делаем, уперлись
    } else
    if ((*buf & 0xF) == ASN_CLASS_OPENING) {
		uint8_t tag = *buf;
        if ((*buf++ & 0xF0)==0xF0) buf++;
        while (*buf!= (tag|ASN_CLASS_CLOSING)){
            buf = r3_asn_next(buf);
        }
        if ((*buf++ & 0xF0)==0xF0) buf++;// такие не используем, упрощение
    } else {
        uint16_t tag = *buf++;
        size_t length= tag & 0x7;
		if ((tag & 0xF8)==ASN_TYPE_BOOLEAN) {// исключением является тип ASN_TYPE_BOOLEAN
			length = 0;
		} else {
			if ((tag & 0xF0)==0xF0) tag |= (uint16_t)(*buf++)<<8;
			if (length == 0x5){
				length = (uint32_t)*buf++;
				if (length == 0xFE) {
					length = __builtin_bswap16(*(uint16_t*)buf);
					buf+=2;// без выравнивания данных
				} else
				if (length > 0xFE) {
					length = __builtin_bswap32(*(uint32_t*)buf);
					buf+=4;// без выравнивания данных
				}
			}
			buf+=length;
		}
    }
    return buf;
}
uint8_t * r3_pspec_decode_param(uint8_t *buf, void* ptr, const PropertySpec_t* desc);
/*! \brief декодирование структурированного типа для представления в памяти
 */
uint8_t* r3_pspec_decode(uint8_t* buf, void* data, const PropertySpec_t* desc, int desc_length)
{
    uint16_t context_id=0;
    for (context_id=0; context_id<desc_length; context_id++)
    {
        uint16_t node_tag;
        size_t   node_length;
        buf = r3_decode_tag(buf, &node_tag, &node_length);
        if ((node_tag & ASN_CLASS_MASK)==ASN_CLASS_CLOSING){
            break;
        }
		if (node_tag & ASN_CLASS_CONTEXT) {
			context_id = (node_tag&0xF0)==0xF0? (node_tag>>8): (node_tag>>4);
			if (context_id>= desc_length)
				return buf + node_length;// не разобрано!!
		} else {
			// сравнить с имеющимся типом
			while (desc[context_id].flags & OPTIONAL) {
				if (((desc[context_id].asn_type ^ node_tag)&0xF0)==0) break;
				context_id++;
			}
		}
        void* ptr = data+desc[context_id].offset;
		buf = r3_pspec_decode_param(buf, ptr, &desc[context_id]);
	}
	return buf;
}
#if 0
uint8_t r3_decode_primitive(uint8_t *buf, uint16_t asn_type)
{
    if ((asn_type == ASN_TYPE_OID) || (asn_type == ASN_TYPE_OID))
        return __builtin_bswap32(*(uint32_t*)buf);
	uint16_t node_tag;
	size_t   node_length = *buf;

    switch (asn_type&0xF0) {
    case ASN_TYPE_ENUMERATED:
    case ASN_TYPE_UNSIGNED:
        if (size>=4) {
            size-=4;
            *(uint32_t*)(data+size) = __builtin_bswap32(*(uint32_t*)buf);
            buf+=4;
        }
        if (size&2) {
            size-=4;
            *(uint32_t*)(data+size) = __builtin_bswap16(*(uint32_t*)buf);
            buf+=4;
        }
        if (size&2) {
            *(uint32_t*)(data) = *buf++;
        }

        switch (size)

    default:

        return __builtin_bswap32(*(uint32_t*)buf);
        break;
    }
    *buf+=node_length;
}
#endif // 0
uint8_t * r3_pspec_decode_param(uint8_t *buf, void* ptr, const PropertySpec_t* desc)
{
	uint16_t node_tag;
	size_t   node_length;
	buf = r3_decode_tag(buf, &node_tag, &node_length);
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
			case 2: value = __builtin_bswap16(*(uint16_t*)buf); break;
			case 3: value = (uint32_t)buf[0]<<16 | (uint32_t)buf[1]<<8 | (uint32_t)buf[2]; break;
			case 4: value = __builtin_bswap32(*(uint32_t*)buf); break;
			default:value = 0; break;
			}
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
				str = (uint8_t*)atomic_pointer_exchange((void**)ptr, str);
				if (str!=NULL/* (SEGMENT(str)!=iFLASH */) {
					free(str);
				}
			}
		} break;
		case ASN_TYPE_BIT_STRING: { // 0x80
			if (node_length>0) {// битовая строка кодируется как число бит в последнем октете
			/*  The initial octet shall encode, as an unsigned binary integer, the number of unused bits in the final subsequent octet.
				The number of unused bits shall be in the range zero to seven, inclusive. */
				uint8_t* bits = ptr;
				uint8_t mask = 0xFF>>buf[0];
				int size = (desc->size<node_length?desc->size:node_length-1)-1;
				int i;
				for (i=0;i<size-1; i++) {
					bits[i] = buf[i+1];
				}
				bits[i] = buf[i+1] & mask; // Undefined bits shall be zero
				size = desc->size;
				for (++i;i<size; i++) {
					bits[i] = 0;
				}
			}
		} break;
		case ASN_TYPE_REAL:         // 0x40
		case ASN_TYPE_DATE:         // 0xA0
		case ASN_TYPE_TIME:         // 0xB0
		case ASN_TYPE_OID :         // 0xC0
		default: // REAL DATE TIME OID
			if (node_length==4)
				*(uint32_t*)ptr = __builtin_bswap32(*(uint32_t*)buf);
			break;
		}
		buf+=node_length;
    }
    return buf;
}
static inline uint8_t* r3_encode_constructed(uint8_t *buf, uint16_t tag)
{
    *buf++ = tag;
//    if (tag>>8) *buf++ = (tag>>8);
    return buf;
}
uint8_t* r3_pspec_encode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const PropertySpec_t* desc);
uint8_t* r3_pspec_encode(uint8_t* buf, void* data, const PropertySpec_t* desc, uint16_t desc_length)
{
    uint16_t context_id=0;
    for (context_id=0; context_id<desc_length; context_id++)
    {
        void* ptr = data+desc[context_id].offset;
        if (desc[context_id].flags & OPTIONAL) {
            if (/* desc[context_id].asn_type==4 && */ *(uint32_t*)ptr==(uintptr_t)desc[context_id].ref) {// значение по умолчанию
                continue;
            }
        }
        buf = r3_pspec_encode_param(buf, ptr, context_id, &desc[context_id]);
    }// for
    return buf;
}
uint8_t* r3_asn_encode_unsigned(uint8_t * buf, uint8_t asn_type, uint32_t value)
{
#if 0
    int size = value!=0? 4-(__builtin_clz(value)>>3) : 0;
    *buf++= asn_type | size;
    switch (size){
    case 4: *(uint32_t*)buf=__builtin_bswap32(value);buf+=4; break;
    case 3: *buf++=value>>16;
    case 2: *buf++=value>>8;
    case 1: *buf++=value;
    default: break;
    }
#else
	uint8_t* tag = buf++;
	do{
		*buf++ = value&0xFF;
	} while(value>>=4);
	*tag = asn_type | (buf-tag-1);
#endif
    return buf;
}
uint8_t* r3_asn_encode_signed(uint8_t * buf, uint8_t asn_type, int32_t value)
{
    uint32_t u32 = (uint32_t)value;
    int size;
    if (value<0) {
        size = ~u32!=0? 4-((__builtin_clz(~u32)-1)>>3) : 1;
    } else {
        size = u32!=0? 4-((__builtin_clz(u32)-1)>>3) : 1;
    }
    *buf++= asn_type | size;
    switch (size){
    case 4: *(uint32_t*)buf=__builtin_bswap32(u32);buf+=4; break;
    case 3: *buf++=u32>>16;
    case 2: *buf++=u32>>8;
    case 1: *buf++=u32;
    default: break;
    }
    return buf;
}
uint8_t* r3_asn_decode_signed(uint8_t *buf, int32_t *value)
{
    int size = *buf++ & 0x7;
    switch (size) {
    default:
    case 0: *value = 0; break;
    case 1: *value = (int8_t)buf[0]; break;
    case 2: *value = (int16_t)__builtin_bswap16(*(uint16_t*)buf); break;
    case 3: *value = (int32_t)buf[0]<<16 | (uint32_t)buf[1]<<8 | (uint32_t)buf[2]; break;
    case 4: *value = (int32_t)__builtin_bswap32(*(uint32_t*)buf); break;
    }
    return buf+size;
}

uint8_t* r3_asn_encode_u32   (uint8_t * buf, uint8_t asn_type, uint32_t value)
{
    *buf++= asn_type | 0x04;
    *(uint32_t*)buf=__builtin_bswap32(value);
    return buf+4;
}
uint8_t* r3_asn_encode_real   (uint8_t * buf, uint8_t asn_type, float value)
{
    *buf++= asn_type | 0x04;
    *(float*)buf = value;
    return buf+4;
}

uint8_t* r3_asn_encode_string(uint8_t *buf, uint8_t asn_type, const char* str, size_t len)
{
    len++;
    if (len>4) {
        *buf++ = asn_type | 0x5;
        *buf++ = len;
    } else {
        *buf++ = asn_type | len;
    }
    *buf++ = 0x00;// unicode utf-8 - лишний символ
	len--;
    __builtin_memcpy(buf, str, len); buf+=len;
    return buf;
}
uint8_t* r3_asn_encode_octets(uint8_t *buf, uint8_t asn_type, const uint8_t* str, size_t len)
{
    if (len>4) {
        *buf++ = asn_type | 0x5;
        *buf++ = len;
    } else {
        *buf++ = asn_type | len;
    }
    __builtin_memcpy(buf, str, len); buf+=len;
    return buf;
}

/*! \brief сформировать ошибку */
uint8_t* r3_asn_encode_error(uint8_t *buf, uint8_t error_class, uint32_t error_code)
{
    *buf++ = 0x91;// ENUMERATED len=1
    *buf++ = error_class;
    if ((error_code>>8)!=0) {
        *buf++ = 0x92;// ENUMERATED len=1|2
        *buf++ = error_code>>8;
    } else {
        *buf++ = 0x91;// ENUMERATED len=1|2
    }
    *buf++ = error_code;
    return buf;
}

// Упрощенный вариант кодирования
uint8_t* r3_asn_encode_tag(uint8_t *buf, uint8_t tag, size_t len)
{
    if (len>=0x05) {
        tag |= 0x05;
		*buf++ = tag;
		if (len>254) {
			*buf++ = 254;
			*buf++ = len>>8;
		}
		*buf++ = len;
	} else {
		*buf++ = tag | len;
	}
    return buf;
}
/*! \brief выполнить кодирование базового типа
	Стараемся Исключить кодирование в таком виде
 */
uint8_t * r3_pspec_encode_param(uint8_t *buf,  void*ptr, uint16_t context_id, const PropertySpec_t* desc)
{
    uint16_t node_tag = desc->asn_type;
    if (node_tag & ASN_CLASS_CONTEXT) {
        node_tag = (context_id>=0xF)?(context_id<<8)|0xF8: (context_id<<4)|0x08;
    }
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
    {// не конструктивно
        size_t node_length = desc->asn_type & 0x7;
        union {
            int32_t i;
            uint32_t u;
        } value = {.u=0};
        switch(desc->asn_type & 0xF0) {
        case ASN_TYPE_NULL:
            node_length = 0;
            break;
        case ASN_TYPE_BOOLEAN: {// 0x10
            value.u = *(bool*)ptr;
            node_length = ((node_tag&ASN_CLASS_CONTEXT) /*|| node->value.b*/)? 1: 0;
        } break;
        case ASN_TYPE_ENUMERATED:// 0x90
        case ASN_TYPE_UNSIGNED:// 0x20
            switch (node_length){
            case 1: value.u = *(uint8_t *)ptr;  break;
            case 2: value.u = *(uint16_t*)ptr;  break;
            default:
            case 4: value.u = *(uint32_t*)ptr;  break;
            }
            node_length = value.u!=0? sizeof(value.u)-(__builtin_clz(value.u)>>3) : 0;
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
        case ASN_TYPE_OCTETS://0x60
			if (node_length==0) node_length = desc->size;
			uint8_t *s=*(uint8_t**) ptr;
            while (node_length!=0 && s[node_length-1]!=0) node_length--;
            break;
        case ASN_TYPE_STRING: {//0x70
            node_length = 0;//strlen(*(char**)ptr)+1;
            char* s=*(char**)ptr;
            if (s) {
                while(s[node_length++]!='\0');
                //node_length++;// ноль копируем?
            }
            ptr = *(char**)ptr;
        } break;
        case ASN_TYPE_BIT_STRING: { //0x80
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
#if 0 // если не бакнет
        if (node_length){
            __builtin_memcpy(buf, ptr, node_length);
            buf+=node_length;
        }
#elif 0
            while (node_length>=4) {
                node_length-=4;
                *(uint32_t*)buf= *(uint32_t*)(ptr); buf+=4;
                ptr+=4;
            }
            if (node_length>=2) {
                node_length-=2;
                *(uint16_t*)buf = *(uint16_t*)(ptr); buf+=2;//value.u>>16;
                ptr+=4;
            }
            if (node_length>0) {
                *buf++ = *(uint8_t*)ptr;
            }
#else
        switch(desc->asn_type & 0xF0) {
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
            buf[0]  = __builtin_clz(bits[node_length-2])-24;// = unused_bits 0..7;
            __builtin_memcpy(&buf[1], ptr, node_length-1);
            buf+=node_length;
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
#endif // 1
    }
    return buf;
}

/* кодирование длины типа */
#define ASN_SIZE(s)  (((s)>4)?((s)<<8)|0x5 : (s))
//#define PROPERTY_ARRAY_SIZE(p) 0xFFFF
/*! \brief пройти по списку и удалить все динамические данные */
void r3_pspec_free(void* data, const PropertySpec_t *desc, int pspec_length)
{
    while (pspec_length--) {
        void* ptr = data+desc->offset;
        if ((desc->asn_type & ASN_CLASS_MASK)==ASN_CLASS_CONSTRUCTIVE) {
            r3_pspec_free(ptr, desc->ref, desc->size);
        } else
        if ((desc->asn_type & (ASN_CLASS_MASK))==(ASN_CLASS_SEQUENCE_OF)) {// 0x5
            List* list = *(List**)ptr;
            while (list) {
                List* next = list->next;
                r3_pspec_free(list->data, desc->ref, desc->size);
                g_slice_free1((desc->asn_type>>8) + sizeof(List), list);
                list=next;
            }
        } else
        {
            switch (desc->asn_type&0xF0) {
            case ASN_TYPE_STRING:
                /* если строка в динамической памяти */
                free(*(void**)ptr);
                break;
            default: break;
            }
        }
        desc++;
    }
}
// это относится к понятию объект
