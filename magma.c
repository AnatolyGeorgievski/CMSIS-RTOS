/*! Национальый алгоритм блочного шифрования с длиной блока 64 бит. TC26 магма
	\see ГОСТ Р 34.12-2015 Информационная технология. КРИПТОГРАФИЧЕСКАЯ ЗАЩИТА ИНФОРМАЦИИ. Блочные шифры
	\see http://tc26.ru

 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
/* 5.1.1 Нелинейное биективное преобразование */

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif
static const 
uint8_t sbox[8][16] = {// см. Gost28147_TC26_paramZ
/* π0' = */ {12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1},
/* π1' = */ {6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15},
/* π2' = */ {11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0},
/* π3' = */ {12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11},
/* π4' = */ {7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12},
/* π5' = */ {5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0},
/* π6' = */ {8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 0, 13, 10, 3, 7},
/* π7' = */ {1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2}
};
#if 0
static uint32_t sbox32[16] = {
    0x1857CB6C, 0x7EDF8384, 0xE2F52526, 0xD56A1832, 0x0698D29A, 0x59214FA5, 0x81C6FA5B, 0x3CAD6DC9,
    0x4FB07E1E, 0xF47901E8, 0xAB83A74D, 0x601E5477, 0x9D4B3CB0, 0xCA34E9D3, 0xB3E2960F, 0x270CB0F1};
#endif
#define ROL(x, n) (((x)<<(n))|((x)>>(32-n)))
// Bit Field Extract
#define BEXTR(x, n, len) (((x) >> (n)) & ((1 << (len))-1))
// Bit Field Insert
//#define BFI(x, n, len) x = (((x) >> (n)) & ((1 << (len))-1))
#define BFI(x, y, n, len) x = ((x) & ~(((1 << (len))-1)<<(n))) | ((y & ((1 << (len))-1))<<(n))

//static uint32_t t(uint32_t a) __attribute__((optimize("O3")));
#pragma GCC optimize("O3")

static inline uint64_t __attribute__((always_inline)) htonll(uint64_t a)
{
    return __builtin_bswap64(a);// LE
}
static inline uint32_t ntohl (uint32_t a)
{
    return __builtin_bswap32(a);// LE
}
static inline uint16_t htons (uint16_t a)
{
    return __builtin_bswap16(a);// LE
}

