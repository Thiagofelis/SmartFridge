#include "app.h"

void App_delayMs (unsigned int ms)
{
	unsigned int i;
	for (i = 0; i < ms; i++)
	{
		__delay_cycles (1000);
	}
}

void App_setarTempDesejada (unsigned char *s, lt* lata, int numero_latas)
{
	char i;
	char flag = false;
	/*
	false := aaaa aaaa - aaaa bbbb - bbbb bbbb  
	         ^
	true  := aaaa aaaa - aaaa bbbb - bbbb bbbb  
						      ^
	*/
	char index = 1; 
	for (i = 0; i < numero_latas; i++)
	{
		if (s[0] & (1 << i))
		{
			
			if (flag == false)
			{
				LATA_SetarTempDesejada (&lata[(int)i], (s[(int)index] << 4) | ((s[(int)index + 1] & 0b11110000) >> 4));
				index += 1;
				flag = true;
			}
			else
			{
				LATA_SetarTempDesejada (&lata[(int)i], ((s[(int)index] & 0b1111) << 8) | s[(int)index + 1]);
				index += 2;
				flag = false;	
			}
		}
	}
}


unsigned char App_lataAtingiuTemp (lt* lata, int numero_latas)
{
	char i;
	unsigned char return_value = 0;
	for (i = 0; i < numero_latas; i++)
	{
		return_value |= LATA_AtingiuTemp (&lata[(int)i]) << i;
	}
	return return_value;
}

void App_gravarTemp (unsigned char *s_temp, unsigned char *index, unsigned char lat, lt* lata, int numero_latas)
{
	// aaaa aaaa - aaaa bbbb - bbbb bbbb - cccc cccc - cccc 0000
	
	unsigned int j, k;
	char i, flag = 0;
	char numero_de_medicoes_a_enviar = 0;
	unsigned char ii = 0, lat_temp = lat;
	while (lat_temp)
	{
		if (lat_temp & BIT0)
		{
			numero_de_medicoes_a_enviar++;			
		}
		lat_temp = lat_temp >> 1; 
	}
	
	if ( (numero_de_medicoes_a_enviar % 2) != 0)
	{
		flag = 1;	
	}
	numero_de_medicoes_a_enviar /= 2;
	
	for (i = 0; i < numero_de_medicoes_a_enviar; i++)
	{
		while (!(lat & BIT0))
		{
			ii++;
			lat = lat >> 1; 
		}
		j = LATA_PegarTemp (&lata[(int)ii]);
		ii++;
		lat = lat >> 1; 
		
		while (!(lat & BIT0))
		{
			ii++;
			lat = lat >> 1; 
		}
		k = LATA_PegarTemp (&lata[(int)ii]);
		ii++;
		lat = lat >> 1; 
		
		s_temp[0 + *index] = j >> 4;
		s_temp[1 + *index] = (j << 4) | (k >> 8);
		s_temp[2 + *index] = k;
		*index += 3;
	}
	if (flag)
	{		
		while (!(lat & BIT0))
		{
			ii++;
			lat = lat >> 1; 
		}
		j = LATA_PegarTemp (&lata[(int)ii]);
		s_temp[0 + *index] = j >> 4;
		s_temp[1 + *index] = j << 4;
		*index += 2;
	}
}

void App_configuraClk ()
{
	/* Configure the clock module - MCLK = 1MHz */
	DCOCTL = 0;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
}

int App_algumaLataPresente (lt *lata, int numero_latas)
{
	// Essa funcao sempre e chamada apos uma medicao de lata, entao e melhor
	// olhar a temperatura obtida do que a presenca
	int i;
	for (i = 0; i < numero_latas; i++)
	{
	//	if (LATA_EstaPresente (&lata[i]) == true) 
		if (LATA_PegarTemp (&lata[i]) != LATA_SEM_MEDICAO)
		{
			return true;
		}
	}
	return false;
}

void App_ativaIntPres (unsigned int sinais_presenca1, unsigned int sinais_presenca2)
{
	P1IE |= sinais_presenca1;
	P2IE |= sinais_presenca2;
}

void App_desativaIntPres (unsigned int sinais_presenca1, unsigned int sinais_presenca2)
{
	P1IE &= ~sinais_presenca1;
	P2IE &= ~sinais_presenca2;
}

