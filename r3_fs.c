#include "cmsis_os.h"
#include "r3_slice.h"
#include "r3_tree.h"
#include <stdio.h>
#include "r3_fs.h"
#include "crc.h"
#define RFS_JOURNAL_SUPERBLOCK 0

#pragma GCC optimize ("Os")
/*! \defgroup _rfs Memory Mapped File System (RFS) 
	\ingroup _system
 */
/// \todo Внимание мы определяем только ту часть которая тут используется, от этой структуры наследуется остальное
struct _DeviceInfo {
    struct _DeviceObject * device; // сам объект
	tree_t* device_objects;
};
extern void r3_pspec_free(void* data, const PropertySpec_t *desc, int pspec_length);

struct _RFSjournal {
	uint32_t block;
	uint8_t *data;// буфер обмена, равен размеру блока
	uint32_t size;
	uint32_t offset;// смещение внутри блока
	uint32_t crc;	// контрольная сумма
	RFSvec* space;  // пространство векторов журнала
};

typedef struct _dtree dtree_t;
// Структура выделяется слайсами из пула
struct _dtree {
//    dtree_t* prev;	//!< левое поддерево
    dtree_t* next;	//!< правое дерево
    void * value;
    uint32_t  key;	//!< ключ
	const char * d_name;
};
dtree_t* dtree_lookup(dtree_t** prev, uint32_t key)
{
    dtree_t* node;
    while ((node = *prev)!=NULL) {
		int32_t cmp = key - node->key;
        if (cmp==0) break;
//        prev = (cmp < 0)? &node->prev: &node->next;
        prev = &node->next;
    }
    return node;
}
dtree_t* dtree_insert(dtree_t** prev, dtree_t* n)
{
    dtree_t* node;
    while ((node = *prev)!=NULL) {
        if (node->key > n->key) break;
        prev = &node->next;
    }
    n->next = node;
    *prev = n;
    return node;
}
dtree_t* dtree_remove(dtree_t** prev, uint32_t key)
{
    dtree_t* node;
    while ((node = *prev)!=NULL) {
        if (node->key == key) {
            *prev = node->next;
            break;
        }
        prev = &node->next;
    }
    return node;
}

struct _RFS {
	uint32_t revision;
	uint32_t volume_id;
	uint32_t volume_size;
	RFSjournal journal;
	RFSvec* space;
	RFSvec* mapped;
    dtree_t* files;
//	tree_t* files;
	BlockMedia* media;
};
/*! \defgroup rfs_vector Фрагментация 
	\ingroup _rfs
	\{
 */