static inline uint32_t bfi(uint32_t a, uint32_t b)
{
	a = (a & ~(0xF<<4)) ^ ((b<<4) & (0xF<<4));
	return a;
}
typedef uint8_t v4qi __attribute__((__vector_size__(4)));
static inline 
uint32_t t2(uint32_t a) 
{
	v4qi r0,r1;
	int i;
	#pragma GCC unroll 4
	for (i=0; i<4; i++){
		r0[i] = sbox[2*i+0][BEXTR(a,(i*8+0),4)];
		r1[i] = sbox[2*i+1][BEXTR(a,(i*8+4),4)];
//		BFI (r,s,(i*4),4);
//		r |= (s & 0xF) <<(i*4);
	}
	return *(uint32_t*)&r0 | (*(uint32_t*)&r1)<<4;
}
static inline uint32_t t4(uint32_t a)
{
    register union {
        struct {
            uint32_t u0:4;
            uint32_t u1:4;
            uint32_t u2:4;
            uint32_t u3:4;
            uint32_t u4:4;
            uint32_t u5:4;
            uint32_t u6:4;
            uint32_t u7:4;
        };
        uint32_t x;
    } r;
	r.x  = a;
    r.u0 = sbox[0][r.u0];
    r.u1 = sbox[1][r.u1];
    r.u2 = sbox[2][r.u2];
    r.u3 = sbox[3][r.u3];
    r.u4 = sbox[4][r.u4];
    r.u5 = sbox[5][r.u5];
    r.u6 = sbox[6][r.u6];
    r.u7 = sbox[7][r.u7];
	return r.x;
}
static inline uint32_t t3(uint32_t a)
{
    register union {
        struct {
            uint32_t u0:4;
            uint32_t u1:4;
            uint32_t u2:4;
            uint32_t u3:4;
            uint32_t u4:4;
            uint32_t u5:4;
            uint32_t u6:4;
            uint32_t u7:4;
        };
		uint8_t b[4];
        uint32_t x;
    } r0, r1, r;
	r.x  = a;
    r0.b[0] = sbox[0][r.u0];
    r1.b[0] = sbox[1][r.u1];
    r0.b[1] = sbox[2][r.u2];
    r1.b[1] = sbox[3][r.u3];
    r0.b[2] = sbox[4][r.u4];
    r1.b[2] = sbox[5][r.u5];
    r0.b[3] = sbox[6][r.u6];
    r1.b[3] = sbox[7][r.u7];
	return (r0.x & (0x0F0F0F0FUL)) | (r1.x & ~(0x0F0F0F0FUL));
}
#if 0
static inline uint32_t t0(uint32_t a)
{
	uint32_t r=0;
	int i;
	for (i=0; i<8; i++){
		register unsigned s = sbox32[BEXTR(a,(i*4),4)];
		r ^= s & (0xF<<(i*4));
	}
	return r;
}
#endif
static inline uint32_t t(uint32_t a)
{
	uint32_t r=0;
	int i;
	for (i=0; i<8; i++){
		register uint32_t s = sbox[i][BEXTR(a,(i*4),4)];
		r ^= s<<(i*4);
	}
	return r;
}
static uint32_t g(uint32_t k, uint32_t a)
{
	return ROL(t(k+a), 11);
}
#pragma GCC optimize("O3")
#if 0
static uint64_t magma_encrypt1(const uint32_t *K, uint64_t v){
	uint32_t n1 = v, n2 = v>>32;
	int i=0;
	int count = 12;
	do{
		i &= 0x7;
		n2 ^= g(K[i++], n1); 
		n1 ^= g(K[i++], n2);
	} while (--count);
	do{
		n2 ^= g(K[--i], n1); 
		n1 ^= g(K[--i], n2);
	} while(i);
	return (uint64_t)n1<<32 | n2;
}
#endif
	
static uint64_t magma_encrypt(const uint32_t *Key, uint64_t v){
	uint32_t n1 = v, n2 = v>>32;
	const uint32_t *K=Key;
	int count = 12;
	do{
		if (K==Key) K = Key+8;
		n2 ^= g(*--K, n1);
		n1 ^= g(*--K, n2);
	} while (--count);
	count = 4;
	do{
//        uint32_t n = n2 ^ g(*K++, n1);
//        n2 = n1;
//        n1 = n;
		n2 ^= g(*K++, n1);
		n1 ^= g(*K++, n2);
	} while(--count);
	return (uint64_t)n1<<32 | n2;
}
#pragma GCC optimize("Os")
/*! \brief Режим выработки имитовставки
    \param K ключ шифрования длиной...
    \param iv начальное значение или вектор инициализации
    \param data сегмент данных от которого вычисляется имитовставка
*/
typedef struct _CCM CCM_t;
struct _CCM {
    uint64_t last_block;
    uint64_t sum;
    const uint32_t *K;
    uint32_t len;
};
static void magma_cmac_init(CCM_t *ctx, const uint32_t * Key)
{
    ctx->K = Key;
    ctx->last_block = 0;
    ctx->sum = 0;
    ctx->len = 0;
}
/*! \brief */
static void magma_cmac_update(CCM_t *ctx, uint8_t* data, size_t len)
{
    const int s=8;
    if ((ctx->len % s)!=0) {// не полный блок.
        int slen = s - ctx->len; // длину берем из данных
        if (slen > len) slen = len;
        __builtin_memcpy(((uint8_t*)&ctx->last_block) + ctx->len, data, slen);
        data+=slen;
        len -=slen;
        ctx->len += slen;
    }
    if (len) {
        uint64_t m = ctx->sum;
        if (ctx->len == s) {// полный блок и
            m^= htonll(ctx->last_block);
            m = magma_encrypt(ctx->K, m);
            ctx->last_block = 0;
        }
        int blocks = (len-1)/s;// число целых блоков
        int i;
        for (i=0; i<blocks; i++){
            //printf("P = %016"PRIx64"\n", *(uint64_t*)data);
            m^= htonll(*(uint64_t*)data); data+=s;
            m = magma_encrypt(ctx->K, m);
        }
        ctx->sum = m;
        ctx->len = len - blocks*s;
        if (ctx->len) {
            __builtin_memcpy((uint8_t*)&ctx->last_block, data, ctx->len);
            //printf("L = %016"PRIx64"\n", ctx->last_block);
        }
    }
}
static 
uint64_t magma_cmac_fini(CCM_t *ctx)
{
    const int s=8;
	uint64_t K1 = magma_encrypt(ctx->K, 0ULL);
	//int of = K1>>63;
	//printf("R = %016"PRIx64"\n", K1);
	//K1=(K1<<1);
	if(__builtin_add_overflow (K1, K1, &K1)) K1^=0x1BULL;// 1th RED BULL
	//printf("K1= %016"PRIx64"\n", K1);
	if (ctx->len%s) {// не полный блок
		if(__builtin_add_overflow (K1, K1, &K1)) K1^=0x1BULL;// это равносильно сдвигу и K1<<1 и переносу
		/// добить нулями и 10000, и единицей
		//ctx->last_block <<= (s - (ctx->len%s))*8;
        ((uint8_t*)&ctx->last_block)[(ctx->len%s)] = 0x80;
        //printf("V = %016"PRIx64"\n", __builtin_bswap64(ctx->last_block));
	}
	uint64_t m = ctx->sum;
	m^= htonll(ctx->last_block)^K1;
	return magma_encrypt(ctx->K, m);
}

