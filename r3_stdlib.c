//-------------------------------------------------------
/*! \addtogroup _stdlib Стандартные утилиты
    \brief Ради избавления от лишней привязанности к стандартным библиотекам
    мы сформулировали свою стандартную библиотеку функций.
	
	\bug 22/03/18 проверить сравнение strcmp strncmp и memcmp -- надо поменять местами a и b
	
	21.03.2020 преобразовал в статическую библиотеку	
	*/


//-------------------------------------------------------
/*! \ingroup _stdlib
    \defgroup r3_queue Очередь объектов

	Очередь применяется для организации обмена между процессами. Например, запросы ставятся
	в очередь в одном процессе, а команды выполняются в другом процессе.

	Элементы очереди организуются таким образом чтобы не заниматься
	распределением динамической памяти. Для этого первым элементом структуры объекта должено
	значится поле R3list.
*/
#include "board.h"
#include <string.h>
#include <stdarg.h>
#include "trace.h"
//#include "board.h"
#include "r3stdlib.h"
//#include "math.h"
//#include "r3rtos.h"
#include "module.h"
#pragma GCC optimize("O2")
#if 0
unsigned char * bin2hex(unsigned char *buffer, unsigned int word)
{
	int count = 8;
//	unsigned char ch=0;
	buffer[count] = 0;
	do {
		count--;
//		ch = word & 0xF;
//		if (ch>9) ch+='A'-10;
//		else ch+='0';
		buffer[count] = bin2char(word);
		word>>=4;
	} while (count);
	return buffer;
}

unsigned char *	bin2str(void * buffer, unsigned char * str, int size)
{
	unsigned char  c;
	int j;
//	char str[80];// = g_new(gchar, size+size+1);
	unsigned char* str_ptr = str;
	for (j=0; j<size; j++){
		c = ((unsigned char*)buffer)[j];
		*str_ptr++ = bin2char(c>>4);
		*str_ptr++ = bin2char(c   );
	}
	*str_ptr = '\0';
	return str;
}
#endif
typedef union
  {
    float f;
	long a;

    /* This is the IEEE 754 single-precision format.  */
    struct
      {

	uint32_t mantissa:23;
	uint32_t exponent:8;
	uint32_t negative:1;
      } ieee;

    /* This format makes it easier to see if a NaN is a signalling NaN.  */
//    struct
//     {
//
//	unsigned int mantissa:22;
//	unsigned int quiet_nan:1;
//	unsigned int exponent:8;
//	unsigned int negative:1;
//
//      } ieee_nan;
  } ieee754_float;
/*! \ingroup _stdlib
    \{
*/

/*! \brief выяснение длины строки 
	\return количество символов предшествующие концу строки '\0'
 */
//size_t strlen(const char *str)  __attribute__((optimize("O3")));
size_t strlen(const char *str)
{
	register const char* s = str;
	while (s[0] != '\0') s++;
	return s-str;
}
/*
size_t strnlen (const char *str, size_t n)
{
	register const char *s = str;
	if (*s!='\0') while ((n--) && *++s != '\0');
	return s-str;
}
*/

/*! \brief поиск первого вхождения символа */
/*
char *index(const char *string, int c)
{
	while(*string != (char)c){
		if (*string == 0) return NULL;
		string++;
	}
	return (char*)string;
}
*/
//! \}
/*! высчитываем остаток от деления
    \see http://www.cs.uiowa.edu/~jones/bcd/divide.html

    Q = A*0.00011001100110011001100110011001101 (b)
	Q = A/10 for all 32-bit unsigned A

*/
static inline
unsigned long rem10ch(unsigned long value, char* str)
{
    unsigned long v = (value*0xCCCCCCCDULL)>>35;//(*value*((1ULL<<35)/10ULL))>>35;
    *str =  (value - v*10)+'0';
    return v;
}

static inline unsigned long div10(unsigned long value)
{
    return (value*0xCCCCCCCDULL)>>35;//(*value*((1ULL<<35)/10ULL))>>35;
}
/*
static inline void debug (char * str){
	while(1) {
		if (ITM->PORT[0].u32!=0){
			if (str[0]=='\0') break;
			ITM->PORT[0].u8 = *str++;
		}
	}
}
*/

/*! \ingroup _stdlib
	\brief запись целого числа в виде числа с фиксированной точкой
	\param buffer -- буфер для записи строки
	\param n -- размер буфера
	\param f -- число знаков после точки
*/
void fp_print(char * buffer, long a, int n, int f) {

    int sign = 0;
    if(a < 0) {
		a = -a;
		sign = 1;
	}

	while(n){
		n--;

		a = rem10ch(a, &buffer[n]);// = rem10((unsigned long*)&a) +'0';
//		a = (unsigned long)a/10;
		if(a == 0 && f == 0) {
			break;
		}
		if (f){ // дробная часть
			f--;
			if(f==0){
				n--;
				buffer[n] = '.';
				if (a == 0) {
					n--;
					buffer[n] = '0';
					break;
				}
			}
		}
	}
	if(sign && n!=0){
		buffer[--n]='-';
	}
	while (n) buffer[--n]=' ';
}

