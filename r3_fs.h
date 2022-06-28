#ifndef R3_FS_H_INCLUDED
#define R3_FS_H_INCLUDED

#include <stdint.h>
#include "r3_object.h"

#if 0
//#define BLOCK_MEDIA_UNIT 4096
typedef struct _BlockMedia BlockMedia;
struct _BlockMedia {
    void*   data;
    int     (*init) (void*);
    void*   (*read) (void* fp,       void* buffer, uint32_t block, uint32_t size);
    int     (*send) (void* fp, const void* buffer, uint32_t block, uint32_t size);
    int     (*over) (void* fp, const void* buffer, uint32_t block, uint32_t offset, uint32_t size);
    int     (*check)(void* fp, uint32_t segment);
    int     (*unref)(void* fp, void* buffer);

//    int     (*attach)(void* fp);
//    int     (*detach)(void* fp);

    uint32_t    num_blocks;
    int         page_size_log2;     //!< размер страницы в блоках  (sec_per_clus_log2)
    int         segment_size_log2;  //!< размер сегмента в блоках, единица стирания данных
    char* name;
};
#endif

//! Атрибуты FAT файла живут в 11-м байте
#define RFS_ATTR_READ_ONLY    0x01
#define RFS_ATTR_HIDDEN       0x02
#define RFS_ATTR_SYSTEM       0x04
//#define FAT32_ATTR_VOLUME_ID    0x08
#define RFS_ATTR_DIRECTORY    0x10  //!< вложенные директории
#define RFS_ATTR_ARCHIVE      0x20  //!< метак используется утилитой архивации(резервного копирования) файлов, для пометки файлов прошедших архивацию. Не используется самой системой
#define RFS_ATTR_TEMPORARY    0x40  //!< файлы с такой пометкой могут автоматически удалятся с носителя в случае нехватки места

#define RFS_EOF (~0UL) //!< код конца файла, выдается при запросе следующего блока на чтение

enum rfs_messages_e { // сообщения в журнале
    RFS_SKIP  =0,       //!< пропустить до конца блока
    RFS_CREATE=1, // создание файла
    RFS_DELETE=2, // удаление файла
    RFS_APPEND=3, // удлинить файл на заданную цепочку кластеров
    RFS_ATTRIB=4, // назначить атрибуты и установить размер
    RFS_MAPPED=5, // переместить блок, пометить кластер как битый, кластер исключается из пространства векторов
    RFS_VOLUME=0xA,    //!< информация о носителе, резервные копии и новые версии журнала создаются такой записью
};

typedef struct _RFS RFS;
typedef struct _RFSfd RFSfd;
typedef struct _RFSvec RFSvec;
typedef struct _FATvec FATvec;
// typedef struct _FileEntry FileEntry;
typedef struct _RFSjournal RFSjournal;
struct _RFSvec {// 4096 блоков по 4kB укладывается в 16 бит
	struct _RFSvec *next;
	uint16_t offset;
	uint16_t size;
};
/*! Вектор для описания структуры FAT */
struct _FATvec {
    uint32_t offset, size, link;
};
struct _RFSfd {
	uint8_t* _p; // буфер в памяти

	uint32_t _r; // число байт до конца файла
//	uint32_t _w; // позиция записи, число байт
	struct _FileObject* file;
	RFSvec* space;  // пространство векторов файла
};

void rfs_journal_init (RFS * rfs, DeviceInfo_t* devinfo, BlockMedia* media, uint32_t offset, uint32_t size);
void*rfs_journal_flush(RFS* rfs);//, uint8_t * data, size_t size);
void rfs_journal_regen(RFS* rfs, DeviceInfo_t* devinfo);
void rfs_journal_clean(RFS* rfs);
void rfs_journal(RFS* rfs, int type, void* object);
void rfs_journal_append(RFS* rfs, struct _Object* object);
void rfs_journal_object(RFS* rfs, struct _Object* object);
extern RFS default_fs;

#endif // R3_FS_H_INCLUDED
