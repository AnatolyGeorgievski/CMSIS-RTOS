#include <stdint.h>
#include <stddef.h>
/*! \file lzjb2.c
    Copyright (C) 2021, 2022 Anatoly Georgievskii <anatoly.georgievski@gmail.com>
	
	Авторская реализация алгоритма потокового сжатия для файловой системы RFS.
	Идея 'copymask' заимствавана из оригинального алгоритма LZJB. 
	Применяется два формата упаковки данных LZ1 LZ2 с глубиной поиска 6 (64Б) и 11 бит (2кБ)
	Степень сжатия получается выше, чем у алгоритма LZ4.
	Скорость распаковки выше. Размер кода на Cortex-M4 = 188 Байт
 */
 
// параметры форматов упаковки смещения
#define LZ1_DEPTH 11
#define LZ2_DEPTH 6

#define LZ1_LEN_EXT ((1<<(16-LZ1_DEPTH))+2)

// длины последовательностей обычно маленькие, этот вариант вероятно оптимальный
static inline void _memmove_x64(uint8_t* dst, uint8_t* s, size_t mlen)
{
    int i;
    if(0)
    if (dst-s>=8){// перекрытие
        for(i=0; i<(mlen>>3); i++) {
            *(uint64_t*)dst = *(uint64_t*)s;
            dst+=8, s+=8;
        }
        mlen&=7;
    }
    for(i=0; i<(mlen); i++)
        *dst++ = *s++;
}
// длины последовательностей обычно маленькие, этот вариант вероятно оптимальный
static inline void _memcpy_x64(uint8_t* dst, uint8_t* s, size_t mlen)
{
    int i;
    if(1) {
    for(i=0; i<(mlen>>3); i++) {
        *(uint64_t*)dst = *(uint64_t*)s;
        dst+=8, s+=8;
    }
    mlen&=7;
    }
    for(i=0; i<(mlen); i++)
        *dst++ = *s++;
}
uint8_t* lzjb2_decompress(uint8_t *dst, uint8_t *src, size_t s_len)
{
    uint32_t mlen, offset;
    uint32_t nlit=0;
    uint8_t*  s_dst = dst;
    uint8_t*  s_end = src+ s_len;
    int bit_offset=8;
    uint32_t copy_map;
	// Nested Function GCC так разрешает делать.
    int stream_bit_test(){
        if (bit_offset == 8) {
            bit_offset -= 8;
            copy_map = *src++;
        }
        return copy_map & (1U<<(bit_offset++));
    }
    while(src<s_end) {
        if (stream_bit_test()) {// используется кодирование
            if (stream_bit_test()) {// формат кодирования 1 байт + 2бита
                uint32_t  data = *src++;
                offset = (data&((1<<LZ2_DEPTH)-1)) + 1;
                mlen   = (data>>LZ2_DEPTH)+2;// 2-5 байт
            } else {// формат кодирования 2 байта 5-11 + 2 бита
                uint32_t data = *(uint16_t* )src; src+=2;
                offset = (data&((1<<LZ1_DEPTH)-1)) + 1;
                mlen   = (data>>LZ1_DEPTH)+3;
                if (mlen==LZ1_LEN_EXT) {
                    mlen += *src++;
                }
            }
            if (mlen>0) {
                _memmove_x64(dst, dst-offset, mlen);
                dst += mlen;
            } else {
                _memcpy_x64(dst, src, offset);
                src += offset, dst+=offset;
            }
        } else {// literal, кодирование не используется + 1бит
            *dst++=*src++;
        }
    }
    return dst;
}