/*! \brief string buffer printf
*/
/*
int snprintf(char* buffer, size_t size, const char *format, ...)
{
    va_list ap;
    char c;
    va_start(ap, format); // the name of the last parameter before  the  variable argument list
    while (*format){
        c = *format++;
        if(c == '%')
        { // выделить нотацию формата
            c = *format++;
            if (c == '%') {
                continue;
            }
// префиксы (опционально)
// длина поля (опция)
            switch (c){
            case 'c': // char
            case 's': // string
            case 'd': // digital int
            case 'u': // unsigned int
            case 'f': // float
                break;
            case 'X': // hex unsigned int
                {
                    unsigned int value = va_arg(ap,  unsigned int);
                    unsigned int count = 8;
                    unsigned char ch=0;
                    buffer[count] = ch;
                    do {
                        count--;
                        ch = value & 0xF;
                        if (ch>9) ch+='A'-10;
                        else ch+='0';
                        buffer[count] = ch;
                        value>>=4;
                    } while (count);
                    buffer+=8;
                }
                break;
            default:
                *buffer++ = c;
            }
        } else { // скопировать символ в буфер
            *buffer++ = c;
        }
    }
    va_end(ap);
    return 0;
}
*/

/*! TODO 
	IEEE Std 1003.1, 2013 Edition \see http://pubs.opengroup.org/onlinepubs/9699919799/
	precision, надо обрабатывать по стандарту %-.*s и другие варианты
	модификаторы (The length modifiers): h и hh, пока не делаем: t z
	%p - указатель.
 */
enum {MOD_NO=0, 
	MOD_LL, 
	MOD_L, 
	MOD_H, 
	MOD_HH, 
	MOD_T, MOD_Z }; 
typedef struct _printf_flags Pflags;
struct _printf_flags {

	unsigned int mod      :4;

	unsigned int is_signed  :1;
	unsigned int is_upper   :1;
	unsigned int zero_padded:1;
} __attribute__((packed));
/* companion functions for vsnprintf */
#define TMP_BUFSIZE 22
static 
int put_hex_padded(char *str, int length, int width, Pflags* flags, unsigned long val) {
    char ch;// = (upper ? 'A'-10 : 'a'-10);
    int count;
    if (val==0) count=1;// в count - число символов для отображения, не меньше 1 не больше 8
    else count= (35-__builtin_clz(val))>>2;
    if (width > 0)
    { // указана ширина поля
        if (width <= count) {
            count = width;
        } else {
            ch = flags->zero_padded?'0':' ';
            length = width - count; // число пробелов или нулей
//			__builtin_memset(str, ch, length); str+=length;
            do{
                *str++ = ch;
            } while (--length);
        }
    } else width = count;
    ch = (flags->is_upper ? 'A'-10 : 'a'-10);
    val <<= (8-count)<<2;
    do {
        unsigned char c = (val>>28);
        *str++ = (c < 10)? c + '0': c + ch;
        val<<=4;
    } while (--count);
    return width; // количество разобранных символов
}

static inline int put_dec_padded(char *str, int length, int width, Pflags* flags, unsigned long val) {
    int neg = 0;
    int len;

    if(flags->is_signed){// && ((long)val < 0)) {
       neg = (long)val>>31; // результат 0 или -1
       if (neg) val = -(long)val;
    }

    if(!width) {
        int num = 0;
        char buf[TMP_BUFSIZE];
        char *tmp = buf;

        len = TMP_BUFSIZE;
        tmp += TMP_BUFSIZE - 1;
        do {
            val = rem10ch(val, tmp);
            tmp--;
            len--;
            num++;
        } while(val && len);

        if(neg && length) {
            *(str++) = '-';
            length--;
        }

        len = num;
        tmp = buf + TMP_BUFSIZE - num;
        while((len--) && (length--)) *(str++) = *(tmp++);

        return (neg ? num + 1 : num);

    } 
	else {
        len = width = (width > length ? length : width);

        str += (len - 1);
        do {
            val = rem10ch(val, str); // записывает остаток от деления на 10
            str--;
            len--;
        } while(val && len);

        if(len) {
            if(flags->zero_padded) {
                while((len--)>1) *(str--) = '0';
                if(neg) {
                    *str = '-';
                } else {
                    *str = '0';
                }
            } else {
                if(neg) {
                    *(str--) = '-';
                    len--;
                }
                while(len--) *(str--) = ' ';
            }
        }

        return width;
    }
}

