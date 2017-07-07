#include "lata.h"

int LATA_EstaPresente (lt* lp)
{
	return (App_lerCanal (lp->canal_presenca) != 0 ? true : false);
}


void LATA_SalvarMedicoes (lt* lp)
{
	if ( (lp->ficou_ausente == true) || (lp->medic_feitas != NUMERO_MEDICOES_NECESSARIAS) )
		return;

	lp->tempfinal = App_tempMedia (lp->amostra);
}

int LATA_PegarTemp (lt* lp)
{
	return lp->tempfinal;
}

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[])
{
	
	// Retorna 2 se a lata nao precisa mais de medicoes
	// Retorna 1 se a medicao teve sucesso e foi armazenada na instancia de lata correspondente.
	// Retorna 0 se a medicao nao foi valida
	// Retorna -1 se a lata nao necessita de medicoes, pois nao esta presente

	// Verifica se em algum ponto das 20 medicoes, a lata deixou de estar presente
	if (lp->ficou_ausente == true)
	{
		return FIM_LATA_FICOU_AUSENTE;
	}

	// Caso a lata ja tenha 20 medicoes, nenhuma medicao a mais é feita
	if (lp->medic_feitas == NUMERO_MEDICOES_NECESSARIAS)
	{
		return FIM_MEDICOES_COMPLETAS;
	}

	// Verifica se a lata a ter a temp medida esta presente
	if (LATA_EstaPresente (lp) == false)
	{
		lp->ficou_ausente  = 1;
		return FIM_LATA_FICOU_AUSENTE;
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
	lp->ficou_ausente = 0;	
	lp->medic_feitas = 0;
}

WORD LATA_PegarCanaisTemp (lt* lp)
{
	return lp->canal_temperatura;	
}

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, lt *lp, unsigned int identific)
{
	lp->canal_temperatura = tempcanal;
	lp->canal_presenca = prescanal;
	lp->id = identific;
	lp->ultimo_TX[0] = 0xff; // situacao impossivel, de modo que a primeira medicao valida feita vai ser necessariamente enviada
	lp->ultimo_TX[1] = ' '; // ^tem q olhar isso
}

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
}

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
