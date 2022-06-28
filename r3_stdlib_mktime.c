#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
/*! \defgroup _lulian люлянский календарь
Царь Юрий (Юлий Цезарь) утвердил календарь Юлианский. В Юлианский календаре в году 365 дней и 1/4, каждые 4 года начисляется ещё один день.
Исчисление ведется от начала времен, которые, было установлено, начинаются в 4800 г. до н.э.
Поп Григорий (Папа Грегори XIII) 4 окт.1582 ввел Григорианский календарь, который уточнял Юлианский.
С тех пор, каждые 100 лет лишний день не начисляется, а каждые 400 всё-таки начисляется.
Продолжительность года составила 365 и 97/400, вернее 365 + 1/4 - 1/100 +1/400
Барон Мюнхгаузен пробобовал ввести свою поправку, но был бит за это.
Ошибка в Григорианском календаре накапливается за 3200 лет.

Синхронизация календарей требуется раз в 100 лет. от 1.3.2000 до 28.2.2100 можно использовать люлянские формулы, упрощенные,
потом убавить день и ещё на сто лет хватит.

Первичным является количество дней, от календаря не зависит.
Количество дней можно перевести в календарную дату число-месяц-год
Из количества дней можно вычислить день недели. День недели не зависит от календаря.

Начало времен в каждой операционке свое. В нашей системе начало времен вымеряется по утренней звезде от 1.1.1970.
Для перевода даты люниксовой в дату люлянскую надо использовать магическое число Люлян - разницу начала исчислений.

В будущих версиях, возможно, понадобится держать время летнее и зимнее - декретное.
Правила перевода часов: часы переводятся в 3 часа ночи в последнее воскресенье марта вперед и
обратно на час назад в последнее воскресенье октября.

    \see http://www.tondering.dk/claus/cal/calendar29.pdf
    \see http://www.chinesecalendar.net
    \{
*/

/*! \brief Преобразование времени
 4.14 Seconds Since the Epoch

A value that approximates the number of seconds that have elapsed since the Epoch. A Coordinated Universal Time name (specified in terms of seconds (tm_sec), minutes (tm_min), hours (tm_hour), days since January 1 of the year (tm_yday), and calendar year minus 1900 (tm_year)) is related to a time represented as seconds since the Epoch, according to the expression below.

If the year is <1970 or the value is negative, the relationship is undefined. If the year is >=1970 and the value is non-negative, the value is related to a Coordinated Universal Time name according to the C-language expression, where tm_sec, tm_min, tm_hour, tm_yday, and tm_year are all integer types:

    tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
        (tm_year-70)*31536000 + ((tm_year-69)/4)*86400 -
        ((tm_year-1)/100)*86400 + ((tm_year+299)/400)*86400
*/

#define LULIAN_OFFSET 2440587 // количество дней между 1.1.1900 и началом счисления 1.1.AD.
#define LULIAN_CALENDAR
#define TM_YEAR_BASE 1900

#define EPOCH_YEAR 1970

static inline uint32_t div5(uint32_t v){
    return (v*0xCCCCCCCDULL)>>34;
}
static inline uint32_t div10(uint32_t v){
    return (v*0xCCCCCCCDULL)>>35;
}
static inline uint32_t div100(uint32_t v) {
    return (v*0xA3D70A3EULL)>>38;
}
static inline uint32_t div400(uint32_t v) {
    return (v*0xA3D70A3EULL)>>40;
}
static inline uint32_t div365(uint32_t v) {// справедливо для чисел 31 бит.
    return (v*0xB38CF9B1ULL)>>40;
}
static inline uint32_t div153(uint32_t v) {
    return (v*0xD62B80D7ULL)>>39;
}
static inline uint32_t div1461(uint32_t v) {// B=1461: 0xB36D8398 nd=42
    return (v*0xB36D8398ULL)>>42;
}
static inline uint32_t div146097(uint32_t v) {// B=146097: 0xE5AC1AF4 nd=49
    return (v*0xE5AC1AF4ULL)>>49;
}
static inline uint32_t rem7(uint32_t v) {// справедливо для чисел 31 бит.
    uint32_t q = (v*0x92492493ULL)>>34;
	return v - q*7;
}