/*! \brief выделить вектор из пространства векторов
    \param size - размер в блоках
    \param actual_size - выделенный размер
    \return смещение вектора в блоках и actual_size - выделенный размер
*/
static uint32_t rfs_vector_alloc (RFSvec ** rfs_space, uint32_t offset, size_t size, size_t *actual_size)
{
	//size_t size = *actual_size;
	RFSvec **prev = rfs_space;
    RFSvec *vec;
	while ((vec = *prev)!=NULL)  {
		if (offset >  vec->offset) {
		} else
		if (offset == vec->offset) {
			if (vec->size <= size) {
				size = vec->size;
				*prev = vec->next;// remove
				g_slice_free(RFSvec, vec);
			} else
			{
				vec->offset += size, vec->size -= size;
                //*(struct _vec*)&vec->v = v;// CAS
			}
		    *actual_size = size;
			return offset;
		} else // (offset <  vec->offset)
			break;
		prev = &vec->next;
	}
	// нет возможности дописать, надо подыскать подходящий фрагмент по размеру
	prev = rfs_space;
	while ((vec = *prev)!=NULL) {
		if (size <= vec->size) {
			offset = vec->offset;
			if (size == vec->size) {
				*prev = vec->next;// remove
				g_slice_free(RFSvec, vec);
			} else {
				vec->offset += size, vec->size -= size;
			}
			return offset;
		}
		prev = &vec->next;
	}
	*actual_size = 0;// не найден
	return 0;
}
/*! \brief вырезать вектор из пространства свободных векторов
	\return 0 - если успешно, 1 - если диапазон не найден. Этот вариант не должен реализоваться
*/
static int rfs_vector_crop(RFSvec ** rfs_space, uint32_t offset, uint32_t *actual_size)
{
	size_t size = *actual_size;
	RFSvec **prev = rfs_space;
    RFSvec *vec;
	while ((vec = *prev)!=NULL) {
		if (offset >  vec->offset+vec->size) {// листать дальше
		} else
        if (offset == vec->offset) {// от начала
			if (vec->size <= size) {//
				*actual_size = vec->size;
				*prev = vec->next; // remove
				g_slice_free(RFSvec, vec);
			} else // vec->size > vector->size
                vec->offset += size, vec->size -= size;
			return 0;
		} else
		if (offset > vec->offset) {
			if ((vec->offset+vec->size)==(offset+size)) {// с конца
				vec->size -= size;
			} else
			if ((vec->offset+vec->size) >(offset+size)) { // попал внутрь диапазона
				RFSvec* vec2 = g_slice_new(RFSvec);
				vec2->offset = vec->offset;
				vec2->size   = offset - vec->offset;
				vec2->next   = vec;
				vec->offset = offset+size;
				vec->size -= size + vec2->size;
				*prev = vec2;
			} else {// пересекает границу диапазона -- этот вариант не должен реализоваться
				size = (offset - vec->offset);
				*actual_size = vec->size - size;
				vec->size = size;
			}
			return 0;
		}
		prev = &vec->next;
	}
//	*actual_size = 0;
	return 1;
}
/*! \brief возвращает вектора в свободное пространство

    \param rfs->space - пространство векторов
    \param offset - вектор, число страниц от начала раздела
    \param size - размер вектора в страницах
*/
static int rfs_vector_release(RFSvec ** rfs_space, uint32_t offset, size_t size)
{
	RFSvec **prev   = rfs_space;
    RFSvec *vec;
    while ((vec=*prev)!=NULL) {
        if (vec->offset+vec->size == offset) { // встраивается после
            vec->size += size;
            // g_slice_free(RFSvec, vector);
			prev = &vec->next;
			RFSvec *next;
			/* while*/
			if ((next = *prev)!=NULL && (vec->offset+vec->size == next->offset))
            {// объединение смежных фрагментов после
                vec->size += next->size;
				prev = &next->next;
                g_slice_free(RFSvec, next);
            }
            return 0;
        } else
        if (vec->offset == offset+size) {// встраивается перед
            vec->offset = offset, vec->size+=size;
			if (0 && prev != rfs_space) {// если не начало списка -- по логике встраивания не используется.
				RFSvec *pvec = ((void*)prev - offsetof(RFSvec, next));
				if (pvec->offset+pvec->size == vec->offset)
				{// объединить смежные фрагменты с предыдущим
					pvec->size += vec->size;
					g_slice_free(RFSvec, vec);
				}
			}
            return 0;// g_slice_free(RFSvec, vector);
        } else
        if (vec->offset > offset) { // встраивается между
			break;
		}
		prev = &vec->next;
    }
	RFSvec* vector = g_slice_new(RFSvec);
	vector->offset = offset, vector->size = size;
	vector->next = vec;
	*prev = vector;
	return 1;
}
/*! \brief добавить вектор в конец списка фрагментов файла

Внимание: Список фрагментов файла вывернут
	\param vec - указывает на последний элемент списка
	\return указатель на последний элемент списка
 */
void rfs_vector_append (RFSvec ** root, uint32_t offset, size_t size)
{
	RFSvec* vec = *root;
	if (vec!=NULL && vec->offset+vec->size == offset) {
		vec->size += size;
	} else {
		RFSvec* vector = g_slice_new(RFSvec);
		vector->offset = offset, vector->size = size;
		vector->next = vec;// prepend
		*root = vector;
	}
}
static uint32_t rfs_vector_next(RFSvec ** rfs_space, uint32_t block)
{
	RFSvec **prev = rfs_space;
	RFSvec *vec;
	while ((vec = *prev)!=NULL) {
		if (vec->offset <= block && block < vec->offset+vec->size) {
			return 0;
		}
		prev = &vec->next;
	}
	return 1;
}
	//!\}
#if 0 // измышление на тему выделения блоков 4096 блоков это 128 слов.
{
	int nbits = (offset)&31;
	uint32_t *map = &rfs->map[offset>>5];
	if (nbits) {
		map[0] |= (~0UL)<<(32-nbits);
		map++;
		size -= 32-nbits;
	}
	for(i=0; i=size>>5; i++){
		map[i] =~0;
	}
	if (size & 31) {
		nbits = (size & 31);
		map[i] |= (~0UL)>>(32-nbits);
	}
}
#endif
/*! \defgroup rfs_encode Кодирование и декодирование данных на носителе 
	\ingroup _rfs
	\{
 */
/*! \brief кодирование пропуска
    переход к следующему блоку вставляется только если заданный размер не укладываетя до конца блока
 */
static uint8_t* rfs_encode_skip(RFS*rfs, uint8_t* data, uint32_t field_size)
{
    uint32_t size = BLOCK_MEDIA_UNIT - (data - rfs->journal.data) - rfs->journal.offset;// до конца блока
    if (field_size <= size) // укладывается
		return data;
    // если поле заданной длины не укладывается до конца блока
	while (size--) { // если до конца блока можно поместить хотябы один символ
        *data++ = RFS_SKIP;
    }
    return rfs_journal_flush(rfs);//, rfs->journal.data+rfs->journal.offset, BLOCK_MEDIA_UNIT-rfs->journal.offset);
}
/*! \brief вычисление длины записи целого числа -1 */
static inline int rfs_size_uint(uint32_t value)
{
	return (value==0)?0:((32 - __builtin_clz(value))>>3);
}

