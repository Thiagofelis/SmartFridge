#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "general.h"
#include "adc.h"
#include "lata.h"
#define numero_latas 1


void main (void)
{
	/* Parte de set-up */
	App_configuraMSP ();
	App_configuraRadio ();
	App_configuraClock (); // dpois tem q olhar se precisa disso, se for pro adc precisa, se for pro uart entao nao
	
	// Setar a seguinte parte do programa de acordo com o uso
	/*----------------------------------*/		
	lt lata[numero_latas];
	
	LATA_Iniciar (BIT4, 23, &lata[0], 3);
	//LATA_Iniciar (BIT5, 21, &lata[1], 'B');
	//LATA_Iniciar (BIT6, 22, &lata[2], 'C');
	/*----------------------------------*/	
	
	WORD medicoes[8]; // Armazena as medicoes feitas a cada ciclo do ADC
	WORD canais_presenca = App_pegarCanaisPresenca (lata); // Pega os pinos utilizados para verificar presenca
	App_configurarADC (medicoes, canais_presenca);
	
	__bic_SR_register_on_exit (GIE); 
	
	/* Parte que repete */
	while (true)
	{
		if (App_algumaLataPresente (lata) == false) // Analisa o arranjo de latas para ver se alguma está presente
		{	
			App_ativaIntPres (canais_presenca);
			
			__bis_SR_register (LPM4_bits); // CPUOFF até que seja detectada uma lata
			
			/* Aposta a interrupção de colocar lata no slot, o código continua aqui */
			App_desativaIntPres (canais_presenca);
		}
		
		App_rstLatas (lata); // Reseta medicoes
		
		App_medirLatas (lata, medicoes);
		
		App_attLedLatas (lata); // Atualiza a led de modo a indicar a mais gelada
		
		App_enviaMed (lata);
		
		/* Esperar 1 min */
	}
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1 (void)
{
	if (P1IFG == BIT3) // Lata foi posta no slot
	{
		__bic_SR_register_on_exit (LPM4_bits); 
	}
	if (P1IFG == Zig_INTPin)
	{
		/* Tratar interrupção de TX/RX */	
	}
	P1IFG = 0x00;
}
