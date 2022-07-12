/*! \defgroup _aes_ Алгоритмы блочного шифрования AES-128

	\see http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
	
	
	\{
 */
#include <inttypes.h>
#include <stdio.h>

#if (__ARM_FEATURE_MVE & 3) == 3
#include <arm_mve.h>
/* MVE integer and floating point intrinsics are now available to use. */
#elif __ARM_FEATURE_MVE & 1
#include <arm_mve.h>
/* MVE integer intrinsics are now available to use. */
#elif __ARM_NEON & 1
#include <arm_neon.h> 
#endif
#if __ARM_FEATURE_SIMD32
#include <arm_acle.h>
#endif

// S-Box lookup table,
static uint8_t S_Box[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16,
};
// InvS-Box lookup table,
static uint8_t InvS_Box[256]={
0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d,
};
static uint32_t SubWord(uint32_t V)
{
    return S_Box[V&0xFF] | (S_Box[(V>>8)&0xFF]<<8) | (S_Box[(V>>16)&0xFF]<<16) | (S_Box[(V>>24)&0xFF]<<24);
}
#if 0
// SubBytes (73744765635354655d5b56727b746f5d) = 8f92a04dfbed204d4c39b1402192a84c
static void SubBytes(uint8_t * d)
{
	int count = 16;
	do {
		*d = S_Box[*d];	d++;
	} while (--count);
}
// (15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,4,3,2,1,0) (11,6,1, 12,7,2, 13,8,3, 14,9,4, 15,10,5, 0)
// ShiftRows (7b5b54657374566563746f725d53475d) = 73744765635354655d5b56727b746f5d
static void ShiftRows(uint32_t *d)
{
    const uint32_t r0 = d[0];
    const uint32_t r1 = d[1];
    const uint32_t r2 = d[2];
    const uint32_t r3 = d[3];
    d[0] = (r0 & 0x000000FF) | (r1 & 0x0000FF00) | (r2 & 0x00FF0000) |  (r3 & 0xFF000000) ;
    d[1] = (r1 & 0x000000FF) | (r2 & 0x0000FF00) | (r3 & 0x00FF0000) |  (r0 & 0xFF000000) ;
    d[2] = (r2 & 0x000000FF) | (r3 & 0x0000FF00) | (r0 & 0x00FF0000) |  (r1 & 0xFF000000) ;
    d[3] = (r3 & 0x000000FF) | (r0 & 0x0000FF00) | (r1 & 0x00FF0000) |  (r2 & 0xFF000000) ;
}
// InvSubBytes (5d7456657b536f65735b47726374545d) = 8dcab9bc035006bc8f57161e00cafd8d
static void InvSubBytes(uint8_t * d)
{
	int count = 16;
	do {
		*d = InvS_Box[*d]; d++;
	} while (--count);
}
//  (15,14,13,12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0) (3,6,9,12,15, 2,5,8,11,14, 1,4,7,10,13, 0)
// InvShiftRows (7b5b54657374566563746f725d53475d) = 5d7456657b536f65735b47726374545d
static void InvShiftRows(uint32_t *d)
{
    const uint32_t r0 = d[0];
    const uint32_t r1 = d[1];
    const uint32_t r2 = d[2];
    const uint32_t r3 = d[3];
    d[0] = (r0 & 0x000000FF) | (r3 & 0x0000FF00) | (r2 & 0x00FF0000) |  (r1 & 0xFF000000) ;
    d[1] = (r1 & 0x000000FF) | (r0 & 0x0000FF00) | (r3 & 0x00FF0000) |  (r2 & 0xFF000000) ;
    d[2] = (r2 & 0x000000FF) | (r1 & 0x0000FF00) | (r0 & 0x00FF0000) |  (r3 & 0xFF000000) ;
    d[3] = (r3 & 0x000000FF) | (r2 & 0x0000FF00) | (r1 & 0x00FF0000) |  (r0 & 0xFF000000) ;
}
#endif
/* оптимизация для прямого замеса */
static inline uint32_t rotr(uint32_t x, int n) {return (x<<(32-n)) ^ (x>>n); }
static inline 
uint32_t gmul_vec4(uint32_t V)
{
#if __ARM_FEATURE_SIMD32
	const uint32_t poly  = 0x1B1B1B1BUL;
	uint32_t r = __uadd8(V,V);
	r = __sel(r^poly, r);
//	r ^= __sel(poly, 0);
#else
    const uint32_t lsb  = 0x01010101UL;
    uint32_t r = ((V<<1)&~lsb) ^ (0x1B*((V>>7) & lsb));// *02 03 01 01
#endif
    r = r ^ rotr(V,16);
    return r^rotr(r^V,8);
//    uint32_t b = r^V;
//    return r ^ rotl(b, 24)^ rotl(V, 16) ^ rotl(V, 8);
}
#if 0
uint32x4_t SubBytes_ShiftRows(uint32x4_t s)
{
	uint8x16_t a = (uint8x16_t)s;
	uint8x16_t r;
	r[ 0] = S_Box[a[0]];
	r[ 5] = S_Box[a[1]];
	r[10] = S_Box[a[2]];
	r[15] = S_Box[a[3]];

	r[ 4] = S_Box[a[4]];
	r[ 9] = S_Box[a[5]];
	r[14] = S_Box[a[6]];
	r[ 3] = S_Box[a[7]];

	r[ 8] = S_Box[a[8]];
	r[13] = S_Box[a[9]];
	r[ 2] = S_Box[a[10]];
	r[ 7] = S_Box[a[11]];

	r[12] = S_Box[a[12]];
	r[ 1] = S_Box[a[13]];
	r[ 6] = S_Box[a[14]];
	r[11] = S_Box[a[15]];
	return (uint32x4_t)r;
}
#endif
static inline
void SubBytes_ShiftRows(uint32_t a[4])
{
	register uint32_t r[4];
	r[0] = (uint32_t)S_Box[(uint8_t)(a[0]>>0)]	<<0;
	r[3] = (uint32_t)S_Box[(uint8_t)(a[0]>>8)]	<<8;
	r[2] = (uint32_t)S_Box[(uint8_t)(a[0]>>16)]	<<16;
	r[1] = (uint32_t)S_Box[(uint8_t)(a[0]>>24)]	<<24;

	r[1]^= (uint32_t)S_Box[(uint8_t)(a[1]>>0)]	<<0;
	r[0]^= (uint32_t)S_Box[(uint8_t)(a[1]>>8)]	<<8;
	r[3]^= (uint32_t)S_Box[(uint8_t)(a[1]>>16)]	<<16;
	r[2]^= (uint32_t)S_Box[(uint8_t)(a[1]>>24)]	<<24;

	r[2]^= (uint32_t)S_Box[(uint8_t)(a[2]>>0)]	<<0;
	r[1]^= (uint32_t)S_Box[(uint8_t)(a[2]>>8)]	<<8;
	r[0]^= (uint32_t)S_Box[(uint8_t)(a[2]>>16)]	<<16;
	r[3]^= (uint32_t)S_Box[(uint8_t)(a[2]>>24)]	<<24;

	r[3]^= (uint32_t)S_Box[(uint8_t)(a[3]>>0)]	<<0;
	r[2]^= (uint32_t)S_Box[(uint8_t)(a[3]>>8)]	<<8;
	r[1]^= (uint32_t)S_Box[(uint8_t)(a[3]>>16)]	<<16;
	r[0]^= (uint32_t)S_Box[(uint8_t)(a[3]>>24)]	<<24;
	a[0] = r[0];
	a[1] = r[1];
	a[2] = r[2];
	a[3] = r[3];
}
static inline
void InvSubBytes_ShiftRows(uint32_t a[4])
{
	register uint32_t r[4];
	r[0] = (uint32_t)InvS_Box[(uint8_t)(a[0]>>0)]	<<0;
	r[1] = (uint32_t)InvS_Box[(uint8_t)(a[0]>>8)]	<<8;
	r[2] = (uint32_t)InvS_Box[(uint8_t)(a[0]>>16)]	<<16;
	r[3] = (uint32_t)InvS_Box[(uint8_t)(a[0]>>24)]	<<24;

	r[1]^= (uint32_t)InvS_Box[(uint8_t)(a[1]>>0)]	<<0;
	r[2]^= (uint32_t)InvS_Box[(uint8_t)(a[1]>>8)]	<<8;
	r[3]^= (uint32_t)InvS_Box[(uint8_t)(a[1]>>16)]	<<16;
	r[0]^= (uint32_t)InvS_Box[(uint8_t)(a[1]>>24)]	<<24;

	r[2]^= (uint32_t)InvS_Box[(uint8_t)(a[2]>>0)]	<<0;
	r[3]^= (uint32_t)InvS_Box[(uint8_t)(a[2]>>8)]	<<8;
	r[0]^= (uint32_t)InvS_Box[(uint8_t)(a[2]>>16)]	<<16;
	r[1]^= (uint32_t)InvS_Box[(uint8_t)(a[2]>>24)]	<<24;

	r[3]^= (uint32_t)InvS_Box[(uint8_t)(a[3]>>0)]	<<0;
	r[0]^= (uint32_t)InvS_Box[(uint8_t)(a[3]>>8)]	<<8;
	r[1]^= (uint32_t)InvS_Box[(uint8_t)(a[3]>>16)]	<<16;
	r[2]^= (uint32_t)InvS_Box[(uint8_t)(a[3]>>24)]	<<24;
	a[0] = r[0];
	a[1] = r[1];
	a[2] = r[2];
	a[3] = r[3];
}
static 
uint32_t gmul_ivec4(uint32_t V)
{
#if __ARM_FEATURE_SIMD32
	const uint32_t poly  = 0x1B1B1B1BUL;
	uint32_t r1 = __uadd8(V,V);
	r1 = __sel(r1^poly, r1);
	uint32_t r2 = __uadd8(r1,r1);
	r2 = __sel(r2^poly, r2);
	uint32_t r3 = __uadd8(r2,r2);
	r3 = __sel(r3^poly, r3);
#else
    const uint32_t lsb  = 0x01010101UL;
    uint32_t r1 = (( V<<1)&~lsb) ^ (0x1B*(( V>>7) & lsb));
    uint32_t r2 = ((r1<<1)&~lsb) ^ (0x1B*((r1>>7) & lsb));
    uint32_t r3 = ((r2<<1)&~lsb) ^ (0x1B*((r2>>7) & lsb));
#endif
    uint32_t a = r3^r2^r1  ;// *0e 0b 0d 09
    uint32_t b = r3   ^r1^V;
    uint32_t c = r3^r2   ^V;
    uint32_t d = r3      ^V;
    return  a ^ rotr(b,8) ^ rotr(c,16) ^ rotr(d,24);
}


