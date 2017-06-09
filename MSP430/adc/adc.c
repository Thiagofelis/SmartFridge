#include "adc.h"


void ADC_Medir (int *medicoes)
{
	ADC10CTL0 &= ~ADC10IFG;
	ADC10CTL0 |= ENC + ADC10SC;
	while (!(ADC10CTL0 & ADC10IFG));

	ADC10SA = (short)&medicoes[0];
}


void ADC_Configurar (int *medicoes, unsigned int canais_set)
{	
	ADC10CTL0 = ADC10ON + ADC10SHT_0 + MSC;

	ADC10CTL1 = ADC10DIV_7 + INCH_7 + CONSEQ_1;

	ADC10AE0 = canais_set;    

	ADC10DTC1 = 0x08;

	ADC10SA = (short)&medicoes[0];
}