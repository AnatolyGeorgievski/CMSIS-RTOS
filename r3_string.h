/*! */
#ifndef R3_STRING_H
#define R3_STRING_H
#include <stdint.h>
typedef struct _String String_t;

struct _String {
    char* str;
    uint16_t len;
    uint16_t allocated_len;
};

String_t* g_string_sized_new (size_t allocated_len);
void g_string_append_hex(String_t* str, const uint8_t* src, size_t len);
void g_string_append_len(String_t* str, const char* src, size_t len);
void g_string_append_(String_t* str, const char* src);

static inline void g_string_append(String_t* str, const char* src)
{
    if (__builtin_constant_p(src)) {
        g_string_append_len(str, src, __builtin_strlen(src));
    } else {
        g_string_append_(str, src);
    }
}
static inline
String_t* g_string_init (String_t* str, char* buffer, size_t allocated_len)
{
//    String_t* str = g_slice_alloc(sizeof(String_t));
    str->str = buffer;
    str->len = 0;
    str->allocated_len = allocated_len;
    return str;
}

static inline void g_string_append_printf(String_t* str, ...)
{
}
static inline void g_string_append_c(String_t* str, char ch)
{
    if(__builtin_expect(str->allocated_len <= str->len,0)) {
        // realloc
    }
    str->str[str->len++] = ch;
}

#endif // R3_STRING_H
