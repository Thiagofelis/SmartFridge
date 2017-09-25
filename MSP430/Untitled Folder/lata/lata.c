#include "lata.h"

int LATA_EstaPresente (lt* lp)
{
	return lp->esta_presente;
}

unsigned int LATA_PegarCanalPresenca (lt* lp)
{
	return lp->canal_presenca;
}

void LATA_AtualizarPresenca (lt* lp)
{
	unsigned int IntTemp;
	// "Writing to P1IES, or P2IES can result in setting the corresponding interrupt flags"
	// ^por isso salvamos PXIFG antes de escrever em PXIES
	if (App_lerCanal (lp->canal_presenca) != 0) 
	{// se a lata esta presente, ativamos int para borda de descida, para avisar quando a lata for tirada do slot	
		lp->esta_presente = true;
		if ((lp->canal_presenca & PX_X) == P1_X)
		{
			IntTemp = P1IFG; 
			P1IES |= (lp->canal_presenca - P1_X);
			P1IFG = IntTemp; 
		}
		if ((lp->canal_presenca & PX_X) == P2_X)
		{
			IntTemp = P2IFG;
			P2IES |= (lp->canal_presenca - P2_X);
			P2IFG = IntTemp; 
		}
	}
	else
	{// se a lata esta ausente, ativamos int de borda de subida, para avisar quando uma lata for colocado no slot
		lp->esta_presente = false;
		if ((lp->canal_presenca & PX_X) == P1_X)
		{
			IntTemp = P1IFG;
			P1IES &= ~(lp->canal_presenca - P1_X);
			P1IFG = IntTemp; 
		}
		if ((lp->canal_presenca & PX_X) == P2_X)
		{
			IntTemp = P2IFG;
			P2IES &= ~(lp->canal_presenca - P2_X);
			P2IFG = IntTemp; 
		}	
	}
}

unsigned int LATA_converterParaTemp (unsigned int x)
{ // ATENCAO, AGR ESSA FUNCAO INTERPOLA PARA VALORES ENTRE 0 E 800, EM Q 0 E -40C E 800 40C
	
	float abs[10] = {5.24, 8.01, 10.0, 12.56, 20.24, 33.63, 57.67, 102.3, 188.2, 360.9};
	unsigned int ord[10] = {800, 700, 650, 600, 500, 400, 300, 200, 100, 0};

	// Converte a temp para um valor de resistencia em Kohm

	float res;
	res = ( 33 / (1023 - (float) x) ) *  (float)x;

	// Pesquisa no vetor as abiscissas em volta de res
	int i = 0;
	while ( !((res > abs[i]) && (res < abs[i + 1])) )
	{
		i++;
	}

	// Calcula temp correspondente
	float calc = (res - abs[i]) * (ord[i] - ord[i + 1]) / (abs[i] - abs[i + 1]);

	return ord[i] + calc;	
}

void LATA_ConverterMedicoesEmTemp (lt* lp)
{
	if ( (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS) )
		return;
	
	lp->tempfinal = LATA_converterParaTemp (LATA_tempMedia (lp->amostra));	
}

unsigned int LATA_PegarTemp (lt* lp)
{
	if ( (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS) )
		return TEMP_INVALIDA;
	return lp->tempfinal;
}

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[])
{
	
	// Retorna 2 se a lata nao precisa mais de medicoes
	// Retorna 1 se a medicao teve sucesso e foi armazenada na instancia de lata correspondente.
	// Retorna 0 se a medicao nao foi valida
	// Retorna -1 se a lata nao necessita de medicoes, pois nao esta presente
	
	// ATENCAO: resolvi tirar a parte do ficou ausente, pois ja que as medicoes sao feitas muito rapidamente,
	// é impossivel que o usuario troque de latas um slot enquanto estiverem sendo feitas as medicoes. O maximo
	// que pode ocorrer é de ele retirar a lata, mas definitivamente nao haveria tempo suficiente para tirar a lata
	// e colocar uma diferente no slot

	// Caso a lata ja tenha 20 medicoes, nenhuma medicao a mais é feita
	if (lp->medic_feitas == NUMERO_MEDICOES_NECESSARIAS)
	{
		return FIM_MEDICOES_COMPLETAS;
	}

	// Verifica se a lata a ter a temp medida esta presente
	if (lp->esta_presente == false) // uma interrupcao de pinagem mantem essa variavel com um valor correto
	{
		return FIM_LATA_AUSENTE;
	}

	// Se a medicao estiver fora do alcance do termistor, ela é descartada
	if (LATA_MedicaoValida (medicoes[LATA_PosicaoVetor (lp->canal_temperatura)]) == false)
	{
		return CONTINUA_MEDICAO_INVALIDA;	
	}

	// Armazena a amostra no vetor correspondente a lata
	lp->amostra[lp->medic_feitas] = medicoes[LATA_PosicaoVetor (lp->canal_temperatura)];
	(lp->medic_feitas)++;
	return CONTINUA_MEDICAO_VALIDA;
}

int LATA_MedicaoValida (unsigned int a)
{
	if ((a > 930) || (a < 145))
		return false;
	return true;
}