/*! \brief кодирование целого числа */
static uint8_t* rfs_encode_uint(uint8_t* data, /* RFS*rfs, */uint8_t asn_type, uint32_t value)
{
    int clz = rfs_size_uint(value); // число ноликов = количество байт -1
//    data = rfs_encode_skip(rfs, data, clz+1);
	*data++ = asn_type|clz;
    while (clz--) {
        *data++ = value;
		value >>= 8;
    }
    return data;
}
static uint8_t* rfs_encode_u32(uint8_t* data,/* RFS*rfs, */uint8_t asn_type,  uint32_t value)
{
//    data = rfs_encode_skip(rfs, data, 5);
	*data++ = asn_type|4;
	*(uint32_t*)data = value;
	return data+4;
}
static uint8_t* rfs_encode_vec(uint8_t* data,/* RFS*rfs, */uint8_t asn_type, RFSvec* vec)
{
	*data++ = asn_type|4;
	*(uint16_t*)data = vec->offset; data+=2;
	*(uint16_t*)data = vec->size;	data+=2;
	return data;
}
/*! \brief декодирование пропусков
    если достигнут конец блока или найдена метка перехода
*/
static uint8_t* rfs_decode_skip(RFS*rfs, uint8_t* data)
{
    uint32_t size = data - rfs->journal.data;
	while (size<BLOCK_MEDIA_UNIT && *data==RFS_SKIP) {
		data++;
		size++;
	}
    if (size == BLOCK_MEDIA_UNIT) {
        BlockMedia* media = rfs->media;
        rfs->journal.block++; // только внутри сегмента так работает
        data = media->read(media->data, rfs->journal.data, rfs->journal.block, BLOCK_MEDIA_UNIT);
    }
    return data;
}
static int _optional(uint8_t* data, uint8_t type){
	return (data[0]>>4) == type;
}

static uint8_t* rfs_decode_slen (RFS* rfs, uint8_t* data, size_t *slen)
{
	const uint8_t mask = 007;
    size_t size = (*data++)& mask;
	if (size == mask) size += *data++;
	*slen  = size;
	return data;
}
/*! \brief декодирование целого числа */
static uint8_t* rfs_decode_uint(RFS* rfs, uint8_t* data, uint32_t* value)
{
    data = rfs_decode_skip(rfs, data); // всё число в одном блоке, не разрывается
    int size = (*data++)&0x7;
    uint32_t val = 0;// ( *data++ ) & (0x7F>>clz);
    while (size--) {
        val = (val<<8) | *data++;
    }
    *value = val;
    return data;
}
static uint8_t* rfs_decode_u32(RFS* rfs, uint8_t* data, uint32_t* value)
{
	data = rfs_decode_skip(rfs, data); // всё число в одном блоке, не разрывается
	*value = *(uint32_t*)data;
	return data+4;
}
	//!\}
/*! \brief сбросить журнал на носитель
	\ingroup _rfs
    \return указатель на начало блока данных, буфер для продолжения записи в журнал
 */
