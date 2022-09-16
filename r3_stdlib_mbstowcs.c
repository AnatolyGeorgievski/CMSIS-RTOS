#include <stdlib.h>

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
int mbtowc(wchar_t * restrict pwc, const char * restrict s, size_t n)
{
	const char* base = s;
	wchar_t wc = (uint8_t)s[0];
	int len = (wc & 0x80)?__builtin_clz(~(wc<<24)) : 0;
	wc = (uint8_t)(*s++) & (0x7F>>len);
	if (len) while (--len>1) {
		wc = (wc<<6)|(*s++ & 0x3F);
	}
	*pwc++ = wc;
	return s-base;
}
size_t mbstowcs(wchar_t * restrict pwcs, const char * restrict s, size_t n){
	return n;
}
int wctomb(char *str, wchar_t ch){
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
/*! \return длина строки в символах */
int mblen(const char *str, size_t n){
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