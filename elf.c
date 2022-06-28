/*! ELF32 file format 

http://www.skyfree.org/linux/references/ELF_Format.pdf
https://static.docs.arm.com/ihi0044/g/aaelf32.pdf

https://github.com/ARM-software/abi-aa/releases

.. glibc/elf/elf.h

$ gcc -DTEST_ELF -I. -o elf.exe r3core/elf.c r3_args.c
$ ./elf.exe -a -i r3core/malloc.o
Дизассемблер
$ arm-none-eabi-objdump.exe -d a.out
$ arm-none-eabi-readelf.exe -a a.out

Для отладки сохраняем ELF перемещаемый от всего проекта и сравниваем результат
$ arm-none-eabi-ld -T device/SH29COL8/stm32f301x8.ld device/SH29COL8/startup_stm32f301x8.o main.o -r
*/
/*!
.bss
    Uninitialized data that contribute to the program's memory image. By definition, the system initializes the data with zeros when the program begins to run. The section occupies no file space, as indicated by the section type SHT_NOBITS.
.comment
    Comment information, typically contributed by the components of the compilation system. This section can be manipulated by mcs(1).
.data, .data1
    Initialized data that contribute to the program's memory image.
.dynamic
    Dynamic linking information. See Dynamic Section for details.
.dynstr
    Strings needed for dynamic linking, most commonly the strings that represent the names associated with symbol table entries.
.dynsym
    Dynamic linking symbol table. See Symbol Table Section for details.
.fini
    Executable instructions that contribute to a single termination function for the executable or shared object containing the section. See Initialization and Termination Routines for details.
.fini_array
    An array of function pointers that contribute to a single termination array for the executable or shared object containing the section. See Initialization and Termination Routines for details.
.got
    The global offset table. See Global Offset Table (Processor-Specific) for details.
.hash
    Symbol hash table. See Hash Table Section for details.
.init
    Executable instructions that contribute to a single initialization function for the executable or shared object containing the section. See Initialization and Termination Routines for details.
.init_array
    An array of function pointers that contributes to a single initialization array for the executable or shared object containing the section. See Initialization and Termination Routines for details.
.interp
    The path name of a program interpreter. See Program Interpreter for details.
.note
    Information in the format described in Note Section.
.plt
    The procedure linkage table. See Procedure Linkage Table (Processor-Specific) for details.
.preinit_array

    An array of function pointers that contribute to a single pre-initialization array for the executable or shared object containing the section. See Initialization and Termination Routines for details.
.rela
    Relocations that do not apply to a particular section. One use of this section is for register relocations. See Register Symbols for details.
.relname, .relaname
    Relocation information, as Relocation Sections describes. If the file has a loadable segment that includes relocation, the sections' attributes include the SHF_ALLOC bit. Otherwise, that bit is off. Conventionally, name is supplied by the section to which the relocations apply. Thus, a relocation section for .text normally will have the name .rel.text or .rela.text.
.rodata, .rodata1
    Read-only data that typically contribute to a non-writable segment in the process image. See Program Header for details.
.shstrtab
    Section names.
.strtab
    Strings, most commonly the strings that represent the names associated with symbol table entries. If the file has a loadable segment that includes the symbol string table, the section's attributes include the SHF_ALLOC bit. Otherwise, that bit is turned off.
.symtab
    Symbol table, as Symbol Table Section describes. If the file has a loadable segment that includes the symbol table, the section's attributes include the SHF_ALLOC bit. Otherwise, that bit is turned off.
.symtab_shndx
    This section holds the special symbol table section index array, as described by .symtab. The section's attributes will include the SHF_ALLOC bit if the associated symbol table section does. Otherwise, that bit is turned off.
.tbss
    This section holds uninitialized thread-local data that contribute to the program's memory image. By definition, the system initializes the data with zeros when the data is instantiated for each new execution flow. The section occupies no file space, as indicated by the section type, SHT_NOBITS. See Chapter 8, Thread-Local Storage for details.
.tdata, .tdata1
    These sections hold initialized thread-local data that contribute to the program's memory image. A copy of its contents is instantiated by the system for each new execution flow. See Chapter 8, Thread-Local Storage for details.
.text
    The text or executable instructions of a program.
*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "elf.h"
/*! \brief контекст разбора */
typedef struct _ElfCtx ElfCtx_t;
struct _ElfCtx {
	Elf32_Shdr_t* shdr;		// таблица сегментов
	Elf32_Sym_t*  symbols;	// таблица символов
	char* strings;// текстовые константы
	ElfHash_t *htable;
	int shnum;// число заголовков секций
	
};
/*! \brief декодирование ULEB128 поля целого числа без знака переменной длины */
uint8_t * dwarf_uleb128_decode(uint32_t *dst, int dst_bits,  uint8_t *src)
{
	Elf32_Word result = 0;
	int shift = 0;
	uint8_t byte;
	do {
		uint8_t byte = *src++;
		result |= (byte&0x7F) << shift;
		shift+=7;
	} while ((byte & 0x80) != 0);
	*dst = result;
	return src;
}
/*! \brief декодирование ULEB128 поля целого числа со знаком */
uint8_t * dwarf_leb128_decode(int32_t *dst, int dst_bits,  uint8_t *src)
{
	Elf32_Word result=0;
	int shift =0;
	uint8_t byte;
	do {
		byte = *src++;
		result |= (byte&0x7F) << shift;
		shift += 7;
	} while ((byte&0x80) != 0);

	if ((shift<dst_bits) && (byte&0x40))
		result |= (~0UL << shift);// sign extend
	*dst = result;
	return src;
}


/*! \brief Расчет ключа для Хеш таблицы символов динамической линковки 
 
 */
static 
uint32_t elf_hash(const char *name)
{
	uint32_t h = 0;
	uint32_t g;
	int ch;
	while ((ch = *name++) != '\0'){
		h = (h << 4) + ch;
		if ((g = (h & 0xF0000000UL)) != 0){
			h ^=  g >> 24;
			h &= ~g;
		}
	}
	return h;
}
/*! \brief выполнить перемещение по быстрому 
Анализируются далеко не все варианты, только те что встречаются на практике
 */
