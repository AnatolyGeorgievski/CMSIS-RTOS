/*! \brief Прикладной протокол

Задачи 
(1) Дефрагментация больших пакетов, пересылка конфигурационных файлов
(2) Повторная доставка фрагментов при потере
(3) Повторные запросы пакетов при потере.
(4) Восстановление позции чтения/записи при потере или дублировании пакета
(5) Асинхронная обработка данных (в другом процессе). 
(6) Безопасность, шифрование данных, целостность, подпись

Поддерживаются два типа сообщений: Запрос с откликом (Confirmed_Service) и без отклика(Unconfirmed_Service). 

Формат пакетов:
Используется один статусный байт
[0..4] -- последовательный номер пакета(фрагмента) seq_number
[5] SEG -- сегмент, фрагментированный пакет
[6] MOR -- указание на продолжение обмена
[7] NAK -- ошибка или отрицание

Типы статусов в ответе:
ACK(0) -- подтверждение получения
NAK(4) -- нарушение последовательности отсылки, аргументом является последний обработанный номер (seq_number_last)
	по номеру восстанавливается порядок загрузки, пропущенный пакет.

R3_ABORT -- остановка сессии, сброс параметров сессии в начальное состояние
R3_RESJECT -- не допустимое значение параметра, из этого состояние можно продолжить обработку данных


 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "thread.h"
#include "atomic.h"
#include "r3v2protocol.h"
#include "crc.h"

#define R3_APDU_SET 0x01
#define R3_APDU_GET 0x81

#define R3_OK 		0x00
#define R3_ERROR 	0x80// NAK
#define R3_REJECT 	0xA0// SEG NAK
#define R3_BUSY		0xC0// MOR NAK
#define R3_ABORT 	0xE0// SEG MOR NAK
#define R3_LastSegment	0x20
#define R3_Segment		0x60
#define R3_SegmentACK	0x00
#define R3_SegmentNAK	0x80

#define SEG 	(1<<5)
#define MOR 	(1<<6)
#define NAK 	(1<<7)
#define SEQ_MASK 0x1F

#define MTU 64
#define CONF_SERV_req (1<<0)
#define CONF_SERV_rsp (1<<1)
#define UNCONF_SERV_req (1<<2)

#define SEC_LVL_NONE  0
#define SEC_LVL_CRC   1
#define SEC_LVL_AUTH  2 /* MGM */
#define SEC_LVL_ENC   3 /* MGM */


uint8_t * apdu_buffer;
size_t    apdu_length=0;
volatile uint32_t apdu_signals;
osService_t* apdu_service=NULL;


int apdu_service_func(osService_t* svc, void* data)
{
	printf("APDU service run\r\n");
	uint32_t signals = osServiceGetSignals(apdu_service);
	if (signals & CONF_SERV_req){
		printf("APDU conf service request\r\n");
	}		
	return 0;
}

static osService_t apdu_service_instance;
static void __attribute__((constructor)) init()
{
	apdu_service = osServiceCreate(&apdu_service_instance, apdu_service_func, NULL);
	osServiceRun(apdu_service);
	
}
/*! \brief 
Безопасность бывает разная. В простейшем случае нам надо быть уверенными, что при передаче целостность сохранилась. 
Это можно доказать с использованием ключевой информации -- вычислением имитовставки IMIT, CMAC, крипто-хеша HMAC или CRC32. 
Вычислять целостность лучше по ходу загрузки фрагментов. 
*/
static bool security_check(uint8_t level, uint8_t * data, size_t data_len)
{
	bool result;
	switch ((uint8_t)/* ctx-> */level) {
	case SEC_LVL_NONE:
		result = true;
		break;
#if 0
	case SEC_LVL_ENCRYPT:
		MGM_Encrypt(K, ICN, P, A) 
		break;
	case SEC_LVL_AUTH:
//		CMAC_t cmac = CMAC_init();
extern uint64_t magma_cmac(const uint32_t *K, uint8_t *iv, size_t vlen, uint8_t* data, size_t len);
		magma_cmac(ctx->K, ctx->iv, ctx->iv_len, data, data_len);
		break;
#endif
	case SEC_LVL_CRC:
		if (data_len<=4) return false;
		CRC32 crc = CRC32_init(0);
		crc = crc_from_block(crc, data, data_len-4);
		crc = CRC32_finalize(crc);
		result = *(CRC32*)(data+data_len) == crc;
		break;
	default:
		result = false;
		break;
	
	}
	return result;
}

