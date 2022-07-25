/*! обновление прошивки

Сценарий 1) ПРошивка живет на флеш диске, надо файл скопировать в другое место.
Cценарий 2) Прошивка заливается в два этапа: заливается загрузчик, затем управление передается загрузчику. 

\todo отступ равен 4 блокам = одна страница памяти (сегмент).
	\sa Motorola S-record file format,
	'S'
	[0] - команда (тип) записи
	[1] - длина данных
	[1..2] - смещение от начала сегмента, младшие 16 бит.
	[... ] - данные
	[FCS] - контрольная сумма
	
 */
// r3cmd_flash.c
#include "board.h"
#include "cmsis_os.h"
#include "r3v2protocol.h"
#include "iflash.h"
#include "crc.h"
#include <stdio.h>
//#include <string.h>
#define FLASH_SIGNAL 21
#define R3_SNMP_FLASH_ISP 	0x61
#define R3_SNMP_FLASH_CRC 	0xE2
#define R3_SNMP_FLASH_COPY 	0x62

extern void iflash_bootloader(uint32_t offset, size_t size);

/*!	\brief запись данных во флеш
	':' - текстовый префикс
	[0] - длина данных
	[1..2] - смещение от начала сегмента, младшие 16 бит.
	[3] - команда
	Команды см Intel Hex Format (Revision A ed.). Intel. 1988-01-06 http://microsym.com/editor/assets/intelhex.pdf

	’00’	Data Record
	’01’	End of File Record
	’02’	Extended Segment Address Record
	’03’	Start Segment Address Record
	’04’	Extended Linear Address Record
	’05’	Start Linear Address Record
 */
static int r3_flash(uint8_t *buffer, int *length)
{
	static uint16_t record_offset=0;
	static uint16_t record_size  =0;
	static uint16_t record_number=0;
	uint8_t code = buffer[3];
	int status=R3_ERR_OK;
//	CRC32 crc=CRC32_Init(0);
	static void* vh = NULL;
//	printf("iFLASH code=%02X len=%d\r\n", code, *length);
	switch (code){
	case 0x00: {// Data Record
		uint32_t size  = buffer[0];
		if (size!=0 && (size+4)==*length) {// размер данных не нулевой 
			
			uint16_t offset // =__builtin_bswap16(*(uint16_t*)&buffer[1]);
				= ((uint16_t)buffer[1]<<8) | (uint16_t)buffer[2];
			if(0) printf("iFLASH request %d %d\r\n", offset, size);
			if (record_offset+record_size!=offset) {
				status=R3_ERR_NAK;// нарушен порядок следования
				printf("iFLASH SegmentNAK %04X %04X\r\n", record_offset+record_size, offset);
				break;
			}
			uint8_t *data  =&buffer[4];
//			crc = crc_from_block(crc, data, size);
			if (vh==NULL) {// записываем в файл
				vh = iflash_open(4, FLASH_SIGNAL);// osSignalRef(osThreadGetId(), FLASH_SIGNAL)
			} else {
				osEvent event = osSignalWait(1<<FLASH_SIGNAL, 1);//100);
				if (event.status & osEventTimeout){
					printf("iFLASH Timeout\r\n");
					status = R3_ERR_BUSY;
				} else {
					record_number= (record_number+1)&0x1F;// подтвержденные номера
					osSignalClear(osThreadGetId(),1<<FLASH_SIGNAL);
					record_offset += record_size;
					//printf("iFLASH Tx Done\r\n");
				}
			}
			if(0) printf("iflash_send %p %d\r\n", data, size);
			iflash_send(vh, data, size);
			record_size=size;
			/// TODO возможно ли не ждать завершения?
//			printf("iFLASH write done\r\n");
		} else {
			printf("iFLASH SegmentNAK %d %d\r\n", size, *length);
			status=R3_ERR_CRC;
		}
	} break;
	case 0x01:// подтверждение последнего пакета
		if (vh==NULL) {
			status = R3_ERR_ABORT;
			break;
		}
		osEvent event = osSignalWait(1<<FLASH_SIGNAL, 1);//100);
		if (event.status & osEventTimeout){
			printf("iFLASH Timeout\r\n");
			status = R3_ERR_BUSY;
		} else {
			osSignalClear(osThreadGetId(),1<<FLASH_SIGNAL);
			iflash_close(vh); 
		}
		vh = NULL; record_offset=0; record_size =0; record_number=0;
		break;
	default:
		printf("iFLASH Abort %02X %d\r\n", code, *length);
		status = R3_ERR_ABORT;
		break;
	}
	*length = 0;
	return status|record_number;
}
static int r3_flash_verify(uint8_t * buf, int* length)
{
	uint32_t size = *(uint32_t*)buf;
	void* data = iflash_recv(NULL, 4); // блок 4, первые 4 резервируем на конфиг
	CRC32 crc = CRC32_init(0);
	crc = crc_from_block(crc, data, size);
	crc = CRC32_finalize(crc);
	*(uint32_t*)buf = crc;
	*length = 4;
}
/*! \brief выполнить проверку CRC от новой прошивки во флеш и выполнить перезагрузку. 
 */
static int r3_flash_copy(uint8_t * buf, int* length)
{
//		extern void ITM_flush();
	uint32_t size = (uint32_t)buf[3]<<24 | (uint32_t)buf[2]<<16 | (uint32_t)buf[1]<<8 | (uint32_t)buf[0]<<0;
	printf("size= %08X\r\n", size);
	uint32_t crc1 = (uint32_t)buf[7]<<24 | (uint32_t)buf[6]<<16 | (uint32_t)buf[5]<<8 | (uint32_t)buf[4]<<0;
	printf("CRC = %08X\r\n", crc1);
	void* data = iflash_recv(NULL, 4); // блок 4, первые 4 резервируем на конфиг
	CRC32 crc = CRC32_init(0);
	crc = crc_from_block(crc, data, size);
	crc = CRC32_finalize(crc);
//	printf("Data = %p CRC = %08X\r\n", data, crc);
//	ITM_flush();
	if (crc == crc1) {
		printf("CRC = %08X ..reboot\r\n", crc);
//		ITM_flush();
//		osDelay(1000);
		//extern void iflash_bootloader(uint32_t block, uint32_t size);
		iflash_bootloader(4<<9, size);
	}
	
	return R3_ERR_OK;
}

R3CMD(R3_SNMP_FLASH_ISP, r3_flash);
R3CMD(R3_SNMP_FLASH_CRC, r3_flash_verify);
R3CMD(R3_SNMP_FLASH_COPY, r3_flash_copy);
