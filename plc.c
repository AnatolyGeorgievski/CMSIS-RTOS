/*! PLC прогрммируемый логический котроллер

Задание. Мы имеем произвовльную линейку сигналов. На каждом цикле обновления линейки происходит  пересчет

https://ru.wikipedia.org/wiki/Ladder_Diagram 
https://www.codesys.com/download.html

Логическая таблица задается в виде маски и инверсии. Инверсия
http://www.beremiz.org
ГОСТ Р 51840-2001 Программируемые контроллеры. Общие положения и функциональные характеристики
ГОСТ IEC 61131-2-2012 Контроллеры программируемые. Часть 2. Требования к оборудованию и испытания
Язык:
релейно-контактные схемы, или релейные диаграммы (LD - Ladder Diagram);

---( )-|  -- выходные сигналы, катушки: (S) - установить, (R) - сбросить, (/) - инвертированный, ( ) - нормальный
---[ ]-|  -- входные сигналы, контакты: [P] - positive фронт сигнала, [N] - negative спадающий фронт, [/] - инверсия, [ ] - нормальный


Идеи организации алогритма 
Надо использовать векторизацию, 



 */
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#define PLC_VECTOR_SIZE 16
typedef uint16_t PLC_t __attribute__((vector_size(PLC_VECTOR_SIZE)));
enum {
	PLC_OPCODE_AND, PLC_OPCODE_NAND, PLC_OPCODE_SET, PLC_OPCODE_RST, 
};
typedef struct _PLC_line PLC_line_t;
struct _PLC_line {
	PLC_t msk; // прямые биты 
	PLC_t inv; // инвертированные биты
	uint8_t opcode;// код операции
	uint8_t bitn; // номер бита
};

static inline 
void coil_set(PLC_t *state, int offset, uint16_t mask)
{
	(*state)[offset] |=  mask;
}
static inline 
void coil_rst(PLC_t *state, int offset, uint16_t mask)
{
	(*state)[offset] &= ~mask;
}

/*!
	\param size размер вектора
 */
//PLC_t 
void plc_update(PLC_t S, PLC_line_t *Y, int size)
{
	int i;
	for (i=0; ;i++, Y++){
//		PLC_t set = (Y->msk &  S);// 1 если контакт есть
//		PLC_t rst = (Y->inv & ~S);// если нет контакта
//		PLC_t equ = (set|rst)==(Y->msk|Y->inv);
		int ena, j;
		PLC_t res = ((Y->msk & (S))|(Y->inv & ~(S))) == (Y->msk|Y->inv);
		for (ena = 1, j=0;i<size; j++) {
			ena = ena && res[i];
			if(!ena) break;
		}
		
		uint16_t mask = (1<<(Y->bitn&0xF));
		int offset = Y->bitn>>4;
		switch(Y->opcode){
		case PLC_OPCODE_SET: // --(S)-|
			if (ena) {
				(S)[offset] |=  mask;
			}
			break;
		case PLC_OPCODE_RST: // --(R)-|
			if (ena) {
				(S)[offset] &= ~mask;
			}
			break;
		case PLC_OPCODE_NAND:// --(/)-|
			if (!ena) {
				(S)[offset] |=  mask;
			} else {
				(S)[offset] &= ~mask;
			}
			break;
		case PLC_OPCODE_AND: // --( )-|
		default:
			if (ena) {
				(S)[offset] |=  mask;
			} else {
				(S)[offset] &= ~mask;
			}
			break;
		}
	}
//	return S;
}

PLC_t map_alloc={0};
char* map_names[256];
/*! 
	по имени найти номер бита
 */
static int plc_get_uid(char* name, int len)
{
	int i;
	for (i=0; i<PLC_VECTOR_SIZE*8; i++)
	{
		int offset = i>>4;
		uint16_t mask = 1<<(i&0xF);
		if ((map_alloc[offset] & mask)!=0 && strncmp(map_names[i],name,len)==0) {// бит занят, имена совпадают
			return i;
		}
	}
	return -1;
}


#include "glib.h"
#include <locale.h>
/// ---------------------------------------------------
typedef struct _MainOptions MainOptions;
struct _MainOptions {
    gchar * input_file;
    gchar * output_file;
    gboolean verbose  ;
    gboolean overwrite;
};
static MainOptions options = {
    .input_file = NULL,
    .output_file = NULL,
    .verbose = FALSE,
    .overwrite = FALSE,
};
static GOptionEntry entries[] =
{
  { "input",    'i', 0, G_OPTION_ARG_FILENAME,  &options.input_file,    "input file name",  "*.xml" },
  { "output",   'o', 0, G_OPTION_ARG_FILENAME,  &options.output_file,   "output file name", "*.xml" },
  { "overwrite",'O', 0, G_OPTION_ARG_NONE,      &options.overwrite,     "overwrite mode",   NULL },
  { "verbose",  'v', 0, G_OPTION_ARG_NONE,      &options.verbose,       "Be verbose",       NULL },
  { NULL }
};

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    GError * error = NULL;
    GOptionContext *context;
    context = g_option_context_new ("- command line interface");
    g_option_context_add_main_entries (context, entries, NULL/*GETTEXT_PACKAGE*/);
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
        g_print ("option parsing failed: %s\n", error->message);
		g_error_free(error);
        exit (1);
    }
    g_option_context_free (context);

	// строка состоит из списка имен "LEVEL NAGREV ", кодирование логических функций 
	// " X5_38 "
	// "|-[ ]---[/]---( )-|", "set sdf=1&2&!sdf" "rst"
	GList* PLC_lines=NULL;
	char* content=NULL;
	gsize length=0;
	if (options.input_file!=NULL){
		g_file_get_contents(options.input_file, &content, &length, &error);
		if (error){
			g_print("ERR: %s\n", error->message);
			g_error_free(error); error=NULL;
			exit(1);
		}
	}
	if (content==NULL) return 0;
	char* s = content;
	while (s[0]!='\0'){
		PLC_line_t *Y = g_new0(PLC_line_t,1);
		PLC_lines = g_list_append(PLC_lines, Y);
		char mode = *s++;
		switch (mode){// тип выходного сигнала (обмотки)
		case 'S':
			Y->opcode = PLC_OPCODE_SET;
			break;
		case 'R':
			Y->opcode = PLC_OPCODE_RST;
			break;
		case '/':
			Y->opcode = PLC_OPCODE_NAND;
			break;
		case ' ':
		default:
			Y->opcode = PLC_OPCODE_AND;
			break;
		}
		
		
		while (s[0]==' ')s++; 
		mode = *s++;
		char* name = s;
		if (s[0]!='\0'){
			while (s[0]!='\0' && s[0]!=' ')s++;
			//name = g_strndup(name, s-name);
			// добавить имя в список имен
			Y->bitn = plc_get_uid(name, s-name);// выделить уникальный идентификатор в массиве
			int offset = (Y->bitn)>>4;
			uint16_t mask = 1<<(Y->bitn&0xF);
			switch (mode){
			case 'N':
			case '/':
				Y->inv[offset] |= mask;
				break;
			case 'P':
			case ' ':
			default:
				Y->msk[offset] |= mask;
				break;
			}
		}
		
	}
	return 0;
}