//-------------------------------------------------------
/*!	Ради избавления от лишней привязанности к стандартным библиотекам
    мы сформулировали свою библиотеку поддержки Си, libс.
	
	21.03.2020 преобразовал в статическую библиотеку	
	*/
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

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
static inline 
size_t strlen(const char *str)
{
	register const char* s = str;
	while (s[0] != '\0') s++;
	return s-str;
}
static inline 
size_t strnlen(const char *str, size_t n)
{
	register const char* s = str;
	while (s[0] != '\0' && (n--)) s++;
	return s-str;
}

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
static inline
unsigned long rem10(unsigned long value, unsigned long* rem)
{
    unsigned long v = value/10;// (value*(0xCCCCCCCDULL>>0))>>35;//(*value*((1ULL<<35)/10ULL))>>35;
    *rem = (value - v*10);
	return v;
}
// число на входе должно быть число меньше 9999,9999
static uint32_t bin2bcd(uint32_t n)
{
	unsigned long rem, bcd=0;
	int i;
	for (i=0; n>=10 && i<28; i+=4) {
		n = rem10(n, &rem);
		bcd += (rem<<i);
	}
	return bcd + (n<<i);
}

static inline unsigned long div10(unsigned long value) {
    return (value*0xCCCCCCCDULL)>>35;//(*value*((1ULL<<35)/10ULL))>>35;
}
// число знаков log10 можно найти по таблице методом bsearch
static int decimal_digits(uint32_t n)
{
	int num=0;
	while(n>=10)
		n = n/10, num++;
	return num+1;
}
static 
int bin2bcd_s(uint8_t *str, uint32_t n)
{
	uint8_t *buf = str;
	int i;
	while(n>=10) {
		unsigned long rem;
		n = rem10(n, &rem);
		*buf++ = rem + '0';
	}
	*buf++ = n + '0';
	return buf-str;
}


#if 0
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
#endif
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
enum {
	MOD_NO=0, 
	MOD_HH, 
	MOD_H, 
	MOD_L, 
	MOD_LL, 
	MOD_J, // Specifies that a following d, i, o, u, x, or X conversion specifier applies to an intmax_t or uintmax_t argument
	MOD_T, // Specifies that a following d, i, o, u, x, or X conversion specifier applies to a ptrdiff_t
	MOD_Z, // Specifies that a following d, i, o, u, x, or X conversion specifier applies to a size_t
	MOD_P, // pointer to void
	MOD_LD // Specifies that a following a, A, e, E, f, F, g, or G conversion specifier applies to a long double
	}; 