static void MixColumns(uint32_t V[], int Nb)
{
	int i;
	for (i=0; i<Nb; i++)
		V[i] = gmul_vec4(V[i]);
};
static void InvMixColumns(uint32_t V[], int Nb)
{
	int i;
	for (i=0; i<Nb; i++)
		V[i] = gmul_ivec4(V[i]);
};
static void AddRoundKey(uint32_t *state, uint32_t *key)
{
    int i;
    for (i=0; i<4; i++) state[i] ^= key[i];
}


/*! \brief AES-128 разгибание ключа шифрования */
void KeyExpansion(uint32_t *key, uint32_t *w, int Nk_)
{
    const int Nk = 4, Nbr = 4*11;
    uint32_t rcon = 1;
    /* [] = { 0x00,
        0x01,0x02,0x04,0x08,
        0x10,0x20,0x40,0x80,
        0x1B,0x36};*/
    int i;
    for (i=0;i<Nk; i++) w[i] = *key++;
    for (i = Nk;i < Nbr/*Nb*(Nr+1)*/; i++)
    {
        uint32_t temp = w[i-1];
        if ((i&3)==0)//Nk)
        {
            temp = SubWord((temp>>8) | (temp<<24)) ^ rcon;//[i>>2];
            rcon <<=1;
            if (rcon & 0x100) rcon ^= 0x11b;
        }
        w[i] = w[i-Nk] ^ temp;
    }
}
/*! \brief AES-128 обратное разгибание ключа шифрования */
void InvKeyExpansion(uint32_t w[][4], int Nk_)
{
    int i;
    for (i=1; i<10; i++)
        InvMixColumns(w[i],4);
}

