#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "adc.h"
#include "lata.h"

unsigned int sinais_presenca1, sinais_presenca2, canais_temp;
unsigned char intType; // tambem e utilizado para indicar se o timer esta sendo utilizado

// LEMBRETE: NAO BOTE DELAYS EM INTERRUPCOES
void main (void)
{
	intType = 0;
	P1DIR = BIT0;
	P1OUT &= ~BIT0;
	
	/* Parte de set-up */
	App_configuraMSP ();
	App_configuraRadio ();
	
	// Setar a seguinte parte do programa de acordo com o uso
	/*----------------------------------*/	
	int numero_latas = 2;
	lt lata[numero_latas];
	
	LATA_Iniciar (BIT7, P2_X | BIT5, &lata[0], 0);
	LATA_Iniciar (BIT3, P2_X | BIT4, &lata[1], 1);
	sinais_presenca1 = 0; 				// sinais de presenca em pinos 1.x
	sinais_presenca2 = BIT4 | BIT5;    	// sinais de presenca em pinos 2.x
	canais_temp = BIT7 | BIT3; 			// canais do ADC10 utilizados nos pinos 1.x (unicos utilizados para o ADC)
	/*----------------------------------*/	
	
	unsigned int medicoes[8]; // Armazena as medicoes feitas a cada ciclo do ADC
	App_configurarADC (medicoes, canais_temp);
	
	App_ativaIntPres (sinais_presenca1, sinais_presenca2);
	__bis_SR_register (GIE); 	
	
	/* Parte que repete */		
	intType |= TIMER_IS_ON;
	App_setTimer1min ();
	while (true)
	{		
		if (intType & LATA)
		{ // ATENCAO, se a int for de lata, NAO guarde a temperatura medida no arranjo de medicoes,
		  // pois as medicoes precisao ser igualmente espacadas, entao so pode guardar quando for int do TIMER
			intType &= ~LATA; // PRECISA ficar no inicio do if

			App_rstLatas (lata, numero_latas); // Reseta medicoes

			App_medirLatas (lata, medicoes, numero_latas);

			App_converterMedicoesEmTemp (lata, numero_latas);

	//		App_attLedLatas (lata, numero_latas); // Atualiza a led de modo a indicar a mais gelada	
			
			unsigned char s[2];
			s[0] = App_lataAtingiuTemp (lata, numero_latas);
			if (s[0] != 0)
			{
				App_enviar (s, 1);
			}
		}
		if (intType & RX)
		{
			intType &= ~RX; // PRECISA ficar no inicio do if
			
			unsigned char s[1 + (numero_latas * 3)];  // 1 para o header de controle e 3 para cada lata
			
			while (zig_RX_getLastPckt () == true)
			{	
				if (Rx.srcAddr[0] == 0x0c && Rx.srcAddr[1] == 0x4f)
				{
					s[0] = 0;
					if (Rx.payload[0] & PING)
					{ 
						s[0] |= PONG;
					}
					
					unsigned char index = 1;			
					
					if (Rx.payload[1] & SETAR_TEMP_TODAS)
					{
						s[0] |= TEMP_SETADA;
						App_setarTempDesejada (&Rx.payload[1], lata, numero_latas);
					}
					App_rstLatas (lata, numero_latas); // Reseta medicoes
					
					App_medirLatas (lata, medicoes, numero_latas);

					App_converterMedicoesEmTemp (lata, numero_latas);	
					
					//App_attLedLatas (lata, numero_latas); // Atualiza a led de modo a indicar a mais gelada		
					
					if (Rx.payload[0] & TEMP_LATA_TODAS)
					{
						s[0] |= (Rx.payload[0] & TEMP_LATA_TODAS) << 3;
						App_gravarTemp (s, &index, Rx.payload[0] & TEMP_LATA_TODAS, lata, numero_latas);
					}	

					s[0] |= App_lataAtingiuTemp (lata, numero_latas);


					if (s[0] != 0)
					{
						App_enviar (s, index);
					}
				}
			}
		}
		if (intType & TIMER)
		{
			intType &= ~TIMER; // NAO PRECISA ficar no inicio do if
			
			App_rstLatas (lata, numero_latas); // Reseta medicoes

			App_medirLatas (lata, medicoes, numero_latas);

			App_converterMedicoesEmTemp (lata, numero_latas);

	//		App_attLedLatas (lata, numero_latas); // Atualiza a led de modo a indicar a mais gelada	
			
			App_salvarMedicoes (lata, numero_latas);
			
			unsigned char s[2];
			s[0] = App_lataAtingiuTemp (lata, numero_latas);
			if (s[0] != 0)
			{
				App_enviar (s, 1);
			}
		}
		if (intType != 0)
		{ 
			// se acontecer uma int durante o processamento de outra, intType sera diferente de 0,
			// entao o fluxo volta para o inicio do loop while para a int ser tratada
			continue;	
		}
		
		if (App_algumaLataPresente (lata, numero_latas) == false) // Analisa o arranjo de latas para ver se alguma está presente
		{	
			intType &= ~TIMER_IS_ON;
			App_desativaTimer ();
			intType |= SLEEPING;
			__bis_SR_register (LPM4_bits); // CPUOFF até que seja detectada uma lata ou RX
			intType &= ~SLEEPING;
			intType |= TIMER_IS_ON;
			App_setTimer1min ();
			continue;
		}
		
		if ( (intType & TIMER_IS_ON) != 0)
		{ 
			// se acontecer uma int de lata enquanto o timer estiver ligado, a int sera tradada e
			// apos isso, a cpu volta para o estado de dormencia nesse ponto.
			// note que apos a cpu voltar do estado de dormencia, o continue joga o fluxo 
			// para o inicio do loop while, como deve ser
			__bis_SR_register (LPM1_bits);
			continue;
		}
	}
}
#pragma vector = PORT1_VECTOR
__interrupt void Port1 (void)
{
	if ( (P1IFG & sinais_presenca1) != 0 ) // Lata foi posta no slot
	{
		intType |= LATA;
		if ( (intType & TIMER_IS_ON) != 0)
		{ // timer ligado, cpu esta em lpm1
			__bic_SR_register_on_exit (LPM1_bits); 
		}
		if ( (intType & SLEEPING) != 0)
		{ // dormindo, cpu esta em lpm4
			__bic_SR_register_on_exit (LPM4_bits); 
		}
		// se nao for nenhum, cpu esta funcionando normalmente
	}
	P1IFG &= ~sinais_presenca1;
}

