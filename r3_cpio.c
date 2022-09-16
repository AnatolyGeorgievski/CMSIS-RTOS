/*!
	\defgroup _fs_init Начальная загрузка файловой иерархии. Пока загружается 
	есть ощущение что формат CPIO
	

	\subsection CPIO file format
     The cpio archive format collects any number of files, directories, and
     other file system objects (symbolic links, device nodes, etc.) into a
     single stream of bytes.

   General Format
     Each file system object in a cpio archive comprises a header record with
     basic numeric metadata followed by the full pathname of the entry and the
     file data.  The header record stores a series of integer values that gen-
     erally follow the fields in struct stat.  (See stat(2) for details.)  The
     variants differ primarily in how they store those integers (binary,
     octal, or hexadecimal).  The header is followed by the pathname of the
     entry (the length of the pathname is stored in the header) and any file
     data.  The end of the archive is indicated by a special record with the
     pathname ``TRAILER!!!''.

   PWB format
     XXX Any documentation of the original PWB/UNIX 1.0 format? XXX

   Old Binary Format
     The old binary cpio format stores numbers as 2-byte and 4-byte binary
     values.  Each entry begins with a header in the following format:

	   struct header_old_cpio {
		   unsigned short   c_magic;
		   unsigned short   c_dev;
		   unsigned short   c_ino;
		   unsigned short   c_mode;
		   unsigned short   c_uid;
		   unsigned short   c_gid;
		   unsigned short   c_nlink;
		   unsigned short   c_rdev;
		   unsigned short   c_mtime[2];
		   unsigned short   c_namesize;
		   unsigned short   c_filesize[2];
	   };

     The unsigned short fields here are 16-bit integer values; the unsigned
     int fields are 32-bit integer values.  The fields are as follows

     magic   The integer value octal 070707.  This value can be used to deter-
	     mine whether this archive is written with little-endian or big-
	     endian integers.

     dev, ino
	     The device and inode numbers from the disk.  These are used by
	     programs that read cpio archives to determine when two entries
	     refer to the same file.  Programs that synthesize cpio archives
	     should be careful to set these to distinct values for each entry.

     mode    The mode specifies both the regular permissions and the file
	     type.  It consists of several bit fields as follows:
	     0170000  This masks the file type bits.
	     0140000  File type value for sockets.
	     0120000  File type value for symbolic links.  For symbolic links,
		      the link body is stored as file data.
	     0100000  File type value for regular files.
	     0060000  File type value for block special devices.
	     0040000  File type value for directories.
	     0020000  File type value for character special devices.
	     0010000  File type value for named pipes or FIFOs.
	     0004000  SUID bit.
	     0002000  SGID bit.
	     0001000  Sticky bit.  On some systems, this modifies the behavior
		      of executables and/or directories.
	     0000777  The lower 9 bits specify read/write/execute permissions
		      for world, group, and user following standard POSIX con-
		      ventions.

     uid, gid
	     The numeric user id and group id of the owner.

     nlink   The number of links to this file.	Directories always have a
	     value of at least two here.  Note that hardlinked files include
	     file data with every copy in the archive.

     rdev    For block special and character special entries, this field con-
	     tains the associated device number.  For all other entry types,
	     it should be set to zero by writers and ignored by readers.

     mtime   Modification time of the file, indicated as the number of seconds
	     since the start of the epoch, 00:00:00 UTC January 1, 1970.  The
	     four-byte integer is stored with the most-significant 16 bits
	     first followed by the least-significant 16 bits.  Each of the two
	     16 bit values are stored in machine-native byte order.

     namesize
	     The number of bytes in the pathname that follows the header.
	     This count includes the trailing NUL byte.

     filesize
	     The size of the file.  Note that this archive format is limited
	     to four gigabyte file sizes.  See mtime above for a description
	     of the storage of four-byte integers.

     The pathname immediately follows the fixed header.  If the namesize is
     odd, an additional NUL byte is added after the pathname.  The file data
     is then appended, padded with NUL bytes to an even length.
*/
	   struct header_odc_cpio {
		   unsigned short   c_magic;
		   unsigned short   c_dev;
		   unsigned short   c_ino;
		   unsigned short   c_mode;
		   unsigned short   c_uid;
		   unsigned short   c_gid;
		   unsigned short   c_nlink;
		   unsigned short   c_rdev;
		   unsigned short   c_mtime[2];
		   unsigned short   c_namesize;
		   unsigned short   c_filesize[2];
	   };