static
int elf_arm_relocate_fast(Elf32_Rel_t* rel, uint32_t symbol_value, 
		uint8_t *segment, uint32_t segment_addr)
{
	uint8_t * address =  segment + rel->r_offset;
	switch((uint8_t)rel->r_info){
	default: return -1; 
		break;
	case R_ARM_TARGET1:// появляется только в разделе .init и .fini
	case R_ARM_ABS32: {// Data: (S + A) | T
		printf(" -- R_ARM_ABS32 0x%08X=>0x%08X\n", *(uint32_t *)address, *(uint32_t *)address + symbol_value);
		*(uint32_t *)address += symbol_value;
	} break;
	case R_ARM_THM_JUMP24:
	case R_ARM_THM_CALL: { // Thumb32: ((S + A) | T) - P, T - признак Thumb 
		symbol_value -= (segment_addr + rel->r_offset +4);
		uint32_t insn = *(uint32_t*)address;
		insn &= ~0x07FF07FF;
		insn |= (symbol_value<<15) & 0x07FF0000;
		insn |= (symbol_value>>12) & 0x000007FF;
		printf(" -- R_ARM_THM_CALL 0x%08X=>0x%08X\n", *(uint32_t *)address, insn);
		*(uint32_t*)address = insn;
	} break;
	}
	return 0;
}
#if 0
static
int elf_arm_relocate_dynamic(Elf32_Rel_t* rel, uint32_t symbol_value, 
		uint8_t *segment, uint32_t segment_addr)
{
	switch ((uint8_t)rel->r_info){
/*
	R_ARM_RELATIVE: // B(S) + A
	R_ARM_ABS32:
	R_ARM_GLOB_DAT: // Data: (S + A) | T - применяется к секции .got -- global offset table
	R_ARM_JUMP_SLOT:// Data: (S + A) | T - должно применяться к секции .plt -- program linkage table
		*/
	default:
		break;
	}
	return 0;
}
#endif

/*! \brief Выполнить перемещение сегмента, 
	Загрузили часть сегмента в буфер, выполнили для него перемещения, загрузили из буфера в память.
	\param segment - адрес буфера для линковки
	\param reloc - таблица перемещения
	\param r_num - число записей в таблице перемещений
	\param segment_addr - адрес сегмента (кода/данных) относительного которого расчитываются перемещения.
 */
static
int elf_relocate_segment(Elf32_Rel_t* reloc, int r_num, 
		uint8_t* segment, Elf32_Off segment_addr, ElfCtx_t *ctx) 
{
	int i;
	for (i=0; i < r_num; i++){
	// relocs[i].r_offset < segment_size
		int n = ELF32_R_SYM(reloc[i].r_info);
		uint32_t symbol_value = ctx->symbols[n].st_value;
		if (ctx->symbols[n].st_shndx < ctx->shnum) 
			symbol_value += ctx->shdr[ctx->symbols[n].st_shndx].sh_addr;// адрес сегмента в котором расположен символ
		else if (ctx->symbols[n].st_shndx==SHN_UNDEF) 
			continue;
		else {
			// ABS COM
		}

		elf_arm_relocate_fast(reloc+i, symbol_value, segment, segment_addr);
		
	}
	return i; // номер следующей записи в сегменте перемещений
}
__attribute__((noinline))
static int elf_section_id(Elf32_Shdr_t *shdr, int shnum, int type_id)
{
	int i;
	if (shdr==NULL) return 0;
	for (i=0; i< shnum; i++){
		if (shdr[i].sh_type == type_id) return i;
	}
	return 0;
}
__attribute__((noinline))
static void* elf_section_load(FILE* fp, uint32_t offset, size_t size) 
{
	fpos_t fpos = offset;
	fsetpos(fp, &fpos);
	void* section = malloc(size);
	fread(section, 1, size, fp);
	return section;
}

#define STN_UNDEF 0
/*! \brief Найти символ с использованием хеш таблиц
	\return возвращает индекс в таблице символов .symtab/.dyntab или STN_UNDEF=0
 */
static uintptr_t elf_hashtable_lookup(const ElfHash_t *htable, const char *name, Elf32_Sym_t* dynsym, const char* dynstr)
{
	uint32_t key = elf_hash(name);
	uint32_t y = htable->bucket[key % htable->nbucket];// первая таблица по хешам
	const uint32_t *chain = htable->bucket + htable->nbucket;
	while (y<htable->nchain && y!=STN_UNDEF) {// y<htable->nchain - можно не проверять
		if (strcmp(dynstr  + dynsym[y].st_name, name)==0) 
			return dynsym[y].st_value;
		y = chain[y];
	}
	return 0;
}
static uint32_t elf_hashtable_init(ElfHash_t *htable,uint32_t nbucket) 
{
	htable->nbucket = nbucket;
	htable->nchain = 0;
//	htable->bucket = malloc((nchain+nbucket)*sizeof(uint32_t));
	int i;
	for (i=0; i<nbucket; i++){
		htable->bucket[i]=STN_UNDEF;
	}
}
/*! \brief Заполнение хеш таблицы

	Таблица хранится в форме массива. Переполнение массива не анализируется. Информация хранится в трех сегментах: символы, имена и хещ таблица. Хеш таблица разделена на две части - ссылки head->next->...->next->STN_UNDEF Таблица инициализируется значением STN_UNDEF.
	
	\return индекс в таблице dynsym
операцию можно сделать атомарно, атомарно изменить счетчик объектов, атомарно работать с цепочкой. 
*/
static uint32_t elf_hashtable_insert(ElfHash_t *htable, const char *name)
{
	uint32_t key = elf_hash(name);
	uint32_t *chain = htable->bucket + htable->nbucket;
	// добавили в таблицу симоволов 'idx'
	// List_prepend_atomic атомарно?? 
	uint32_t y = htable->nchain++;// atomic_fetch_add(htable->nchain, 1);// увеличиваем число объектов (семафор)
	// добавить запись в таблице символов, в конец таблицы 
	Elf32_Word* head = &htable->bucket[key % htable->nbucket];
	chain[y] = *head;
	*head = y;
/* для динамической заменить на списки односвязные */
	return y;
}

typedef struct _dlCtx dlCtx_t;
struct _dlCtx {
	ElfHash_t  * htable;
	Elf32_Sym_t* dynsym;
	char * dynstr;
};