void* rfs_journal_flush(RFS* rfs)//, uint8_t * data, size_t size)
{
	RFSjournal* journal = &rfs->journal;
	BlockMedia* media   =  rfs->media;
	media->append(media->data, (journal->block<<9)+journal->offset, journal->data, journal->size);
	journal->offset+=journal->size;
	journal->crc = crc_from_block(journal->crc, journal->data, journal->size);
	if (journal->offset == BLOCK_MEDIA_UNIT){
		size_t size = 1;
		journal->block = rfs_vector_alloc(&rfs->space, journal->block+1, size, &size);
		rfs_vector_release(&journal->space, journal->block, size);// добавить вектор
		journal->offset= 0;
	}
	journal->size = 0;
    return journal->data;
}
#if 1 //BACNET
static void _object_delete(DeviceInfo_t* devinfo, uint32_t object_identifier)
{
	tree_t *leaf = tree_remove(&devinfo->device_objects, object_identifier);
	if (leaf != NULL) {
#ifndef RFS_IMAGE
		Object_t* object = leaf->value;
		if (object) {// удаление не требуется в утилите
			const PropertySpec_t* pspec = object->property_list.array;
			int pspec_length = object->property_list.count;

			r3_pspec_free(object, pspec, pspec_length);// освободить все что связано с объектом

			free(object); leaf->value=NULL;
		}
#endif
		g_slice_free1(sizeof(tree_t), leaf);
	}
}
static Object_t * _object_lookup(DeviceInfo_t* devinfo, uint32_t object_identifier)
{
    Object_t * object;
    object = tree_lookup(devinfo->device_objects, object_identifier);
    return object;
}
static Object_t* _object_new(uint32_t object_identifier)
{
    const DeviceObjectClass *object_class = object_classes_lookup(object_identifier>>22);
    if (object_class==NULL)
        return NULL;

    Object_t* object = malloc(object_class->size);
    __builtin_bzero(object, object_class->size);
    object->object_identifier = object_identifier;
    object->property_list.array = object_class->pspec;
    object->property_list.count = object_class->pspec_length;

    return object;
}
static Object_t* _object_init(DeviceInfo_t *devinfo, uint32_t object_identifier, Object_t* object)
{
    object->object_identifier = object_identifier;
    tree_t *leaf = g_slice_alloc(sizeof(tree_t));
	tree_init(leaf, object_identifier, object);
	if (devinfo->device_objects==NULL)
        devinfo->device_objects = leaf;
	else
        tree_insert_tree(&devinfo->device_objects, leaf);
    return object;
}
#endif
/*! \brief производит загрузку журнала в память
	\ingroup _rfs
    \param offset - смещение от начала носителя выражается в страница памяти 4к
    \param size - размер выражается в страница памяти 4к или 512b
*/
void rfs_journal_init(RFS * rfs, DeviceInfo_t* devinfo, BlockMedia* media,
	uint32_t offset, uint32_t size)
{
    //rfs->media = media;
    rfs->space = NULL;
    rfs->mapped= NULL; //bad clusters
//    rfs->files = NULL;
    rfs->revision = 0;
//    rfs->blocks_per_cluster_log2 = rfs->media->page_size_log2;
//    rfs->volume_size = (size>>rfs->blocks_per_cluster_log2); // размер выражается в кластерах
    rfs->volume_id = 0xDEC0DDED;
	// создать пространство векторов из предложенного
	RFSvec* vec = g_slice_new(RFSvec);
	vec->offset = offset, vec->size = size;
	vec->next = NULL;
	rfs->space = vec;


// Журнал может располагаться во внутренней флеш памяти или Quad-SPI
	rfs->journal.block = RFS_JOURNAL_SUPERBLOCK;//
//	uint8_t *data = media->read(media->data, NULL, rfs->journal.block, BLOCK_MEDIA_UNIT);
	uint8_t *data = mmap(media->data, BLOCK_MEDIA_UNIT, PROT_READ, MAP_FIXED, dsk, rfs->journal.block);

    rfs->journal.data = data;
	rfs->journal.offset=0;
// Журнал может занимать один или два блока. Проверить оба.
	if (*data == RFS_VOLUME) {
		data++;
		data = rfs_decode_u32(rfs, data, &rfs->volume_id);
		data = rfs_decode_uint(rfs, data, &rfs->revision);
		printf ("Volume ID: %08X\n", rfs->volume_id);
		printf ("Revision : %d\n", rfs->revision);
	}
#if 0
	data += (size-1)*BLOCK_MEDIA_UNIT;
	if (*data == RFS_VOLUME){// вторая метка найдера
		data++;
		uint32_t revision=0;
		data = rfs_decode_u32(rfs, data, &revision);
		if (revision > rfs->revision){
			rfs->revision = revision;
			rfs->journal.data = data;
			rfs->journal.block= offset+(size-1);
		}
	}
#endif
	size = 1;
	rfs_vector_crop(&rfs->space, rfs->journal.block, &size);


	//data = rfs->journal.data;


//	uint8_t *top  = data+BLOCK_MEDIA_UNIT;
	while (*data != 0xFF) {// читаем до конца блока или до метки завершения записи
		switch(*data++) {
		case RFS_SKIP: {
		} break;
		case RFS_VOLUME: {
//			data = rfs_decode_u32(rfs, data, &rfs->volume_id);
//			data = rfs_decode_u32(rfs, data, &rfs->volume_size);
			data = rfs_decode_u32(rfs, data, &rfs->revision);

		} break;
		case RFS_CREATE: {// создать объект
			OID_t  object_identifier, parent_oid;
			size_t slen;
			mode_t mode;// short
			const char* object_name;
			data = rfs_decode_u32(rfs, data, &object_identifier);
			// сборка объекта _FILE
			if (_optional(data, ASN_CONTEXT(0)))
				data = rfs_decode_u32(rfs, data, &parent_oid);
			if (_optional(data, ASN_CONTEXT(1))) {// directory entry
				data = rfs_decode_slen(rfs, data, &slen);
				dir_insert(parent_oid, object_identifier, object, data, slen) ;
				data+= slen;
			}
			if (_optional(data, ASN_CONTEXT(2)))
				data = rfs_decode_uint(rfs, data, &object_name);
			// сериализация _FILE
			if (object->)
			Object_t* object = _object_lookup(devinfo, object_identifier);
			if (object==NULL) {
				object = _object_new(object_identifier);
			}
			_object_init(devinfo, object_identifier, object);
		} break;
		case RFS_DELETE: {// удалить объект
			uint32_t  object_identifier;
			data = rfs_decode_u32(rfs, data, &object_identifier);
			_object_delete(devinfo, object_identifier);
		} break;
		case RFS_DENTRY: {/* установить связь - это может быть директория или symlink или rename или unlink. 
			Удалить можно методом object_identifier2 = UNDEFINED */
			const char* name =NULL;
			size_t slen =0;
			data = rfs_decode_u32(rfs, data, &object_identifier);
			if (_optional(data, ASN_CONTEXT(0)))
				data = rfs_decode_u32(rfs, data, &object_identifier2);
			if (_optional(data, ASN_CONTEXT(1))) {
				data = rfs_decode_slen(rfs, data, &slen);
				name = data, data+=slen;
			}
			
			if (name) {
				dentry->d_name = name;
				dentry->d_ino  = object_identifier2;
				dtree_replace(dirp, dentry);
			} else 
				dtree_remove (dirp, NULL);
		} break;
		case RFS_ATTRIB: {// свойства объекта
			Object_t* object = _object_lookup(devinfo, object_identifier);
			
		} break;
		case RFS_APPEND: {/* дописать вектор по идентификатору файла 
			можно добавить штамп времени
			*/
			uint32_t  object_identifier, offset, size;
			data = rfs_decode_u32 (rfs, data, &object_identifier);
			data = rfs_decode_uint(rfs, data, &offset);
			data = rfs_decode_uint(rfs, data, &size);
			Object_t* object = _object_lookup(devinfo, object_identifier);
			struct _FileObject *file = (struct _FileObject *)object;
			RFSfd* fd = file->fp;
			rfs_vector_crop(&rfs->space, offset, &size);
			rfs_vector_append(&fd->space, offset, size);
		} break;
		case RFS_MAPPED: {// карта памяти убитых|исключенных кластеров (страниц памяти)
			uint32_t offset, size;
			data = rfs_decode_uint(rfs, data, &offset);
			data = rfs_decode_uint(rfs, data, &size);
			rfs_vector_crop(&rfs->space, offset, &size);
			rfs_vector_release(&rfs->mapped, offset, size);
		} break;
		default:
			// не должно встречаться, предупредить
			break;
		}
		rfs->journal.offset = (data - rfs->journal.data);// позиция чтения/записи внутри блока
		if ((data - rfs->journal.data)==BLOCK_MEDIA_UNIT) {
			rfs->journal.block++;
			if(rfs_vector_next(&rfs->journal.space, rfs->journal.block))
			{// найти селедующий блок
				size_t size =1, actual_size;
				rfs->journal.block = rfs_vector_alloc(&rfs->space, rfs->journal.block, size, &actual_size);
				rfs_vector_release(&rfs->journal.space, rfs->journal.block, size);
			}
			data = media->read(media->data, rfs->journal.data, rfs->journal.block, BLOCK_MEDIA_UNIT);
			rfs->journal.data = data;
		}
	}
	printf("R3FS journal size=%d\n", rfs->journal.offset);

//    return rfs;
}

