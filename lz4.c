/*
https://github.com/lz4/lz4/blob/dev/doc/lz4_Block_format.md
https://github.com/jibsen/blz4/blob/master/lz4_leparse.h
*/
#include <stdint.h>
#include <stddef.h>
#define LE16(x) (x)
static inline void lz4_memcpy(uint8_t *dst, uint8_t *src, uint32_t len) {
    __builtin_memcpy(dst,src, len);
}
static inline void lz4_memmove(uint8_t *dst, uint8_t *src, uint32_t len) {
    __builtin_memmove(dst,src, len);
}
static uint8_t * lsic_decode(uint8_t* src, uint32_t*t1)
{
    uint32_t n=*t1;
    uint32_t e;
    do{
        e = *src++;
        n+=e;
    } while(e==0xFF);
    *t1 = n;
    return src;
}

uint8_t* lz4_decode(uint8_t* dst, uint8_t* src, size_t slen)
{
    uint8_t *s_end = src+slen;
    while(1) {
        uint32_t token = *src++;
        uint32_t t1 = (token>> 4);// literal length
        if(t1 != 0) {
            if(t1==0xF) src = lsic_decode(src, &t1);
            lz4_memcpy(dst, src, t1);// copy literal
            dst+=t1, src+=t1;
        }
        /* Check for last incomplete sequence */
        if (src==s_end) break;
        uint32_t t2 = (token&0xF);// rle length
        //if(t2 != 0)
        {
            uint32_t offset = LE16(*(uint16_t *)src);
            src+=2;
            if(t2==0xF) src = lsic_decode(src, &t2);
            t2+=4;
            if (offset<t2){
				lz4_memmove(dst, dst-offset, t2);
            } else
                lz4_memcpy(dst, dst-offset, t2);// смешение может быть меньше длины
            dst+=t2;
        }
    }
    return dst;
}