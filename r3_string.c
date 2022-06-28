/*! Строки */
//g_string_new()
#include <stdint.h>
#include <stddef.h>
#include "r3_slice.h"
#include "r3_string.h"

static inline unsigned char bin2char(unsigned char c)
{
	c &= 0x0F;
	if(c < 10) c+='0';
	else c+='A'-10;
	return c;
}

void g_string_append_ (String_t* str, const char* src)
{
    register char* dst = &str->str[str->len];
    while (src[0]!='\0')
    {
        *dst++ = *src++;
    }
    str->len = dst - str->str;
}

void g_string_append_len(String_t* str, const char* src, size_t len)
{
    register char* dst = &str->str[str->len];
    do {
        *dst++ = *src++;
    } while (--len);
    // записать нолик?
    str->len = dst - str->str;
}
void g_string_append_hex(String_t* str, const uint8_t* src, size_t len)
{
    register char* dst = &str->str[str->len];
    do {
        register uint8_t ch = *src++;
        *dst++ = bin2char(ch>>4);
        *dst++ = bin2char(ch);
    } while(--len);

    str->len = dst - str->str;
}
/*! \brief
    функция используется для добавляения строки с запрешенными символами xs:string

    entities "quot", "amp", "apos", "lt", and "gt" (e.g., "&gt;"). используются в разметке XML
    */
void g_string_append_xs(String_t* str, const char* src, size_t len)
{
    register char* dst = &str->str[str->len];
    do {
        register char ch = *src++;
        switch (ch) {
        case '<':
            __builtin_memcpy(dst, "&lt;", 4); dst+=4;
            break;
        case '>':
            __builtin_memcpy(dst, "&gt;", 4); dst+=4;
            break;
        case '&':
            __builtin_memcpy(dst, "&amp;", 5); dst+=5;
            break;
        case '\"':
            __builtin_memcpy(dst, "&quot;", 6); dst+=6;
            break;
        case '\'':
            __builtin_memcpy(dst, "&apos;", 6); dst+=6;
            break;
        default:
            *dst++ = ch;
            break;
        }
    } while (--len);

    str->len = dst - str->str;
}
/*! \brief
    функция используется для добавляения строки с кавычками для JSON
    */
void g_string_append_escaped(String_t* str, const char* src, size_t len)
{
    register char* dst = &str->str[str->len];
    do {
        register char ch = *src++;
#if 0
        switch(ch) {
        case 0x22:// "    quotation mark  U+0022
            *dst++ = '\\'; *dst++ = ch; break;
//        case 0x2F:// /    solidus         U+002F
//        case 0x08:// b    backspace       U+0008
//            *dst++ = '\\'; *dst++ = 'b'; break;
//        case 0x09:// t    tab             U+0009
//            *dst++ = '\\'; *dst++ = 't'; break;
//        case 0x0A:// n    line feed       U+000A
//            *dst++ = '\\'; *dst++ = 'n'; break;
//        case 0x0C:// f    form feed       U+000C
//            *dst++ = '\\'; *dst++ = 'f'; break;
//        case 0 ... 7:
//        case 0x0B:
//            break;
//        case 0x0D:// r    carriage return U+000D
//            *dst++ = '\\'; *dst++ = 'r'; break;
    //              %x75 4HEXDIG )  ; uXXXX                U+XXXX
        case 0x5C:// \    reverse solidus U+005C
            *dst++ = '\\'; *dst++ = ch; break;
        default: break;
        }
#endif // 0
        if (__builtin_expect((ch=='"' || ch=='\\'),0))
            *dst++ = '\\';
        *dst++ = ch;
    } while (--len);

    str->len = dst - str->str;
}
