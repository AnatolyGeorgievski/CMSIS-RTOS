/*! \brief R3 безопасность команд
Функция (0x0С) — отсылка пакета с аутентификацией
Запрос:  [ 0x0С8 || InvokeId8 || SecuredData || MAC32]
Отклик: [ 0x0С8 || InvokeId8 || SecuredData || MAC32]
Данные передаются в открытом виде, с выработкой имитовставки и последующим усечением результата до 32 бит.
*/
#include <cmsis_os.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "r3v2protocol.h"

extern int R3_cmd_snmp_recv2(uint8_t code, uint8_t * buffer, int *length):
extern uint64_t magma_cmac(uint32_t *K, uint8_t* data, size_t len);
extern R3cmdInfo __start_r3_cmd_callbacks[];
extern R3cmdInfo __stop_r3_cmd_callbacks[];
//#define R3_SNMP_SET 0x72
#define R3_SNMP_SET_KEYINFO 	(0x04)
#define R3_SNMP_GET_KEYINFO 	(0x84)
#define R3_SNMP_SET_SECURED 	(0x0C)
#define R3_SNMP_GET_SECURED 	(0x8C)
#define R3_SNMP_SET_ENCRYPTED 	(0x0E)
#define R3_SNMP_GET_ENCRYPTED 	(0x8E)


struct {
	time_t not_before;
	time_t not_after;
	uint32_t* private_key;
	uint8_t* key_id;
} Keys[8] CONFIG_ATTR = {0};

int r3_set_keyinfo(uint8_t* buffer, int* length)
{
	return R3_ERR_OK;
}

int r3_get_keyinfo(uint8_t* buffer, int* length)
{
	return R3_ERR_OK;
}
/*! \brief Функция (0x0С) — отсылка пакета с аутентификацией

	Данные передаются в открытом виде, с выработкой имитовставки и последующим усечением результата до 32 бит.
	Запрос: [ 0x0С8 || InvokeId8 || SecuredData || MAC32]
	Отклик: [ 0x0С8 || InvokeId8 || SecuredData || MAC32]
	
	
	
 */
int r3_get_secured(uint8_t* buffer, int* length)
{
	
	
    const R3cmdInfo* r3cmd = __start_r3_cmd_callbacks;
    const R3cmdInfo* r3cmd_stop = __stop_r3_cmd_callbacks;
//    unsigned int code = buffer[0];
    while (r3cmd < r3cmd_stop)
    {
        if (code == r3cmd->code){
			uint8_t key_id = r3cmd->level;//?0:r3cmd->level;// ctz?
			if (Keys[key_id].private_key==NULL) 
			{
				break;
			}
			*length-=1;
            buffer[0] = r3cmd->cb(&buffer[1], length);
			*length+=1
			uint32_t* Key = Keys[key_id];
			uint64_t cmac= magma_cmac(Key, buffer, *length);
			memcpy(buffer+*length, cmac, CMAC_LEN); *length+=CMAC_LEN;
            return length;
        }
        r3cmd++;
    }
	buffer[0]=R3_ERR_ABORT;
	return 1;
}

int r3_set_secured(uint8_t* buffer, int* length, uint8_t key_id)
{
	uint8_t invoke_id = buf[len++];
	uint8_t code = buf[len++];
	
	uint32_t* Key = Keys[key_id];
	
	uint64_t cmac= magma_cmac(Key, data, data_len);
	if (cmac==*(uint64_t*)&buffer[*length-8]){
		
		// найти по коду
		R3_cmd_snmp_recv(code, buffer+2, length);
	} else {
		
	}
	memcpy(buf+len, cmac, 8); len+=8;
	return R3_ERR_OK;
}

R3CMD_SECURED(R3_SNMP_SET_KEYINFO, r3_set_keyinfo, KEY_DEVICE_MASTER);
R3CMD(R3_SNMP_GET_KEYINFO, r3_get_keyinfo);
R3CMD(R3_SNMP_SET_SECURED, r3_set_secured);
R3CMD(R3_SNMP_GET_SECURED, r3_get_secured);