static int r3_apdu_set(uint8_t *buffer, int *length)
{
	/*! \brief Загрузка фрагмента */
	void save_segment(uint8_t *buffer, size_t length){
		memcpy(apdu_buffer+apdu_length, buffer, length);
		apdu_length+=length;
	}
/*! \todo статус */
	static struct {
		enum {IDLE, SEGMENTED_REQUEST} state;
		uint8_t security_level; // LVL_AUTH
		uint8_t seq_number_last;
	} ctx = {.state = IDLE, .security_level=SEC_LVL_NONE};

	uint8_t pdu_type = buffer[0];
	uint8_t seq_number=pdu_type&SEQ_MASK;
	int status=R3_OK;
	size_t len  = *length;
	switch (pdu_type&~SEQ_MASK){
	case (SEG|MOR):{// Data Record
		if (*length) {// размер данных не нулевой 
			if (ctx.state!=SEGMENTED_REQUEST){
				if (seq_number!=0) {
					status = R3_ABORT;
					break;
				}
				ctx.state = SEGMENTED_REQUEST;
				apdu_length  =0;
			} else 
			if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {
				status = NAK;
				seq_number = ctx.seq_number_last;
				break;
			}
			ctx.seq_number_last = seq_number;
			save_segment(buffer+1, len-1);
			break;
		} else {
			status = R3_REJECT;
		}
	} break;
	case (SEG):// подтверждение последнего пакета
		if (ctx.state!=SEGMENTED_REQUEST) {
			status = R3_ABORT;
			break;
		}
		if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {
			break;
		}
	case (0):// запрос не сегментирован
		ctx.seq_number_last = seq_number;
		save_segment(buffer+1, len-1);
		if(security_check(ctx.security_level, apdu_buffer, apdu_length))
			osServiceSignal(apdu_service, CONF_SERV_req);
		else 
			status = R3_ABORT;
		break;
	case (NAK):// запрос не без ответа
		ctx.seq_number_last = seq_number;
		save_segment(buffer+1, len-1);
		if(security_check(ctx.security_level, apdu_buffer, apdu_length))
			osServiceSignal(apdu_service, UNCONF_SERV_req);
		else 
			status = R3_ABORT;
		break;
	case R3_ABORT:
		ctx.state = IDLE;
		break;
	default:
		status = R3_REJECT;
		break;
	}
	*length = 0;
	return status|seq_number;
}

static int r3_apdu_get(uint8_t *buffer, int *length)
{
	static struct {
		enum {IDLE, SEGMENTED_RESPONSE} state;
		uint8_t seq_number_last;
		size_t offset;
	} ctx = {.state = IDLE};
	uint8_t pdu_type = buffer[0];
	uint8_t seq_number=pdu_type&SEQ_MASK;
	int status=R3_OK;
	size_t len = 0;
	switch (pdu_type&~SEQ_MASK){
	case (  0):{// запрос данных
		if ((apdu_signals & (CONF_SERV_rsp|CONF_SERV_req))== (CONF_SERV_req)){
			status = R3_BUSY;
			break;
		} else 
		if ((apdu_signals & (CONF_SERV_rsp|CONF_SERV_req))==0) {
			status = R3_ABORT;
			break;
		}
		if ((int32_t)apdu_length<0) {// можно флагами выяснять
			len = ~apdu_length;
			status = R3_ERROR;
		} else
		if (apdu_length>MTU) {
			status = SEG|MOR;
			ctx.seq_number_last=seq_number;
			ctx.state = SEGMENTED_RESPONSE;
			len = MTU;
		} else {
			len = apdu_length;
		}
		ctx.offset=0;
		memcpy(buffer, apdu_buffer, len);
	} break;
	case SEG:// послединий сегмент
		if (ctx.state != SEGMENTED_RESPONSE){
			status = R3_REJECT;
			break;
		}
		if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {
			status = SEG|NAK;
			seq_number = ctx.seq_number_last;
			break;
		}
		ctx.seq_number_last = seq_number;
		if (apdu_length==ctx.offset) {
			atomic_fetch_and(&apdu_signals, ~(CONF_SERV_rsp|CONF_SERV_req));// освободить буфер!
			ctx.state=IDLE;
			break;
		}
		break;
	case (SEG|MOR):// подтверждение сегмента
	// надо разрешить повторную передачу
		if (ctx.state != SEGMENTED_RESPONSE){
			status = R3_REJECT;
			break;
		}
		if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {
			status = SEG|NAK;
			seq_number = ctx.seq_number_last;
			break;
		}
		ctx.seq_number_last = seq_number;
		if (apdu_length>ctx.offset+MTU) {
			status = SEG|MOR;
			len = MTU;
		} else {
			status = SEG;
			len = apdu_length-ctx.offset;
		}
		memcpy(buffer, apdu_buffer+ctx.offset, len);
		ctx.offset+=len;
		break;
	default:
		status = R3_REJECT;
		break;
	case R3_ABORT:
		ctx.state=IDLE; ctx.offset=0;
		break;
	}
	*length = len;
	return status|ctx.seq_number_last;
}

R3CMD(R3_APDU_SET, r3_apdu_set);
R3CMD(R3_APDU_GET, r3_apdu_get);
