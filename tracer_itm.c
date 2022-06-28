/*! \ingroup _tracer
	\defgroup _tracer_itm Вывод системных сообщений в отладочный порт ITM 
	Через SWO
	The bit TRCEN of the Debug Exception and Monitor Control Register must be enabled 
before you program or use the ITM.
*/
#include "board.h"

//#include "r3rtos.h"
#include "module.h"
#include "trace.h"

#if defined(GD32E10X)
#include "gd32e10x_dbg.h"
#endif


#if defined(ITM_BASE)

#ifndef CoreDebug_DEMCR_TRCENA
#define CoreDebug_DEMCR_TRCENA (1UL<<24)
#endif

#define SWO_SPEED 500000
#define BUF_SIZE 1
static uint8_t term_buffer[BUF_SIZE];
static Terminal itm_terminal;

static const int port = 0;//BOARD_DEFAULT_USART;

/*

To output a simple value to the TPIU:
● Configure the TPIU and assign TRACE I/Os by configuring the DBGMCU_CR (refer to 
Section 25.16.2: TRACE pin assignmentand Section 25.15.3: Debug MCU 
configuration register)
● Write 0xC5ACCE55 to the ITM Lock Access Register to unlock the write access to the 
ITM registers
● Write 0x00010005 to the ITM Trace Control Register to enable the ITM with Sync 
enabled and an ATB ID different from 0x00
● Write 0x1 to the ITM Trace Enable Register to enable the Stimulus Port 0
● Write 0x1 to the ITM Trace Privilege Register to unmask stimulus ports 7:0
● Write the value to output in the Stimulus Port Register 0: this can be done by software 
(using a printf function)

28.15.14   Example of configuration
• Set the bit TRCENA in the Debug Exception and Monitor Control Register (DEMCR)
• Write the TPIU Current Port Size Register to the desired value (default is 0x1 for a  1-bit port size)
• Write TPIU Formatter and Flush Control Register to 0x102 (default value)
• Write the TPIU Select Pin Protocol to select the sync or async mode. Example: 0x2 for async NRZ mode (UART like)
• Write the DBGMCU control register to 0x20 (bit IO_TRACEN) to assign TRACE I/Os for async mode. A TPIU Sync packet is emitted at this time (FF_FF_FF_7F)
• Configure the ITM and write the ITM Stimulus register to output a value
*/

static void* ITM_terminal_init(void* data) 
{
#if defined(DCB_BASE)
	DCB->DHCSR |= 1;
	DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
#else
	CoreDebug->DHCSR |= 1;
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA;
#endif
/*
uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;

// Asynchronous Clock Prescaler Register, TPIU_ACPR 0xE0040010
*dwt_ctrl = 0x400113FF;
//TPIU->ACPR = (36000000 / SWO_SPEED) - 1;
*((volatile unsigned *)0xE0040010) = 0x3F;//(36000000 / SWO_SPEED) - 1;
*((volatile unsigned *)0xE0040004) = 0x00000001; // 0xE0040004 Current port size
*((volatile unsigned *)0xE0040304) = 0x00000102;//2; // 0xE0040304 Formatter and flush control
*((volatile unsigned *)0xE00400F0) = 0x00000002; // 0xE00400F0 Selected pin protocol 01: Serial Wire Output - manchester (default value), 10: Serial Wire Output - NRZ
*/
#if defined(GD32E10X)
DBG_CTL &= ~DBG_CTL_TRACE_MODE;
DBG_CTL |=  DBG_CTL_TRACE_IOEN;
#else// для STM32
DBGMCU->CR &= ~DBGMCU_CR_TRACE_MODE;
DBGMCU->CR |=  DBGMCU_CR_TRACE_IOEN; // включить  SWO, трасе моде 00
#endif
//*((volatile unsigned *)0xE0000FB0)= 0xC5ACCE55;// lock access
ITM->LAR = 0xC5ACCE55;// lock access
ITM->TCR = 0x00010009;// trace control (ITM_TCR_SWOENA_Pos|ITM_TCR_ITMENA_Msk)

ITM->TER = (0x1UL<<port);// trace enable
ITM->TPR = (0x1UL<<port);// trace privileges
//ITM->PORT[port].u8 = '+';

//    usart_init (&port);
    TRACE_terminal_init(&itm_terminal);
    return NULL;//(void*)&port;
}

static void ITM_terminal_scan(void* data)
{
	if (ITM->PORT[port].u32!=0){
		int term_buffer_size = TRACE_gets(&itm_terminal, term_buffer, BUF_SIZE);
		if (term_buffer_size) 
			ITM->PORT[port].u8 = (uint8_t) term_buffer[0];
	}
}
// очистить содеижимое буфера 
void ITM_flush(void* data)
{
	while (1)
	if (ITM->PORT[port].u32!=0){
		int term_buffer_size = TRACE_gets(&itm_terminal, term_buffer, BUF_SIZE);
		if (term_buffer_size) 
			ITM->PORT[port].u8 = (uint8_t) term_buffer[0];
		else break;
	}
}

MODULE(ITM_Terminal, ITM_terminal_init, ITM_terminal_scan);
#endif