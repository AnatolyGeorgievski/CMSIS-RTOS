/*!	\ingroup _system
	\defgroup _config Конфигурация системы
    \update 25.01.2009
	\update 18.09.2019
- переделал вызовы драйвера с тем чтобы сделать возможным прямое чтение из
    флеш - памяти без копирования в буфер.
- При записи данных драйвер может вернуть состояние R3_ERR_BUSY если предыдущая запись не завершена.
	\{
    */

#include "board.h"
#include "cmsis_os.h"
//#include "trace.h"
#include "config.h"
//#include "module.h"
#include "r3v2protocol.h"
#include "r3stdlib.h"
//#include "r3rtos.h"
#include "r3_object.h"
#include "crc.h"
#include "semaphore.h"
#include "atomic.h"
#include <string.h>

//extern uint32_t bacnet_crc32k(uint8_t * data, size_t length);
//#pragma weak crc_from_block = bacnet_crc32k

extern unsigned char __start_config[];
extern unsigned char __stop_config[];

static uint32_t serial_number CONFIG_ATTR = 0;//!< серийный номер устройства
static unsigned char CONFIG_MAGIC[] __attribute__((aligned(4))) CONFIG_ATTR = BOARD_NAME" "__DATE__" "__TIME__;

// [1] - reserved
// [2] - reserved
// [3] - месяц/minor
// [4] - год
//unsigned long upgrade_to_revision CONFIG_ATTR = 0;
//const unsigned long current_to_revision = R3_RTOS_REVISION;

#define CONFIG_SIZE ((unsigned int)(__stop_config - __start_config +3) & ~3)

#define ALIGN(v, n) (((v)+(n-1))& ~(n-1))

extern BlockMedia iflash_block_media;
extern BlockMedia quadspi_block_media;
extern BlockMedia df_block_media;
// место хранения конфигурации - во внутири флеша или снаружи
static BlockMedia * default_media_driver
#ifdef BOOT_INTERNAL_FLASH
= &iflash_block_media;//df_block_media;
#elif defined(BOOT_INTERNAL_QUADSPI)
= &quadspi_block_media;
#else // dataflash
= &df_block_media;
//#warning "BOOT external DataFLASH"
#endif
#ifndef FS_CONFIG_BASE
#warning "Default value FS_CONFIG_BASE = 0"
#define FS_CONFIG_BASE 0
#endif

static int r3_sys_get_version(unsigned char* buffer, int*size)
{
    //printf("%*s\r\n", sizeof(CONFIG_MAGIC)-1, CONFIG_MAGIC);
    memcpy(buffer, CONFIG_MAGIC, sizeof(CONFIG_MAGIC));
    *size = sizeof(CONFIG_MAGIC);
    return 0;
}
static int r3_sys_reset(unsigned char* buffer, int*size)
{
	NVIC_SystemReset();
	return 0;
}
/*! \brief Изменить серийный номер устройства 
	[0..3] - серийный номер u32
 */
static int r3_sys_set_serial(uint8_t* buffer, int* length)
{
    //serial_number = 
	serial_number = *(uint32_t*)buffer;// (uint32_t)buffer[0] | (buffer[1]<<8) | (buffer[2]<<16) | (buffer[3]<<24);
	*length = 0;
    return R3_ERR_OK;
}
/*! \brief Запросить серийный номер устройства 
	[0..3] - серийный номер u32 LE
*/
static int r3_sys_get_serial(uint8_t* buffer, int* length)
{
	__builtin_memcpy(buffer, &serial_number, 4);//(uint32_t)buffer[0] + (buffer[1]<<8) + (buffer[2]<<16)+ (buffer[3]<<24);
	*length = 4;
    return R3_ERR_OK;
}

R3CMD(R3_SNMP_GET_VERSION, r3_sys_get_version);
R3CMD(R3_SNMP_SOFT_RESET, r3_sys_reset);
R3CMD(R3_SNMP_ID, r3_sys_set_serial);// серийный номер устройства
R3CMD(R3_SNMP_GET_ID, r3_sys_get_serial);

