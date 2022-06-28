#include <time.h>
#include <stdint.h>
#define TM_YEAR_BASE 1900
#define EPOCH_YEAR 1970

/*! 
	https://pubs.opengroup.org/onlinepubs/9699919799/functions/strftime.html
Реализуем только необходимый минимум символов
Вариант struct tm BCD см. struct tm в файле time.h


In the "C" locale, the E and O modifiers are ignored and the replacement strings for the
following specifiers are:
%a the first three characters of %A.
%A one of ‘‘Sunday’’, ‘‘Monday’’, ... , ‘‘Saturday’’.
%b the first three characters of %B.
%B one of ‘‘January’’, ‘‘February’’, ... , ‘‘December’’.
%c equivalent to ‘‘%a %b %e %T %Y’’.
%p one of ‘‘AM’’ or ‘‘PM’’.
%r equivalent to ‘‘%I:%M:%S %p’’.
%x equivalent to ‘‘%m/%d/%y’’.
%X equivalent to %T.
%Z implementation-defined.

 */
#define BEXTR(x, n, len) (((x) >> (n)) & ((1 << (len))-1))
static char* bcd2str(char* s, uint32_t value)
{
#if 0// c11
	*s++ = (value /10)+'0';
	*s++ = (value %10)+'0';
#else
	*s++ = (value>>4)+ '0';
	*s++ = (value&0xF)+ '0';
#endif
	return s;
}
static uint32_t bcd2bin(uint32_t value)
{
	return value - (value>>4)*6;
}

static const char  wday_name[ 8*4]= "Sun Mon Tus Wed Thu Fri Sat Sun";
static const char month_name[12*4]= "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";

size_t     strftime(char * s, size_t len, const char * fmt, const struct tm * tv)
{
	uint32_t i=0;
	char* str = s;
	while (fmt[i]!='\0' && s<str+len) {
		if (fmt[i++]=='%') {
			//if (fmt[i]=='E' || fmt[i]=='O') i++;
			switch(fmt[i++]) {
			case 'a':{
				const char* src = &wday_name[tv->tm_wday*4];
				*s++ = src[0];
				*s++ = src[1];
				*s++ = src[2];
			} break;
			case 'b': {
				const char* src = &month_name[(bcd2bin(tv->tm_mon)-1)<<2];
				*s++ = src[0];
				*s++ = src[1];
				*s++ = src[2];
			} break;
			case 'Y':// год [2000,2099]
				*s++= '2';
				*s++= (tv->tm_year>>8) + '0';
			case 'y':// год [0,99]
				s = bcd2str(s, tv->tm_year&0xFF);
				break;
			case 'm':// месяц [01,12]
				s = bcd2str(s, tv->tm_mon);
				break;
			case 'd':// день месяца
				s = bcd2str(s, tv->tm_mday);
				break;
			case 'H':// часы [00,23]
				s = bcd2str(s, tv->tm_hour);
				break;
			case 'M':// минуты [00,59]
				s = bcd2str(s, tv->tm_min);
				break;
/*			case 'T':// is equivalent to ‘‘%H:%M:%S’’ (the ISO 8601 time format)
				s = bcd2str(s, tv->tm_hour);
				*s++ = ':';
				s = bcd2str(s, tv->tm_min);
				*s++ = ':';*/
			case 'S':// секунды [00,59]
				s = bcd2str(s, tv->tm_sec);
				break;
			// %F is equivalent to ‘‘%Y−%m−%d’’ (the ISO 8601 date format)
/*			case 'n': *s++ = '\n'; break;
			case 't': *s++ = '\t'; break;
			case '%': *s++ = '%'; break;*/
			default:
				*s++ = fmt[i-1];
				break;
			}
		} else
			*s++ = fmt[i-1];
	}
	*s = '\0';
	return s-str;
}

#ifdef TEST_STRFTIME
#include <stdio.h>
int main()
{
	time_t ts;
	struct tm t= {.tm_hour =0, .tm_year=2020-1900, .tm_mon=10, .tm_mday=22};
	return 0;
	
}

#endif
