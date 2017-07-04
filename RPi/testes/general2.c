#include "general2.h"

void Blink (int i)
{
	if (i != 0)
	{
		P1OUT ^= BIT0;
		__delay_cycles (200000);
		P1OUT ^= BIT0;
		__delay_cycles (200000);	
	}
	else
	{
		P1OUT ^= BIT6;
		__delay_cycles (200000);
		P1OUT ^= BIT6;
		__delay_cycles (200000);
	}
}

void BlinkBinary (BYTE data)
{
	int il;
	for (il = 0; il < 8; il++)
	{
		
		if (data & 0x01)
		{
			P1OUT ^= BIT0;
			__delay_cycles (200000);
			P1OUT ^= BIT0;
			__delay_cycles (200000);
		}
		else
		{
			P1OUT ^= BIT6;
			__delay_cycles (200000);
			P1OUT ^= BIT6;
			__delay_cycles (200000);
		}
		data = data >> 1;
	}
}