uint64_t magma_cmac(const uint32_t *K, uint8_t *iv, size_t vlen, uint8_t* data, size_t len)
{
    CCM_t ctx;
    magma_cmac_init(&ctx, K);
    if (vlen) magma_cmac_update(&ctx, iv, vlen);
    if ( len) magma_cmac_update(&ctx, data, len);
    return magma_cmac_fini(&ctx);
}

/*! \brief Режим гаммирования */
void magma_ctr(const uint32_t *K, uint32_t iv, uint8_t* data, size_t len)
{
    const int s=8;
	uint64_t m,v;
	union {
        uint32_t u[2];
        uint64_t u64;
	} ctr;
    ctr.u[0] =  0;
    ctr.u[1] = iv;
	int i=0;
    int blocks = (len)>>3;
	for (; i<blocks; i++){
		__builtin_memcpy(&v, &data[8*i], s);
		m = magma_encrypt(K, ctr.u64);
		v^= htonll(m>>(8*(8-s)));
		__builtin_memcpy(&data[8*i], &v, s);
		ctr.u[0]++;
	}
	int r = len&0x7;
	if (r){
		__builtin_memcpy(&v, &data[8*i], r);
		m = magma_encrypt(K, ctr.u64);
		v^= htonll(m>>(8*(8-s)));
		__builtin_memcpy(&data[8*i], &v, r);
	}
}
#if 0
typedef uint64_t poly64x2_t __attribute__((__vector_size__(16)));
uint64_t GF64_mul_ui (uint64_t a, uint8_t b) {
	int i;
	uint64_t r=0;
	for(i=0; i<8; i++){
		if (r&(1ULL<<63)) {
			r = (r<<1) ^ 0x1BULL;
		} else
			r = (r<<1);
		if (b&0x80){
			r ^=a;
		}
		b<<=1;
	}
	return r;
}
/*! таблица для редуцирования после умножения
	перенос (4бита) по таблице добавить к остатку 
*/
static const uint8_t gf2m_64[] = {
0x00, 0x1B, 0x36, 0x2D,
0x6C, 0x77, 0x5A, 0x41,
0xD8, 0xC3, 0xEE, 0xF5,
0xB4, 0xAF, 0x82, 0x99,
};
uint64_t GF64_mulm   (uint64_t a, uint64_t b)
{
	const uint64_t P = 0x1BULL;
	int i,n;
	uint64_t aa[16];// 128 байт
	// расчитать таблицу умножения для 16 значений
	for (n=0; n<16;n++) aa[n] = 0;
	for (i=0; i<4; i++){
		for (n=0; n<16; n++)
			if (n & (1<<i)) aa[n] ^= a;
			
		if (a&(1ULL<<63))
			a = (a<<1) ^ P;
		else
			a = (a<<1);
	}
	uint64_t r = 0;
	for (i=15; i>=0; i--){
		uint8_t cy = r>>60;
		r = (r<<4);
		r^= aa[(b>>(4*i))&0xF];
		r^= gf2m_64[cy];// редуцирование
	}
	return r;
}
uint64_t gf64_reduction(poly64x2_t s)
{
	uint64_t x1 = s[1];
	uint64_t x0 = s[0];
	x1 = x1 ^ x1>>63 ^ x1>>61 ^ x1>>60;
	return x0 ^ x1 ^ x1<<1 ^ x1<<3 ^ x1<<4;
}
void CL_MLA64(poly64x2_t *s, uint64_t a, uint64_t b) {
	const uint64_t P = 0x1BULL;
	int i,n;
	uint64_t aa[16];// таблица умножения 128 байт, 16 элементов
	// расчитать таблицу умножения для 16 значений
	for (n=0; n<16;n++) aa[n] = 0;
	for (i=0; i<4; i++){
		for (n=0; n<16; n++)
			if (n & (1<<i)) aa[n] ^= a;
		if (a&(1ULL<<63))
			a = (a<<1) ^ P;
		else
			a = (a<<1);
	}
	uint64_t r = 0;
	uint32_t cy= 0;
	for (i=15; i>=0; i--){
		cy = (cy<<4)|(r>>60);
		r = (r<<4) ^ aa[(b>>(4*i))&0xF];
	}
	(*s)^=(poly64x2_t){r,cy};
}
#endif
#ifdef TEST_MAGMA2
#include <cmsis_os.h>
static const uint32_t key_1[] = {
        0xfcfdfeff, 0xf8f9fafb, 0xf4f5f6f7, 0xf0f1f2f3,
        0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
        };