void* dlopen(const char* filename, int flags)
{
	FILE* fp = fopen(filename, "rb");
	ElfHeader_t* elf_header = malloc(sizeof(ElfHeader_t));
	// загрузкить заголовок
	fread(&elf_header, 1, sizeof(ElfHeader_t), fp);
	uint32_t e_shoff = elf_header->e_shoff;	// смещение секции заголовков сегментов
	uint32_t shnum   = elf_header->e_shnum;	// число заголовков сегментов
	free(elf_header);
	dlCtx_t * ctx = malloc(sizeof(dlCtx_t));
	Elf32_Shdr_t* shdr = elf_section_load(fp, e_shoff, shnum * sizeof(Elf32_Shdr_t));
	int idx;
	// выполнить Relocations
	// можно загрузить секцию DYNAMIC 
	idx = elf_section_id(shdr, shnum, SHT_HASH);
	ctx->htable = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	idx = elf_section_id(shdr, shnum, SHT_DYNSYM);
	ctx->dynsym = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	idx = elf_section_id(shdr, shnum, SHT_STRTAB);
	ctx->dynstr = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	free(shdr);
	fclose(fp);
	return ctx;
}
void* dlsym(void* handler, const char* name)
{
	dlCtx_t * ctx = handler;
	uintptr_t addr = elf_hashtable_lookup(ctx->htable, name, ctx->dynsym, ctx->dynstr);
	return (void*)addr;
}
extern
void  dlclose(void* handler)
{
	dlCtx_t * ctx = handler;
	free(ctx->htable);
	free(ctx->dynsym);
	free(ctx->dynstr);
	free(ctx);
}
/*
#define EXPORT_SYMBOL(sym) extern __typeof__(sym) sym
EXPORT_SYMBOL(dlopen);
EXPORT_SYMBOL(dlsym);
EXPORT_SYMBOL(dlclose);
*/


#ifdef TEST_ELF
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "r3_args.h"

typedef struct _Names Names_t;
struct _Names {
	uint32_t key; 
	const char* name;
};

const Names_t pt_names[] = {
	{PT_NULL, 		"NULL"},
	{PT_LOAD, 		"LOAD"},
	{PT_DYNAMIC, 	"DYNAMIC"},
	{PT_INTERP, 	"INTERP"},
	{PT_NOTE, 		"NOTE"},
	{PT_SHLIB, 		"SHLIB"},
	{PT_SHDR, 		"SHDR"},
	{PT_ARM_EXIDX, 	"EXIDX"},
};

/*!
HOWTO()
The following nomenclature is used for the operation:
• S (when used on its own) is the address of the symbol.
• A is the addend for the relocation.
• P is the address of the place being relocated (derived from r_offset).
• Pa is the adjusted address of the place being relocated, defined as (P & 0xFFFFFFFC).
• T is 1 if the target symbol S has type STT_FUNC and the symbol addresses a Thumb instruction; it is 0 otherwise.
• B(S) is the addressing origin of the output segment defining the symbol S. The origin is not required to be the base address of the segment. This value must always be word-aligned.
• GOT_ORG is the addressing origin of the Global Offset Table (the indirection table for imported data addresses). This value must always be word-aligned. See Proxy generating relocations (page 40).
• GOT(S) is the address of the GOT entry for the symbol S.
*/
const Names_t rel_type_names[] = {
	{ 0, 	"R_ARM_NONE"},
	{ 2, 	"R_ARM_ABS32"}, 	// Data:  (S + A) | T
	{10, 	"R_ARM_THM_CALL"},  // Thumb32: ((S + A) | T) - P :=X & 0x01FFFFFE \sa R_ARM_THM_PC22
	{20,	"R_ARM_COPY"},		// Misc
	{21,	"R_ARM_GLOB_DAT"},  // Data:  (S + A) | T
	{22, 	"R_ARM_JUMP_SLOT"}, // Data:  (S + A) | T
	{23, 	"R_ARM_RELATIVE"},  // Data: B(S)+ A
	{30, 	"R_ARM_THM_JUMP24"},// Thumb32: ((S + A) | T) - P
	{38,	"R_ARM_TARGET1"}, 	// Misc
	{42, 	"R_ARM_PREL31"},	// Data, 
	{47,	"R_ARM_THM_MOVW_ABS_NC"},// Thumb32:(S + A) | T	:lower16 
	{48,	"R_ARM_THM_MOVT_ABS"}, 	 // Thumb32: S + A		:upper16
	{51, 	"R_ARM_THM_JUMP19"},	 // ((S + A) | T) - P
//	{93, 	"R_ARM_THM_TLS_CALL"},
//	{102, 	"R_ARM_THM_JUMP11"},	 // Thumb16: S + A - P
//	{103, 	"R_ARM_THM_JUMP8"},	 	 // Thumb16: S + A - P
//  Armv8.1-M Mainline Branch Future relocations:
	{136, 	"R_ARM_THM_BF16"},	// ((S + A) | T) - P :=X & 0x0001FFFE
	{137, 	"R_ARM_THM_BF12"},	// ((S + A) | T) - P :=X & 0x00001FFE
	{137, 	"R_ARM_THM_BF18"},	// ((S + A) | T) - P :=X & 0x0007FFFE

};

#if 0
/* 
bacnet://<device>/<object>

The <device> segment is the device instance number in decimal. A <device> identifier of ".this" means 'this device' so that it can be used in static files that do not need to be changed when the device identifier changes.

The <object> identifier is in the form "<type>,<instance>" where <type> is either a decimal number or exactly equal to the Clause 21 identifier text of BACnetObjectType, and <instance> is a decimal number.

Link with -ldl. 
*/
#include <dlfcn.h>
void* 	dlopen(const char* filename, int flag);
void*  	dlsym(void* dl, const char* name);
int 	dlclose(void *dl); 

#include <unistd.h>
int execve(const char *filename, char *const argv[], char *const envp[]); 

#include <sys/mman.h>
// shm_open - open a shared memory object (REALTIME)[SHM]
// shm_unlink - remove a shared memory object (REALTIME)[SHM]


// об int fildes = BACNET_OID(FILE, ); 
int fileno(FILE*);
void *mmap  (void *addr, size_t len, int prot, int flags, 
			int fildes, off_t off);
int   munmap(void *addr, size_t len);
#endif
#define ARM_THM_ABS32_BDEP(insn,a)  ((insn)+(a))
/* Encoding T1 All versions of the Thumb instruction set. B<c> <label>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 |
  1  1  0  1|  cond   |      imm8      |
 imm32 = SignExtend(imm8:'0', 32); */
#define ARM_THM_JUMP8_BDEP(insn,a)  ((((a) & 0x0000001FE)>>1) | ((insn)&~0x00FF))
/* Encoding T2 All versions of the Thumb instruction set. B<c> <label>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 |
  1  1  1  0  0|       imm11           |
 imm32 = SignExtend(imm11:'0', 32); */
#define ARM_THM_JUMP11_BDEP(insn,a)  ((((a) & 0x000000FFE)>>1) | ((insn)&~0x07FF))
/* Encoding T3 ARMv7-M: B<c>.W <label>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
  1  1  1  1  0| S|  cond |    imm6    |  1  0|J1  0 J2|        imm11         |
  imm32 = SignExtend(S:J2:J1:imm6:imm11:'0', 32); */
