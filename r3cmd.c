#include "board.h"
#include "r3v2protocol.h"
#include <stdio.h>

extern R3cmdInfo __start_r3_cmd_callbacks[];
extern R3cmdInfo __stop_r3_cmd_callbacks[];
//uint8_t device_secured CONFIG_ATTR = ;
/*! \brief  эта функция создана специально для демонстрации работы команд SNMP через сеть 
	\return длину байтовой строки. Строка начинается с кода завершения операции.
*/
int R3_cmd_snmp_recv(uint8_t code, uint8_t * buffer, int length)
{
    const R3cmdInfo* r3cmd = __start_r3_cmd_callbacks;
    const R3cmdInfo* r3cmd_stop = __stop_r3_cmd_callbacks;
//    unsigned int code = buffer[0];
    while (r3cmd < r3cmd_stop)
    {
        if (code == r3cmd->code){
			//if (device_secured && security_level>=r3cmd->level) 
			{
				
			}
//			printf("code=%02X len=%d\r\n", code, length);
			length-=1;
            buffer[0] = r3cmd->cb(&buffer[1], &length);
            return length+1;
        }
        r3cmd++;
    }
	buffer[0]=R3_ERR_ABORT;
	return 1;
}
#if 1
/* Где используется? BACnet:
	16.2 ConfirmedPrivateTransfer Service
	16.3 UnconfirmedPrivateTransfer Service
*/
int R3_cmd_snmp_recv2(uint8_t code, uint8_t * buffer, int *length)
{
    const R3cmdInfo* r3cmd = __start_r3_cmd_callbacks;
    const R3cmdInfo* r3cmd_stop = __stop_r3_cmd_callbacks;
//    unsigned int code = buffer[0];
    while (r3cmd < r3cmd_stop)
    {
        if (code == r3cmd->code){
			printf("code=%02X len=%d\r\n", code, *length);
            int result = r3cmd->cb(buffer, length);
            return result;
        }
        r3cmd++;
    }
	return R3_ERR_ABORT;
}
#endif
#if 0
/*! \brief вывод списка доступных команд
 */
static int R3_cmd_show(uint8_t *buf, int *length)
{
    //SEGMENT_USE(r3_cmd_callbacks);
    const R3cmdInfo* r3cmd = __start_r3_cmd_callbacks;
    const R3cmdInfo* r3cmd_stop = __stop_r3_cmd_callbacks;
    while (r3cmd < r3cmd_stop)
    {
        printf("0x%02X\r\n", r3cmd->code/*, r3cmd->name*/);
        r3cmd++;
    }
    return R3_ERR_OK;
}
R3CMD(R3_SNMP_SHOW_R3, R3_cmd_show);
#endif

static int R3_cmd_mask(uint8_t *buf, int *length)
{
    //SEGMENT_USE(r3_cmd_callbacks);
    const R3cmdInfo* r3cmd = __start_r3_cmd_callbacks;
    const R3cmdInfo* r3cmd_stop = __stop_r3_cmd_callbacks;
	int len=32;
	__builtin_memset(buf, 0, len);
    while (r3cmd < r3cmd_stop)
    {
//        printf("0x%02X\r\n", r3cmd->code/*, r3cmd->name*/);
		buf[r3cmd->code >> 3] |= 1<<(r3cmd->code&7);
        r3cmd++;
    }
	*length=len;
    return R3_ERR_OK;
}

R3CMD(R3_SNMP_GET_R3, R3_cmd_mask);// список поддерживаемых команд
