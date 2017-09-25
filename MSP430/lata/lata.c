#include "lata.h"

int LATA_EstaPresente (lt* lp)
{
	if (App_lerCanal (lp->canal_presenca) != 0)
		return true;
	return false;
}

unsigned int LATA_PegarCanalLed (lt* lp)
{
	return lp->canal_led;	
}

unsigned int LATA_PegarCanalPresenca (lt* lp)
{
	return lp->canal_presenca;
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
	{
		lp->tempfinal = LATA_SEM_MEDICAO; 
	}
	else
	{
		lp->tempfinal = LATA_converterParaTemp (LATA_tempMedia (lp->amostra));	
	}
}

unsigned int LATA_PegarTemp (lt* lp)
{
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
	if (LATA_EstaPresente (lp) == false) 
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
		return 1;
	}
	else
	{ // temp nao atingiu a meta
		return 0;
	}
}

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, unsigned int led, lt *lp, unsigned int identific)
{
	lp->canal_led = led;
	lp->canal_temperatura = tempcanal;
	lp->canal_presenca = prescanal;
	lp->id = identific;
	lp->temp_desejada = NAO_TEM;
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