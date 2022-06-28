/*
Ошибки: 
*/
#include <stdint.h>
#include <cmsis_os.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "config.h"
#include "modbus.h"
#include "pio.h"
// --------------------
/* CRC контроль целостности
MODBUS
    width=16 poly=0x8005 init=0xffff refin=true refout=true xorout=0x0000 check=0x4b37 name="MODBUS"
    CRC presented low byte first.
*/

#include <stdint.h>
#include <stdio.h>
#include "bsearch.h"

static const uint16_t CRC16M_Lookup4[16]={
    0x0000, 0xCC01, 0xD801, 0x1400,
	0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401,
	0x5000, 0x9C01, 0x8801, 0x4400
};
#define MODBUS_INIT  0xFFFF
#define MODBUS_FINI  0x0000
static uint16_t modbus_crc(uint8_t* data, int length)
{
	uint16_t crc = MODBUS_INIT;//
	int i;
	for (i=0; i<length;i++){
        register uint8_t val = data[i];
        crc = (crc>>4) ^ CRC16M_Lookup4[(crc ^ (val   )) & 0xF];
        crc = (crc>>4) ^ CRC16M_Lookup4[(crc ^ (val>>4)) & 0xF];
	}
	crc ^= MODBUS_FINI;
    return crc;
}
#define WEAK __attribute__ ((weak))

/*! \brief Проверка контрольной суммы кадра
 */

#define MODBUS_MAX_PDU_LENGTH 260
static uint8_t pdu[MODBUS_MAX_PDU_LENGTH];
static uint32_t deviceID_mem CONFIG_ATTR = 0x17;// адрес по умолчанию для автоматической настройки
#define BOARD_REV "16.0724" // дата YY.MMDD
static ModbusEncObj device_mei[] = {
	{0, 6, "Geolab"},
	{1, sizeof(BOARD_NAME), BOARD_NAME},
	{2, sizeof(BOARD_REV), BOARD_REV},
};
static uint32_t serial = 0x12345678;
static ModbusMap device_map[] = {
	{130, 2, &serial}, // Serial Number
};

void modbus(void const* user_data)
{
	/* запустить службу обработки протокола */
	void* vh = NULL; // дескриптор записи во флеш
	uint8_t deviceID = deviceID_mem;
	ModbusCtx ctx = {
		.mei=device_mei,
		.mei_count = sizeof(device_mei)/sizeof(ModbusEncObj),
		.map_regs_ro = device_map, .map_regs_ro_num = sizeof(device_map)/sizeof(ModbusMap), 
	};

	osEvent event;
	void * h = usart_open(0, osSignalRef(osThreadGetId(), 1));
	do {
		osTransfer * transfer = usart_recv(h, pdu, MODBUS_MAX_PDU_LENGTH);
		event = osSignalWait(1<<1, 10000);
		if (event.status & osEventTimeout){
			printf("USART1 Rx Timeout\r\n");
			//usart_stop(h);
		} 
		if (event.status & osEventSignal) {
			osSignalClear(osThreadGetId(),1<<1);
			if (transfer->status!=0){
				printf("Rx error = %d\r\n", transfer->status);
			} else {
				int i;
				int len = transfer->size;
				if (len>4) {
				//if (pdu[len-1]=='\0') len--; // опять лишний байт получили
				uint16_t fcs16 = *(uint16_t*)&pdu[len-2];
				uint16_t crc16 = modbus_crc(&pdu[0], len-2);
				for(i=0; i<len; i++){
					printf("%02X ", pdu[i]);
				}
				printf(">> Modbus: L=%d,FCS=%04X ..%s\r\n", len, fcs16, crc16==fcs16?"ok":"fail");
				if (crc16==fcs16) {
					len = modbus_server_state(&ctx, &pdu[1], len-2);
					len++;// шапку добавили
					*(uint16_t*)&pdu[len] = crc16 = modbus_crc(&pdu[0], len);
					len+=2;// CRC добавили
					pio_set_output(BOARD_USART1_TE);
					usart_send(h, pdu, len);
					event = osSignalWait(1<<1, 1000);
					pio_reset_output(BOARD_USART1_TE);
					osSignalClear(osThreadGetId(),1<<1);
					for(i=0; i<len; i++){
						printf("%02X ", pdu[i]);
					}
					printf("<< Modbus: L=%d,FCS=%04X\r\n", len, crc16);
				}
				//break;
				}
			}
		}
	} while(1);
}
/*! \brief заполнение пакета в ответ на MEI - Modbus encapsulated
 */