static inline int put_str_padded(char *str, int length, int width, char *src) {
//    int num;
    int len = strlen(src);
    len = (len > length)?length:len;
    if(width) {
        len = (len > width ? width : len);
    } else {
        width = len;
    }
//    num = width;

    int sp = width - len;
    while(sp--) *str++ = ' ';
    while(/* *src && */(len--)) *(str++) = *(src++);

    return width;
}

//#define L_MOD 1
//#define LL_MOD 2
/*! \ingroup _stdlib
	\brief Stores the result of a formatted string into another string. Format
	arguments are given in a va_list instance.
	\return the number of characters written
	\param pStr    Destination string.
	\param length  Length of Destination string.
	\param pFormat Format string.
	\param ap      Argument list.
*/
int vsnprintf(char *pStr, size_t length, const char *pFormat, va_list ap)
{
    Pflags flags = {0,};

    int width = 0;
    int num = 0;
    int size = 0;
    int remains = 0;

    *pStr = 0;
    // Phase string
    while (*pFormat != 0 && size < length) {
        if (*pFormat != '%') { // Normal character
            *pStr++ = *pFormat++;
            size++;
        }
        else if (pFormat[1] == '%') {// Escaped '%'
            *pStr++ = *pFormat;//'%';
            pFormat+=2;
            size++;
        }
        else { // Token delimiter
            width = 0;
            flags.zero_padded = 0;
			flags.mod =0;
//            flags.l_mod = 0;
//            flags.ll_mod = 0;
            flags.is_signed = 0;
            num = 0;

            pFormat++;
            remains = length - size;
            if (*pFormat == '0') {// Parse filler
                flags.zero_padded = 1;
                pFormat++;
            }
            if (*pFormat == '*') { // Parse width
                width = va_arg(ap, unsigned int);
                pFormat++;
            } else
            while ((*pFormat >= '0') && (*pFormat <= '9')) {
                width = (width*10) + *pFormat-'0';
                pFormat++;
            }

            // parse length modifiers
            if(*pFormat == 'l') {
                pFormat++;
                if(*pFormat == 'l') {
					flags.mod = MOD_LL;
                    pFormat++;
                } else {
					flags.mod = MOD_L;
                }
            } else 
			if(*pFormat == 'h') {
				pFormat++;
				if(*pFormat == 'h') {
					flags.mod = MOD_HH;
					pFormat++;
				} else {
					flags.mod = MOD_H;
				}
			}
            // Check if there is enough space
            if(width > remains)  width = remains;

            // Parse type
            switch (*pFormat) {

            // decimal
            case 'd':
            case 'i': flags.is_signed = 1;
            case 'u':
                {
                    unsigned long val;
                    if(flags.mod==MOD_LL) {
                        val = va_arg(ap, unsigned long long);
                    } else if (flags.mod==MOD_L) {
                        val = va_arg(ap, unsigned long);
					} else {
                        val = va_arg(ap, unsigned int);
                    }
                    num = put_dec_padded(pStr, remains, width, &flags, val);
                }
                break;
			// value of the pointer
			case 'p': {
				uint32_t val = (uint32_t) va_arg(ap, void*);
				num = put_hex_padded(pStr, remains, width, &flags, val);
			} break;
            // hexadecimal
            case 'X': flags.is_upper = 1;
            case 'x':
                {
                    unsigned long val;
                    if(flags.mod==MOD_LL) {
                        val = va_arg(ap, unsigned long long);
                    } else if (flags.mod==MOD_L) {
                        val = va_arg(ap, unsigned long);
					} else {
                        val = va_arg(ap, unsigned int);
                    }
                    num = put_hex_padded(pStr, remains, width, &flags, val);
                }
                break;

            // string
            case 's':
                num = put_str_padded(pStr, remains, width, va_arg(ap, char *));
                break;

            // char
            case 'c':
                *pStr = va_arg(ap, int);
                num = 1;
                break;
			// float 32
			case 'f':
//				break;
            default:
                return -1;
            }

            pFormat++;
            pStr += num;
            size += num;
        }
    }

    // NULL-terminated (final \0 is not counted)
    if (size < length) {

        *pStr = 0;
    }
    else {

        *(--pStr) = 0;
        size--;
    }

    return size;
}

/*! \ingroup _stdlib
	\brief Stores the result of a formatted string into another string. Format
	arguments are given in a va_list instance.
	\param pString Destination string.
	\param length  Length of Destination string.
	\param pFormat Format string.
	\param ...     Other arguments
	\return the number of characters written.
*/
int snprintf(char *pString, size_t length, const char *pFormat, ...)
{
    va_list ap;
    int rc;

    va_start(ap, pFormat);
    rc = vsnprintf(pString, length, pFormat, ap);
    va_end(ap);

    return rc;
}
