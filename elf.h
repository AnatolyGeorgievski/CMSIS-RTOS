#ifndef ELF32_H
#define ELF32_H
#include <stdint.h>

#define EI_MAGIC (0x464C457F)
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFOSABI_NONE 	0 // Unix System V
#define ELFOSABI_SYSV   0        /* Alias.  */
#define ELFOSABI_GNU 	3
#define ELFOSABI_LINUX  ELFOSABI_GNU
#define ELFOSABI_HURD  	4		/* GNU/Hurd */
#define ELFOSABI_SOLARIS 	6
#define ELFOSABI_FREEBSD    9	/* FreeBSD.  */
#define ELFOSABI_ARM_AEABI  64	/* ARM EABI */
#define ELFOSABI_ARM        97  /* ARM */
#define ELFOSABI_STANDALONE	255 /* Standalone (embedded) application */

#define EM_386		3		/* Intel 80386 */
#define EM_PPC    	20      /* PowerPC */
#define EM_PPC64	21		/* 64-bit PowerPC */
#define EM_ARM    	40      /* ARM */
#define EM_X86_64	62		/* AMD x86-64 */
#define EM_AARCH64 	183        /* ARM AARCH64 */
#define EM_AVR32  	185        /* Amtel 32-bit microprocessor */
#define EM_STM8   	186        /* STMicroelectronics STM8 */
#define EM_MCHP_PIC	204        /* Microchip 8-bit PIC(r) */
#define EM_AMDGPU	224        /* AMD GPU */
#define EM_BPF		247        /* Linux BPF -- in-kernel virtual machine */
#define EM_NUM                253

#define ET_NONE 	0 //!< No file type
#define ET_REL 		1 //!< Relocatable file
#define ET_EXEC 	2 //!< Executable file
#define ET_DYN 		3 //!< Shared object file
#define ET_CORE 	4 //!< Core file
#define ET_LOPROC 0xff00 //!< Processor-specific
#define ET_HIPROC 0xffff //!< Processor-specific

typedef uint32_t Elf32_Word;
typedef  int32_t Elf32_Sword;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off; //!< смещение 
typedef uint16_t Elf32_Half;

typedef struct _ElfHeader32 ElfHeader_t;
typedef struct _ElfHash ElfHash_t;
typedef struct _Elf32_Shdr Elf32_Shdr_t;

struct _ElfHash {
	Elf32_Word nbucket;
	Elf32_Word nchain;
	Elf32_Word bucket[0];// два массива подряд =nbucket+nchain
};
struct _ElfHeader32 {
	struct {
		uint32_t magic;	// 0x7f 0x45 0x4c 0x46
		uint8_t class;	// 1- 32 bit, 2- 64 bit
		uint8_t data;	// 1- little endian, 2- big endian
		uint8_t version;// 1- current
		uint8_t os_abi;
		uint8_t abi_version;
		uint8_t pad[7];
	} e_ident;
	Elf32_Half e_type;		//!< 1 - перемещаемый, 2 - исполняемый, 3 - разделяемый, 4 - ядро, 
	Elf32_Half e_machine;	//!< 40 - ARM, 62 - x86-64
	Elf32_Word e_version;	//!< 1 - current
	Elf32_Addr e_entry;		//!< точка входа
	Elf32_Off  e_phoff;		//!< смещение таблицы заголовков программы
	Elf32_Off  e_shoff;		//!< смещение таблицы заголовков секций
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;	//!< размер заголовка 56 или 64
	Elf32_Half e_phentsize; //!< размер одного заголовка программы 32 или 56
	Elf32_Half e_phnum;		//!< число заголовков программы
	Elf32_Half e_shentsize;	//!< размер одного заголовка секции 40 или 64
	Elf32_Half e_shnum;		//!< число заголовков секций
	Elf32_Half e_shstrndx;	//!< индекс записи в талице заголовков, названия секций .shstrtab или 0.
};