#define ARM_THM_JUMP19_BDEP(insn,a) ((((a) & 0x00000FFE)<<15) | (((a) & 0x0003F000)>>12) | (((a) & 0x00040000)<<11) | (((a) & 0x00080000)<< 8) | (((a) & 0x00100000)>>10) | ((insn)&~0x2FFF043F))
#if 0
/* Encoding T4 ARMv7-M: B<c>.W <label>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
  1  1  1  1  0| S|      imm10         |  1  0|J1  1 J2|        imm11         |
  I1 = NOT(J1 EOR S); I2 = NOT(J2 EOR S); imm32 = SignExtend(S:I1:I2:imm10:imm11:'0', 32); */
#define ARM_THM_JUMP24_BDEP(insn,a) ((((a) & 0x00000FFE)<<15) | (((a) & 0x003FF000)>>12) | (((a) & 0x00400000)<<5) | (((a) & 0x00800000)<< 6) | (((a) & 0x01000000)>>14) | ((insn)&~0x2FFF07FF))
/* Encoding T1 All versions of the Thumb instruction set: BL<c> <label>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
  1  1  1  1  0| S|      imm10         |  1  1|J1  1 J2|        imm11         |
  I1 = NOT(J1 EOR S); I2 = NOT(J2 EOR S); imm32 = SignExtend(S:I1:I2:imm10:imm11:'0', 32); */
#define ARM_THM_CALL_BDEP(insn,a)   ((((a) & 0x00000FFE)<<15) | (((a) & 0x003FF000)>>12) | (((a) & 0x00400000)<<5) | (((a) & 0x00800000)<< 6) | (((a) & 0x01000000)>>14) | ((insn)&~0x2FFF07FF))
#else
#define ARM_THM_JUMP24_BDEP(insn,a)   ((((a) & 0x00000FFE)<<15) | (((a) & 0x003FF000)>>12) | (((a) & 0x01000000)>>14) | ((insn)&~0x07FF07FF))
#define ARM_THM_CALL_BDEP(insn,a)   ((((a) & 0x00000FFE)<<15) | (((a) & 0x003FF000)>>12) | (((a) & 0x01000000)>>14) | ((insn)&~0x07FF07FF))
#endif


/* Encoding T3 ARMv7-M: MOVW<c> <Rd>,#<imm16>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
  1  1  1  1  0| i 1 0|0 1 0 0|  imm4  |  0|  imm3  |    Rd   |     imm8      |
  imm32 = ZeroExtend(imm4:i:imm3:imm8, 32);	*/
#define ARM_THM_MOVW_BDEP(insn,a) ((((a) & 0x0000F000)>>12) | (((a) & 0x00000700)<<20) | (((a)& 0x00000800)>> 1) | (((a) & 0x000000FF)<<16) | ((insn)&~0x70FF040F))
/* Encoding T1 ARMv7-M:	MOVT<c> <Rd>,#<imm16>
 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
  1  1  1  1| 0  i 1 0|1 1 0 0|  imm4  |  0|  imm3  |    Rd   |     imm8      |
  imm16 = imm4:i:imm3:imm8;	*/
#define ARM_THM_MOVT_BDEP(insn,a) ((((a) & 0xF0000000)>>28) | (((a) & 0x07000000)<< 4) | (((a)& 0x08000000)>>17) | (((a) & 0x00FF0000)>> 0) | ((insn)&~0x70FF040F))
int elf_arm_relocate(Elf32_Rel_t* rel, const Elf32_Sym_t * symbols, 
		uint8_t *segment, int increment, uint32_t segment_addr)
{
	int n = ELF32_R_SYM(rel->r_info);
	uint8_t * address =  segment + rel->r_offset;
	switch(rel->r_info & 0xFF){
//	case R_ARM_PREL31: // используется в LLVM для перемещения непонятно чего
	default:
		return -1;
		break;
	case R_ARM_TARGET1:// появляется только в разделе .init и .fini
	case R_ARM_ABS32: {//  (S + A) | T
		uint32_t insn = *(uint32_t*)address;
		//printf(" insn= %08x ", insn);
		uint32_t addend = symbols[n].st_value + increment;
		insn = ARM_THM_ABS32_BDEP(insn, addend);
		if(0)printf(" = %08x seg=%d\n", insn, symbols[n].st_shndx);
		*(uint32_t *)address = insn;
	} break;
#if 0
	case R_ARM_THM_MOVW_ABS_NC: {// LLVM для загрузки адреса в регистр, две инструкции (S + A) | T
		uint32_t insn = *(uint32_t*)address;
		uint32_t addend = symbols[n].st_value + increment;
		if ((symbols[n].st_info&0xFF)==STT_FUNC) addend |= 1;
		*(uint32_t*)address = ARM_THM_MOVW_BDEP(insn, addend);// bit deposition MOVW :lower16
	} break;
	case R_ARM_THM_MOVT_ABS: {// LLVM для загрузки адреса в регистр, две инструкции (S + A)
		uint32_t insn = *(uint32_t*)address;
		uint32_t addend = symbols[n].st_value + increment;
		*(uint32_t*)address = ARM_THM_MOVT_BDEP(insn, addend);// bit deposition MOVT :upper16
	} break;
	case R_ARM_THM_JUMP19: { // Thumb32: ((S + A) | T) - P   imm32 = SignExtend(S:J2:J1:imm6:imm11:'0', 32);
		uint32_t insn = *(uint32_t*)address;
		uint32_t addend = symbols[n].st_value + increment-(segment_addr + rel->r_offset +4);
		*(uint32_t*)address = ARM_THM_JUMP19_BDEP(insn, addend);
	} break;
#endif
	case R_ARM_THM_JUMP24: /*{ // Thumb32: ((S + A) | T) - P
		uint32_t insn = *(uint32_t*)address;
		uint32_t addend = symbols[n].st_value + increment-(segment_addr + rel->r_offset +4);
		insn = ARM_THM_JUMP24_BDEP(insn, addend);
		printf(" = %04X %04X\n", insn&0xFFFF, (insn>>16)&0xFFFF );
		*(uint32_t*)address = insn;
	} break;*/
	case R_ARM_THM_CALL: { // Thumb32: ((S + A) | T) - P
		uint32_t insn = *(uint32_t*)address;
//		printf(" = %04X %04X s.val=%08x sec=%d ", insn&0xFFFF, (insn>>16)&0xFFFF , symbols[n].st_value, symbols[n].st_shndx);
		uint32_t addend = symbols[n].st_value + increment-(segment_addr + rel->r_offset +4);
		insn = ARM_THM_CALL_BDEP(insn, addend);
		if (0) printf(" = %04X %04X seg=%d \n", insn&0xFFFF, (insn>>16)&0xFFFF, symbols[n].st_shndx);
		*(uint32_t*)address = insn;
	} break;
	}
	return 0;
}