static int modbus_encapsulated(ModbusCtx* ctx, uint8_t *pdu, int size)
{
    uint8_t mei_type = pdu[1];
    if (mei_type!=0x0E) {
        printf("-- MEI unknown extension\r\n");
        return -2;
    }
    uint8_t dev_id = pdu[2];  //
    uint8_t obj_id = pdu[3];  // ReadDevId code
    int len = 3;
    pdu[len++] = 0x01; // 0x01: basic identification (stream access only) // Conformity Level
    pdu[len++] = 0x00; // More Follows
    pdu[len++] = 0x00; // Next Object Id
    pdu[len++] = 0x00; // Number Of Objects
    // {Id, Length, Value}
	int i;
	for (i=obj_id; i<ctx->mei_count; i++)
	{
		ModbusEncObj*obj = &ctx->mei[i];//->data;
        pdu[len++] = i;
        pdu[len++] = obj->len;
		__builtin_memcpy(&pdu[len], obj->data, obj->len);
		len+=obj->len;
	}
	//if (i<ctx->mei_count) { pdu[4] =; pdu[5]=;}
	pdu[6] = i - obj_id;
    return len;
}
/*! \brief сравнение элемента ModbusMap*/
static int modbus_key_compare_cb(const void* a, const void* b)
{
	const uint16_t addr = *(const uint16_t*)a;
	const ModbusMap* elt = b;
	if ((elt->reg_addr < addr) && (elt->reg_addr + elt->reg_length) > addr) return 0;
	else return addr - elt->reg_addr;
	//return (*da > *db) - (*da < *db);
}
int modbus_server_state(ModbusCtx* ctx, uint8_t *pdu, int size)
{
    uint16_t address,count;//,value;
    uint8_t exception_code = 0, res;
    
    switch (pdu[0]){// uint8_t function_code = pdu[0];
	case 0x03: // 6.3  03 (0x03) Read Holding Registers 
	{
		address = ntohs(*(uint16_t*)&pdu[1]);
		count   = ntohs(*(uint16_t*)&pdu[3]);
		if (count==0 || count > 0x07D) { exception_code = ERR_ILLEGAL_DATA_VALUE; break; }
		ModbusMap* map = bsearch(&address, ctx->map_regs_ro, ctx->map_regs_ro_num, sizeof(ModbusMap), modbus_key_compare_cb);
		if (map) {
			int offset = map->reg_addr - address;
			uint16_t* base = (uint16_t*)map->data + offset;
			if (count+address > map->reg_addr + map->reg_length) { exception_code = ERR_SERVER_DEVICE_FAILURE; break; }
			//__builtin_memcpy(&pdu[2], base, count<<1);
			int i;
			uint8_t* dst = &pdu[2];
			for (i=0; i<count;i++){
				((uint16_t*)dst)[i] = htons(base[i]);
			}
			pdu[1] = count<<1;
			res = 2 + (count<<1);
		} else {
			exception_code = ERR_ILLEGAL_DATA_ADDRESS;
		}
	} break;
    case 0x2B: // 6.19 43 (0x2B) Encapsulated Interface Transport
        //6.21 43 / 14 (0x2B / 0x0E) Read Device Identification
//        g_print("-- MEI extension\n");
        res = modbus_encapsulated(ctx, pdu, size);
        break;
	default:
		exception_code = ERR_ILLEGAL_FUNCTION;
		break;
	}
    if (exception_code){
        pdu[0] |= 0x80;
        pdu[1] = exception_code;
        res = 2;
    }
    return res;
}