void AES_encrypt(uint32_t* state, uint32_t key[][4])
{
    int round=0;
__asm volatile("# LLVM-MCA-BEGIN AES_encrypt");
    do {
		AddRoundKey(state, key[round]); round++;
//        SubBytes((void*)state);
//        ShiftRows(state);
		SubBytes_ShiftRows(state);
        if (round==10) break;
		MixColumns(state,4);
    } while (1);
__asm volatile("# LLVM-MCA-END AES_encrypt");
	AddRoundKey(state, key[round]);
}
void AES_decrypt(uint32_t* state, uint32_t key[][4])
{
    int round=10;
    goto into;
    do{
//        InvSubBytes((void*)state);
//        InvShiftRows(state);
		InvSubBytes_ShiftRows(state);
        if (round!=0)InvMixColumns(state,4);
    into:
        AddRoundKey(state, key[round]);
    } while (round--);
}
	//! \}
#ifdef TEST_SBOX
#define ROTL8(x,n) (((x)<<(n)) ^((x)>>(8-(n))))
// https://en.wikipedia.org/wiki/Rijndael_S-box
/*
теория генерации: берем генератор g=3 и возводим в степень 
B = M*A + C 
M= 
11111000 // rotl(0,1,2,3,4)
01111100
00111110
00011111
10001111
11000111
11100011
11110001
Аффинные преобразования опредедены как (Poly)^3 mod x8+1
The affine transformation is defined as a degree 7 polynomial multiplication modulo x8+1.

Левая колонка или нижняя строка (x7 x6 x5 x4 1)^3 mod x8+1 = x7 x5 x2

Или методом подбора (x7 x6 x5 x4 1) (x7 x5 x2) mod x8+1 == 1
000001111000100
001111000100000
111100010000000
-----
110010101100100 mod x8 1
000000001100101
----
       00000001
Как расчитать матрицу обратного аффинного преобразования?

C= 0x63
A = M^{-1}*(B-C) = M^{-1}*B + D
D= 00000101 = 0x05
M-= 
01010010 // rotl(1,3,6)
00101001
10010100
01001010
00100101
10010010
01001001
10100100
Таблица нередуцируемых полиномов =30 шт
x^8 + x^7 + x^5 + x^4 + 1
x^8 + x^6 + x^5 + x^4 + 1
x^8 + x^7 + x^5 + x^3 + 1
x^8 + x^6 + x^5 + x^3 + 1
x^8 + x^5 + x^4 + x^3 + 1
x^8 + x^7 + x^6 + x^5 + x^4 + x^3 + 1
x^8 + x^6 + x^5 + x^2 + 1
x^8 + x^7 + x^6 + x^5 + x^4 + x^2 + 1
x^8 + x^7 + x^3 + x^2 + 1
x^8 + x^6 + x^3 + x^2 + 1
x^8 + x^5 + x^3 + x^2 + 1
x^8 + x^4 + x^3 + x^2 + 1
x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1
x^8 + x^7 + x^5 + x^4 + x^3 + x^2 + 1
x^8 + x^7 + x^6 + x + 1
x^8 + x^7 + x^5 + x + 1
x^8 + x^6 + x^5 + x + 1
x^8 + x^7 + x^6 + x^5 + x^4 + x + 1
x^8 + x^7 + x^3 + x + 1
x^8 + x^5 + x^3 + x + 1
x^8 + x^4 + x^3 + x + 1 - AES
x^8 + x^6 + x^5 + x^4 + x^3 + x + 1
x^8 + x^7 + x^2 + x + 1
x^8 + x^7 + x^6 + x^5 + x^2 + x + 1
x^8 + x^7 + x^6 + x^4 + x^2 + x + 1
x^8 + x^6 + x^5 + x^4 + x^2 + x + 1
x^8 + x^7 + x^6 + x^3 + x^2 + x + 1
x^8 + x^7 + x^4 + x^3 + x^2 + x + 1
x^8 + x^6 + x^4 + x^3 + x^2 + x + 1
x^8 + x^5 + x^4 + x^3 + x^2 + x + 1

Рекомендуемые S-box на полиномах: 
// 11D 125 12D 14D 15F 163 165 169
// 18D 1A9 187 1C3 1E7 1CF 1F5 1E1
165 - BelT
11B - AES x8 + x4 + x3 + x + 1
1C3 - это Кузнечик и Stribog.

Есть способ преобразовния 
We combine various linear operations into two affine transforms (one on each side), A1 and A2. Here affine transform consists of a multiplication with a 8x8 binary matrix M and addition of a 8-bit constant C.

SM4-S(x) = A2(AES-S(A1(x))
A1(x) = M1*x + C1
A2(x) = M2*x + C2

Аффинные преобразования могут быть разложены на две подстановки, от старшей и младшей части. 
https://github.com/mjosaarinen/sm4ni/blob/master/sm4ni.c


*/
uint32_t pmul(uint32_t p, uint32_t i){
	uint32_t r=0;
	while (i) {
		if (i&1)r^=p;
		p<<=1;
		i>>=1;
	}
	return r;
}
uint32_t gf2p8_mul(uint32_t p, uint32_t i, uint32_t Poly){
	uint32_t r=0;
	while (i) {
		if (i&1)r^=p;
		if (p&0x80){
			p=(p<<1) ^ Poly;
		} else
			p=(p<<1);
		
		i>>=1;
	}
	return r;
}
// вычисление методом подбора обратного полинома для AF
// нахождение обратного числа в поле x8+1 методом перебора
uint8_t inverse_af(uint8_t poly, uint32_t P)
{
	uint32_t p = poly & 0xFF;
	uint32_t r;
	uint32_t i;
	for (i=1; i<256; i++) {
		r = gf2p8_mul(p, i, P);
		if (r==1) break;
	}
	printf ("poly=0x%02X inv=0x%02X\n", poly, i);
	return i;
}
void inverse_af2(uint32_t poly)
{
	const int bits = 8;
	uint32_t mask = ~0UL>>(32-bits);
	uint32_t p = poly & mask;
	uint32_t r;
	r = pmul(p, p);
	r = (r ^ (r>>bits))&mask;
	r = pmul(r, p);
	r = (r ^ (r>>bits))&mask;
	printf ("poly=0x%02X inv=0x%02X\n", poly, r);
}
static 
void initialize_aes_sbox(uint8_t sbox[256], uint8_t inv_sbox[256]) {
	uint8_t p = 1, q = 1;
	
	/* loop invariant: p * q == 1 in the Galois field */
	do {
		/* multiply p by 3 */
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

		/* divide q by 3 (equals multiplication by 0xf6) */
		// q = (q>>1) ^ (q & 0x01 ? 0x8D : 0);
		q ^= q << 1;
		q ^= q << 2;
		q ^= q << 4;
		q ^= q & 0x80 ? 0x09 : 0; 

/* compute the affine transformation 
Сдвиги можно представить как умножение без переносов (q(*)31) mod P, P=x^8+1
 */		
uint8_t AF(uint8_t q) {
	uint8_t x = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);//0xF1>>>1 - отражение и сдвиг
	x ^= 0x63;
	return x;
}
/* compute the inverse affine transformation - обратное преобразование M^{-1} */
uint8_t BF(uint8_t x) {
	uint8_t p = ROTL8(x, 1) ^ ROTL8(x, 3) ^ ROTL8(x, 6);// 0xA4>>>1  - отражение
	p ^= 0x05;
	return p;
}
//		printf("x,y = %02X %02X\n", q,y);
		sbox[p] = AF(q);
		inv_sbox[AF(q)] = p;
//		inv_sbox[(q)] = p;
	} while (p != 1);

	/* 0 is a special case since it has no inverse */