/*! \brief блокирует одновременный доступ к операции сохранения во флеш */
static volatile int FLASH_wr_lock = 1;
/*! \brief Сохранение статической конфигурации системы в NV-RAM

Функция сохраняет, как умеет, в бинарном виде сегмент памяти CONFIG, где
хранятся настройки.
Команда выдается без параметров C003
Если надо восстановить конфигурацию, то надо присвоить system.ID=0xFF и сохранить.
c000FF
c003
*/
struct _Header {
	uint16_t size;
	uint16_t offset;
};
static volatile uint32_t log_offset=0;

int FLASH_SaveConfig(unsigned char *buffer, int *length)
{
    BlockMedia *media = default_media_driver;
	if (length) *length = 0;

    printf("SaveConfig: base:0x%08lX, size:0x%08X\r\n", (unsigned long)media->data, CONFIG_SIZE);

	int block = FS_CONFIG_BASE;
	int size = CONFIG_SIZE;
	unsigned char *src = __start_config;
	CRC32 crc = crc_from_block(0, src, size);
	if (crc == *(CRC32*)(src+CONFIG_SIZE)) return R3_ERR_OK; /* данные не изменились, нет необходимости сохранять */ 
	
	if(!semaphore_enter(&FLASH_wr_lock)) return R3_ERR_BUSY;

	struct _Header hdr = {.size=CONFIG_SIZE};
	
	*(CRC32*)(src+ALIGN(size,4)) = crc;// с выравниванием
	//*(uint32_t*)(src+size+sizeof(CRC32)) = ~0UL;// 0xFFFF FFFF, т.е. стираем журнал
	size+=sizeof(CRC32);//+sizeof(uint32_t);// записываем вместе с нулевой шапкой 
	log_offset = size;
	do {
		int pagesize = size > BLOCK_MEDIA_UNIT? BLOCK_MEDIA_UNIT: size;
		media->send(media->data, src, block, pagesize); block++;
		src  += pagesize;
		size -= pagesize;
	} while (size);
	/* очистить под журнал еще один блок или более блоков */
	media->send(media->data, src, block, 0); block++;
//	log_position=0;
/* TODO дублировать в памяти */
	semaphore_leave(&FLASH_wr_lock);
	
    return R3_ERR_OK;
}
/*! \brief дописать в журнал */
int FLASH_LogData(unsigned char *buffer, int length)
{
    BlockMedia *media = default_media_driver;

	if (length>0) {// запись в конфигурацию
		struct _Header hdr;
		hdr.size = length+sizeof(struct _Header);
		int offset = buffer - __start_config;
		// hdr.offset = offset;// или 0xFFFF если это запись не в конфигурацию
		if (0<offset && offset+length<=CONFIG_SIZE) {
			hdr.offset = offset;
		} else {
			hdr.offset = 0xFFFF;
		}
		CRC32 crc  = crc_from_block(0, (void*)&hdr, sizeof(struct _Header));
		crc  = crc_from_block(crc, buffer, length);
		if (0)	{// отладка
			printf("LogData start =%d\n", log_offset);
			printf("LogData offset=%d, size=%d,crc=%08X\n", hdr.offset, hdr.size, crc);
		}
		offset = log_offset;//atomic_pointer_exchange(&log_offset, );
		log_offset = offset+ALIGN(hdr.size,4)+sizeof(CRC32);// атомарно!!!
		media->append(media->data, offset, &hdr, sizeof(struct _Header));
		offset+=sizeof(struct _Header);
		media->append(media->data, offset, buffer, length);
		offset+=length;
		media->append(media->data, offset, &crc, sizeof(CRC32));
	}
	return 0;
}
#if 0
/*! \brief выделить */
void* FLASH_file_open(struct _FileObject* f)
{
    BlockMedia *media = default_media_driver;

	if (f->file_size>0) {
		struct _Header hdr;
		hdr.size = f->file_size+sizeof(struct _Header);
		hdr.offset = -1;
		volatile int* ptr = (volatile int*)&log_offset;
		int offset, new_offset;
		do {
			offset = *(int*)ptr; //log_offset;//atomic_pointer_exchange(&log_offset, );
			new_offset = offset + ALIGN(hdr.size,4)+sizeof(CRC32);
		} while(!atomic_int_compare_and_exchange(ptr, offset, new_offset));
		media->append(media->data, offset, &hdr, sizeof(struct _Header));
		f->fp = media->data + offset + sizeof(struct _Header);
	}
	return f;
}
int FLASH_file_write(struct _FileObject* f, unsigned char *buffer, int length)
{
    BlockMedia *media = default_media_driver;
	
	size_t offset = (f->fp - media->data) + f->start_position;
	return media->append(media->data, offset, buffer, length);
}
void FLASH_file_close(struct _FileObject* f)
{
    BlockMedia *media = default_media_driver;
	CRC32 crc;
	crc = crc_from_block(0, (void*)f->fp - sizeof(struct _Header),  sizeof(struct _Header));
	crc = crc_from_block(crc, f->fp, f->file_size);
	size_t offset  = (f->fp - media->data) + ALIGN(f->file_size,4);
	media->append(media->data, offset, &crc, sizeof(CRC32));
}
#endif