void App_rstLatas (lt *lata, int numero_latas)
{
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		LATA_Resetar (&lata[i]);
	}
}

void App_medirLatas (lt *lata, WORD medicoes[], int numero_latas)
{
	int iii, jjj;
	// Faz medicao ate ter 20 medicoes validas para cada lata, 40 tentativas no maximo	
	for (iii = 0; iii < MAX_TENTATIVAS_MEDICAO; iii++)
	{
		ADC_Medir (medicoes);
		
		int finalizou = true;
		int medicao;	
		
		// Carrega nos vetores de amostra de cada instancia de lata os valores medidos
		for (jjj = 0; jjj < numero_latas; jjj++)
		{
			medicao = LATA_CarregarMedicoes (&lata[jjj], medicoes);

			// Verifica se a lata analisada esta ausente ou ja terminou as medicoes
			if ( (medicao != FIM_MEDICOES_COMPLETAS) && (medicao != FIM_LATA_AUSENTE) )
			{ // ou seja, as medicoes nao finalizaram
				finalizou = false;
			}
		}
		
		if (finalizou == true)
		{
			return;
		}
	}
}

void App_setarPino (unsigned int pino, int estado)
{
	if ( (pino & PX_X) == P1_X)
	{
		pino &= 0b11111111;
		if (estado == true)
		{
			P1OUT |= pino;
		}
		else
		{
			P1OUT &= ~pino;
		}
	}
		
	if ( (pino & PX_X) == P2_X)		
	{
		pino &= 0b111111;
		if (estado == true)
		{
			P2OUT |= pino;
		}
		else
		{
			P2OUT &= ~pino;
		}
	}
}

void App_attLedLatas (lt *lata, int numero_latas, unsigned int sinal_led1, unsigned int sinal_led2)
{	
	App_setarPino (P1_X | sinal_led1, false);
	App_setarPino (P2_X | sinal_led2, false);

	// Note que nao precisamos verificar a presenca da lata aqui. Essa funcao so e chamada apos
	// medicoes da temperatura. Portanto, podemos apenas olhar para a temperatura e ver se a lata
	// tem alguma medicao valida.

	int i, j = 0;
	unsigned int temp;
	for (i = 0; i < numero_latas; i++)
	{ // Acha a primeira lata presente e com temperatura valida
		temp = LATA_PegarTemp (&lata[i]);
		if (temp != LATA_SEM_MEDICAO)
		{
			break;
		}
	}
	
	if (temp != LATA_SEM_MEDICAO)
	{ // se houver uma lata presente e com temperatura valida, procuramos a partir dela a lata mais gelada
      // se a ultima lata do arranjo for escolhida, nao procuramos pois geraria segfault
		j = i;
		
		for (i = i + 1 ; i < numero_latas; i++)
		{
			temp = LATA_PegarTemp (&lata[i]);
			if ( (temp != LATA_SEM_MEDICAO) && (temp < LATA_PegarTemp (&lata[j])) )
			{
				j = i;	
			}
		}
		App_setarPino (LATA_PegarCanalLed (&lata[j]), true);
	}

}

void App_converterMedicoesEmTemp (lt* lata, int numero_latas)
{
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		LATA_ConverterMedicoesEmTemp (&lata[i]);
	}
}

void App_setTimer1min ()
{ // FUNCIONA :)
	globalSeg = 5 * 60; 
	
	// CCR0 interrupt enabled
	TA0CCTL0 = CCIE;                  	    	
	TA0CCR0 = 25001;
	
	// TASSEL = SMCLK, ID = /8, MC = COUNTS TO CCR0
	TA0CTL = TASSEL_2 + ID_3 + MC_1; 	
	/*
		f = 10^6 / 8 , t = 8 * 10^-6
		t * 25000 * 5 = 1 s
	*/
}
void App_desativaTimer ()
{
	TA0CTL = MC_0;
}

