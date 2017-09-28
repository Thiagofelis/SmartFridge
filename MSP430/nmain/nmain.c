#include "msp430g2553.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "adc.h"
#include "lata.h"

unsigned int 	sinais_presenca1 = BIT6, 
				sinais_presenca2 = BIT4 | BIT5, 
				canais_temp = BIT7 | BIT3 | BIT0, 
				led1 = BIT5,  
				led2 = BIT2 | BIT0; // add bit0

unsigned char intType = 0; // tambem e utilizado para indicar se o timer esta sendo utilizado

// LEMBRETE: NAO BOTE DELAYS EM INTERRUPCOES
void main (void)
{
	/* Parte de set-up */
	App_configuraMSP (led1, led2, sinais_presenca1, sinais_presenca2);
	App_configuraRadio ();
	App_configuraClk ();
	
	// Setar a seguinte parte do programa de acordo com o uso
	/*----------------------------------*/	
	//            ADC   PRESENCA     LED     
	lt lata[3];
	int numero_latas = 3;
	LATA_Iniciar (BIT7, P2_X | BIT5, P2_X | BIT2, &lata[0], 0);
	LATA_Iniciar (BIT3, P2_X | BIT4, P1_X | BIT5, &lata[1], 1);
	LATA_Iniciar (BIT0, P1_X | BIT6, P2_X | BIT0, &lata[2], 2);
	/*----------------------------------*/	

	unsigned int medicoes[8]; // Armazena as medicoes feitas a cada ciclo do ADC
	App_configurarADC (medicoes, canais_temp);
		
	__bis_SR_register (GIE); 
	App_ativaIntPres (sinais_presenca1, sinais_presenca2); // DEPOIS, tenta colocar em cima do bis_SR se der problema
	
	// Vamos comecar o programa com uma interrupcao de lata
	intType |= LATA; 
	
	while (true)
	{		
		if (intType & (LATA | TIMER)) 
		{ 	
			intType &= ~(LATA | TIMER); // PRECISA ficar no inicio do if <- se for chamada outra int durante, o codigo e executado denovo
			
			App_rstLatas (lata, numero_latas); // Reseta medicoes

			App_medirLatas (lata, medicoes, numero_latas);

			App_converterMedicoesEmTemp (lata, numero_latas);
			
			App_attLedLatas (lata, numero_latas, led1, led2); // Atualiza a led de modo a indicar a mais gelada	

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

					App_attLedLatas (lata, numero_latas, led1, led2); // Atualiza a led de modo a indicar a mais gelada	
					
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

		if ((intType & ~TIMER_IS_ON) != 0)
		{ 
			// se acontecer uma int durante o processamento de outra, intType sera diferente de 0,
			// entao o fluxo volta para o inicio do loop while para a int ser tratada
			continue;	
		}
		if (App_algumaLataPresente (lata, numero_latas) == true) // Analisa o arranjo de latas para ver se alguma está presente
		{
			if (intType & TIMER_IS_ON)
			{
				__bis_SR_register (LPM1_bits);
			}
			else
			{
				intType |= TIMER_IS_ON;
				App_setTimer1min ();
				__bis_SR_register (LPM1_bits);
			}
		}
		else
		{	
			intType &= ~TIMER_IS_ON;
			App_desativaTimer ();
			intType |= SLEEPING;
			__bis_SR_register (LPM4_bits); // CPUOFF até que seja detectada uma lata ou RX
			intType &= ~SLEEPING;
			continue;
		}
	}
}
#pragma vector = PORT1_VECTOR
__interrupt void Port1 (void)
{		
	if ( (P1IFG & sinais_presenca1) != 0 ) // Lata foi posta no slot -> Debounce GPIO
	{
	// Coloca um timer de um pouco menos de um segundo. Após isso, é chamada a interrupção "verdadeira", a do timer 1
	// Fazemos isso para que esperar o ruído que ocorre logo após pressionar o botão. Lembrando que o timer nao funciona em Lmp4
		
		if (intType & SLEEPING)
		{
			__bic_SR_register_on_exit(LPM4_bits);
			__bis_SR_register_on_exit(LPM1_bits);
		}
		
		TA1CCTL0 |= CCIE;
		TA1CCR0 = ~0;
		TA1CTL = TASSEL_2 + ID_3 + MC_1;
	}
	P1IFG &= ~sinais_presenca1;
}

#pragma vector = PORT2_VECTOR
__interrupt void Port2 (void) // RX/TX Interrupt routine
{		
	if ( (P2IFG & sinais_presenca2) != 0 ) // Lata foi posta no slot -> Debounce GPIO
	{ 
	// Coloca um timer de um pouco menos de um segundo. Após isso, é chamada a interrupção "verdadeira", a do timer 1
	// Fazemos isso para que esperar o ruído que ocorre logo após pressionar o botão. Lembrando que o timer nao funciona em Lmp4
		
		if (intType & SLEEPING)
		{
			__bic_SR_register_on_exit(LPM4_bits);
			__bis_SR_register_on_exit(LPM1_bits);
		}
		
		TA1CCTL0 |= CCIE;
		TA1CCR0 = ~0;
		TA1CTL = TASSEL_2 + ID_3 + MC_1;
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
			
			// é importante sair do estado de dormencia para que a int de
			// rx seja tratada. note que no caso de int de tx, nao é necessario
			// sair do estado de dormencia, pois a int ja faz todo o necessario
			
			// Se o timer esta ligado, a cpu esta em lpm1, entao tiramos desse estado
			if ( (intType & TIMER_IS_ON) != 0)
			{
				__bic_SR_register_on_exit (LPM1_bits); 
			}

			// Se a cpu esta dormindo, a cpu esta em lpm4, entao tiramos desse estado
			if ( (intType & SLEEPING) != 0)
			{
				__bic_SR_register_on_exit (LPM4_bits); 
			}

			// Se nao for nenhum dos casos abaixo, a CPU esta acordada
		}
	}
	P2IFG &= ~(ZIG_INTPIN | sinais_presenca2);
}

#pragma vector = TIMER0_A0_VECTOR //Timer 0
__interrupt void TimerA(void)
{
	globalSeg--;
	if (globalSeg == 0)
	{
		TA0CTL = MC_0;
		intType &= ~TIMER_IS_ON;
		intType |= TIMER;
		__bic_SR_register_on_exit (LPM1_bits); 
	}
}

#pragma vector = TIMER1_A0_VECTOR //Timer 1 -> GPIO debouncer
__interrupt void TimerA1(void)
{ 
	// Para o timer
	TA1CTL = MC_0;
	
	// Sinaliza a interrupcao de latas
	intType |= LATA;
	// Timer nao funciona em lmp1, sempre que chegamos aqui cpu ja esta em lpm4
	if (intType & (SLEEPING | TIMER_IS_ON))
	{
		__bic_SR_register_on_exit(LPM1_bits);
	}

	// Se nao for nenhum dos casos abaixo, a CPU esta acordada
}