typedef struct _printf_flags Pflags;
struct _printf_flags {
	unsigned int order		:8;// In format strings containing the "%n$" form of a conversion specification, where m is a decimal integer in the range [1,{NL_ARGMAX}]
	unsigned int mod      	:4;
	unsigned int is_signed  :1;
	unsigned int is_upper   :1;
	unsigned int zero_padded:1;
	unsigned int left_justified:1;
	unsigned int show_sign	:1;
	unsigned int alternate:1;
} __attribute__((packed));
/* companion functions for vsnprintf */
#define TMP_BUFSIZE 22
static int _padding(uint8_t * buf, int remains, int width, int digits, int precision)
{
	int padding = 0;
	if (precision > 0) {
		padding = precision;
		uint8_t pad = '0';
		int count = padding;
		do{
			*buf++ = pad;
		} while(--count);
	}
	return padding;
}
/*! Дробная часть числа float
	\param precision число знаков после запятой
 эта операция быстрее чем вычисление остатка, однако требует выравнивания числа
*/
static int fmt_fix(char *buf, int width, int precision, unsigned long val){
	if (precision>width) precision = width;
	int i=0;
    for (;i<precision;i++, val*=10) *buf++ = ((val*10ULL)>>32)+'0';
	return i;
}
#if 0
static int _bsearch(const uint32_t key, const uint32_t *base, int num)
{
        int l = 0, u = num, mid;
        while (l < u) {
                mid = (l + u)>>1;
                register int result = (key - base[mid]);//cmp(key, p);
                if (result < 0)
                        u = mid;
                else if (result > 0)
                        l = mid+1;
                else
                        return mid+1;
        }
        return l;
}
static const uint32_t decs[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000,};
static inline int _digits (uint32_t n)
{
	return _bsearch(n, decs, 8);
}
#endif
// для чисел >0, precision <= width
static int fmt_dec(char *buf, int remains, int width, int precision, unsigned long val, Pflags* flags){
	char bcd[12];
	char* s = bcd;
	char sign=0;
	if (flags->is_signed && (val>>31)) {
		sign = '-';
		val = -(int32_t)val;
	}
#if 0
	else if (flags->show_sign){
		sign = '+';
	}
#endif
int base = 10;
if (base==10) {
    while (val>=10) 
		val = rem10ch(val, s++);
	*s++ = val + '0';

} else {
	static const char digits[] = "0123456789ABCDEF";
	while (val>=base) {
		*s++ = digits[val%base];
		val = val/base;
	}
	*s++ = digits[val];
}
	int num = s-bcd;
	if (sign) num++;
	if (num > remains) num = remains;
	
	if (width<num) width= num;
//	if (num>width) num = width;
	if (precision<num) precision=num;
	int i=0;
	if (sign) *buf++ = sign, i++, num--;
	for (;i<precision-num;i++) *buf++ = '0';
	for (;i<precision;i++) *buf++ = *--s;
	if(0 && flags->left_justified)for (;i<width;i++) *buf++ = ' ';
	return i;
}
static int fmt_hex(char *buf, int remains, int width, int precision, unsigned long val, Pflags* flags){
	int num = (35 - __builtin_clz(val))>>2;
	if (num > remains) num = remains;
	val <<=32 - num*4;
	int i=0;
	if (width<num) width= num;
	if (precision<num) precision=num;
	int padding =0;
	// это работает, только место занимает. Я решил место экономить
	if (flags->left_justified==0 && precision<width) {
		padding = width-precision;
		uint8_t pad = flags->zero_padded? '0': ' ';
		for (;i<padding;i++) *buf++ = pad;
		precision+=padding;
	}
	for (;i<precision-num;i++) *buf++ = '0';
    for (;i<precision;i++, val<<=4) {
		char c = (val>>(32-4));
		*buf++ = c + (c>=10? 'A'-10:'0');
	}
	if (flags->left_justified){// это тоже работает
		for (;i<width;i++) *buf++ = ' ';
	}
	return i;
}
static int fmt_oct(char *buf, int remains, int width, int precision, unsigned long val, Pflags* flags){
	int num = (unsigned)(34 - __builtin_clz(val))/3;
	if (num > remains) num = remains;
	val <<=32 - num*3;
	if (width<num) width= num;
	if (precision<num) precision=num;
	int i=0;
	for (;i<precision-num;i++) *buf++ = '0';
    for (;i<precision;i++, val<<=3) *buf++ = (val>>(32-3))+'0';
	return i;
}
static int fmt_bin(char *buf, int remains, int width, int precision, unsigned long val, Pflags* flags){
	int num = 32-__builtin_clz(val);
	if (num > remains) num = remains;
	val <<=32 - num;
	if (width<num) width= num;
	if (precision<num) precision=num;
	int i=0;
	for (;i<precision-num;i++) *buf++ = '0';
	for (;i<precision;i++, val<<=1) *buf++ = (val>>(32-1))+'0';
	return i;
}
/*! 
signed decimal (i,d) in the style [-]dddd
unsigned decimal (u), in the style dddd; 
The precision specifes the minimum number of digits to appear; if the value being converted
can be represented in fewer digits, it is expanded with leading zeros. The default precision
is 1. 
*/
/*!
	\brief Stores the result of a formatted string into another string. Format
	arguments are given in a va_list instance.
	\ingroup _stdio _libc
	\return the number of characters written
	\param pStr    Destination string.
	\param length  Length of Destination string.
	\param pFormat Format string.
	\param ap      Argument list.
*/
int vsnprintf(char *pStr, size_t length, const char *pFormat, __builtin_va_list ap)
{
    Pflags flags = {0,};
    int width = 0, width_frac = 0;
    int num = 0;
    int size = 0;
    int remains = 0;
	int precision = 0;

length--;
//    *pStr = 0;
    // Phase string
    while (*pFormat != '\0' && size < length) {
        if (*pFormat != '%') { // Normal character
            *pStr++ = *pFormat++;
            size++;
			continue;
        } else 
		if (pFormat[1] == '%') {// Escaped '%'
            *pStr++ = '%';
            pFormat+=2;
            size++;
        }
        else { // Token delimiter
			flags = (Pflags){0};
            flags.left_justified = 0;
            flags.zero_padded = 0;
//			  flags.mod =MOD_NO;
//            flags.l_mod = 0;
//            flags.ll_mod = 0;
//            flags.is_signed = 0;
            pFormat++;
            remains = length - size;
            if (*pFormat == '-') {// Parse filler
                flags.left_justified = 1;
                pFormat++;
            }
			
            if (*pFormat == '+') {// Parse filler
                //flags.show_sign = 1;
                pFormat++;
            } else 
            if (*pFormat == ' ') {// Parse filler
                //flags.show_space = 1; -- выравнивание, когда знака нет
                pFormat++;
			}
            if (*pFormat == '#') {// Parse filler
                flags.alternate = 1;
                pFormat++;
            }
            if (*pFormat == '0') {// Parse filler
                flags.zero_padded = 1;
                pFormat++;
            }
			if (*pFormat == '*') { // Parse width
				width = va_arg(ap, unsigned int);
				pFormat++;
			} else {
				width = 0;
				while (('0' <= *pFormat) && (*pFormat <= '9')) {
					width = (width*10) + *pFormat-'0';
					pFormat++;
				}
			}
			if (*pFormat == '.') {// For d, i, o, u, x, and X conversions, if a precision is specifed, the 0 ﬂag is ignored.
				pFormat++;
				if (*pFormat == '*') { // Parse width
					precision = va_arg(ap, unsigned int);
					pFormat++;
				} else {
					precision = 0;
					while (('0' <= *pFormat) && (*pFormat <= '9')) {
						precision = (precision*10) + *pFormat-'0';
						pFormat++;
					}
				}
			} else 
				precision = 1;
			

            // parse length modifiers
            if(*pFormat == 'l') {
                if(pFormat[1] == 'l') {
					flags.mod = MOD_LL;
                    pFormat++;
                } else {
					#if (ULONG_MAX!= UINT_MAX)
					flags.mod = MOD_L;
					#endif
                }
				pFormat++;
            } else 
			if(*pFormat == 'h') {
				if(pFormat[1] == 'h') {
//					flags.mod = MOD_HH;
					pFormat++;
				} else {
//					flags.mod = MOD_H;
				}
				pFormat++;
			} else 
			if(*pFormat == 'z') { // size_t 
				#if (SIZE_MAX!= UINT_MAX)
				flags.mod = MOD_Z;
				#endif
				pFormat++;
			} else 
			if(*pFormat == 'j') { // intmax_t 
				#if (UINTMAX_MAX!= UINT_MAX)
				flags.mod = MOD_J;
				#endif
				pFormat++;
			} else 
			if(*pFormat == 't') { // ptrdiff_t 
				#if (PTRDIFF_MAX!= INT_MAX)
				flags.mod = MOD_T;
				#endif
				pFormat++;
			}
            // Check if there is enough space
            if(width > remains)  width = remains;
			int (*fmt_func)(char*, int, int, int, unsigned long val, Pflags* flags);
			fmt_func = NULL;
            // Parse type
            switch (*pFormat) {

            // signed decimal: The int argument is converted to signed decimal in the style [-]dddd. 
#if 0
                if (0) {
                    long val;
                    switch(flags.mod) {
					case MOD_LL:
                        val = va_arg(ap, unsigned long long);
						break;
					#if (UINT_MAX!= ULONG_MAX)
					case MOD_L:
                        val = va_arg(ap, unsigned long);
						break;
					#endif
					#if (SIZE_MAX!= UINT_MAX)
					case MOD_Z:
                        val = va_arg(ap, size_t);
						break;
					#endif
					default:
                        val = va_arg(ap, unsigned int);
						break;
                    }
					if (val == 0) {// The default precision is 1. The result of converting a zero value with a precision of zero is no characters.
						*pStr = '0';
						num = 1;//_padding(pStr, remains, width, 0, precision);
					} else {
						num = fmt_dec(pStr, remains, width, precision, val, &flags);
						//num = long_to_decstring(pStr, width, precision, val, &flags);
					}
					pStr+=num;
                }
                break;
#endif
			case 'n': {
				int *val = va_arg(ap, int*);
				*val = length - remains;
			} break;
			// value of the pointer
            // hexadecimal
            case 'b': fmt_func = fmt_bin; break;
            case 'o': fmt_func = fmt_hex; break;// хотим отказаться от использования восьмиричной системы
            case 'd': flags.is_signed = 1; fmt_func = fmt_dec; break;
            case 'i': flags.is_signed = 1; fmt_func = fmt_dec; break;
            case 'u': fmt_func = fmt_dec; break;
			case 'p': 
					#if (UINTPTR_MAX!= UINT_MAX)
					flags.mod = MOD_P;
					#endif
            case 'X': flags.is_upper = 1; fmt_func = fmt_hex; break;
            case 'x': fmt_func = fmt_hex; break;
            case 's': {// string
				char* s = va_arg(ap, char *);
				if (s==NULL) s = "(null)";
				if (precision>remains) 
					precision=remains;
				num = strnlen(s, (precision==1)?remains:precision);
				if ((precision==1)) precision = num;
				//if ((width<num==1)) precision = num;
				int padding = 0;
#if 0
				if (flags.left_justified) {
				} else {
					padding = width - num;
				}
#endif
				int i=0;
				for (;i<padding;i++) *pStr++ = ' ';
				for (;i<num+padding;i++) *pStr++ = *s++;
				if(0)for (;i<width;i++) *pStr++ = ' ';
				num = i;
			} break;
            // char
            case 'c':
                *pStr++ = va_arg(ap, int);
                num = 1;
                break;
			// float 32
			case 'f':
				break;
            default:
                return -1;
            }
			if (fmt_func){
				unsigned long val;
				switch(flags.mod) {
				case MOD_LL:
					val = va_arg(ap, unsigned long long);
					break;
				#if (ULONG_MAX != UINT_MAX)
				case MOD_L:
					val = va_arg(ap, unsigned long);
					break;
				#endif
				#if (UINTMAX_MAX!= UINT_MAX)
				case MOD_J:
					val = va_arg(ap, uintmax_t);
					break;
				#endif
				#if (PRTDIFF_MAX!= INT_MAX)
				case MOD_T:
					val = va_arg(ap, prtdiff_t);
					break;
				#endif
				#if (SIZE_MAX!= UINT_MAX)
				case MOD_Z:
					val = va_arg(ap, size_t);
					break;
				#endif
				#if (UINTPTR_MAX!= UINT_MAX)
				case MOD_P:
					val = (uintptr_t) va_arg(ap, void*);
					break;
				#endif
				default:
					val = va_arg(ap, unsigned int);
					break;
				}
				//
				if (val == 0) {// The result of converting a zero value with a precision of zero is no characters. Default precision =1;
					*pStr = '0';
					num = 1;//_padding(pStr, width, 0, precision);
				} else {
					num = fmt_func(pStr, remains, width, precision, val, &flags);
				}
				pStr+=num;
            }

            pFormat++;
           // pStr += num;
            size += num;
        }
    }

    // NULL-terminated (final \0 is not counted)
    *pStr = 0;
    return size;
}