const Names_t snt_names[] = {
	{SHT_NULL, 		"NULL"},
	{SHT_PROGBITS, 	"PROGBITS"},
	{SHT_SYMTAB, 	"SYMTAB"},
	{SHT_STRTAB, 	"STRTAB"},
	{SHT_RELA, 		"RELA"},
	{SHT_HASH, 		"HASH"},
	{SHT_DYNAMIC, 	"DYNAMIC"},
	{SHT_NOTE, 		"NOTE"},
	{SHT_NOBITS, 	"NOBITS"},
	{SHT_REL, 		"REL"},
	{SHT_SHLIB, 	"SHLIB"},
	{SHT_DYNSYM, 	"DYNSYM"},
	{SHT_INIT_ARRAY, 		"INIT_ARRAY"},
	{SHT_FINI_ARRAY, 		"FINI_ARRAY"},
	{SHT_ARM_EXIDX, 		"ARM_EXIDX"},
	{SHT_ARM_ATTRIBUTES, 	"ARM_ATTR"},
};
const Names_t shn_names[] = {
	{SHN_UNDEF,  "UND"},
	{SHN_ABS, 	 "ABS"},
	{SHN_COMMON, "COM"},
};
const Names_t stb_names[] = {
	{STB_LOCAL,  "LOCAL"},
	{STB_GLOBAL, "GLOBAL"},
	{STB_WEAK,   "WEAK"},
};
const Names_t type_names[] = {
	{ET_NONE,  "No file type"},
	{ET_REL,   "Relocatable file"},
	{ET_EXEC,  "Executable file"},
	{ET_DYN,   "Shared object file"},
	{ET_CORE,  "Core file"},
};
const Names_t stt_names[] = {
	{STT_NOTYPE,  "NOTYPE"},
	{STT_OBJECT,  "OBJECT"},
	{STT_FUNC,    "FUNC"},
	{STT_SECTION, "SECTION"},
	{STT_FILE,    "FILE"},
};
enum {
	DT_NULL=0,
	DT_NEEDED,
	DT_PLTRELSZ,
	DT_PLTGOT,
	DT_HASH,
	DT_STRTAB,
	DT_SYMTAB,
	DT_RELA,
	DT_RELASZ,
	DT_RELAENT,
	DT_STRSZ,
	DT_SYMENT,
	DT_INIT,
	DT_FINI,
	DT_SONAME,
	DT_RPATH,
	DT_SYMBOLIC,
	DT_REL,
	DT_RELSZ,
	DT_RELENT,
	DT_PLTREL,
	DT_DEBUG,
	DT_TEXTREL,
	DT_JMPREL,
	DT_RELCOUNT = 0x6ffffffa,
};
const Names_t dt_names[] = {
	{DT_NULL,    "NULL"},
	{DT_NEEDED,  "NEEDED"},
	{DT_PLTRELSZ,"PLTRELSZ"},
	{DT_PLTGOT,  "PLTGOT"},
	{DT_HASH,  	 "HASH"},
	{DT_STRTAB,  "STRTAB"},
	{DT_SYMTAB,  "SYMTAB"},
	{DT_RELA,    "RELA"},
	{DT_RELASZ,  "RELASZ"},
	{DT_RELAENT, "RELAENT"},
	{DT_STRSZ,   "STRSZ"},
	{DT_SYMENT,  "SYMENT"},
	{DT_INIT,  	 "INIT"},
	{DT_FINI,  	 "FINI"},
	{DT_SONAME,  "SONAME"},
	{DT_RPATH,   "RPATH"},
	{DT_SYMBOLIC,"SYMBOLIC"},
	{DT_REL,     "REL"},
	{DT_RELSZ,   "RELSZ"},
	{DT_RELENT,  "RELENT"},
	{DT_PLTREL,  "PLTREL"},
	{DT_DEBUG,   "DEBUG"},
	{DT_TEXTREL, "TEXTREL"},
	{DT_JMPREL,  "JMPREL"},
	{DT_RELCOUNT,"RELCOUNT"},
};

// bsearch(uint32_tconst void *key, const void *base, size_t num, size_t size,
//              int (*cmp)(const void *key, const void *))
static 
const char * get_name(uint32_t key, const Names_t * names, size_t num)
{
	size_t l = 0, u = num;
	while (l < u) {
		register const size_t mid = (l + u)>>1;
		register int result = key - names[mid].key;
		if (result < 0)
			u = mid;
		else if (result > 0)
			l = mid + 1;
		else
			return names[mid].name;
	}
	return NULL;
}
#define NAMES(arr) arr, sizeof(arr)/sizeof(Names_t)

typedef struct _MainOptions MainOptions;
struct _MainOptions {
    char * input_file;
    char * output_file;
    char * config_file;
    char * kernel;
    int file_header  ;
    int program_headers  ;
    int section_headers  ;
    int relocations  ;
    int symbols  ;
    int comments;
    int all  ;
    int verbose  ;
    int version  ;
    int overwrite;
};
static MainOptions options = {
    .input_file = NULL,
    .output_file = NULL,
    .config_file = "a.conf",
    .kernel = "test",
};
static GOptionEntry entries[] =
{
  { "input",  'i', 0, G_OPTION_ARG_FILENAME, &options.input_file,  "input ELF32 filename", 	"*.elf, *.so, *.exe, *.o" },
  { "output", 'o', 0, G_OPTION_ARG_FILENAME, &options.output_file, "output ELF32 filename", "*.o" },
  { "config", 'c', 0, G_OPTION_ARG_FILENAME, &options.config_file, "config file name", 		"*.conf" },
  { "all", 'a', 0,  G_OPTION_ARG_NONE, &options.all, "show all info", NULL },
  { "file-header", 'h', 0,  G_OPTION_ARG_NONE, &options.file_header, "show ELF file header", NULL },
  { "program-headers", 'l', 0,  G_OPTION_ARG_NONE, &options.program_headers, "show program headers", NULL },
  { "section-headers", 'S', 0,  G_OPTION_ARG_NONE, &options.section_headers, "show section headers", NULL },
  { "relocs",  'r', 0,  G_OPTION_ARG_NONE, &options.relocations, "show relocations", NULL },
  { "symbols", 's', 0,  G_OPTION_ARG_NONE, &options.symbols, "show symbol table", NULL },
  { "comments",'C', 0,  G_OPTION_ARG_NONE, &options.comments, "show commens", NULL },
  { "overwrite", 'O', 0,  G_OPTION_ARG_NONE, &options.overwrite, "overwtite output", NULL },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &options.verbose, "Be verbose", NULL },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &options.version, "program info", NULL },
  { NULL }
};
int g_str_has_suffix(const char  *str, const char  *suffix)
{
	int str_len = strlen(str);
	int suffix_len = strlen(suffix);
	if (str_len < suffix_len)
		return 0;
	return strcmp (str + str_len - suffix_len, suffix) == 0;
}