/*
struct header_cpio {
char c_magic[6];// Octal number
char c_dev 6 Octal number
char c_ino 6 Octal number
char c_mode 6 Octal number
char c_uid 6 Octal number
char c_gid 6 Octal number
char c_nlink 6 Octal number
char c_rdev 6 Octal number
char c_mtime 11 Octal number
char c_namesize 6 Octal number
char c_filesize 11 Octal number
//c_name c_namesize Pathname string
//c_filedata c_filesize Data
*/
#include <stdint.h>
#include <cpio.h>
#include <sys/stdio.h>// Device_t, FILE, DIR...

typedef struct _File File_t;

#define ASN_CLASS_OPENING  0x6
#define ASN_CLASS_CLOSING  0x7
#define ASN_CLASS_CONTEXT  0x8

#define ASN_MASK 		0xF8				//!< Маска типа
#define LEN_MASK		0x07				//!< Кодирование длины
#define ASN_CONTEXT(n) (((n)<<4)|ASN_CLASS_CONTEXT)		//!< Контекстно зависимый тип
#define ASN_CONTEXT_ID(n) ((n)>>4)		//!< Контекстно зависимый тип
#define ASN_NULL		0x00				//!< Null или SKIP
#define ASN_UINT		0x20				//!< Null или SKIP
#define ASN_INT			0x30				//!< Число со знаком
#define ASN_OCTETS		0x60				//!< Строка бинарная
#define ASN_STRING		0x70				//!< Строка текстовая
#define ASN_DATE		0xA0				//!< Дата
#define ASN_TIME		0xB0				//!< Время
#define ASN_OID			0xC0				//!< Маска типа

#define RFS_SKIP		0xFF 
#define RFS_CREATE 		(ASN_CONTEXT(1)|ASN_CLASS_OPENING)
#define RFS_APPEND 		(ASN_CONTEXT(2)|ASN_CLASS_OPENING)
#define RFS_HTABLE		(ASN_CONTEXT(2)|ASN_CLASS_OPENING)

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define BE32(x) __builtin_bswap32(x)
#define BE16(x) __builtin_bswap16(x)
#else
#define BE32(x) (x)
#define BE16(x) (x)
#endif

static uint8_t* rfs_decode_u32(uint8_t* data, int type, uint32_t * value){
	*value = BE32(*(uint32_t*) data);
	return data+4;
}
static uint8_t* rfs_decode_u16(uint8_t* data, int type, uint16_t * value){
	*value = BE16(*(uint16_t*) data);
	return data+4;
}
static inline 
int rfs_decode_tag(uint8_t * data, int type){
	return ((data[0] & ASN_MASK)==type);
}
static inline uint8_t* _memmove_x64(uint8_t* dst, size_t off, size_t mlen)
{
    int i;
    for(i=0; i<(mlen); i++)
        dst[i] = dst[i-off];
	return dst+mlen;
}
// длины последовательностей обычно маленькие, этот вариант вероятно оптимальный
static inline uint8_t* _memcpy_x64(uint8_t* dst, uint8_t* s, size_t mlen)
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
	return dst;
}
static inline uint8_t* _memzero_x64(uint8_t* dst, size_t mlen)
{
    int i;
    if(1) {
		for(i=0; i<(mlen>>3); i++) {
			*(uint64_t*)dst = 0;
			dst+=8;
		}
		mlen&=7;
    }
    for(i=0; i<(mlen); i++)
        *dst++ = 0;
	return dst;
}
static 
uint8_t* rfs_decode_slen(uint32_t tag, uint8_t *data, size_t *slen)
{
    size_t len = tag & 7;
    if (len > 4) {
		int sz = 1<<(len-5);// число байт 1,2,4
		len = *(size_t*)data, data+=sz;
		len&= ~(~0UL <<(sz*8));// маска бит
	}
	*slen = len;
	return data;
}
static 
uint8_t* rfs_decode_uint(uint8_t *data, uint32_t tag, uint32_t *var, uint32_t default_value)
{
	switch (tag & 3) {
	case 0:*var = *data++; break;
	case 1:*var = *(uint16_t*)data, data+=2; break;
	case 2:*var = *(uint32_t*)data, data+=4; break;// копирование без выравнивания
	default:
		*var = default_value; break;
	}
	return data;
}
/*! Метод кодирования в стиле LZ4 но формат кодирования своместимый с RFS
 */
