#include <stdint.h>
#include <stddef.h>
// #define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(const guchar *)(p)])
char* g_utf8_next_char(char* str) 
{
	uint32_t ch = *(uint8_t*)str;
	return str + ((ch & 0x80)?__builtin_clz(~(ch<<24)) : 1);
}


uint_least32_t g_utf8_get_char (const char* str) 
{
	uint8_t* s = (uint8_t*)str;
	if ((s[0]&0x80)==0){
//			ch++;
	} else {
		if ((s[0]&0xE0)==0xC0 && (s[1]&0xC0)==0x80){
			return (uint_least32_t)(s[0] & 0x1FU)<<6 | (s[1] & 0x3FU);
//			ch++;
		} else
		if ((s[0]&0xF0)==0xE0 && (s[1]&0xC0)==0x80 && (s[2]&0xC0)==0x80){
			return (uint_least32_t)(s[0] & 0x0FU)<<12 | (uint_least32_t)(s[1] & 0x3FU)<<6 | (s[2] & 0x3FU);
//			ch+=2;
		}
	}
	return s[0];
}
/*! \return длина строки в символах */
int32_t g_utf8_strlen (const char* str, size_t n) 
{
	uint8_t* s = (uint8_t*)str;
	int i=0;
	while (s[0]!='\0' && n>0){
		if ((s[0]&0x80)==0) {
			s ++;
		} else {
			uint32_t ch = s[0];
			s += __builtin_clz(~(ch<<24));
		}
		i++, n--;
	}
	return i;
}
// для константы
#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))

/*
Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
   
   Определены 17 страниц по 16 бит.

*/
int32_t g_unichar_to_utf8(uint_least32_t ch, char* str)
{
	int32_t len;// = (ch<0x80)? 1: (35 - __builtin_clz(ch))/5;// 12(+3/5) = 3, 17()=4, 22(+5)=5 27(+5)=6
	if (ch<0x80){
		if (str) 
			str[0] = ch;
		len=1;
	} else {
//		 (ch>=0x00110000) 
		len = (35 - __builtin_clz(ch))/6;
		if (str) {
//			int count = len-1;
			int i;
			for (i = len - 1; i > 0; --i)
			{
				str[i] = (ch & 0x3F) | 0x80;
				ch >>= 6;
			}
			str[0] = (0xFF<<(8-len)) | ch;
		}
	}
	return len;
}
const char* g_utf8_prev_char (const char *s)
{
	while (1){
		if ((*(--s) & 0xc0) != 0x80)
			return (char *)s;
	}
}
char* g_utf8_offset_to_pointer  (const char *str, uint32_t offset)
{
	char *s = (char *)str;

	if (offset > 0) 
    do {
      s = g_utf8_next_char (s);
	} while (--offset);
	return s;
}
#if 0
#include <uchar.h>

typedef struct _mbstate mbstate_t;

struct _mbstate {
	uint32_t ch; // буффер для накопления обрывочных данных
	
};
int mbsinit(const mbstate_t *ps)
{
	if (ps) *ps = 0;
	return ps==NULL;
}

size_t mbrlen(const char *restrict s, size_t n, mbstate_t *restrict ps)
{
//mbstate_t internal;
//mbrtowc(NULL, s, n, ps != NULL ? ps : &internal);
	size_t i=0;
	while (s[0]!='\0' && i<n) {
		s = g_utf8_next_char(s);
		i++;
	}
	if(ps) *ps = 
	return 
}
size_t mbrtoc32(uint_least32_t * restrict pc, const char * restrict s, size_t n, 
	mbstate_t * restrict ps) 
{
	uint_least32_t ch = g_utf8_get_char(s);
	if (pc) *pc = ch;
	if (ps) *ps = g_utf8_next_char(s);
	
}
#endif