static size_t ar_size2ul (char* s, char** tail, int n)
{
	size_t v = 0;
	int i;
	for(i=0;i<n;i++){
		if (s[i]<='9' && s[i]>='0') {
			v = v*10  + (s[i] -'0');
		} else break;
	}
	if (tail) *tail = s+i;
	return v;
}
uint32_t ntohl(uint32_t x)
{
	return __builtin_bswap32(x);
}
int main (int argc, char *argv[])
{
	ElfHeader_t elf_header;
	
	GOptionContext *opt_context;
    opt_context = g_option_context_new ("- command line interface");
    g_option_context_add_main_entries (opt_context, entries, NULL/*GETTEXT_PACKAGE*/);
    if (!g_option_context_parse (opt_context, &argc, &argv, NULL))
    {
        //printf ("option parsing failed: %s\n", error->message);
        _Exit(1);
    }
    g_option_context_free (opt_context);
	if (options.version) {
		printf ("ELF v0.03 (c) Anatoly Georgievski\n");
	}
	if (options.input_file==NULL) return 0;
	FILE* fp = fopen(options.input_file, "rb");
	if (fp==NULL) return 0;
	if (g_str_has_suffix(options.input_file, ".a")){// библиотеки

#define  ARMAG   "!<arch>\n"  /* magic string */
#define  SARMAG  8            /* length of magic string */ 
#define  ARFMAG    "`\n"      /* header trailer string */ 

struct _AR_hdr {              /* file member header */ 
	char    ar_name[16];      /* '/' terminated file member name */ 
	char    ar_date[12];      /* file member date */ 
	char    ar_uid[6];        /* file member user identification */ 
	char    ar_gid[6];        /* file member group identification */ 
	char    ar_mode[8];       /* file member mode (octal) */ 
	char    ar_size[10];      /* file member size */ 
	char    ar_fmag[2];       /* header trailer string */ 
}; 
typedef struct _Elf_Arsym Elf_Arsym_t;
struct _Elf_Arsym {
	char * as_name;
	off_t  as_off;
	unsigned long as_hash;
};

		struct _AR_hdr file_hdr;
		char str[16];
		size_t size = fread( str, 1, SARMAG, fp);
		if (size!=SARMAG || strncmp(str, ARMAG, SARMAG)!=0) return 0;
		printf("AR file '%s'\n", options.input_file);
		char* ar_symbols=NULL;
		char* ar_strings=NULL;
		Elf_Arsym_t * ar_symtab = NULL;
		size = fread(&file_hdr, 1, sizeof(struct _AR_hdr), fp);
		if (file_hdr.ar_name[0]=='/' && file_hdr.ar_name[1]==' ') {
			printf(" symbols: '%-16.16s', size=%10.10s\n", file_hdr.ar_name, file_hdr.ar_size);
			size = ar_size2ul(file_hdr.ar_size, NULL, 10);
			ar_symbols = malloc(size);
			size = fread(ar_symbols, 1, size, fp);
			int ar_sym_num = ntohl(*(uint32_t*)ar_symbols);
			printf(" ..count = %d\n", ar_sym_num);
			ar_symtab = malloc((ar_sym_num)* sizeof(Elf_Arsym_t));
			int i;
			uint32_t *ar_offtab = (void*)(ar_symbols+4);
			for (i=0;i<ar_sym_num; i++) {
				ar_symtab[i].as_off = ntohl(ar_offtab[i]);
//				ar_symtab[i].as_name = NULL;
			}
			//ar_symtab[i].as_off=0;
			char * s = (void*)&ar_offtab[i];
			for (i=0; i<ar_sym_num; i++) {
				ar_symtab[i].as_name = s;
				ar_symtab[i].as_hash = elf_hash(s);
				printf(" %4d: %-32.32s : %d\n", i, s, ar_symtab[i].as_off);
				s+=strlen(s)+1;
			}
			//ar_symtab[i].as_name = NULL;
			/*... На этом разбор библиотеки в формате AR закончен
				В результате можно создать хеш таблицу для быстрого доступа к элементам массива
				
			 */
		} else return 0;
		size = fread(&file_hdr, 1, sizeof(struct _AR_hdr), fp);
		if (file_hdr.ar_name[0]=='/' && file_hdr.ar_name[1]=='/' && file_hdr.ar_name[2]==' ') {
			printf(" strings: '%-16.16s', size=%10.10s\n", file_hdr.ar_name, file_hdr.ar_size);
			size = ar_size2ul(file_hdr.ar_size, NULL, 10);
			ar_strings = malloc(size);
			size = fread(ar_strings, 1, size, fp);
			
		} else {
			printf(" fail next header %-10.10s\n", file_hdr.ar_name);
			return 0;
		}
	
		int count=1000;
		while (!feof(fp) && --count){
			size = fread(&file_hdr, 1, sizeof(struct _AR_hdr), fp);
			if (size!= sizeof(struct _AR_hdr)) break;
			char* name;
			int name_size;
			if (file_hdr.ar_name[0]=='/'){
				uint32_t off = ar_size2ul(file_hdr.ar_name+1, NULL, 10);
				name = ar_strings+off;
				char* s = name;
				while (s[0]!='/' && s[1]!='\n')s++;
				name_size = s-name+1;
			} else {
				name = file_hdr.ar_name;
				name_size = 16;
			}
			fpos_t pos;
			fgetpos(fp, &pos);
			printf(" file: '%-36.*s': %-6d size=%10.10s\n", name_size, name, pos-sizeof(struct _AR_hdr), file_hdr.ar_size);
			size = ar_size2ul(file_hdr.ar_size, NULL, 10);
			pos += size;// символ завешения
			if (fsetpos(fp, &pos)!=0) break;
		}
		if (ar_symtab) free(ar_symtab);
		if (ar_symbols)free(ar_symbols);
		if (ar_strings)free(ar_strings);
		return 0;
	}
	
	size_t size = fread( &elf_header, 1, sizeof(ElfHeader_t), fp);
	if (!(size==sizeof(ElfHeader_t))) return 0;
	if (!(elf_header.e_ident.magic==EI_MAGIC)) return 0;
/*!
Порядок разбора:
1. Выделить заголовок формата, убедиться что это ELF32 для данной платформы, перемещаемый, загружаемый и т.д отвечает цели. Содержит таблицу секций и/или таблицу программ.
2. Загрузить таблицу секций.
*/
	if (options.file_header || options.all) {// отобразить заголовок
		printf("ELF Header\n");
		printf("\tMagic:\t\t%08X\n",elf_header.e_ident.magic);
		printf("\tClass:\t\t%s\n",  elf_header.e_ident.class==1?"ELF32": "ELF64");
		printf("\tData:\t\t%s\n",   elf_header.e_ident.data==1?"LSB": "MSB");
		printf("\tVersion:\t%s\n",  elf_header.e_ident.version==1?"1 (current)": "unknown");
		printf("\tOS/ABI:\t\t%s\n", elf_header.e_ident.os_abi==0?"UNIX - System V":"unknown");
		printf("\tABI version:\t%d\n", elf_header.e_ident.abi_version);
		printf("\tType:\t\t(%d) %s\n", elf_header.e_type, get_name(elf_header.e_type, NAMES(type_names)));
		printf("\tMachine:\t(%d) %s\n",   elf_header.e_machine, elf_header.e_machine==40?"ARM":"unknown");
		printf("\tVersion:\t%d\n",        elf_header.e_version);
		printf("\tProgram offset:\t%d\n", elf_header.e_phoff);
		printf("\tSection offset:\t%d\n", elf_header.e_shoff);
		printf("\tFlags:\t\t0x%08X\n",    elf_header.e_flags);//  Version5 EABI
		printf("\tThis hdr size:\t%d\n",  elf_header.e_ehsize);
		printf("\tProgram header size:\t%d\n",   	elf_header.e_phentsize);
		printf("\tNumber of Program hdrs:\t%d\n",   elf_header.e_phnum);
		printf("\tSection header size:\t%d\n",   	elf_header.e_shentsize);
		printf("\tNumber of Section hdrs:\t%d\n",   elf_header.e_shnum);
		printf("\tSection hdr str table:\t%d\n",   	elf_header.e_shstrndx);
	}

	Elf32_Shdr_t *shdr=NULL; // таблица секций
	char* shstrtab=NULL; // таблица имен секций
	if (1 && elf_header.e_shoff!=0 && elf_header.e_shentsize==sizeof(Elf32_Shdr_t)) {// загрузить заголовки секций
		shdr = elf_section_load(fp, elf_header.e_shoff, elf_header.e_shnum * sizeof(Elf32_Shdr_t));
/*		fpos_t fpos = elf_header.e_shoff;
		fsetpos(fp, &fpos);
		shdr = calloc(elf_header.e_shnum, sizeof(Elf32_Shdr_t));
		size = fread(shdr, elf_header.e_shnum, sizeof(Elf32_Shdr_t), fp); */
		if (elf_header.e_shstrndx) {// таблица строк может отсутствовать
			shstrtab = elf_section_load(fp, shdr[elf_header.e_shstrndx].sh_offset, shdr[elf_header.e_shstrndx].sh_size);
/*			shstrtab = malloc(shdr[elf_header.e_shstrndx].sh_size);
			fpos = shdr[elf_header.e_shstrndx].sh_offset;
			fsetpos(fp, &fpos);
			size = fread(shstrtab, 1, shdr[elf_header.e_shstrndx].sh_size, fp);*/
		}
		if (options.section_headers || options.all) {// отобразить таблицу секций
			printf("Section headers (%d):\n"
			// [Нм] Имя               Тип             Адрес    Смещ   Разм   ES Флг Сс Инф Al
			 " [No] Name                 Type       Address  Offset Size   ES Flg Ln Inf Al\n"
			, elf_header.e_shnum);
			int i;
			for (i=0; i<elf_header.e_shnum; i++){
				printf(" [%2d] %-20s %-10.10s %08x %06x %06x %02x %c%c%c %2d %3d %2d\n", 
					i, shstrtab+shdr[i].sh_name, 
					get_name(shdr[i].sh_type, NAMES(snt_names)),
					shdr[i].sh_addr, shdr[i].sh_offset, shdr[i].sh_size,
					shdr[i].sh_entsize,
					(shdr[i].sh_flags & SHF_WRITE)?'W':' ',
					(shdr[i].sh_flags & SHF_ALLOC)?'A':' ',
					(shdr[i].sh_flags & SHF_EXECINSTR)?'X':' ',
					shdr[i].sh_link,
					shdr[i].sh_info,
					shdr[i].sh_addralign);
			}
		}
	}
	if ((options.program_headers || options.all) && elf_header.e_phoff!=0) {// загрузить заголовки программ
		       //Тип        Смещ.    Вирт.адр   Физ.адр    Рзм.фйл Рзм.пм  Флг Выравн
		printf("Program headers (%d):\n"
			   " Type       Offset   Virt.addr  Phys.addr  File sz Mem.sz  Flg Align\n",
			   elf_header.e_phnum);
		Elf32_Phdr_t* phdr= elf_section_load(fp, elf_header.e_phoff, elf_header.e_phnum * sizeof(Elf32_Phdr_t));
/*		fpos_t fpos = elf_header.e_phoff;
		fsetpos(fp, &fpos);
		Elf32_Phdr_t phdr[elf_header.e_phnum];
		size = fread(&phdr[0], elf_header.e_phnum, sizeof(Elf32_Phdr_t), fp);*/
		int i;
		for (i=0; i<elf_header.e_phnum; i++){
			printf(" %-10s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n", 
				get_name(phdr[i].p_type, NAMES(pt_names)),  //DYNAMIC
				phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_paddr,
				phdr[i].p_filesz, phdr[i].p_memsz,
				(phdr[i].p_flags & PF_R)?'R':'-',
				(phdr[i].p_flags & PF_W)?'W':'-',
				(phdr[i].p_flags & PF_X)?'X':'-',
				phdr[i].p_align);
		}
		free(phdr);
	}

int section_id(int type_id, char* name)
{
	int i;
	if (shdr==NULL) return -1;
	for (i=0; i< elf_header.e_shnum; i++){
		if (shdr[i].sh_type == type_id && strcmp(name, shstrtab+shdr[i].sh_name)==0) return i;
	}
	return -1;
}
int section_index(const char* name)
{
	int i;
	if (shdr==NULL) return -1;
	for (i=0; i< elf_header.e_shnum; i++){
		if (strcmp(shstrtab+shdr[i].sh_name, name)==0) return i;
	}
	return -1;
}
	int idx;
	if (options.comments) {
		idx = section_index(".comment");
		if (idx >= 0) {
			printf("Comment (%d):\n", shdr[idx].sh_size);
			char* comment = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
//			if (*comment=='\0') comment++;
			printf("%s\n",comment+1);
			free(comment);
			
		}
	}
	ElfHash_t*	 htable  = NULL;
	Elf32_Dyn_t* tags    = NULL;
	Elf32_Sym_t* symbols = NULL;
	char* 		 strings = NULL; 
	idx = section_id(SHT_HASH, 	 ".hash");
	if (idx >= 0) {
		htable  = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	}	
	idx = section_id(SHT_DYNAMIC, ".dynamic");
	if (idx >= 0){
		tags = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	}
	if ((options.symbols || options.all) && tags!=NULL) {
		printf("Dynamyc table '%s' contains %d items:\n"
		//"Чис:    Знач   Разм Тип     Связ   Vis      Индекс имени"
		" Tag                Name/Value\n", 
		shstrtab+shdr[idx].sh_name,
		shdr[idx].sh_size/sizeof(Elf32_Dyn_t));

		int i;
		for (i=0; i<shdr[idx].sh_size/sizeof(Elf32_Dyn_t); i++){
			const char* name = get_name(tags[i].d_tag, NAMES(dt_names));
			printf("  %-20.20s %x\n", name, tags[i].d_un.d_val);
			if (tags[i].d_tag==DT_NULL) break;
		}
	}
		
	idx = section_id(SHT_STRTAB, ".strtab");
	if (idx >= 0) {
		strings = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	}	
	idx = section_id(SHT_STRTAB, ".dynstr");
	if (idx >= 0) {
		strings = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	}	
	idx = section_id(SHT_DYNSYM, ".dynsym");// по типу искать
	if (idx >= 0) {
		symbols = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
	} else {
		idx = section_id(SHT_SYMTAB, ".symtab");// по типу искать
		if (idx >= 0) {
			symbols = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);
		}
	}
	// отдельно строить динамическую таблицу и таблицу симвлов
	if ((options.symbols || options.all) && symbols!=NULL && strings!=NULL) {// построить таблицу символов
		printf("Symbol table '%s' contains %d items:\n"
		//"Чис:    Знач   Разм Тип     Связ   Vis      Индекс имени"
		" No: Value    Size Type    Bind   Vis      Idx Name\n", 
		shstrtab+shdr[idx].sh_name,
		shdr[idx].sh_size/sizeof(Elf32_Sym_t));
		char buf[12];
		int i;
		for (i=0; i<shdr[idx].sh_size/sizeof(Elf32_Sym_t); i++){
			const char* shn = get_name(symbols[i].st_shndx, NAMES(shn_names));
			if (shn==NULL) shn = itoa(symbols[i].st_shndx, buf, 10);
			printf("%3d: %08x %4d %-7.7s %-6.6s %-3.3s %-24.24s\n", i,
			symbols[i].st_value, symbols[i].st_size, 
			get_name(ELF32_ST_TYPE(symbols[i].st_info), NAMES(stt_names)),
			get_name(ELF32_ST_BIND(symbols[i].st_info), NAMES(stb_names)),
			shn,// ABS UND COM #
			ELF32_ST_TYPE(symbols[i].st_info)==STT_SECTION? shstrtab+shdr[symbols[i].st_shndx].sh_name: strings+symbols[i].st_name);
			
			// проверить hash
			if (0 && symbols[i].st_name!=0 && htable!=NULL) {
				uint32_t k = elf_hashtable_lookup(htable, strings+symbols[i].st_name, symbols,strings);
				if (k==STN_UNDEF) printf("hash fail..\n");
				else printf("hash '%s' ..ok\n", strings+symbols[k].st_name);
			}
		}
		printf("\n");
	}
	if ((options.relocations || options.all) && shdr!=NULL && shstrtab!=NULL && symbols!=NULL)// relocate sections
	for (idx=0; idx< elf_header.e_shnum; idx++) {
		if (shdr[idx].sh_type == SHT_REL) {// для каждой секци подобного типа
			printf("Relocatable section '%s' at offset 0x%x contains %d items:\n"
				//" Смещение   Инфо    Тип             Знач.симв  Имя симв."
				  " Offset     Info    Type            Val.symbol Name symb.\n",
				shstrtab+shdr[idx].sh_name, 
				shdr[idx].sh_offset, shdr[idx].sh_size/sizeof(Elf32_Rel_t));
			Elf32_Rel_t * rel = elf_section_load(fp, shdr[idx].sh_offset, shdr[idx].sh_size);

			uint8_t* segment = NULL;
			uint32_t segment_addr = 0;
			if (strcmp(shstrtab+shdr[idx].sh_name, ".rel.dyn")==0 ){
				//section_id(SHT_PROGBITS, name+4)
			} else
			if (shdr[idx-1].sh_type == SHT_PROGBITS) {// можно найти путем section_id(SHT_PROGBITS, name+4)
				segment = elf_section_load(fp, shdr[idx-1].sh_offset, shdr[idx-1].sh_size);
				segment_addr = shdr[idx-1].sh_addr;
			}
			int i;
			for (i=0; i< shdr[idx].sh_size/sizeof(Elf32_Rel_t); i++){
				int n = ELF32_R_SYM(rel[i].r_info);// идентификатор символа
				printf(" %08x %08x %-18.18s %08x %-6.6s %2d: %-24.24s\n", 
					rel[i].r_offset, rel[i].r_info, 
					get_name(rel[i].r_info & 0xFF, NAMES(rel_type_names)),
					symbols[n].st_value,
					get_name(ELF32_ST_TYPE(symbols[n].st_info), NAMES(stt_names)),
					symbols[n].st_shndx,
					ELF32_ST_TYPE(symbols[n].st_info)==STT_SECTION? 
						shstrtab+shdr[symbols[n].st_shndx].sh_name:
						strings +symbols[n].st_name
				);
				if (segment ) {// && symbols[i].st_shndx != SHN_UNDEF
					int n = ELF32_R_SYM(rel[i].r_info);
					uint32_t symbol_value = symbols[n].st_value;
					if (symbols[n].st_shndx < elf_header.e_shnum) {
						symbol_value += shdr[symbols[n].st_shndx].sh_addr;// адрес сегмента в котором расположен символ
					} else 
					if (symbols[n].st_shndx==SHN_UNDEF) 
						continue;
					else {
						// ABS COM
					}					
					elf_arm_relocate_fast(rel+i, symbol_value, segment, segment_addr);
//					printf(" -- elf_arm_relocate_fast\n");
				}
			}
			printf("\n");
			if (segment) free(segment);
			free(rel);
		}
	}
	free(shstrtab);
	free(symbols);
	free(strings);
	free(shdr);
	fclose(fp);
	fflush(stdout);
	return 0;
}

#endif