static 
uint8_t* lz4_decode(uint8_t* data, uint8_t* dst, off_t *slen)
{
	const uint8_t* base=dst;
	size_t len, off;
	uint8_t token;
    while ((token=*data++)!=0xFF) {
		data = rfs_decode_slen((token>>4), data, &len);
		data = rfs_decode_slen((token>>0), data, &off);
		if (token & 0x80) {
			dst = _memcpy_x64(dst, data, len);// copy literal
			data+=len;
		} else {
			if (off>0)
				dst = _memmove_x64(dst, off, len);
			else
				dst = _memzero_x64(dst, len);// zero fill
		}
	}
	*slen = (dst - base);
	return data;
}

static uint8_t* rfs_decode_skip(uint8_t * data, int type, size_t * slen){
	
	//do {}while((type|ASN_CLASS_CLOSING)
}

static inline 
uint8_t* _align(uint8_t* data, int n){
	return (uint8_t*)(((uintptr_t)data + ((1U<<n)-1)) & ~((1U<<n)-1));
}

void rfs_mountat(Device_t *dirp, Device_t* dev)
{
	
	uint8_t* data = *(uint8_t**)(dev+1);
	uint8_t* base = data;
	uint32_t oid;
	struct timespec ts;
	uint32_t mode;
	const char* path;
	uint32_t offset;
	uint32_t size;
	size_t slen;
	int xfer_align =4;
	int xfer_mask = (1U<<xfer_align)-1;
	uint8_t token;


	void _decode_skip(){
	}
	uint8_t* _decode_path(){
		uint8_t* path = data+1;
		data+=(*(uint8_t*)data +1)<<xfer_align;
		return path;
	}
	uint32_t _decode_optional_uint(int type, uint32_t default_value){
		if ((type & ASN_CLASS_CONTEXT)==0 || (token & (1<<ASN_CONTEXT_ID(type)))==0) 
			return BE32(*(uint32_t*)data); data += 4;
		return default_value;
	}
	uintptr_t _decode_optional_uintptr(int type, uintptr_t default_value){
		if ((type & ASN_CLASS_CONTEXT)==0 || (token & (1<<ASN_CONTEXT_ID(type)))==0) 
			return (*(uintptr_t*)data); data += 4;
		return default_value;
	}
	uint32_t _decode_uint(int type, uint32_t default_value){
		uint32_t value = BE32(*(uint32_t*)data); data += 4;
		return value;
	}
/*! Правила упаковки:

Данные выравниваются на 32 бита

Если указана нулевая строка, то объект создается с уникальным именем, префиксом и номером 0-4 байта.
Если указан OID с полем ino=0 объекту назначается уникальный идентификатор. 

 */
	while ((token=*data++) != 0xFF){// CPIO_MAGIC
		oid  = _decode_optional_uint(ASN_CONTEXT(0), 0);
		mode = _decode_uint(ASN_UINT, 0644);
//		uint8_t uid, gid;
		path = _decode_path(ASN_STRING, NULL);
		dev = dtree_path(dirp, path, &path);// AT_FOLLOW_SYMLINK
		dev = dtree_mknodat(dev, path, mode , oid);
		if (dev==NULL) {
			_decode_skip();
			continue;
		}
		File_t* file = (File_t*)(dev +1);
//		if (mode & C_ISUID) file->uid  = mode>>16;
//		if (mode & C_ISGID) file->gid  = mode>>24;
		ts.tv_sec  = _decode_optional_uint(ASN_CONTEXT(2), ts.tv_sec);
		ts.tv_nsec = _decode_optional_uint(ASN_CONTEXT(3), ts.tv_nsec);
		file->size = _decode_optional_uint(ASN_CONTEXT(4), 0);
		file->phandle= (void*)_decode_optional_uintptr(ASN_CONTEXT(5), 0);// физический адрес
		if (file->phandle!=NULL) {
			data = lz4_decode(data, file->phandle, &file->size);
		} else {
			file->phandle = _align(data,2);
			data += file->size;
		}
	}
}
#ifdef CPIO	
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <r3_args.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CliOptions CliOptions;
struct _CliOptions
{
    char* input_file;
    char* output_file;
    uint32_t page_size;
    uint32_t segment_size;
    uint32_t volume_size;
    uint32_t volume_offset;
    char* volume_label;
    uint32_t bad_cluster;
    bool verbose;
    bool gigabytes;
    bool megabytes;
    bool kilobytes;
    bool add;
    bool setattr;
    bool del;
    bool delnode;
    bool create;
    bool extract;
    bool regen;
    bool test;
    bool clean;
    uint32_t crash;
    struct {
        bool readonly;
        bool system;
        bool hidden;
        bool archive;
        bool temporary; // создавать временные файлы
    } attr;
    bool fat;
//    bool export;
};
struct _CliOptions  cli =
{
    .input_file = NULL,
    .output_file = "test.img",
    .page_size = 0,
    .segment_size = 0,
    .bad_cluster = (~0UL),
    .volume_size = 1024,
    .volume_offset = 0,
    .volume_label = NULL,
    .verbose = false,
    .gigabytes= false,
    .megabytes= false,
    .kilobytes= false,
    .setattr = false,
    .add = false,
    .del = false,
    .delnode = false,
    .create = false,
    .extract = false,
    .regen = false,
    .test = false,
    .crash= 0,
    .clean= false,
    .fat = false,
//    .export = FALSE,
};


