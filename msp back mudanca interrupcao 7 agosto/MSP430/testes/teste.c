#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "app.h"

void main (void)
{
	App_configuraMSP ();
	P1DIR = BIT0;
	P1OUT &= ~BIT0;
	
//	App_configuraRadio ();
	App_configuraClk ();
	__bis_SR_register (GIE); 
	while (1)
	{
		P1OUT ^= BIT0;
		App_sleep1seg (10);
		
/*		App_delayMs(1000);
		while (!Radio.RX_empty)
		{
			zig_RX_getLastPckt ();
			if (Rx.srcAddr[0] == 0x0c && Rx.srcAddr[1] == 0x4f)
			{
				App_enviar (Rx.payload, Rx.payloadSize);
			}
		}
*/	}
	return;
}
#pragma vector = TIMER0_A0_VECTOR //Timer0,TAIFG interrupt vector
__interrupt void TimerA(void)
{
	globalSeg--;
	if (globalSeg == 0)
	{
		TACTL = MC_0;
		__bic_SR_register_on_exit (LPM1_bits); 
	}
}