extern uint8_t * r3cmd_object_store (uint8_t *buf, void* object, uint32_t object_id, const PropertySpec_t *pspec, uint32_t pspec_len);
#ifndef RFS_IMAGE
static void _object_serialize(uint32_t object_identifier, void* value, void* user_data)
{
	Object_t *object = value;
	RFS* rfs = user_data;
	BlockMedia* media = rfs->media;
	uint8_t* data = rfs->journal.data;
	*data++ = RFS_CREATE;
/* 	uint16_t type_id = object_identifier>>22;
	const DeviceObjectClass *object_class = object_classes_lookup(type_id);
*/

// Есть вариант инкрементно, те объекты которые были изменены, или все подряд
	size_t size = r3cmd_object_store(data, object, object_identifier, object->property_list.array, object->property_list.count) - data;
	media->append(media->data, (rfs->journal.block<<9)+rfs->journal.offset, data, size);
	if ((object_identifier>>22) == _FILE) {
		data = rfs->journal.data;
//		rfs_journal_append(rfs, file, );
#if 0

		for () {// по блокам копирование (дефрагментация)
			buffer = media->read(media->data, buffer, );
			media->append(media->data, file, buffer, size);
		}
#endif
        rfs_journal_append(rfs, object);
/*
		struct _FileObject* file = (struct _FileObject*) object;
		RFSfd * fd = file->fp;
		if (fd && fd->space) {
			*data++ =RFS_APPEND;
			data = rfs_encode_u32 (data, ASN_CONTEXT(0), object_identifier);
			RFSvec* vec = fd->space;
			while (vec) {
				data = rfs_encode_vec (data, ASN_CONTEXT(1), vec);
				vec = vec->next;
			}
		} */
/*		rfs_vector_crop(offset, size);
		media->append(rfs->media->data, ...);
*/
	}
}

