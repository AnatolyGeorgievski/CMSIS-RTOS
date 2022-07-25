/*! */
#include "board.h"
#include "cmsis_os.h"
#include "r3v2protocol.h"
#include "trace.h"
#define BUF_SIZE 48
static Terminal r3_terminal;
static void __attribute__((constructor)) _init()
{
	TRACE_terminal_init(&r3_terminal);
}
static int r3_trace(uint8_t* buffer, int* length)
{
	*length = TRACE_gets(&r3_terminal, buffer, BUF_SIZE);
	return R3_ERR_OK;
}
R3CMD(R3_SNMP_TRACE, r3_trace);