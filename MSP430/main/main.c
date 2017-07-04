#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "adc.h"
#include "lata.h"

unsigned int sinais_presenca1, sinais_presenca2, canais_temp;

void main (void)
{
	P1DIR = BIT0;
	P1OUT &= ~BIT0;
	
	/* Parte de set-up */
	App_configuraMSP ();
	App_configuraRadio ();
	
	// Setar a seguinte parte do programa de acordo com o uso
	/*----------------------------------*/	
	int numero_latas = 1;
	lt lata[numero_latas];
	
	LATA_Iniciar (BIT7, P1_X | BIT6, &lata[0], 2);
	sinais_presenca1 = BIT6; // sinais de presenca em pinos 1.x
	sinais_presenca2 = 0;    // sinais de presenca em pinos 2.x
	canais_temp = BIT7;      // canais do ADC10 utilizados nos pinos 1.x (unicos utilizados para o ADC)
	/*----------------------------------*/	
	
	unsigned int medicoes[8]; // Armazena as medicoes feitas a cada ciclo do ADC
	App_configurarADC (medicoes, canais_temp);
	
	__bis_SR_register (GIE); 
	
	/* Parte que repete */
	while (true)
	{

		if (App_algumaLataPresente (lata, numero_latas) == false) // Analisa o arranjo de latas para ver se alguma está presente
		{	
			App_ativaIntPres (sinais_presenca1, sinais_presenca2);
			
			__bis_SR_register (LPM4_bits + GIE); // CPUOFF até que seja detectada uma lata
			
			/* Apost a interrupção de colocar lata no slot, o código continua aqui */
			App_desativaIntPres (sinais_presenca1, sinais_presenca2);
		}

		App_rstLatas (lata, numero_latas); // Reseta medicoes
		
		App_medirLatas (lata, medicoes, numero_latas);
		
		App_salvarMedicoes (lata, numero_latas);
		
//		App_attLedLatas (lata, numero_latas); // Atualiza a led de modo a indicar a mais gelada
		
		App_enviaMed (lata, numero_latas);
		
		/* Esperar 1 min */
		//App_sleep10seg (1); 
	}
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1 (void)
{
	if ( ((P1IFG & sinais_presenca1) != 0) || ((P2IFG & sinais_presenca2) != 0) ) // Lata foi posta no slot
	{
		__bic_SR_register_on_exit (LPM4_bits); 
	}
	P1IFG = 0x00;
}