/*! Таблица заголовков разделов .shsymtab */
struct _Elf32_Shdr {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
};
/*! Таблица символов .symtab */
typedef struct _Elf32_Sym  Elf32_Sym_t;
struct _Elf32_Sym {
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
};
/*! Таблица перемещений .rel */
typedef struct _Elf32_Rel Elf32_Rel_t;
struct _Elf32_Rel {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
};
/*! Таблица перемещений .rela */
typedef struct _Elf32_Rela Elf32_Rela_t;
struct _Elf32_Rela {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
};
/*! Таблица программ */
typedef struct _Elf32_Phdr Elf32_Phdr_t;
struct _Elf32_Phdr {
	Elf32_Word p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
};
/*! Таблица динамических параметров */
typedef struct _Elf32_Dyn Elf32_Dyn_t;
struct _Elf32_Dyn {
	Elf32_Sword d_tag;
	union {
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
};

typedef struct _Elf32_Chdr Elf32_Chdr_t;
struct _Elf32_Chdr {
  Elf32_Word        ch_type;        /* Compression format.  */
  Elf32_Word        ch_size;        /* Uncompressed data size.  */
  Elf32_Word        ch_addralign; 	/* Uncompressed data alignment.  */
};
#define ELFCOMPRESS_ZLIB        1	/* ZLIB/DEFLATE algorithm.  */


#define ELF32_ST_BIND(i) ((i)>>4)
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOPROC 13
#define STB_HIPROC 15

#define ELF32_ST_TYPE(i) ((i)&0xF)
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 13
#define STT_HIPROC 15

/* Additional symbol types for Thumb.  */
#define STT_ARM_TFUNC                STT_LOPROC /* A Thumb function.  */
#define STT_ARM_16BIT                STT_HIPROC /* A Thumb label.  */
/* ARM-specific program header flags */
#define PF_ARM_SB 	0x10000000 /* Segment contains the location addressed by the static base. */
#define PF_ARM_PI   0x20000000 /* Position-independent segment.  */
#define PF_ARM_ABS  0x40000000 /* Absolute segment.  */


/*
Обозначения флагов:
  W (запись), A (назнач), X (исполняемый), M (слияние), S (строки),
  I (инфо), L (порядок ссылок), O (требуется дополнительная работа ОС),
  G (группа), T (TLS), C (сжат), x (неизвестно), o (специфич. для ОС), 
  E (исключён), y (чистый код), p (processor specific)
*/
#define SHF_WRITE 		0x01 /* Writable */
#define SHF_ALLOC 		0x02 /* Occupies memory during execution */
#define SHF_EXECINSTR 	0x04 /* Executable */
#define SHF_MERGE		0x10 /* Might be merged */
#define SHF_STRINGS		0x20 /* Contains nul-terminated strings */
#define SHF_INFO_LINK 	0x40 /* 'sh_info' contains SHT index */
#define SHF_LINK_ORDER 	0x80 /* Preserve order after combining */
#define SHF_OS_NONCONFORMING	0x100 /* Non-standard OS specific handling required */
#define SHF_GROUP		0x200 /* Section is member of a group.  */
#define SHF_TLS			0x400 /* Section hold thread-local data.  */
#define SHF_COMPRESSED	0x800 /* Section with compressed data. */
#define SHF_MASKOS 		0x0FF00000 /* OS-specific.  */
#define SHF_MASKPROC 	0xF0000000 /* Processor-specific */
#define SHF_ORDERED 	0x40000000
#define SHF_EXCLUDE 	0x80000000

#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xF))

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define SHN_UNDEF 	0
#define SHN_ABS 	0xFFF1
#define SHN_COMMON 	0xFFF2

#define PF_R 4
#define PF_W 2
#define PF_X 1

#define PT_NULL 	0
#define PT_LOAD 	1
#define PT_DYNAMIC 	2
#define PT_INTERP  	3
#define PT_NOTE    	4
#define PT_SHLIB   	5
#define PT_SHDR    	6
#define PT_TLS      7               /* Thread local storage segment */
#define PT_ARM_ARCHEXT 	0x70000000
#define PT_ARM_EXIDX 	0x70000001

// Section Types, sh_type
#define SHT_NULL		0
#define SHT_PROGBITS	1
#define SHT_SYMTAB		2
#define SHT_STRTAB		3
#define SHT_RELA		4
#define SHT_HASH		5
#define SHT_DYNAMIC		6
#define SHT_NOTE		7
#define SHT_NOBITS		8
#define SHT_REL			9
#define SHT_SHLIB		10
#define SHT_DYNSYM		11
#define SHT_NUM			12
#define SHT_INIT_ARRAY	14
#define SHT_FINI_ARRAY	15
#define SHT_PREINIT_ARRAY	16
#define SHT_GROUP			17
#define SHT_SYMTAB_SHNDX	18
#define SHT_RELR 			19                        // Relocation entries; only offsets.
#define SHT_LOOS 			0x60000000                // Lowest operating system-specific type.
   // Android packed relocation section types.
   // https://android.googlesource.com/platform/bionic/+/6f12bfece5dcc01325e0abba56a46b1bcf991c69/tools/relocation_packer/src/elf_file.cc#37
#define SHT_ANDROID_REL		0x60000001
#define SHT_ANDROID_RELA	0x60000002
#define SHT_LLVM_ODRTAB		0x6fff4c00         // LLVM ODR table.
#define SHT_LLVM_LINKER_OPTIONS		0x6fff4c01 // LLVM Linker Options.
#define SHT_LLVM_CALL_GRAPH_PROFILE	0x6fff4c02 // LLVM Call Graph Profile.
#define SHT_LLVM_ADDRSIG 	0x6fff4c03         // List of address-significant symbols
#define SHT_LLVM_DEPENDENT_LIBRARIES	0x6fff4c04 // LLVM Dependent Library Specifiers.
#define SHT_LLVM_SYMPART	0x6fff4c05      // Symbol partition specification.
#define SHT_LLVM_PART_EHDR	0x6fff4c06      // ELF header for loadable partition.
#define SHT_LLVM_PART_PHDR	0x6fff4c07      // Phdrs for loadable partition.
   // Android's experimental support for SHT_RELR sections.

#define SHT_GNU_ATTRIBUTES 	0x6ffffff5      // Object attributes.
#define SHT_GNU_HASH 		0x6ffffff6    	// GNU-style hash table.
#define SHT_GNU_verdef 		0x6ffffffd      // GNU version definitions.
#define SHT_GNU_verneed 	0x6ffffffe      // GNU version references.
#define SHT_GNU_versym 		0x6fffffff      // GNU symbol versions table.
#define SHT_HIOS 			0x6fffffff      // Highest operating system-specific type.

#define SHT_LOPROC				0x70000000
#define SHT_ARM_EXIDX    		0x70000001
#define SHT_ARM_PREEMPTMAP  	0x70000002
#define SHT_ARM_ATTRIBUTES 		0x70000003 // ARM: Object file compatibility attributes
#define SHT_ARM_DEBUGOVERLAY 	0x70000004
#define SHT_ARM_OVERLAYSECTION 	0x70000005
#define SHT_HIPROC				0x7FFFFFFF
/*! Спецификация типов перемещений для ARM */
enum {
	R_ARM_NONE			=0,
	R_ARM_ABS32			=2,
	R_ARM_THM_CALL		=10,
// Dynamic relocations
	R_ARM_GLOB_COPY		=20,
	R_ARM_GLOB_DAT		=21,
	R_ARM_JUMP_SLOT		=22,
	R_ARM_RELATIVE		=23,

	R_ARM_THM_JUMP24	=30,
	R_ARM_TARGET1		=38,
	R_ARM_THM_MOVW_ABS_NC=47,
	R_ARM_THM_MOVT_ABS	=48,
	R_ARM_THM_JUMP19	=51,
};

#endif // ELF32_H