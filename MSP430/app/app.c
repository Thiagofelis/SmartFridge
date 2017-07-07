#include "app.h"

int App_algumaLataPresente (lt *lata, int numero_latas)
{
	int i;
	for (i = 0; i < numero_latas; i++) // lembrando que numero_latas é um macro definido em main.c
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

void App_salvarMedicoes (lt* lata, int numero_latas)
{
	int i = 0;
	for (i = 0; i < numero_latas; i++)
	{
		LATA_SalvarMedicoes (&lata[i]);
	}
}

void App_sleep10seg (unsigned int times)
{
	globalSeg = 23*times; // 10seg * times
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 65535;
	TACTL = TASSEL_2 + ID_3 + MC_2;                  // SMCLK, contmode

	__bis_SR_register(LPM1_bits + GIE);       // Enter LPM0 w/ interrupt
}

void App_enviaMed (lt *lata, int numero_latas)
{
	BYTE s[numero_latas * TAMANHO_PACOTE];
	int i, j, k = 0;
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
	
	zig_TX_PayloadToBuffer (s, k);
	do 
	{
		j = zig_TX_Transmit ();
	} while (j == -1); // interrupcao do timer vai contar e resetar o radio se demorar mt TEM Q IMPLMENETAR ESSE TREM AINDA
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
	
	zig_TX_config (PACKET_TYPE_DATA, ACK_REQUIRED_DISABLED, PAN_ID_COMP_ENABLED, 
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
	P2IE = BIT1;
	P2IES |= BIT1; // seleciona borda de descida
	P2IFG &= ~BIT1;	
	// Pulldown das presencas ja e selecionado no puc
	
}

unsigned int _round (float i) //math.h ta dando problema 
{	
	return (unsigned int)(i + 0.5);
}

unsigned int App_tempMedia (unsigned int vec[])
{
	// Calculo do desvio padrao e da media
	
	int i;
	unsigned int med = 0;

	for (i = 0; i < NUMERO_MEDICOES_NECESSARIAS; i++)
	{
		med += vec[i];
	}
	med = med/NUMERO_MEDICOES_NECESSARIAS;

	float _dev = 0;

	for (i = 0; i < NUMERO_MEDICOES_NECESSARIAS; i++)
	{
		_dev += pow (vec[i] - med, 2);
	}
	_dev = _dev/(NUMERO_MEDICOES_NECESSARIAS - 1);
	_dev = sqrt (_dev);
	unsigned int dev = (int) _round (_dev);

	// Calcula nova media, sem os valores muito diferentes da media antiga
	unsigned int nova_media = 0, num = 0;

	for (i = 0; i < NUMERO_MEDICOES_NECESSARIAS; i++)
	{
		if ((abs (vec[i] - med)) <= (2 * dev))
		{
			num++;
			nova_media += vec[i];
		}
	}
	nova_media = nova_media / num;

	return nova_media;
}



int App_pegarTemp (int x)
{
	double abs[10] = {5.24, 8.01, 10.0, 12.56, 20.24, 33.63, 57.67, 102.3, 188.2, 360.9};
	int ord[10] = {400, 300, 250, 200, 100, 0, -100, -200, -300, -400};

	// Converte a temp para um valor de resistencia em Kohm

	double res;
	res = ( 33 / (1023 - (double) x) ) *  (double)x;

	// Pesquisa no vetor as abiscissas em volta de res
	int i = 0;
	while ( !((res > abs[i]) && (res < abs[i + 1])) )
	{
		i++;
	}

	// Calcula temp correspondente
	int temp_final;

	double calc = (res - abs[i]) * (ord[i] - ord[i + 1]) / (abs[i] - abs[i + 1]);

	temp_final = ord[i] + calc;

	return temp_final;
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

#pragma vector = TIMER0_A0_VECTOR //Timer0,TAIFG interrupt vector
__interrupt void TimerA(void)
{
	globalSeg--;
	if (globalSeg == 0)
	{
		TACTL = MC_0;
		__bic_SR_register_on_exit (LPM1_bits + GIE); 
	}
}