#include "spi.h"


void SPI_StartSlave ()
{
/* P1.1 -> RX
   P1.2 -> TX
   P1.4 -> CLK */
	P1SEL |= BIT1 + BIT2 + BIT4;
	P1SEL2 |= BIT1 + BIT2 + BIT4;
	UCA0CTL1 |= UCSWRST;
	UCA0CTL0 |= UCMSB + UCSYNC + UCCKPL + UCCKPH;
	UCA0CTL1 &= ~UCSWRST;
	
}

void SPI_StartMaster ()
{
/* P1.1 -> RX
   P1.2 -> TX
   P1.4 -> CLK */
	P1SEL |= BIT1 + BIT2 + BIT4;
	P1SEL2 |= BIT1 + BIT2 + BIT4;
	UCA0CTL1 |= UCSWRST;
	UCA0CTL0 |= UCMSB + UCMST + UCSYNC + UCCKPH; // inativo em low, amostragem em borda de subida
	UCA0MCTL = 0;      
	UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 |= 0x02;
    UCA0BR1 = 0;
	UCA0CTL1 &= ~UCSWRST;
	
}

BYTE SPI_Send (BYTE data)
{
	BYTE read;
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = data;
	while (!(IFG2 & UCA0RXIFG));
	read = UCA0RXBUF;
	return read;
}

void SPI_SlaveRst ()
{
	P1OUT &= ~BIT7;
	__delay_cycles (5);
	P1OUT |= BIT7;		
	__delay_cycles (100);
}