/*! \brief регенерация журнала системы
	\ingroup _rfs
*/
void rfs_journal_regen(RFS* rfs, DeviceInfo_t* devinfo)
{
    rfs->journal.offset = 0;
    rfs->journal.block = RFS_JOURNAL_SUPERBLOCK;

//    rfs->stat.deleted = rfs->stat.created = 0; // заново посчитается

	BlockMedia* media = rfs->media;
    if (media->erase != NULL) {
        media->erase(media->data, RFS_JOURNAL_SUPERBLOCK/* rfs->journal.superblock>>(rfs->media->segment_size_log2 - rfs->blocks_per_cluster_log2)*/);
    }

// освободить вектора занятые журналом
    uint8_t * data =rfs->journal.data;
    *data++ = RFS_VOLUME;// новая версия
//    data = rfs_encode_u32 (data, ASN_CONTEXT(1), data, rfs->volume_id);
//    data = rfs_encode_uint(data, data, rfs->volume_size); // измерено в кластерах
    data = rfs_encode_uint(data, ASN_CONTEXT(1), rfs->revision++);

    //if (rfs->mapped)
	RFSvec* vec = rfs->mapped;
	if (vec) {
		*data++ = RFS_MAPPED;
		while (vec) {
			data = rfs_encode_vec (data, ASN_CONTEXT(1), vec);
			vec = vec->next;
		}
	}

	// балансировать дерево
	// tree_depth() - расчитать баланс дерева
	tree_balance(&devinfo->device_objects);
	tree_foreach(devinfo->device_objects, _object_serialize, rfs);
//    rfs_journal_flush(rfs);
    // printf("journal size =%d+%d B\n", (rfs->journal.block<<9),rfs->journal.offset );
}
#endif
/*! \breif Записть объекта в журнал	
	\ingroup _rfs
 */
void rfs_journal_object(RFS* rfs, struct _Object* object)
{
    uint8_t * data = rfs->journal.data + rfs->journal.size;
    *data++ =RFS_CREATE;
    data = r3cmd_object_store(data, object, object->object_identifier,
                             object->property_list.array, object->property_list.count);
    rfs->journal.size = data - rfs->journal.data;
}
/*! \breif Дописывание фрагментов файлов в журнал	
	\ingroup _rfs
 */
void rfs_journal_append(RFS* rfs, struct _Object* object)
{
    uint8_t * data = rfs->journal.data + rfs->journal.size;
    struct _FileObject* file = (struct _FileObject*) object;
    RFSfd * fd = file->fp;
    if (fd && fd->space) {
        *data++ =RFS_APPEND;
        data = rfs_encode_u32 (data, ASN_CONTEXT(0), file->instance.object_identifier);
        RFSvec* vec = fd->space;
        while (vec != NULL) {
            data = rfs_encode_vec (data, ASN_CONTEXT(1), vec);
            vec = vec->next;
        }
    }
    rfs->journal.size = data - rfs->journal.data;
}
#if 0
/*! \brief внести в журнал запись о файле */
void rfs_journal(RFS* rfs, int type, void* object)
{
    uint8_t * data = rfs->journal.data+ rfs->journal.size;
    data = rfs_encode_type(rfs, data, type);
    switch (type) {
    case RFS_CREATE:
    {
        FileEntry* file = object;
        data = rfs_encode_uint  (rfs, data, file->id);
        data = rfs_encode_uint  (rfs, data, file->size);
        data = rfs_encode_uint  (rfs, data, file->timestamp);//-rfs->last_timestamp); // разница с последней операцией
        data = rfs_encode_string(rfs, data, file->fullname);
        rfs->stat.created++;
    }
    break;
    case RFS_DELETE:
    {
        FileEntry* file = object;
        data = rfs_encode_uint  (rfs, data, file->id);
        rfs->stat.deleted++;
    }
    break;
    case RFS_ATTRIB:
    {
        FileEntry* file = object;
        data = rfs_encode_uint  (rfs, data, file->id);
        data = rfs_encode_uint  (rfs, data, file->size);
        data = rfs_encode_uint  (rfs, data, file->attr);
    }
    break;
    case RFS_APPEND:
    {
        FileEntry* file = object;
        data = rfs_encode_uint(rfs, data, file->id);
        GList *vectors = file->vectors;//g_list_last(file->vectors);
        int len_en = 1;
        if (file->flush_pos != (~0UL)) while (vectors)
        {// пропускаем уже записанные блоки
            RFSvec* vector = vectors->data;
            if ((vector->offset<=file->flush_pos) && (vector->offset+vector->size > file->flush_pos))
            {// нашли позицию записи последнего сектора
                if (vector->offset+vector->size-1 != file->flush_pos)
                {// запишем хвост от вектора
                    uint32_t flush_size = vector->size - (file->flush_pos - vector->offset +1);
                    if (flush_size) {
                        data = rfs_encode_uint(rfs, data, g_list_length(vectors));
                        data = rfs_encode_uint(rfs, data, file->flush_pos +1);
                        data = rfs_encode_uint(rfs, data, flush_size  );
                        file ->flush_pos = vector->offset + vector->size-1;
                        len_en = 0;
                        printf("--> vector append (%u,%u)\n", (unsigned int) file ->flush_pos, (unsigned int)flush_size);
                    }
                } else {
                        printf("--> vector append next (%u) to '%s'\n", (unsigned int) file ->flush_pos, file ->fullname);
                }
                vectors = vectors->next;
                break;
            }
            vectors = vectors->next;
        }
        if (len_en) {
            data = rfs_encode_uint(rfs, data, g_list_length(vectors));
        }

        while (vectors)
        {
            RFSvec* vector = vectors->data;
            data = rfs_encode_uint(rfs, data, vector->offset);
            data = rfs_encode_uint(rfs, data, vector->size  );
            file->flush_pos = vector->offset+ vector->size-1;
            vectors = vectors->next;
        }

    }
    break;
    case RFS_MAPPED:
    {
        GList* bad_blocks = object;
        uint32_t length = g_list_length(bad_blocks);
        data = rfs_encode_uint(rfs, data, length);
        GList *vectors = bad_blocks;
        while(vectors)
        {
            RFSvec* vector = vectors->data;
            data = rfs_encode_uint(rfs, data, vector->offset);
            data = rfs_encode_uint(rfs, data, vector->size);
            vectors = vectors->next;
        }

    }
    break;
    default:
        break;
    }
    rfs->journal.size = data - rfs->journal.data;
}
#endif

