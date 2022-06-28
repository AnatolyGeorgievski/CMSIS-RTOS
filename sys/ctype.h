#ifndef CTYPE_H
#define CTYPE_H
// Character handling <ctype.h> c11
/* для локали koi8-r  https://tools.ietf.org/html/rfc1489

ГОСТ Р 34.303-92. Наборы 8-битных кодированных символов. 8-битный код обмена и обработки информации

	0xC0<= c<= 0xDF || 0xA3='ё' - islower	: 
	0xE0<= c<= 0xFF || 0xB3='Ё' - isupper	: 
для локали UTF8 0x400 - 0x04FF кириллица
	0x430<=c<= 0x44F || 0x451='ё' - isupper и еще много маленьких буквочек
	0x410<=c<= 0x42F || 0x401='Ё' - isupper
unsigned short 
koi8r_unicode[128] = {
     0x2500,0x2502,0x250c,0x2510,0x2514,0x2518,0x251c,0x2524,
     0x252c,0x2534,0x253c,0x2580,0x2584,0x2588,0x258c,0x2590,
     0x2591,0x2592,0x2593,0x2320,0x25a0,0x2219,0x221a,0x2248,
     0x2264,0x2265,0x00a0,0x2321,0x00b0,0x00b2,0x00b7,0x00f7,
     0x2550,0x2551,0x2552,0x0451,0x2553,0x2554,0x2555,0x2556,
     0x2557,0x2558,0x2559,0x255a,0x255b,0x255c,0x255d,0x255e,
     0x255f,0x2560,0x2561,0x0401,0x2562,0x2563,0x2564,0x2565,
     0x2566,0x2567,0x2568,0x2569,0x256a,0x256b,0x256c,0x00a9,
	 
     0x044e,0x0430,0x0431,0x0446,0x0434,0x0435,0x0444,0x0433,
     0x0445,0x0438,0x0439,0x043a,0x043b,0x043c,0x043d,0x043e,
     0x043f,0x044f,0x0440,0x0441,0x0442,0x0443,0x0436,0x0432,
     0x044c,0x044b,0x0437,0x0448,0x044d,0x0449,0x0447,0x044a,
	 
     0x042e,0x0410,0x0411,0x0426,0x0414,0x0415,0x0424,0x0413,
     0x0425,0x0418,0x0419,0x041a,0x041b,0x041c,0x041d,0x041e,
     0x041f,0x042f,0x0420,0x0421,0x0422,0x0423,0x0416,0x0412,
     0x042c,0x042b,0x0417,0x0428,0x042d,0x0429,0x0427,0x042a
   };
unsigned short
cp866_unicode[128] = {
	 0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
	 0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,0x041F,
	 0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
	 0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,0x042F,
	 0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
	 0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,0x043F,
	 0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,
	 0x2555,0x2563,0x2551,0x2557,0x255D,0x255C,0x255B,0x2510,
	 0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x255E,0x255F,
	 0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x2567,
	 0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256B,
	 0x256A,0x2518,0x250C,0x2588,0x2584,0x258C,0x2590,0x2580,
	 0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
	 0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F,
	 0x0401,0x0451,0x0404,0x0454,0x0407,0x0457,0x040E,0x045E,
	 0x00B0,0x2219,0x00B7,0x221A,0x2116,0x00A4,0x25A0,0x00A0
	};
unsigned short
cp1251_unicode[128] = {
	 0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
	 0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
	 0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
	 0x0000,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
	 0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
	 0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
	 0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
	 0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
	 0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
	 0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,0x041F,
	 0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
	 0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,0x042F,
	 0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
	 0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,0x043F,
	 0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
	 0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F
	};
 */

static inline int isalpha(int c) {
	return (0x40<c && c<=0x5A) || (0x60<c && c<=0x7A);
}
static inline int isdigit(int c){
	return (0x30<=c && c<0x3a);
}
int isalnum(int c) {
	return isdigit(c) || isalpha(c);
}
static inline int islower(int c){
	return (0x40<c && c<=0x5A);
}
static inline int isblank(int c){
	return c==' ' || c=='\t';
}
int iscntrl(int c);
int isgraph(int c);
int isprint(int c);
int ispunct(int c);
static inline int isspace(int c){
/* space (' '), form feed ('\f'), new-line ('\n'), carriage return ('\r'), 
	 horizontal tab ('\t'), and vertical tab ('\v'). */
	return (c==' ') || (c=='\t') || (c=='\n') || (c=='\r');
	//return (_char_map[c] & _IS_SPACE);
}
static inline int isupper(int c){
	return (0x60<c && c<=0x7A);
}
static inline int isxdigit(int c) {
	return isdigit(c) || ('A'<=c && c<='F') || ('a'<=c && c<='f');
}
static inline int tolower(int c){
	if (isupper(c)) c-= 0x20;
	return c;
}
static inline int toupper(int c){
	if (islower(c)) c+= 0x20;
	return c;
}

#endif // CTYPE_H