#pragma vector = PORT2_VECTOR
__interrupt void Port2 (void) // RX/TX Interrupt routine
{	
	if ( (P2IFG & sinais_presenca2) != 0 ) // Lata foi posta no slot
	{
		intType |= LATA;
		if ( (intType & TIMER_IS_ON) != 0)
		{ // timer ligado, cpu esta em lpm1
			__bic_SR_register_on_exit (LPM1_bits); 
		}
		if ( (intType & SLEEPING) != 0)
		{ // dormindo, cpu esta em lpm4
			__bic_SR_register_on_exit (LPM4_bits); 
		}
		// se nao for nenhum, cpu esta funcionando normalmente
	}
	if (P2IFG & ZIG_INTPIN)
	{
		BYTE intLog = zig_GetShort (INTSTAT);
		if (intLog & BIT0) // TX interruption
		{
			Radio.TX_busy = false;
			if (Radio.TX_awatingAck)
			{
				Radio.TX_awatingAck = false;
				BYTE txLog = zig_GetShort (TXSTAT);
				if (txLog & BIT0) // not successful?
				{		
					Radio.TX_lastPackFail = true;
					if (txLog & BIT5) // fail due to busy channel?
					{
						Radio.TX_busyChannel = true;
					}
					else
					{	
						Radio.TX_busyChannel = false;
					}
				}
				else
				{	
					Radio.TX_lastPackFail = false;
				}
			}
		}
		if (intLog & BIT3) // RX interruption
		{
			intType |= RX;
			if (Radio.RX_full == true)
			{
				Radio.RX_lastPackWasIgnored = true;
			}
			else
			{ // tamanho total do pacote == m + n + 5 (contando o header e os indicadores de qualidade)
				zig_SetShort (BBREG1, BIT2); // stop rx fifo from receiving packets
				buffer[Radio.RX_buffRear][0] = zig_GetLong (0x300); // == m + n + 2
				if ( (buffer[Radio.RX_buffRear][0] + 3) > MAX_RX_PACKET_SIZE)
				{
					Radio.RX_lastPackWasTruncated = true;
					zig_ContiguousRead (0x301, &buffer[Radio.RX_buffRear][1], MAX_RX_PACKET_SIZE - 1); 
				}
				else
				{
					zig_ContiguousRead (0x301, &buffer[Radio.RX_buffRear][1], buffer[Radio.RX_buffRear][0] + 2); 
				}
				Radio.RX_empty = false;
				Radio.RX_buffRear = (Radio.RX_buffRear + 1) % BUFFER_SIZE;
				
				if (Radio.RX_buffRear == Radio.RX_buffFront)
				{
					Radio.RX_full = true;
				}
				zig_SetShort (BBREG1, 0);
			}		
			if ( (intType & TIMER_IS_ON) != 0)
			{ // timer ligado, cpu esta em lpm1
				__bic_SR_register_on_exit (LPM1_bits); 
			}
			if ( (intType & SLEEPING) != 0)
			{ // dormindo, cpu esta em lpm4
				__bic_SR_register_on_exit (LPM4_bits); 
			}
			// se nao for nenhum, cpu esta funcionando normalmente
		}
	}
	P2IFG &= ~(ZIG_INTPIN | sinais_presenca2);
}

#pragma vector = TIMER0_A0_VECTOR //Timer0,TAIFG interrupt vector
__interrupt void TimerA(void)
{
	globalSeg--;
	if (globalSeg == 0)
	{
		intType |= TIMER;		
		globalSeg = 5 * 60; 
		__bic_SR_register_on_exit (LPM1_bits); 
	}
}
