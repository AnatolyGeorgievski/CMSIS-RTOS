/*!
	\defgroup _fs_init Начальная загрузка файловой системы. Пока загружается 
	есть ощущение что это CPIO

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

#define ASN_CLASS_OPENING  0x6
#define ASN_CLASS_CLOSING  0x7

#define ASN_MASK 		0xF8				//!< Маска типа
#define LEN_MASK		0x07				//!< Кодирование длины
#define ASN_CONTEXT(n) (((n)<<4)|0x08)		//!< Контекстно зависимый тип
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



struct romfs {
	uint32_t mode:8;
	uint32_t uid:8;
	uint32_t gid:8;
	uint32_t slen:8;
};
#define BE32(x) __builtin_bswap32(x)
#define BE16(x) __builtin_bswap16(x)

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
static uint8_t* rfs_decode_slen(uint8_t * data, int type, size_t * slen){
	//if ((data[0] & ASN_MASK)==type) 
	{
		uint32_t len = *data++ & LEN_MASK;
		if (len == 0x5) {
			len += *data++; // 255+5
			if (len>256){// можно кодировать любые длины... но нас интересуют только строки длиной до 256 байт
			}
		}
		*slen= len;
	}
	return data;
}
static uint8_t* rfs_decode_skip(uint8_t * data, int type, size_t * slen){
	
	//do {}while((type|ASN_CLASS_CLOSING)
}

static inline 
uint8_t* _align(uint8_t* data, int n){
	return (uint8_t*)(((uintptr_t)data + ((1U<<n)-1)) & ~((1U<<n)-1));
}
__attribute__((noinline))
static uint8_t* _decode_uint(uint8_t* data, uint32_t* value){
	int len = *data++ & LEN_MASK;
	unsigned long val = 0;
	while (len--) val = (val<<8) | *data++;
	*value = val;
	return data;
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
	uint8_t options;
int _type(int type){
	return (ASN_MASK & data[0]) == type;
}
int _option(int id){
	return (options>>=1)&1;
}
void _decode_skip(){
}
uint8_t* _decode_path(){
	uint8_t* path = data+1;
	data+=(*(uint8_t*)data +1)<<xfer_align;
	return path;
}
uint32_t _decode_uint(int type, uint32_t default_value){
	uint32_t value = BE32(*(uint32_t*)data); data += 4;
	return value;
}
/*! Правила упаковки:
Данные выравниваются на 32 бита

Если указана нулевая строка, то объект создается с уникальным именем, префиксом и номером 4 байта.
Если указан OID с полем ino=0 объекту назначается уникальный идентификатор. 

 */
	while (*(uint32_t*)data != ~0UL){// CPIO_MAGIC
		oid  = _decode_uint(ASN_OID, DEV_FIL);
		mode = _decode_uint(ASN_UINT, 0644);
/*		uint8_t uid, gid;
		if (mode & C_ISUID) uid  = mode>>16;
		if (mode & C_ISGID) gid  = mode>>24; */
		path = _decode_path(ASN_STRING, NULL);
		dev = dtree_path(dirp, path, &path);// AT_FOLLOW_SYMLINK
		if (mode & C_ISVTX) {// дописывание файла ведет к удалению
			dtree_unref(dev);
			continue;
		}
		dev  = dtree_mknodat(dev, path, mode , oid);
		if (dev==NULL) {
			_decode_skip();
			continue;
		}
		FILE* file = (FILE*)(dev +1);
		switch (DEV_ID(dev)) {
		case DEV_FIL:{
			file->size = _decode_uint(ASN_UINT, 0);
			ts.tv_sec  = _decode_uint(ASN_UINT, ts.tv_sec);
			ts.tv_nsec = _decode_uint(ASN_UINT, ts.tv_nsec);
			offset     = _decode_uint(ASN_UINT, 0);
			file->mtim = ts;
			if (offset)
				file->phandle = base + (offset << xfer_align);
			else
				file->phandle = data;
		} break;
		case DEV_SHM: {
			file->phandle= (void*)_decode_uint(ASN_UINT, 0);
			file->size   = _decode_uint(ASN_UINT, 0);
		} break;
		case DEV_SEM: {
			file->phandle= (void*)_decode_uint(ASN_UINT, 0);
		} break;
		case DEV_LNK: {
			path = _decode_path(ASN_STRING, NULL);
			file->phandle = dtree_path(dirp, path, &path);// AT_FOLLOW_SYMLINK
		} break;
		default: break;
		}
	}
}
	