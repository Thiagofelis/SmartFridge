#include "lata.h"

int LATA_VerificarPresenca (int canal)
{
	
	return (Ler_Canal (canal));
}

void LATA_SalvarMedicoes (lt* lp)
{
	if ( (lp->presenca_flag == 1) || (lp->medic_feitas != 20) )
		return;
	
	int aux;
	
	aux = Temp_Media (lp->amostra);
	aux = Pegar_Temp (aux);
	lp->tempfinal = aux;
}

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[])
{
	
	// Retorna 2 se a lata nao precisa mais de medicoes
	// Retorna 1 se a medicao teve sucesso e foi armazenada na instancia de lata correspondente.
	// Retorna 0 se a medicao nao foi valida
	// Retorna -1 se a lata nao necessita de medicoes, pois nao esta presente

	// Verifica se em algum ponto das 20 medicoes, a lata deixou de estar presente
	if (lp->presenca_flag == 1)
	{
		return -1;
	}

	// Caso a lata ja tenha 20 medicoes, nenhuma medicao a mais é feita
	if (lp->medic_feitas == 20)
	{
		return 2;
	}

	// Verifica se a lata a ter a temp medida esta presente
	if (!LATA_VerificarPresenca (lp->porta_presenca))
	{
		lp->presenca_flag  = 1;
		return -1;
	}

	// Se a medicao estiver fora do alcance do termistor, ela é descartada
	if (!LATA_MedicaoValida ((int) medicoes[LATA_PosicaoVetor (lp->canal_temperatura)]))
	{
		return 0;	
	}

	// Armazena a amostra no vetor correspondente a lata
	lp->amostra[lp->medic_feitas] = (int) medicoes[LATA_PosicaoVetor (lp->canal_temperatura)];
	(lp->medic_feitas)++;
	return 1;
}

int LATA_MedicaoValida (unsigned int a)
{
	if ((a > 930) || (a < 145))
		return 0;
	return 1;
}


int LATA_PosicaoVetor (unsigned int a)
{
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
	lp->presenca_flag = 0;	
	lp->medic_feitas = 0;
}

unsigned int LATA_PegarCanais (lt* lp)
{
	return lp->canal_temperatura;	
}

void LATA_Iniciar (unsigned int tempcanal, int presscanal, lt *lp, unsigned int identific)
{
	lp->canal_temperatura = tempcanal;
	lp->porta_presenca = presscanal;
	lp->id = identific;
	lp->ultimo_TX[0] = 0xff; // situacao impossivel, de modo que a primeira medicao valida feita vai ser necessariamente enviada
	lp->ultimo_TX[1] = ' ';
	lp->ultimo_TX[2] = '\0';
}

void LATA_Enviar (lt* lp)
{
	int a;
	char s[3];

	if ( (lp->presenca_flag == 1) )
	{
		a = 0x1000; // valor para lata nao detectada
		a -= 400;
	}
	else 	
	{
		if ( (lp->medic_feitas != 20) )
		{
			a = 0x2000;  // valor para medicoes invalidas
			a -= 400;
		}	
		else
		{
			a = lp->tempfinal; 
		}
	}
	
	Bitmap (s, lp->id, a);
	
	if (strcmp (s, lp->ultimo_TX) != 0) // verifica se a mensagem a ser enviada é igual a ultima enviada
	{
		UART_Enviar (s);
		strcpy (lp->ultimo_TX, s);
	}
}