static void __attribute__((constructor)) _init() 
{
	uint64_t r64 = 0xfedcba9876543210ULL;
	r64 = magma_encrypt(key_1, r64);
	if (0x4ee901e5c2d8ca3dULL==r64) printf("magma encrypt test..ok\r\n");
	uint32_t ts = osKernelSysTick();
	int count = 1000;
	do {
		r64 = magma_encrypt(key_1, r64);
	} while (--count);
	printf("=T %u %X\r\n", osKernelSysTick() - ts, (uint32_t)(r64>>32));
//	extern void ITM_flush();
//	ITM_flush();
}
#endif
#if 0
#pragma GCC optimize("Os")
void magma_ekb (uint32_t* K, const uint8_t *key, int klen, int ekb)
{
	const uint32_t* kk = (const uint32_t*)key;
	int i;
#if 0 // LSB/MSB
	for (i=0;i<8; i++){
		uint32_t k = __builtin_bswap32(kk[i]);
		K[i+ 0] = k;
		K[i+ 8] = k;
		K[i+16] = k;
		K[31-i] = k;
	}
#else
	for (i=0;i<8; i++){
		uint32_t k = kk[i];
		K[ 7-i] = k;
		K[15-i] = k;
		K[23-i] = k;
		K[24+i] = k;
	}
#endif

}
#endif
#ifdef TEST_MAGMA
uint64_t magma_decrypt(uint32_t *K, uint64_t v){
	uint32_t a0 = v, a1 = v>>32;
	int i;
	for (i=31; i>=0; i-=2){
		a1 ^= g(K[i  ], a0);
		a0 ^= g(K[i-1], a1);
	}
	return (uint64_t)a0<<32 | a1;
}