//	sbox[0] = 0;
	sbox[0] = 0x63;
	inv_sbox[0x63] = 0;
	//inv_sbox[0] = 0;
}
void initialize_inv(uint8_t sbox[256], uint8_t g, uint32_t Poly)
{
	uint8_t p = 1, q = 1;
	uint8_t Inv3 = inverse_af(g, Poly);
	do {
		p = gf2p8_mul(p, g, Poly);
		q = gf2p8_mul(q, Inv3, Poly);
		sbox[p] = q;
	} while(p != 1);
	sbox[0] = 0;
}
int main()
{
	uint8_t sbox[256];
	uint8_t inv_sbox[256];
	initialize_aes_sbox(sbox, inv_sbox);
	int i;
	printf ("S-Box:");
	for (i=0; i<256; i++ ) {
		if ((i&0xF)==0) printf("\n");
		printf(" %02X", sbox[i]);
	}
	printf ("\n");
	printf ("Inv S-Box:");
	for (i=0; i<256; i++ ) {
		if ((i&0xF)==0) printf("\n");
		printf(" %02X", inv_sbox[i]);
	}
	for (i=0; i<256; i++ ) sbox[i] = 0;
	printf ("\n");
	initialize_inv(sbox, 2, 0x11D);
	printf ("Inv(x):");
	for (i=0; i<256; i++ ) {
		if ((i&0xF)==0) printf("\n");
		printf(" %02X", sbox[i]);
	}
	printf ("\n");
	
	
	inverse_af (0xF1,0x11B);
	inverse_af (0xF1,0x101);
	inverse_af2(0xF1);

	inverse_af (0xA4, 0x101);
	inverse_af2(0xA4);
	inverse_af (0xF6, 0x11B);
	inverse_af (0x03, 0x11B);
	inverse_af (0x03, 0x165);// BelT
	inverse_af (0xDC, 0x165);// BelT
	inverse_af2(0xF6);
}