/* Is YEAR + TM_YEAR_BASE a leap year?  */
/*! \brief Является ли год високосным
	\param year год НЭ ...2021...
 */
static bool g_date_is_leap_year (uint32_t year)
{
  /* Don't add YEAR to TM_YEAR_BASE, as that might overflow.
     Also, work even if YEAR is negative.  */
#if 1
	uint32_t q = div100(year);
	uint32_t r = (year - q*100);
	return ((r!=0?year:q) & 3) == 0;
#else // этот вариант взят из Glib
  return ( (((year % 4) == 0) && ((year % 100) != 0)) ||
           (year % 400) == 0  );
#endif
}
#if 0
/*! \return  число дней с начала Эпохи 01.01.1900 */
static int32_t _days (uint32_t tm_year) {
//	return (tm_year-70)*365 + (tm_year-69)/4 - (tm_year-1)/100 + (tm_year+299)/400;
	const int N=0; // можно от души добавить любое количество циклов по 400 лет.
	tm_year+=299+400*N; // =1900-1
	uint32_t y_100 = div100(tm_year);
	y_100 -= (y_100>>2);
	return tm_year*365 + (tm_year>>2) - y_100 -(134774 + 146097*N);// 146097 - число дней в цикле 400лет
};
#endif
// extern int32_t timezone= (3*3600);
/*! \brief Создает штамп времени и расчитывает день недели

	Внимание: штамп времени - может прокручиваться в 2105 году, например
 */
time_t _mktime(struct tm* t)
{
	uint32_t tm_year = t->tm_year;// 1900+
	uint32_t tm_mon  = t->tm_mon;//0-11  % 12;
	uint32_t tm_yday = t->tm_mday;//1-31

    uint32_t y = tm_year + 1900 +4800 - !!(tm_mon<2);
    static const uint16_t b[] = {306,337,0,31,61,92,122,153,184,214,245,275};
#ifndef LULIAN_CALENDAR
	uint32_t y_100 = div100(y);
    tm_yday += b[tm_mon] + 365*y + (y>>2) - y_100 +(y_100>>2) - (32046 + LULIAN_OFFSET);
#else // работает до 2100 года.
    tm_yday += b[tm_mon] + 365*y + (y>>2) - (32097 + LULIAN_OFFSET);
#endif

/*
	tm_yday += leapyear (tm_year+1900)?_yday[1][tm_mon]:_yday[0][tm_mon];
	t->tm_yday = tm_yday; // число дней в году
	tm_yday += _days(tm_year);// количество дней с 1900 г.
	*/
	t->tm_wday = rem7(tm_yday + 4);// % 7; -- 0-6 вторник=2 четверг=4 воскресенье =0
	time_t ts = t->tm_sec + t->tm_min*60 + t->tm_hour*3600 + tm_yday*86400 + timezone;
	return ts;
}
#if 0
#include "rtc.h"
time_t time(time_t* tloc)
{
	struct tm t;
	rtc_time_get(&t);// преобразовать bcd формат
	time_t ts = mktime(&t);
	if (tloc) *tloc = ts;
	return ts;
}
#endif