/*
void App_enviaMed (lt *lata, int numero_latas)
{
	BYTE s[numero_latas * TAMANHO_PACOTE];
	int i, k = 0;
	for (i = 0; i < numero_latas; i++)
	{
		if (LATA_MontarPacote (&lata[i], s + TAMANHO_PACOTE * k) == PACOTE_DIFERENTE)
		{
			
			k += TAMANHO_PACOTE;
		}
	}
	
	if (k == 0)
	{
		s[0] = 0xff; // Oxff significa que nao a nada a enviar
		k++;
	}
	
	App_enviar (s, k);
}
*/
void App_enviar (unsigned char *s, unsigned char size)
{
	int j;
	zig_TX_PayloadToBuffer (s, size);
	
	j = zig_TX_Transmit ();
	
	int i = 0;
	
	while (j == FAIL_BUSY)
	{
		if (i == 10)
		{ // se nao ficar disponivel em 100 ms agnt reseta o radio
			zig_reset ();
			zig_TX_Transmit ();
			break;
		}
		App_delayMs (10);
		i++;
		j = zig_TX_Transmit ();
	}
}

void App_ativarIntBordaSubida (unsigned int presenca1, unsigned int presenca2)
{/*
//	unsigned int IntTemp = P1IFG; 
	P1IES &= ~presenca1;
	//P1IFG = IntTemp; 
//	IntTemp = P2IFG; 
	P2IES &= ~presenca2;
//	P2IFG = IntTemp; */
}
	
void App_ativarIntBordaDescida (unsigned int presenca1, unsigned int presenca2)
{
//	unsigned int IntTemp = P1IFG; 
	P1IES |= presenca1;
//	P1IFG = IntTemp; 
//	IntTemp = P2IFG; 
	P2IES |= presenca2;
//	P2IFG = IntTemp; 
}	
	
void App_configurarADC (WORD* medicoes, WORD canais_temp)
{
	ADC_Configurar (medicoes, canais_temp);
}
/*
WORD App_pegarCanaisTemp (lt *lata, int numero_latas)
{
	WORD canais_presenca = 0;
	int j;	
	for (j = 0; j < numero_latas; j++)
	{
		canais_presenca += LATA_PegarCanaisTemp (&lata[j]);
	}
	return canais_presenca;
}*/

void App_configuraRadio ()
{
	SPI_StartMaster ();
	
	BYTE shortAddr[2] = {0x1a, 0xbc}, PANid[2] = {0xf2, 0x5a}, longAddr[8] = {0x11, 0x29, 0x04, 0x39, 0x7c, 0x22, 0x14, 0xae};
	
	zig_Init (17, longAddr, shortAddr, PANid);
	
	zig_TX_config (PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_ENABLED, 
				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR);
	
	BYTE dstShortAddr[2] = {0x0c, 0x4f}, dstPANid[2] = {0xef, 0x4d}, dstLongAddr[8] = {0x3e, 0xd1, 0x8a, 0x09, 0x2a, 0xdc, 0x68, 0xff};	
	
	zig_TX_configDstAddr (dstShortAddr, dstPANid);
}

void App_configuraMSP (unsigned int sinaisled1, unsigned int sinaisled2, unsigned int presenca1, unsigned int presenca2)
{ // 2.3: chip select, 2.2: ~rst, 2.0: wake
	
	/* Desativa WDT */
	WDTCTL = WDTPW  + WDTHOLD;
	
	/* Seta pinagem */
	P1DIR = sinaisled1;
	P2DIR = sinaisled2 + BIT3; 
	P2REN |= presenca2;
	P1REN |= presenca1;
	P1OUT = 0;
	P2OUT = 0;
	P1IES |= presenca1;
	P2IES |= presenca2;
	P2IFG = 0;
	P1IFG = 0;	
}

unsigned int App_lerCanal (unsigned int pino)
{
	if ( (pino & PX_X) == P1_X)
	{
		pino &= ~P1_X;
		return P1IN & pino; 
	}
		pino &= ~P2_X;
	return P2IN & pino; 
}

int App_pesquisaLataApartirIntPresenca (unsigned int pino, lt* lata, int numero_latas)
{ // Retorna o indice da lata que tem a pinagem da presenca correspondente ao valor em pino
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		if (LATA_PegarCanalPresenca (&lata[i]) == pino)
		{
			return i;
		}
	}
	return FALHOU;
}

/*
#pragma vector = TIMER0_A0_VECTOR //Timer0,TAIFG interrupt vector
__interrupt void TimerA(void)
{
	globalSeg--;
	if (globalSeg == 0)
	{
		TACTL = MC_0;
		__bic_SR_register_on_exit (LPM1_bits + GIE); 
	}
}*/