int LATA_PosicaoVetor (unsigned int a)
{ // Faz a correspondencia entre o canal do ADC medido e a posicao do valor medido no vetor de medicoes
	int i;
	
	switch (a)
	{
		case BIT0 :
			i = 0;
			break;
			
		case BIT1 :
			i = 1;
			break;
			
		case BIT2 :
			i = 2;
			break;
			
		case BIT3 :
			i = 3;
			break;
			
		case BIT4 :
			i = 4;
			break;
			
		case BIT5 :
			i = 5;
			break;
			
		case BIT6 :
			i = 6;
			break;
			
		case BIT7 :
			i = 7;
			break;
	}
	
	i = 7 - i;
	
	return i;
}

void LATA_SalvarMedicoes (lt *lp)
{
	if ( (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS) )
		return;
	
	if (lp->ultimo_x < 4)
	{
		lp->medicoes_x[lp->ultimo_x] = lp->ultimo_x;
		lp->medicoes_y[lp->ultimo_x] = lp->tempfinal;
	} // as primeiras 4 medicoes sao guardadas
	else
	{
		if (lp->ultimo_x < 40)
		{ // a partir da 5, as medicoes sao guardadas a cada 5 minutos
			if ( (((lp->ultimo_x) % 20) % 5) == 0 )
			{
				lp->medicoes_x[((lp->ultimo_x) % 20) / 5] = lp->ultimo_x;
				lp->medicoes_y[((lp->ultimo_x) % 20) / 5] = lp->tempfinal;
			}
		}
		else
		{ // a partir da 40o medicao, as medicoes sao guardadas a cada 8 minutos
			if ( (((lp->ultimo_x) % 32) % 8) == 0 )
			{
				lp->medicoes_x[((lp->ultimo_x) % 32) / 8] = lp->ultimo_x;
				lp->medicoes_y[((lp->ultimo_x) % 32) / 8] = lp->tempfinal;
			}
		}
	}
	lp->ultimo_x++;
}

void LATA_Resetar (lt *lp)
{
	lp->medic_feitas = 0;
}

WORD LATA_PegarCanaisTemp (lt* lp)
{
	return lp->canal_temperatura;	
}

void LATA_SetarTempDesejada (lt* lp, unsigned int temp)
{
	lp->temp_desejada = temp;
}

unsigned char LATA_AtingiuTemp (lt* lp)
{
	if ( (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS) )
	{ // medicao foi mal sucedida
		return 0;
	}
	if ( (lp->temp_desejada != NAO_TEM) && (lp->tempfinal <= lp->temp_desejada) )
	{ // temp da lata atingiu a meta
		lp->temp_desejada = NAO_TEM;
		lp->ultimo_x = PAROU; // DEPOIS, tem q olhar o trem da previsao, ta uma bagunca e n ta funcionando ainda
		return 1;
	}
	else
	{ // temp nao atingiu a meta
		return 0;
	}
}

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, lt *lp, unsigned int identific)
{
	lp->canal_temperatura = tempcanal;
	lp->canal_presenca = prescanal;
	lp->id = identific;
	lp->ultimo_x = 0;
	lp->temp_desejada = NAO_TEM;
	//lp->ultimo_TX[0] = 0xff; // situacao impossivel, de modo que a primeira medicao valida feita vai ser necessariamente enviada
//	lp->ultimo_TX[1] = ' '; // ^tem q olhar isso
}
/*
int LATA_MontarPacote (lt* lp, BYTE* s)
{
	unsigned int val;

	if (lp->ficou_ausente == true)
	{
		val = LATA_AUSENTE;
	}
	else
	{
		if (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS)
		{ 
			val = LATA_SEM_MEDICOES_VALIDAS;
		}
		else
		{
			val = lp->tempfinal;
		}
	}
	
	LATA_Bitmap (s, lp->id, val);
	
	if (memcmp (s, lp->ultimo_TX, TAMANHO_PACOTE * sizeof (unsigned char)) != 0) // verifica se a mensagem a ser enviada é igual a ultima enviada
	{
		memcpy (lp->ultimo_TX, s, TAMANHO_PACOTE * sizeof (unsigned char));
		return PACOTE_DIFERENTE;
	}
	return PACOTE_REPETIU;
}*/

void LATA_Bitmap (unsigned char *mem, unsigned int id, unsigned int temp)
{
	// Converte para DDKK PPII IIII IIII 
	//               BBBB BBBB AAAA AAAA
	// D sao os bits identificadores e I os bits do valor da temperatura
	// (esse valor ocupa no maximo os 10 primeiros bits)
	// os bits Ks sao onde sao sinalizados os erros e os Ps sao os bits de paridade
	
	unsigned int tempA, tempB;
	
	if (temp == LATA_AUSENTE)
	{
		tempA = 0;
		tempB = (id << 6) | BIT4; // BIT4 => LATA_AUSENTE
	}
	else 
	{
		if (temp == LATA_SEM_MEDICOES_VALIDAS)
		{ 	
			tempA = 0;
			tempB = (id << 6) | BIT5; // BIT5 => LATA_SEM_MEDICOES_VALIDAS
		}
		else
		{
			tempA = 0xff & temp;
			tempB = 0xff00 & temp;
			tempB = tempB >> 8;
			tempB = (id << 6) | tempB;
		}
	}
	
	mem[0] = (unsigned char) (tempB | LATA_Paridade (tempA, tempB));
	mem[1] = (unsigned char) tempA;
}

unsigned int LATA_Paridade (unsigned int a, unsigned int b) 
	// zero se par
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

unsigned int LATA_tempMedia (unsigned int vec[])
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

unsigned int _round (float i) //math.h ta dando problema 
{	
	return (unsigned int)(i + 0.5);
}