/*! Преобразует штамп времени в секундах в время суток
*/
static inline uint32_t g_timestamp_to_hms (time_t timestamp, struct tm*  t)
{
    uint32_t days= (timestamp*0xC22E4507ULL)>>48;// 1/(3600*24)
    uint32_t n = timestamp - days*(3600*24);

    uint32_t v;
    v = (n*0x88888889ULL)>>37; // n/60;
    t->tm_sec = n - v*60; // %60
    n = v;
    v = (n*0x88888889ULL)>>37; // 1/60
    t->tm_min = n - v*60; // %60
    t->tm_hour= v;
	return days;
}
#if 1
/*! \brief преобразует люлянский день в число, месяц и год в Гришином исчислении
    \param lulian_days - количество люлянских дней от магической люлянской даты
    \param day - число, 1..31
    \param month - месяц, 0..11
    \param year - год, ...2010...
*/
static struct tm* g_days_to_dmy (uint32_t days, struct tm*  t)
{
  /* Formula taken from the Calendar FAQ; the formula was for the
   *  Julian Period which starts on 1 January 4713 BC, so we add
   *  1,721,425 to the number of days before doing the formula.
   *
   * I'm sure this can be simplified for our 1 January 1 AD period
   * start, but I can't figure out how to unpack the formula.
   */
/*
  uint32 A, B, C, D, E, M;
  A = d->julian_days + 1721425 + 32045;
  B = ( 4 *(A + 36524) )/ 146097 - 1;
  C = A - (146097 * B)/4;
  D = ( 4 * (C + 365) ) / 1461 - 1;
  E = C - ((1461*D) / 4);
  M = (5 * (E - 1) + 2)/153;

  m = M + 3 - (12*(M/10));
  day = E - (153*M + 2)/5;
  y = 100 * B + D - 4800 + (M/10);
  */
    /*  d = JD-AD -- количество дней от начала летосчисления
    1461 дней на четыре года + 1 количество високосных годов = 365*4+1
    146097 - дней на 400 лет - каждые 100 лет високосный год отменяется, каждые 400 лет не отменяется
    */
    uint32_t a = days + (2440587 + 32045);
    uint32_t b = div146097(4*a+3);// /146097; -- число пропусков циклов 400лет
    uint32_t c = a - ((146097*b)>>2);
    uint32_t d = div1461(4*c + 3);// /1461; -- число пропусков високосных лет каждые 4 года
    uint32_t e = c - ((1461*d)>>2);
    uint32_t m = div153(5*e + 2);// /153;
    t->tm_mon  = m + 2 - (12*div10(m));// 0..11
    t->tm_mday = e - div5(153*m + 2)+1;
    uint32_t year = 100 * b + d - 4800 + div10(m);
	t->tm_year = year - 1900;
	// надо посчитать число yday - _days(year) -- день года
//	printf (" Y=%d\r\n", year);
	return t;
}
#endif
struct tm* localtime_r(const time_t *timer, struct tm*  t)
{
	time_t ts = *timer;
	ts -= timezone;
	uint32_t yday = g_timestamp_to_hms(ts, t);// число дней от 1970
	t->tm_wday = rem7(yday+4);// % 7; 1.1.1970 - четверг

	return g_days_to_dmy(yday, t);
}
#ifdef TEST_MKTIME

/*! \brief переводит календарную дату в люлянское исчисление
    \param day - число, 1..31
    \param month - месяц, 1..12
    \param year - год, ...2010...
    \return количество дней от начала люлянского исчисления
    */
uint32_t g_lulian_from_dmy (uint8_t day, uint8_t month, uint16_t year)
{
    uint32_t y = year +4800 - !!(month<3);
    static const uint16_t b[] = {275,306,337,0,31,61,92,122,153,184,214,245,275};
#ifndef LULIAN_CALENDAR
    return day + b[month] + 365*y + (y>>2) - div100(y) +(div100(y)>>2) - 32046; // гришина формула
#else
    return day +  b[month] +365*y + (y>>2) - 32097; // магическое люлянское число для грегорианского календаря 365 дней и 97/400
#endif
    // - 32083; // люлянское исчисление проще 365 и 1/4
}

/*! \brief Преобразование штампа времени в дату и время */
int main()
{
	time_t ts;
	struct tm t= {.tm_hour =7, .tm_year=2020-1900, .tm_mon=5, .tm_mday=22};
	int i;
	for (i=0; i<500; i++) {
		t.tm_year=i+70;
		ts = mktime(&t);
		if (ts==(time_t)-1) break;
//		printf("i=%d\r\n", i);
		printf("1:WDay=(%d) YDay=%d, %02d/%02d/%4d time=%08X\r\n", t.tm_wday, t.tm_yday,
			t.tm_mday, t.tm_mon+1, t.tm_year+1900,
			(unsigned int)ts);
		fflush(stdout);
		localtime_r(&ts, &t);
		fflush(stdout);
		ts = _mktime(&t)-ts;
		printf("2:WDay=(%d) YDay=%d, %02d/%02d/%4d time=%08X\r\n", t.tm_wday, t.tm_yday,
			t.tm_mday, t.tm_mon+1, t.tm_year+1900,
			(unsigned int)ts);
		fflush(stdout);
		if (ts!=0) break;
	}
	printf("i=%d\r\n", i);
	return 0;

}

#endif