/*!	\brief Stores the result of a formatted string into another string. Format
	arguments are given in a va_list instance.
	\ingroup _stdio _libc
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

#ifdef TEST_PRINTF
// $ gcc -DTEST_PRINTF -o printf r3core/r3_stdlib.c

//#include <stdio.h>
//extern void puts(char*);
int main ()
{
	char buf[80];
	char str[] = "Hello World!";
	printf("|%12.8s|\n", str);
	snprintf(buf, 80, "|%12.8s|", str);
	puts(buf);
	printf("|%12.12s|\n", str);
	snprintf(buf, 80, "|%12.12s|", str);
	puts(buf);
	printf("|%-12.8s|\n", str);
	snprintf(buf, 80, "|%-12.8s|", str);
	puts(buf);

	printf("|%7s|\n", str);
	snprintf(buf, 80, "|%7s|", str);
	puts(buf);
	printf("|%-.*s|\n", 8, str);
	snprintf(buf, 80, "|%-.*s|", 8, str);
	puts(buf);
	
	int d= +12345;
	printf("|%-12.*x|\n", 8, d);
	snprintf(buf, 80, "|%-12.*x|", 8, d);
	puts(buf);
	printf("|%12.*x|\n", 8, d);
	snprintf(buf, 80, "|%12.*x|", 8, d);
	puts(buf);
	printf("|%4x|\n",  d);
	snprintf(buf, 80, "|%4x|", d);
	puts(buf);
	printf("|%3x|\n",  d);
	snprintf(buf, 80, "|%3x|", d);
	puts(buf);
	printf("|%+1x|\n",  d);
	snprintf(buf, 80, "|%+1x|", d);
	puts(buf);
	printf("|%x|\n",  d);
	snprintf(buf, 80, "|%x|", d);
	puts(buf);
	
	d= +210;
	printf("|%-12.*b|\n", 8, d);
	snprintf(buf, 80, "|%-12.*b|", 8, d);
	puts(buf);
	printf("|%12.*b|\n", 8, d);
	snprintf(buf, 80, "|%12.*b|", 8, d);
	puts(buf);
	printf("|%4b|\n",  d);
	snprintf(buf, 80, "|%4b|", d);
	puts(buf);
	printf("|%3b|\n",  d);
	snprintf(buf, 80, "|%3b|", d);
	puts(buf);
	printf("|%+1b|\n",  d);
	snprintf(buf, 80, "|%+1b|", d);
	puts(buf);
	printf("|%b|\n",  d);
	snprintf(buf, 80, "|%b|", d);
	puts(buf);
	
	d= +543210;
	printf("|%-12.*d|\n", 8, d);
	snprintf(buf, 80, "|%-12.*d|", 8, d);
	puts(buf);
	printf("|%12.*d|\n", 8, d);
	snprintf(buf, 80, "|%12.*d|", 8, d);
	puts(buf);
	printf("|%4d|\n",  d);
	snprintf(buf, 80, "|%4d|", d);
	puts(buf);
	printf("|%3d|\n",  d);
	snprintf(buf, 80, "|%3d|", d);
	puts(buf);
	printf("|%+1d|\n",  d);
	snprintf(buf, 80, "|%+1d|", d);
	puts(buf);
	printf("|%d|\n",  d);
	snprintf(buf, 80, "|%d|", d);
	puts(buf);
	
	d= +3210;
	printf("|%-12.*d|\n", 8, d);
	snprintf(buf, 80, "|%-12.*d|", 8, d);
	puts(buf);
	printf("|%12.*d|\n", 8, d);
	snprintf(buf, 80, "|%12.*d|", 8, d);
	puts(buf);
	printf("|%4d|\n",  d);
	snprintf(buf, 80, "|%4d|", d);
	puts(buf);
	printf("|%3d|\n",  d);
	snprintf(buf, 80, "|%3d|", d);
	puts(buf);
	printf("|%+1d|\n",  d);
	snprintf(buf, 80, "|%+1d|", d);
	puts(buf);
	printf("|%d|\n",  d);
	snprintf(buf, 80, "|%d|", d);
	puts(buf);

	d=+1;
	printf("|%+1.0d|\n",  d);
	snprintf(buf, 80, "|%+1.0d|", d);
	puts(buf);
	d=0;
	printf("|%+1.0d|\n",  d);
	snprintf(buf, 80, "|%+1.0d|", d);
	puts(buf);
	d=0;
	printf("|% 1.0d|\n",  d);
	snprintf(buf, 80, "|% 1.0d|", d);
	puts(buf);
	d=0;
	printf("|%1d|\n",  d);
	snprintf(buf, 80, "|%1d|", d);
	puts(buf);
	d=-1;
	printf("|%+1.0d|\n",  d);
	snprintf(buf, 80, "|%+1.0d|", d);
	puts(buf);
	
	return 0;
}
#endif