#define BLOCK_MEDIA_PAGE_SIZE 4096
extern BlockMedia default_block_media;
static BlockMedia* media = &default_block_media;
RFSvec* rfs_space =NULL;
RFS default_fs;

static FILE* _file_open(struct _FileObject * file, const char* mode)
{
	RFSfd * fd = file->fp;
	if (fd==NULL) {// создать
		fd = g_slice_new(RFSfd);
		fd->file = file;
		// выравниваем на страницу, число полных страниц
		size_t size = (file->file_size + (BLOCK_MEDIA_PAGE_SIZE-1))/BLOCK_MEDIA_PAGE_SIZE;
		uint32_t offset = rfs_vector_alloc(&rfs_space, 0, size, &size);
		RFSvec* vec = g_slice_new(RFSvec);
		vec->offset = offset, vec->size = size;
		vec->next = NULL;
		fd->space = vec;
		file->fp = fd;
	}
	fd->_p = media->read(media->data, NULL, fd->space->offset, fd->space->size*BLOCK_MEDIA_PAGE_SIZE);
	fd->_r = fd->space->size*BLOCK_MEDIA_PAGE_SIZE;// позиция чтения, число байт до конца файла = позиция записи.
	return (FILE*)fd;
}
extern DeviceInfo_t * ThisDevice;

static volatile int file_uid = 0;
struct _FileObject * _file_new(char * filename, uint8_t * buffer, size_t len)
{
    uint32_t object_identifier = OID(_FILE, (++file_uid));
    Object_t* object = _object_new(object_identifier);
    _object_init(ThisDevice, object_identifier, object);
    object->object_name = filename;

    struct _FileObject * file = (struct _FileObject *)object;//malloc(sizeof(struct _FileObject));
    file->file_size = len;
    size_t size = (file->file_size + (BLOCK_MEDIA_PAGE_SIZE-1))/BLOCK_MEDIA_PAGE_SIZE;
    uint32_t offset = rfs_vector_alloc(&rfs_space, 0, size, &size);
    RFSvec* vec = g_slice_new(RFSvec);
    vec->offset = offset, vec->size = size;
    vec->next = NULL;
    RFSfd* fd = g_slice_new(RFSfd);
    fd->space = vec;
    fd->_p = buffer;
    fd->_r = fd->space->size*BLOCK_MEDIA_PAGE_SIZE;// позиция чтения, число байт до конца файла = позиция записи.