int main()
{
/*	t(fdb97531) = 2a196f34,
	t(2a196f34) = ebd9f03a,
	t(ebd9f03a) = b039bb3d,
	t(b039bb3d) = 68695433.*/
	uint32_t a[] = {0xfdb97531, 0x2a196f34, 0xebd9f03a, 0xb039bb3d};
    int i;
    for (i=0; i<4; i++) {
		printf("t(%08x) = %08x\n", a[i], t(a[i]));
	}

/*	g[87654321](fedcba98) = fdcbc20c,
	g[fdcbc20c](87654321) = 7e791a4b,
	g[7e791a4b](fdcbc20c) = c76549ec,
	g[c76549ec](7e791a4b) = 9791c849.*/
	uint32_t a1 = 0xfedcba98;
	uint32_t k1 = 0x87654321;
	uint32_t r;
    for (i=0; i<4; i++) {
		r = g(k1, a1);
		printf("g[%08x](%08x) = %08x\n", k1, a1, r);
		a1 = k1;
		k1 = r;
	}
#if 0
	uint8_t *key =
		"\xff\xee\xdd\xcc\xbb\xaa\x99\x88\x77\x66\x55\x44\x33\x22\x11\x00"
		"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
#endif
	uint32_t key[] = {
        0xfcfdfeff, 0xf8f9fafb, 0xf4f5f6f7, 0xf0f1f2f3,
        0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
        };
	uint64_t r64;
	uint64_t a64 = 0xfedcba9876543210ULL;
	printf("A.2.3 Алгоритм развертывания ключа\n");
	uint8_t *k = (void*)key;
	for (i=0; i<32; i++) {
        printf("\\x%02x",k[i]);
	}
	printf("\n");
	MagmaCtx ctx;
	magma_ekb(ctx.K, (void*)key, 32, 0);
	printf("a   = %016llx\n", a64);
	r64 = 0x92def06b3c130a59ULL;
	printf("a   = %016llx\n", r64);
	
	printf("ГОСТ Р 34.13-2015\n");
	printf("A.2.4 Алгоритм зашифрования\n");
	r64 = magma_encrypt(ctx.K, r64);
	printf("enc = %016llx\n", r64);
	printf("A.2.4 Алгоритм расшифрования\n");
	r64 = magma_decrypt(ctx.K, r64);
	printf("dec = %016llx\n", r64);
	
	uint32_t Pt[] = {0x3c130a59, 0x92def06b,0xf8189d20,0xdb54c704,0x67a8024c,0x4a98fb2e,0x17b57e41,0x8912409b};
	printf("А.2.2 Режим гаммирования\nP[] =");
	for (i=0; i<4; i++) {
		printf(" %016llx", *(uint64_t*)&Pt[i*2]);
	}
	printf("\n");
	printf("A.2.2.1 Зашифрование\nC[] =");
	uint32_t iv = 0x12345678;
	uint8_t Ct[4*8];
	memcpy(Ct, Pt, 4*8);
	magma_ctr(ctx.K, iv, (uint8_t*)Ct, (uint8_t*)Pt,4*8);
	for (i=0; i<4; i++) {
		printf(" %016llx", *(uint64_t*)&Ct[i*8]);
	}
	printf("\n");
	printf("A.2.2.2 Расшифрование\nP[] =");
	magma_ctr(ctx.K, iv, (uint8_t*)Ct, (uint8_t*)Ct,4*8);
	for (i=0; i<4; i++) {
		printf(" %016llx", *(uint64_t*)&Ct[i*8]);
	}
	printf("\n");
	
	printf("A.2.6.2 Вычисление имитовставки MAC = 154e7210.\n");
	r64 = magma_cmac(ctx.K, (uint8_t*)Pt, sizeof(Pt));
	printf("cmac32 = %08x\n", (uint32_t)(r64>>32));
	
/*	for(i=0;i<4;i++){
		r64 = magma_encrypt(ctx.K, Pt[i]);
		printf("ECB (%08x%08x) = %08x%08x\n", Pt[i][1],Pt[i][0],r64[1],r64[0]);
		uint32_t p0 = __builtin_bswap32(Pt[i][0]);
		uint32_t p1 = __builtin_bswap32(Pt[i][1]);
		printf("%08x%08x => ", p0, p1);
		uint32_t r0 = __builtin_bswap32(r64[0]);
		uint32_t r1 = __builtin_bswap32(r64[1]);
		printf("%08x%08x\n", r0, r1);
	}
*/
    return 0;
}
#endif
