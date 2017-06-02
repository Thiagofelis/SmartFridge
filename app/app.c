

int App_algumaLataPresente (lt *lata)
{
	int i;
	for (i = 0; i < numero_latas; i++) // lembrando que numero_latas é um macro definido em main.c
	{
		if (LATA_VerificarPresenca2 (&lata[i]) == true)
		{
			return true;
		}
	}
	return false;
}

void App_ativaIntPres (WORD canais_presenca)
{
	P1IE |= canais_presenca;
}

void App_desativaIntPres (WORD canais_presenca)
{
	P1IE &= ~canais_presenca;
}

void App_rstLatas (lt *lata)
{
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		LATA_Resetar (&lata[i]);
	}
}

void App_medirLatas (lt *lata, WORD medicoes[])
{
	// Faz medicao ate ter 20 medicoes validas para cada lata, 40 tentativas no maximo	
	for (iii = 0; iii < 40; iii++)
	{
		ADC_Medir (medicoes);
		
		int control_flag = 1;
		int valor_retornado;	

		// Carrega nos vetores de amostra de cada instancia de lata os valores medidos
		for (jjj = 0; jjj < numero_latas; jjj++)
		{
			valor_retornado = LATA_CarregarMedicoes (&lata[jjj], medicoes);

			// Verifica se a lata analisada esta ausente ou ja terminou as medicoes
			if (!( (valor_retornado == 2) || (valor_retornado == -1) ))
			{
				control_flag = 0;
			}
		}

		if (control_flag == 1)
		{
			break;
		}
	}
}

void App_attLedLatas (lt *lata)
{
	int i, j = 0;
	for (i = 1; i < numero_latas; i++)
	{
		if (LATA_VerificarPresenca2 (&lata[i]) == false)
		{
			continue;
		}
		
		if (LATA_VerificarPresenca2 (&lata[j]) == false)
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

void App_enviaMed (lt *lata)
{
	BYTE s[3];
	int i;
	for (i = 0; i < numero_latas; i++)
	{
		if (LATA_Enviar (&lata[i], s) == true)
		{
			zig_TX_PayloadToBuffer (s, 3);
			zig_TX_Transmit ();
		}
	}
}

void App_configurarADC (WORDPNT medicoes, WORD canais_presenca)
{
	ADC_Configurar (medicoes, canais_presenca);
}

WORD App_pegarCanaisPresenca (lt *lata)
{
	WORD canais_presenca = 0;
	int i;	
	for (i = 0; j < numero_latas; j++)
	{
		canais_presenca += LATA_PegarCanais (&lata[j]);
	}
	return canais_presenca;
}

void App_configuraRadio ()
{
	SPI_StartMaster ();
	
	BYTE shortAddr[2] = {0x21, 0x42}, PANid[2] = {0x31, 0x62}, longAddr[8] = {0x10, 0x29, 0x38, 0x47, 0x56, 0x65, 0x47, 0x38};
	
	zig_Init (11, longAddr, shortAddr, PANid);
	
	zig_TX_config (PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_ENABLED, 
				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR);
	
	BYTE dstShortAddr[2] = {0x08, 0x97}, dstPANid[2] = {0x63, 0x72};
	
	zig_TX_configDstAddr (dstShortAddr, dstPANid);
}

void App_configuraMSP ()
{
	WDTCTL = WDTPW  + WDTHOLD; // Desativa WDT
	/* Seta pinagem */
}

int _round (float i) //math.h ta dando problema 
{
	int k = (int) i;
	if ((i - (float)k) >= 0.5)
		return (k + 1);
	else
		return k;
}

int App_numDig (int a)
{
	int num = 1;
	
	if (a < 0)
		num++;
	if (abs (a) > 10)
		num++;
	if (abs (a) > 100)
		num++;
	if (abs (a) > 1000)
		num++;
	return num;
}

void App_configuraClock ()
{
	BCSCTL1 = CALBC1_1MHZ;     
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 = 0x00;     	
}

int App_tempMedia (int vec[])
{
	// Calculo do desvio padrao e da media
	
	int i, med = 0;

	for (i = 0; i < 20; i++)
	{
		med += vec[i];
	}
	med = med/20;

	float _dev = 0;

	for (i = 0; i < 20; i++)
	{
		_dev += pow (vec[i] - med, 2);
	}
	_dev = _dev/19;
	_dev = sqrt (_dev);
	int dev = _round(_dev);

	// Calcula nova media, sem os valores muito diferentes da media antiga
	int nova_media = 0, num = 0;

	for (i = 0; i < 20; i++)
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
	int ord[10] = {400, 300, 200, 250, 100, 0, -100, -200, -300, -400};

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

int App_lerCanal (int canal)
{
	int i;
	// canal = xy <=> porta x_y. ex. se canal = 24, é lida a porta 2_4
	switch (canal)
	{
		case 10:
			i = P1IN & BIT0;
			break;
		case 11:
			i = P1IN & BIT1;
			break;
		case 12:
			i = P1IN & BIT2;
			break;
		case 13:
			i = P1IN & BIT3;
			break;
		case 14:
			i = P1IN & BIT4;
			break;
		case 15:
			i = P1IN & BIT5;
			break;
		case 16:
			i = P1IN & BIT6;
			break;
		case 17:
			i = P1IN & BIT7;
			break;
		case 20:
			i = P2IN & BIT0;
			break;
		case 21:
			i = P2IN & BIT1;
			break;
		case 22:
			i = P2IN & BIT2;
			break;
		case 23:
			i = P2IN & BIT3;
			break;
		case 24:
			i = P2IN & BIT4;
			break;
		case 25:
			i = P2IN & BIT5;
			break;
		case 26:
			i = P2IN & BIT6;
			break;
		case 27:
			i = P2IN & BIT7;
			break;
	}	
	return i;
}

void App_bitmap (char *mem, unsigned int id, int temp)
{
	// Converte para DDKK PPII IIII IIII 
	//               BBBB BBBB AAAA AAAA
	// D sao os bits identificadores e I os bits do valor da temperatura
	// (esse valor ocupa no maximo os 10 primeiros bits)
	// os bits Ks sao onde sao sinalizados os erros e os Ps sao os bits de paridade
	
	// Soma-se 400 ao valor da temperatura, de modo que o menor valor
	// possivel, -400, vira 0 e a variavel vira unsigned
	
	unsigned int new = temp + 400;
	unsigned int tempA, tempB;
	tempA = 0xff & new;
	tempB = 0xff00 & new;
	tempB = tempB >> 8;
	tempB = (id << 6) | tempB;
 
	//mem[0] = (char) tempB;
	mem[0] = (char) (tempB | App_paridade (tempA, tempB));
	mem[1] = (char) tempA;
	mem[2] = '\0';
}

WORD App_paridade (unsigned int a, unsigned int b) // zero se par
{
	// a = IIII IIII
	// b = DDKK 00II <- 0s em BIT2 e BIT3 pois paridade ainda nao foi adicionada
	
	a ^= a >> 8;
	a ^= a >> 4;
	a ^= a >> 2;
	a ^= a >> 1;
	a = a & 1; // paridade em a
	
	b ^= b >> 8;
	b ^= b >> 4;
	b ^= b >> 2;
	b ^= b >> 1;
	b = b & 1; // paridade em b
	
	// 0000 0000 0000 ba00 <-localizacao dos bits de paridade no valor de retorno
	return ( (a << 2) | (b << 3) );
	// IMPORTANTE: quando for demapear, retirar os bits de paridade
	// antes de verificar a paridade
}

	