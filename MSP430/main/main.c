#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include "general.h"
#include "adc.h"
#include "lata.h"


void main (void)
{
	// LEMBRAR DE SETAR PINAGEM NO CONFIGURAMSP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	/* Parte de set-up */
	App_configuraMSP ();
	App_configuraRadio ();
	
	// Setar a seguinte parte do programa de acordo com o uso
	/*----------------------------------*/	
	int numero_latas = 1;
	lt lata[numero_latas];
	
	LATA_Iniciar (BIT7, 16, &lata[0], 1);
	//LATA_Iniciar (BIT5, 21, &lata[1], 'B');
	//LATA_Iniciar (BIT6, 22, &lata[2], 'C');
	/*----------------------------------*/	
	
	WORD medicoes[8]; // Armazena as medicoes feitas a cada ciclo do ADC
	App_configurarADC (medicoes, App_pegarCanaisTemp (lata, numero_latas));
	
	__bis_SR_register (GIE); 
	
	/* Parte que repete */
	while (true)
	{
		if (App_algumaLataPresente (lata, numero_latas) == false) // Analisa o arranjo de latas para ver se alguma está presente
		{	
			App_ativaIntPres (BIT6);
			
			__bis_SR_register (LPM4_bits + GIE); // CPUOFF até que seja detectada uma lata
			
			/* Apost a interrupção de colocar lata no slot, o código continua aqui */
			App_desativaIntPres (BIT6);
		}
		
		App_rstLatas (lata, numero_latas); // Reseta medicoes
		
		App_medirLatas (lata, medicoes, numero_latas);
		
		App_attLedLatas (lata, numero_latas); // Atualiza a led de modo a indicar a mais gelada
		
		App_enviaMed (lata, numero_latas);
		
		/* Esperar 1 min */
		//App_sleep10seg (1); 
	}
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1 (void)
{
	if ((P1IFG & BIT6) != 0) // Lata foi posta no slot
	{
		__bic_SR_register_on_exit (LPM4_bits); 
	}
	P1IFG = 0x00;
}