/* 0 1 8D CB E8 74 3A 1D 83
poly=0x03 inv=0xF6 Poly=0x11B AES
Inv(x):
 00 01 8D F6 CB 52 7B D1 E8 4F 29 C0 B0 E1 E5 C7
 74 B4 AA 4B 99 2B 60 5F 58 3F FD CC FF 40 EE B2
 3A 6E 5A F1 55 4D A8 C9 C1 0A 98 15 30 44 A2 C2
 2C 45 92 6C F3 39 66 42 F2 35 20 6F 77 BB 59 19
 1D FE 37 67 2D 31 F5 69 A7 64 AB 13 54 25 E9 09
 ED 5C 05 CA 4C 24 87 BF 18 3E 22 F0 51 EC 61 17
 16 5E AF D3 49 A6 36 43 F4 47 91 DF 33 93 21 3B
 79 B7 97 85 10 B5 BA 3C B6 70 D0 06 A1 FA 81 82
 83 7E 7F 80 96 73 BE 56 9B 9E 95 D9 F7 02 B9 A4
 DE 6A 32 6D D8 8A 84 72 2A 14 9F 88 F9 DC 89 9A
 FB 7C 2E C3 8F B8 65 48 26 C8 12 4A CE E7 D2 62
 0C E0 1F EF 11 75 78 71 A5 8E 76 3D BD BC 86 57
 0B 28 2F A3 DA D4 E4 0F A9 27 53 04 1B FC AC E6
 7A 07 AE 63 C5 DB E2 EA 94 8B C4 D5 9D F8 90 6B
 B1 0D D6 EB C6 0E CF AD 08 4E D7 E3 5D 50 1E B3
 5B 23 38 34 68 46 03 8C DD 9C 7D A0 CD 1A 41 1C

poly=0x03 inv=0xAC Poly=0x1F5 SM4
Inv(x):
 00 01 FA AC 7D 64 56 95 C4 73 32 E0 2B 76 B0 8F
 62 D6 C3 F4 19 EE 70 81 EF 14 3B 82 58 F3 BD 61
 31 8C 6B 84 9B A9 7A 51 F6 F0 77 0C 38 7E BA 75
 8D 20 0A E1 E7 FF 41 E3 2C 7F 83 1A A4 4C CA 5A
 E2 36 46 96 CF CC 42 97 B7 9E AE 86 3D A5 D2 92
 7B 27 78 B3 C1 5F 06 94 1C F2 3F CB 5D 5C C0 55
 BC 1F 10 D7 05 7C 8A 6F 89 9C 85 22 DA D8 8B 67
 16 80 C5 09 BB 2F 0D 2A 52 B2 26 50 65 04 2D 39
 71 17 1B 3A 23 6A 4B AF 9D 68 66 6E 21 30 B1 0F
 A1 C6 4F D3 57 07 43 47 E4 B5 A8 24 69 88 49 B6
 C7 90 E9 A6 3C 4D A3 E8 9A 25 D5 B9 03 FB 4A 87
 0E 8E 79 53 E5 99 9F 48 D4 AB 2E 74 60 1E D0 DC
 5E 54 F5 12 08 72 91 A0 F8 DE 3E 5B 45 CE CD 44
 BE DD 4E 93 B8 AA 11 63 6D DB 6C D9 BF D1 C9 F9
 0B 33 40 37 98 B4 FE 34 A7 A2 ED FD FC EA 15 18
 29 F7 59 1D 13 C2 28 F1 C8 DF 02 AD EC EB E6 35


poly=0x02 inv=0x8E Poly=0x11D Stribog TKlog https://eprint.iacr.org/2019/092.pdf
Inv(x):
 00 01 8E F4 47 A7 7A BA AD 9D DD 98 3D AA 5D 96
 D8 72 C0 58 E0 3E 4C 66 90 DE 55 80 A0 83 4B 2A
 6C ED 39 51 60 56 2C 8A 70 D0 1F 4A 26 8B 33 6E
 48 89 6F 2E A4 C3 40 5E 50 22 CF A9 AB 0C 15 E1
 36 5F F8 D5 92 4E A6 04 30 88 2B 1E 16 67 45 93
 38 23 68 8C 81 1A 25 61 13 C1 CB 63 97 0E 37 41
 24 57 CA 5B B9 C4 17 4D 52 8D EF B3 20 EC 2F 32
 28 D1 11 D9 E9 FB DA 79 DB 77 06 BB 84 CD FE FC
 1B 54 A1 1D 7C CC E4 B0 49 31 27 2D 53 69 02 F5
 18 DF 44 4F 9B BC 0F 5C 0B DC BD 94 AC 09 C7 A2
 1C 82 9F C6 34 C2 46 05 CE 3B 0D 3C 9C 08 BE B7
 87 E5 EE 6B EB F2 BF AF C5 64 07 7B 95 9A AE B6
 12 59 A5 35 65 B8 A3 9E D2 F7 62 5A 85 7D A8 3A
 29 71 C8 F6 F9 43 D7 D6 10 73 76 78 99 0A 19 91
 14 3F E6 F0 86 B1 E2 F1 FA 74 F3 B4 6D 21 B2 6A
 E3 E7 B5 EA 03 8F D3 C9 42 D4 E8 75 7F FF 7E FD

poly=0x02 inv=0xB8 Poly=0x171 Stribog MDS KK13 https://eprint.iacr.org/2013/556.pdf
Inv(x):
 00 01 B8 D0 5C 9F 68 86 2E AD F7 8B 34 30 43 75
 17 FC EE 53 C3 CC FD 10 1A B5 18 B4 99 79 82 49
 B3 E8 7E 8D 77 DC 91 F8 D9 AF 66 BC C6 6C 08 AC
 0D 35 E2 54 0C 31 5A 94 F4 DF 84 CB 41 E0 9C 44
 E1 3C 74 0E 3F 9D FE 8F 83 1F 6E 9A F0 96 7C BA
 D4 C4 EF 13 33 E3 5E C1 63 A3 36 95 04 9E 56 C0
 BE EB A2 58 71 7B 2A BD 06 87 A0 A9 2D C7 4A 9B
 7A 64 D7 ED 42 0F DD 24 98 1D 70 65 4E BB 22 8C
 C8 AB 1E 48 3A CA 07 69 A7 B0 F6 0B 7F 23 FF 47
 F9 26 B7 F3 37 5B 4D F1 78 1C 4B 6F 3E 45 5D 05
 6A A8 62 59 CF E7 B1 88 A1 6B C9 81 2F 09 D8 29
 89 A6 E9 20 1B 19 F2 92 02 D1 4F 7D 2B 67 60 EA
 5F 57 CD 14 51 D5 2C 6D 80 AA 85 3B 15 C2 E6 A4
 03 B9 FB E4 50 C5 EC 72 AE 28 DB DA 25 76 F5 39
 3D 40 32 55 D3 FA CE A5 21 B2 BF 61 D6 73 12 52
 4C 97 B6 93 38 DE 8A 0A 27 90 E5 D2 11 16 46 8E

poly=0x03 inv=0xBE Poly=0x1C3 Stribog Kuzneychik
Inv(x):
 00 01 E1 BE 91 6A 5F 81 A9 7F 35 65 CE 53 A1 26
 B5 A3 DE 1E FB 76 D3 31 67 D0 C8 94 B1 23 13 DF
 BB F4 B0 1D 6F 6C 0F A0 9C 5D 3B F1 88 CD F9 C3
 D2 17 68 61 64 0A 4A DA B9 AE F0 2A E8 84 8E 93
 BC 71 7A 8B 58 F6 EF 7C D6 DD 36 DB E6 A7 50 AA
 4E AB CF 0D FC 9E 99 72 44 F7 87 A5 9D 29 80 06
 69 33 EA ED 34 0B D1 18 32 60 05 90 25 6E 6D 24
 BD 41 57 98 78 C4 15 FA 74 C5 42 8A 47 EE A8 09
 5E 07 D9 F2 3D E9 A4 5A 2C CC 7B 43 96 C7 3E 92
 6B 04 8F 3F 1B C9 8C C6 73 56 B2 B7 28 5C 55 FD
 27 0E B4 11 86 5B E7 4D 7E 08 4F 51 AD AC 39 B8
 22 1C 9A B6 A2 10 B3 9B AF 38 F5 20 40 70 03 E0
 D5 E2 F8 2F 75 79 97 8D 1A 95 E4 FF 89 2D 0C 52
 19 66 30 16 E3 C0 48 DC F3 82 37 4B D7 49 12 1F
 BF 02 C1 D4 CA FE 4C A6 3C 85 62 EC EB 63 7D 46
 3A 2B 83 D8 21 BA 45 59 C2 2E 77 14 54 9F E5 CB

poly=0x03 inv=0xDC Poly=0x165 BelT
Inv(x):
 00 01 B2 DC 59 97 6E F5 9E 53 F9 41 37 AB C8 AE
 4F B9 9B FB CE 27 92 BA A9 70 E7 31 64 3F 57 CA
 95 46 EE 42 FF 79 CF 15 67 5B A1 F3 49 CC 5D 9D
 E6 1B 38 B4 C1 8A AA 0C 32 B5 AD D6 99 D9 65 1D
 F8 0B 23 EF 77 6A 21 94 CD 2C 8E B0 D5 91 B8 10
 81 7D 9F 09 E2 6C CB 1E 96 04 66 29 9C 2E FC A5
 73 E9 BF E1 1C 3E 5A 28 D2 D0 45 76 55 E3 06 F4
 19 A8 E8 60 E4 F6 6B 44 FE 25 DE EA 80 51 BC F0
 7C 50 B7 A7 A3 8D C5 EC 89 88 35 C0 A2 85 4A B1
 D4 4D 16 BB 47 20 58 05 D8 3C FA 12 5C 2F 08 52
 F2 2A 8C 84 FD 5F B6 83 71 18 36 0D D7 3A 0F C9
 4B 8F 02 DD 33 39 A6 82 4E 11 17 93 7E F1 E0 62
 8B 34 C6 DA ED 86 C2 DB 0E AF 1F 56 2D 48 14 26
 69 D3 68 D1 90 4C 3B AC 98 3D C3 C7 03 B3 7A EB
 BE 63 54 6D 74 F7 30 1A 72 61 7B DF 87 C4 22 43
 7F BD A0 2B 6F 07 75 E5 40 0A 9A 13 5E A4 78 24


*/