    file->fp = fd;
    return file;
}
/* выделить место */
void _file_init(BlockMedia *m, uint32_t offset, size_t size)
{
    RFSvec* vec = g_slice_new(RFSvec);
    vec->offset = offset, vec->size = size;
    vec->next = NULL;
    RFS* rfs = &default_fs;
    rfs->space = vec;
    rfs->media = m;
    rfs->mapped= NULL;
    rfs->revision = 0;
    rfs->volume_id = 0xDEC0DDED;
    rfs->volume_size=offset+size;
    rfs->journal.data = malloc(BLOCK_MEDIA_PAGE_SIZE);
    rfs->journal.size = 0;
    rfs->journal.block =0;

/// выделить пространство для журнала
    offset = rfs_vector_alloc(&rfs->space, offset, 1, &size);

    vec = g_slice_new(RFSvec);
    vec->offset = offset, vec->size = size;
    vec->next = NULL;

    rfs->journal.block = offset;
    rfs->journal.space = vec;
    // не все инициализировал



    media = m;
    rfs_space = rfs->space;
//rfs_journal_create/regen?
    uint8_t * data = rfs->journal.data + rfs->journal.size;
    *data++ = RFS_VOLUME;
    data = rfs_encode_u32 (data, ASN_CONTEXT(0), rfs->volume_id);
    data = rfs_encode_uint(data, ASN_CONTEXT(1), rfs->revision++);
    data = rfs_encode_uint(data, ASN_CONTEXT(2), rfs->volume_size);
    rfs->journal.size = data - rfs->journal.data;
    // flush

}
#if 0
// Краткость - сестра таланта
size_t fwrite(const void * restrict buffer, size_t length, size_t nitems, FILE*restrict fp)
{
	length *= nitems;
	RFSfd* fd = (RFSfd *)fp;
	size_t actual_size = 0, size;// число записанных байт
	// выделяется сегмент, проверить size
	do {
		if (fd->_r == 0) {// выделить следующий вектор
			size = BLOCK_MEDIA_PAGE_SIZE;// увеличить до единицы стирания
			uint32_t offset = rfs_vector_alloc(&rfs_space, fd->space->offset+fd->space->size, size, &size);// выделить блок
			if (size==0) {
				break;
			}
			rfs_vector_append(&fd->space, offset, size);
			fd->_r = size*BLOCK_MEDIA_PAGE_SIZE;// доступно для записи
		}
		size = length - actual_size;
		if(fd->_r < size) size = fd->_r;
		media->append(media->data, (fd->space->offset + fd->space->size)*BLOCK_MEDIA_PAGE_SIZE - fd->_r, buffer, size);
		fd->_r -= size, buffer+=size;
		actual_size += size;
	} while (actual_size!=length);
	return actual_size;
}
size_t fread(void *restrict buffer, size_t length, size_t nitems, FILE *restrict fp)
{
	length *= nitems;
	RFSfd* fd = (RFSfd *)fp;
	size_t actual_size = 0;// число прочитанных байт
	do {
		if (fd->_r == 0) {// читать невозможно, подгрузить следующий вектор
			if(fd->space == NULL || fd->space->next==NULL) break;
			fd->space = fd->space->next; // теряется предыдущий?
			fd->_p = media->read(media->data, fd->_p, fd->space->offset, fd->space->size*BLOCK_MEDIA_PAGE_SIZE);
			// если последний фрагмент
			fd->_r =  fd->space->size*BLOCK_MEDIA_PAGE_SIZE;
		}
		size_t size = length - actual_size;
		if(fd->_r < size) size = fd->_r;
		__builtin_memcpy(buffer, fd->_p, size);
		fd->_p += size, fd->_r -= size;// доступно для чтения
		actual_size += size;
	} while (actual_size!=length);
	return   actual_size;
}
int fclose(FILE* fp)
{
	// если открыт на запись, вернуть в систему остаток файла
	// if (media->unref) media->unref();
	return 0;
}
/*
int feof(FILE* fp){
	return (fp->_r == 0);
}*/
long ftell(FILE* fp)
{
	RFSfd * fd = (RFSfd *)fp;
	// позиция чтения, сколько осталось до конца -- плохо
	return (fd->space->size*BLOCK_MEDIA_PAGE_SIZE) - fd->_r;
}
int fseek(FILE* fp, long offset, int whence)
{
	RFSfd* fd = (RFSfd *)fp;
	if (whence == SEEK_SET)
		fd->_r  = offset;
	else if (whence == SEEK_CUR)
		fd->_r += offset;
	else if (whence == SEEK_END)
		fd->_r  = 0;//fd->_w;
	return 0;
}
#endif // 0
/*! \brief Инициализация файловой системы
	\ingroup _rfs
 */
void* RFS_init(void* data) {
    printf("%s\r\n", __FUNCTION__);
    //BlockMedia *
	media = (BlockMedia*)data;

    int status = media->init(media->data);
    if(status != 0) return NULL;

    rfs_journal_init(&default_fs, ThisDevice,  media, 0, media->num_blocks);
	return data;
}
