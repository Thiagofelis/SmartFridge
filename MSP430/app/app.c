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

void App_salvarMedicoes (lt* lata, int numero_latas)
{
	int i;
	for (i = 0; i < numero_latas; i++) 
	{
		LATA_SalvarMedicoes (&lata[i]);
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

	/*for (i = 0; i < numero_de_medicoes_a_enviar ; i++)
	{
		App_delayMs (500);
		P1OUT ^= BIT0;
	}*/
	
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
		s_temp[0 + *index] = j >> 8;
		s_temp[1 + *index] = j;
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
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		if (LATA_EstaPresente (&lata[i]) == true)
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
			if ( (medicao != FIM_MEDICOES_COMPLETAS) && (medicao != FIM_LATA_FICOU_AUSENTE) )
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

void App_attLedLatas (lt *lata, int numero_latas)
{
	int i, j = 0;
	for (i = 1; i < numero_latas; i++)
	{
		if (LATA_EstaPresente (&lata[i]) == false)
		{
			continue;
		}
		
		if (LATA_EstaPresente (&lata[j]) == false)
		{
			j = i;
			continue;
		}
		
		
		if (LATA_PegarTemp (&lata[i]) > LATA_PegarTemp (&lata[j]))
		{
			j = i;
		}
	}
	
	/* Acende o led da lata j */
}

void App_converterMedicoesEmTemp (lt* lata, int numero_latas)
{
	int i = 0;
	for (i = 0; i < numero_latas; i++)
	{
		LATA_ConverterMedicoesEmTemp (&lata[i]);
	}
}

void App_sleep1seg (unsigned int times)
{ // FUNCIONA :)
	globalSeg = 5 * times; 
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 25001;
	TACTL = TASSEL_2 + ID_3 + MC_1;                  // TASSEL = SMCLK, ID = /8, MC = COUNTS TO CCR0
	/*
		f = 10^6 / 8 , t = 8 * 10^-6
		t * 25000 * 5 = 1
	*/
	__bis_SR_register (LPM1_bits);       // Enter LPM1
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

void App_configurarADC (WORD* medicoes, WORD canais_temp)
{
	ADC_Configurar (medicoes, canais_temp);
}

WORD App_pegarCanaisTemp (lt *lata, int numero_latas)
{
	WORD canais_presenca = 0;
	int j;	
	for (j = 0; j < numero_latas; j++)
	{
		canais_presenca += LATA_PegarCanaisTemp (&lata[j]);
	}
	return canais_presenca;
}

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

void App_configuraMSP ()
{
	/* Desativa WDT */
	WDTCTL = WDTPW  + WDTHOLD;
	
	/* Seta pinagem */
	P2DIR = BIT3 + BIT2 + BIT0;  // 2.3: chip select, 2.2: ~rst, 2.0: wake
	P2OUT = BIT2;
	//P2IE = BIT1; <<so é ativado na funcao de inicializacao do radio
	P2IES |= BIT1; // seleciona borda de descida
	P2IFG &= ~BIT1;	
	// Pulldown das presencas ja e selecionado no puc
	
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