#endif
#ifdef TEST_AES
// Тестирование алгоритма
int main()
{
    int i;
#if 0
// ShiftRows (7b5b54657374566563746f725d53475d) = 73744765 63535465 5d5b5672 7b746f5d
{
    uint32_t V[4] = {0x5d53475d, 0x63746f72, 0x73745665, 0x7b5b5465};
    ShiftRows(V);
    printf("ShiftRows  %08X %08X %08X %08X \n", V[3],V[2],V[1],V[0]);
}

// MixColumns  627A6F66 44B109C8 2B18330A 81C3B3E5 -> 7B5B5465 73745665 63746F72 5D53475D,
    uint32_t V[4] = {0x81c3b3e5,0x2b18330a, 0x44b109c8,0x627a6f66};
    printf("MixColumns  %08X %08X %08X %08X -> ", V[3],V[2],V[1],V[0]);
    MixColumns (V,4);
    printf("%08X %08X %08X %08X, \n", V[3],V[2],V[1],V[0]);

//InvMixColumns  8DCAB9DC 035006BC 8F57161E 00CAFD8D -> D635A667 928B5EAE EEC9CC3B C55F5777,
    uint32_t H[4] = {0x00cafd8d,0x8f57161e, 0x035006bc,0x8dcab9dc};
    printf("InvMixColumns  %08X %08X %08X %08X -> ", H[3],H[2],H[1],H[0]);
    InvMixColumns (H,4);
    printf("%08X %08X %08X %08X, \n", H[3],H[2],H[1],H[0]);
// SubWord(73744765) - 8f92a04d
    uint32_t W = 0x73744765;
    printf("SubWord %08X -> %08X, \n", W, SubWord(W));

    uint8_t sw[16] = {0x73,0x74,0x47,0x65,0x63,0x53,0x54,0x65,0x5d,0x5b,0x56,0x72,0x7b,0x74,0x6f,0x5d};
    printf("SubBytes (");
    for(i=0;i<16;i++) printf("%02X", sw[i]);
    printf(") = ");
    SubBytes(sw);
    for(i=0;i<16;i++) printf("%02X", sw[i]);
    printf("\n");

    uint8_t is[16] = {0x5d,0x74,0x56,0x65,0x7b,0x53,0x6f,0x65,0x73,0x5b,0x47,0x72,0x63,0x74,0x54,0x5d};
    printf("InvSubBytes(");
    for(i=0;i<16;i++) printf("%02X", is[i]);
    printf(") = ");
    InvSubBytes(is);
    for(i=0;i<16;i++) printf("%02X", is[i]);
    printf("\n");
#endif
{
    uint32_t key[] = {0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c};
    uint32_t w[11][4];
    KeyExpansion(key, (uint32_t *)w, 4);
    for (i=0; i < 11; i++)
        printf("%08X %08X %08X %08X\n", w[i][3], w[i][2], w[i][1], w[i][0]);
    uint32_t d[4] = {0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc};
    printf("AES-INPUT:\n");
    printf("%08X %08X %08X %08X\n", d[3], d[2], d[1], d[0]);
    AES_encrypt(d, (void*)w);
    printf("AES-ENC-128:\n");
    printf("%08X %08X %08X %08X\n", d[3], d[2], d[1], d[0]);

    InvKeyExpansion(w, 4);
    AES_decrypt(d, (void*)w);
    printf("AES-DEC-128:\n");
    printf("%08X %08X %08X %08X\n", d[3], d[2], d[1], d[0]);
}

if(0){
    uint32_t key[] = {0, 0, 0, 0};
    uint32_t w[11][4];
    KeyExpansion(key, &w[0][0], 4);
    for (i=0; i < 11; i++)
        printf("%08X %08X %08X %08X\n", w[i][3], w[i][2], w[i][1], w[i][0]);
    uint32_t d[4] = {0, 0, 0, 0};
    AES_encrypt(d, w);
    printf("AES 128:\n");
    printf("%08X %08X %08X %08X\n", d[3], d[2], d[1], d[0]);
}

    return 0;
}
#endif