static GOptionEntry cli_entries[] =
{
//    { "input",  'i', 0, G_OPTION_ARG_FILENAME,  &cli.input_file, "input R3-FS image", "*.rfs" },
    { "output", 'o', 0, G_OPTION_ARG_FILENAME,  &cli.output_file, "output filename", "test.img" },
// геометрия носителя
    { "page",     0, 0, G_OPTION_ARG_INT,       &cli.page_size,     "size of cluster 2^n", "2048"},
    { "segment",  0, 0, G_OPTION_ARG_INT,       &cli.segment_size,  "size of segment 2^n", "2048"},
    { "size",   's', 0, G_OPTION_ARG_INT,       &cli.volume_size,   "volume size in blocks(512) or k/M/GB", "1024"},
    { "bad",      0, 0, G_OPTION_ARG_INT,       &cli.bad_cluster,   "mark bad clusters", "21"},
// информация о томе
    { "label",  'L', 0, G_OPTION_ARG_STRING,       &cli.volume_label,   "volume label"},
// операции над файлами, применяться должен только один из ключей:
    { "add",    'a', 0, G_OPTION_ARG_NONE, &cli.add,    "add file to archive"},
    { "delete", 'd', 0, G_OPTION_ARG_NONE, &cli.del,    "remove file from image"},
    { "delnode", 'D', 0, G_OPTION_ARG_NONE, &cli.delnode,    "remove file from image by inode"},
    { "create", 'c', 0, G_OPTION_ARG_NONE, &cli.create, "create R3-FS image"},
    { "extract",'e', 0, G_OPTION_ARG_NONE, &cli.extract,"extract files from image"},
    { "regen",  'r', 0, G_OPTION_ARG_NONE, &cli.regen,  "regenerate r3-fs image"},
    { "test",   't', 0, G_OPTION_ARG_NONE, &cli.test,   "test R3-FS image"},
    { "crash",   0, 0, G_OPTION_ARG_INT, &cli.crash,   "crash test for R3-FS image"},
    { "fat",    'f', 0, G_OPTION_ARG_NONE, &cli.fat,    "make FAT image"},
    { "clean",    0, 0, G_OPTION_ARG_NONE, &cli.clean,    "clean R3-FS image"},
//    { "export", 'x', 0, G_OPTION_ARG_NONE, &cli.export, "convert R3-FS image to FAT"},
// аттрибуты файлов
    { "attr",       0  , 0, G_OPTION_ARG_NONE, &cli.setattr,      "set attributes to file"},
    { "readonly",   'R', 0, G_OPTION_ARG_NONE, &cli.attr.readonly,   "set file attribute READ_ONLY"},
    { "system",     'S', 0, G_OPTION_ARG_NONE, &cli.attr.system,     "set file attribute SYSTEM"},
    { "hidden",     'H', 0, G_OPTION_ARG_NONE, &cli.attr.hidden,     "set file attribute HIDDEN"},
    { "archive",    'A', 0, G_OPTION_ARG_NONE, &cli.attr.archive,    "set file attribute ARCHIVE"},
    { "tmp",        'T', 0, G_OPTION_ARG_NONE, &cli.attr.temporary,  "set file attribute TEMPORARY"},
// размерность носителя, применяться должен только один из ключей:
    { "GB", 'G',0, G_OPTION_ARG_NONE, &cli.gigabytes, "units = GB"},
    { "MB", 'M',0, G_OPTION_ARG_NONE, &cli.megabytes, "units = MB"},
    { "kB", 'K',0, G_OPTION_ARG_NONE, &cli.kilobytes, "units = kB"},
    { NULL }
};
bool _file_get_contents(const char *restrict  filename, uint8_t** buffer, size_t * len, struct stat *st)
{
    if (!S_ISREG(st->st_mode)) return false;
    if (st->st_size<=0) return false;
//    if (mtime) *mtime = st->st_mtime;
    /*
    struct tm tv;
    localtime_r(&st.st_mtime, &tv);
    bacnet_localtime(file->modification_date, &tv);
    */
    FILE* fp = fopen(filename, "rb");
    if (fp==NULL) return false;
    uint8_t *buf = malloc(st->st_size);
    if (buf==NULL) return false;
    *len = fread(buf, 1, st->st_size, fp);
    *buffer = buf;
    fclose(fp);

    return true;

}
//  gcc -DCPIO -Ir3core -o cpio r3core/r3_cpio.c r3core/r3_args.c r3core/r3_dtree.c r3core/r3_slice.c
int main(int argc, char **argv)
{
        const uint32_t kB = 1024, MB = 1024*1024;//, GB = 1024*1024*1024; // размерности

    GOptionContext *context;

    context = g_option_context_new ("- R3-FS/FAT conversion");
    g_option_context_add_main_entries (context, cli_entries,NULL/* GETTEXT_PACKAGE*/);
//  g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (context, &argc, &argv, NULL))
    {
        printf ("option parsing failed\n");
        //exit (1);
    }
    if (cli.megabytes) cli.volume_size*= 2*kB; // в блоках 512
    else if (cli.gigabytes) cli.volume_size*= 2*MB;
    else if (cli.kilobytes) cli.volume_size*= 2;
/*
	uint32_t attr = 0; // аттрибуты файлов
    if (cli.attr.hidden)    attr |= RFS_ATTR_HIDDEN;
    if (cli.attr.system)    attr |= RFS_ATTR_SYSTEM;
    if (cli.attr.archive)   attr |= RFS_ATTR_ARCHIVE;
    if (cli.attr.temporary) attr |= RFS_ATTR_TEMPORARY;
    if (cli.attr.readonly)  attr |= RFS_ATTR_READ_ONLY;
	*/
	FILE* media_fp = NULL;
    if (cli.create)
    { // создание образа rfs
        printf("r3-fs create image\n");
//        printf(" page size    = %d B\n", 1<<(media->page_size_log2+9));
//        printf(" segment size = %d kB\n", 1<<(media->segment_size_log2-1));
        FILE* media_fp = fopen(cli.output_file,"wb");
        if (media_fp==NULL) return 1;
        if (argc>1) {
            printf("args list:\n");
			struct tm tm;
			char buf[512];
            int i;
            for (i=1; i< argc; i++) {
                const char* filename = argv[i];
                struct stat st;
                time_t mtime;
                uint8_t *buffer=NULL;
                size_t len=0;
				printf("filename\t: %s\n", filename);
			    if (stat(filename, &st)) continue;
				struct tm *t = localtime(&st.st_mtime);
				strftime(buf,64, "%d−%m−%Y", t);
				printf("file date\t: %s\n", buf);
				printf("file ino,dev\t: {%d,%u}\n", st.st_ino, st.st_dev);
				printf("file mode\t: %o\n", st.st_mode);
				printf("file size\t: %u\n", (int)st.st_size);
				printf("file uid,gid\t: %u,%u\n", st.st_uid,st.st_gid);
				// Если ino не указан
				// различие только в mode, заполняем dev_id на основе mode.
				char* data = buf;
				uint8_t token=0;
				if (S_ISREG(st.st_mode)) token |= (1<<4);// size
				*(uint8_t *)data = token |((1<<2)|(1<<4)); data+=1;//token ~oid, mode, mtime
				*(uint16_t*)data = st.st_mode;  data+=2;
				
				len = strlen(filename)+1;
				*data++ = len;// добавить выравнивание
				memcpy(data, filename, len);
				data+= len;
				*(uint32_t*)data = st.st_mtime; data+=4;

				fwrite(data, data- buf, 1, media_fp);
				
                if (S_ISREG(st.st_mode) && _file_get_contents(filename, &buffer, &len, &st)){
					data = buf;
					*(uint32_t*)data = len; data+=4;
					fwrite(data, data- buf, 1, media_fp);

						printf("file length\t: %u\n", len);
						// struct tm *gmtime_r(const time_t *timer, struct tm *buf);
					fwrite(buffer, len, 1, media_fp);
				}
			}
		}
    } else {
        if (cli.input_file==NULL) cli.input_file = cli.output_file;
        media_fp = fopen(cli.input_file,"rb");
        if (media_fp==NULL) return 1;
        printf("Expected volume size: %d\n", cli.volume_size);
    }
//	fflush(stdout);

    if (media_fp!=NULL) fclose(media_fp);
}
#endif