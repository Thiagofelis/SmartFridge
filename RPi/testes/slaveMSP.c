#include <stdio.h>
#include <stdlib.h>
#include "msp430g2553.h"
#include "spi.h"
#include "zig.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char* BYTEPNT;


void configureClocks()
{
     BCSCTL1 = CALBC1_1MHZ;
     DCOCTL = CALDCO_1MHZ;
}

void delay_us(unsigned int us)
{
	while (us)
	{
		__delay_cycles(1); // 1 for 1 Mhz set 16 for 16 MHz
		us--;
	}
}

void delay_ms(unsigned int ms)
{
	while (ms)
	{
		__delay_cycles(1000); 
		ms--;
	}
}

void main (void)
{	
	WDTCTL = WDTPW  + WDTHOLD; // Desativa WDT
	// p2.2 ~rst; p2.3 ~cs;
	configureClocks(); 
	P1DIR = BIT0 | BIT6;
	P1OUT = 0x00;
	P2DIR = BIT2 | BIT0 | BIT3;
	P2OUT = BIT2 | BIT3;
	P2IE = BIT1;
	P2IES |= BIT1; // seleciona borda de descida
	P2IFG &= ~BIT1;	
	
	

	SPI_StartMaster ();
	
	BYTE shortAddr[2] = {0x1a, 0xbc}, PANid[2] = {0xf2, 0x5a}, longAddr[8] = {0x11, 0x29, 0x04, 0x39, 0x7c, 0x22, 0x14, 0xae};
	BYTE dstShortAddr[2] = {0x0c, 0x4f}, dstPANid[2] = {0xef, 0x4d}, dstLongAddr[8] = {0x3e, 0xd1, 0x8a, 0x09, 0x2a, 0xdc, 0x68, 0xff};	
	delay_ms (2);

	zig_Init (17, longAddr, shortAddr, PANid);		
			
	zig_TX_config (PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_ENABLED, 
				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR);
	
	
	zig_TX_configDstAddr (dstShortAddr, dstPANid);
	
	__bis_status_register(GIE);
	
	unsigned char s[5] = {0x21, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s, 4);
	zig_TX_Transmit ();

	unsigned char s2[5] = {0x41, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s2, 4);
	zig_TX_Transmit ();

	unsigned char s3[5] = {0xa1, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s3, 4);
	zig_TX_Transmit ();

	unsigned char s4[5] = {0xb2, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s4, 4);
	zig_TX_Transmit ();

	unsigned char s5[5] = {0x2f, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s5, 4);
	zig_TX_Transmit ();

	unsigned char s6[5] = {0x01, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s6, 4);
	zig_TX_Transmit ();

	unsigned char s7[5] = {0x00, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s7, 4);
	zig_TX_Transmit ();

	unsigned char s8[5] = {0xff, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s8, 4);
	zig_TX_Transmit ();

	unsigned char s9[5] = {0x12, 0xA2, 0x23, 0xff};
	zig_TX_PayloadToBuffer (s9, 4);
	zig_TX_Transmit ();
	
	while (1);
}