/*! \brief Загрузка статической конфигурации системы из NV-RAM

- DataFlash или другой совместимый по смыслу носитель. Данные храняться по номеру блока 0.

Если версия системы изменилась, то восстановление надо производить
из другого источника, например из конфигурации в формате SNMP-report BER-TLV

Функция вызывается из main()
*/
void FLASH_LoadConfig()
{
	log_offset = CONFIG_SIZE+sizeof(CRC32);
    BlockMedia *media = default_media_driver;
    int status = media->init(media->data);
    if (status != R3_ERR_OK) return;

    unsigned char *buffer = NULL;// [sizeof(CONFIG_MAGIC)];
    const int block = FS_CONFIG_BASE;
	int offset = CONFIG_MAGIC - __start_config;
    buffer = media->read(media->data, NULL, block+(offset/BLOCK_MEDIA_UNIT), (offset%BLOCK_MEDIA_UNIT) + sizeof(CONFIG_MAGIC));
    int res = memcmp(&buffer[offset%BLOCK_MEDIA_UNIT], CONFIG_MAGIC, sizeof(CONFIG_MAGIC)-1);
	
	// \todo что если CONFIG_MAGIC пересекает блок памяти?
    if (!res) printf("Device Config: %*s\r\n", (int)sizeof(CONFIG_MAGIC)-1,buffer);
    else {
		if (media->unref) media->unref(media->data, buffer);
		return; //В память ничего не писалось. Незачем это загружать.
	}
	/* проверить серийный номер устройства */
	offset = (uint8_t*)&serial_number - __start_config;
/*	if ((offset/BLOCK_MEDIA_UNIT)!=0 && (offset/BLOCK_MEDIA_UNIT)!=0) 
	{
		buffer = media->read(media->data, NULL, block+(offset/BLOCK_MEDIA_UNIT), (offset%BLOCK_MEDIA_UNIT) + sizeof(serial_number));
	}*/
	uint32_t serial_flash = *(uint32_t*)&buffer[offset%BLOCK_MEDIA_UNIT];
	if(serial_flash == ~0) {
		printf("Use system defaults\r\n");
		if (media->unref) media->unref(media->data, buffer);
		return;
	}
	/* подсчет контрольной суммы */
	struct _Header hdr = {.size=CONFIG_SIZE};
	//while (hdr.offset<CONFIG_SIZE && hdr.size+hdr.offset<=CONFIG_SIZE) 
	offset = 0;// от начала блока
	buffer = media->read(media->data, NULL, block, offset+ALIGN(hdr.size,4)+(sizeof(CRC32)+ sizeof(struct _Header)));
	do {
		unsigned char * dst = __start_config + hdr.offset;
		CRC32 crc = crc_from_block(0, buffer+offset, hdr.size);
		if (*(CRC32*)(buffer+offset+ALIGN(hdr.size,4)) != crc) {
			printf("CRC error = %08X <> %08X\r\n", crc, *(CRC32*)(buffer+offset+ALIGN(hdr.size,4)));
			if(media->unref) media->unref(media->data, buffer);
			return;
		}
		printf("Config size: %d b\n", hdr.size);
		if (offset!=0) {
			offset += sizeof(struct _Header);
			hdr.size-=sizeof(struct _Header);
		}
/*		if (hdr.offset==0xFFFF){// это скорее файл чем конфиг
			
		} else*/
		if (buffer+offset != dst) 
		{
			uint32_t *d = (void*)dst;
			uint32_t *s = (void*)(buffer+offset);
			int count = (hdr.size+3)>>2; // конфигурация должна быть выравнена на слово 32 бит
			if (count) do {
				*d++ = *s++;
			} while (--count);
		} 
		printf("LogData offset=%d, size=%d,crc=%08X\n", hdr.offset, hdr.size, crc);
		offset+=ALIGN(hdr.size,4)+sizeof(CRC32);
		hdr = *(struct _Header*)(buffer+offset);
		//offset+=sizeof(struct _Header);
	} while ((uint32_t)hdr.size+hdr.offset<=CONFIG_SIZE);// это может быть запись журнала или файл
	if(media->unref) media->unref(media->data, buffer);
	log_offset = offset;
#if 0
	block = FS_CONFIG_BASE;
	unsigned char * dst = __start_config;
	int fullsize = CONFIG_SIZE+sizeof(CRC32);
	do {
		int size = fullsize>BLOCK_MEDIA_UNIT?BLOCK_MEDIA_UNIT: fullsize;
		buffer = media->read(media->data, dst, block, size);
		block++;
		if (buffer != dst) { // если устройство по какой-то причине не способно писать в буфер
			uint32_t *d = (void*)dst;
			uint32_t *s = (void*)buffer;
			int count = (size+3)>>2; // конфигурация должна быть выравнена на слово 32 бит
			if (count) do {
				*d++ = *s++;
			} while (--count);
		}
		// я освобождаю буфер. При доступе к внутренней Flash
		// освобождение буфера бессмысленно потому что буфер и есть
		// ссылка на Flash. Для конкретного типа носителя может
		// не поддерживаться операция освобождения буфера.
		if(media->unref) media->unref(media->data, buffer);
		dst+= size;
		fullsize -= size;
	} while(fullsize);
// Надо записать журнал поверх конфигурации
#endif
}
// todo hdr->size исключить sizeof(struct _Header)
// надо доработать концепцию чтобы работала на блочном устройстве
void* FLASH_LogRead(unsigned char *buffer, size_t * size) 
{
	BlockMedia *media = default_media_driver;
	struct _Header * hdr;
	if (buffer==NULL) {
		const int block = FS_CONFIG_BASE;
		
		buffer = media->read(media->data, NULL, block, sizeof(struct _Header));
		buffer+= CONFIG_SIZE+sizeof(CRC32);
	} else {
		buffer -= sizeof(struct _Header);
		hdr = (void*)buffer;
		buffer += ALIGN(hdr->size, 4) + sizeof(CRC32);
	}
	hdr = (void*)buffer;
	while (hdr->size!=0xFFFF) {
		CRC32 crc = crc_from_block(0, buffer, hdr->size);
		if (*(CRC32*)(buffer+ALIGN(hdr->size,4)) == crc){
			if (hdr->offset==0xFFFF) {
				*size = hdr->size-sizeof(struct _Header);
				return buffer+sizeof(struct _Header);
			} else {
				if ((uint32_t)hdr->size + hdr->offset<=CONFIG_SIZE) {// обновляем параметры конфигурации
					unsigned char * dst = __start_config + hdr->offset;
					memcpy(dst, buffer+sizeof(struct _Header), hdr->size);
				}
			}
			buffer+=ALIGN(hdr->size,4)+sizeof(CRC32);
		} else {
			break;
		}
		hdr = (void*) buffer;
	}
	if(media->unref) media->unref(media->data, buffer);
	return NULL;
}

/*
static int FLASH_ShowConfig(unsigned char* buffer, int* length)
{
    return R3_ERR_OK;
}*/
R3CMD(R3_SNMP_SAVE_CONFIG, FLASH_SaveConfig);
//R3CMD(R3_SNMP_SHOW_CONFIG, FLASH_ShowConfig);
//! \}
