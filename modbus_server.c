/*! \brief Сервис
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "modbus.h"

uint8_t pdu[MODBUS_MAX_ADU_LENGTH];

void* modbus_slave_init(int argc, char *argv[])
{
    printf("Modbus Server!\n");
    int i, rc;
    int fd = open(argv[1], O_RDWR);// порт /dev/usart1
	usart_slave_id(address)
	modbus_set_slave(argv[2]);// идентификатор
	
}
void modbus_slave_scan(void* data)
{
	rc = recv(fd, pdu, MODBUS_MAX_ADU_LENGTH);
	if (rc > 0) {
		len = modbus_state(pdu, rc);
		rc = send(fd, pdu, len);
	} else if (rc  == -1) {
		/* Connection closed by the client or error */
		break;
	} else {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
			strerror(errno));
	}
}
