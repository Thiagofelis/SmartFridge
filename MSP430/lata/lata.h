#ifndef LATA_H
#define LATA_H


#include "msp430g2553.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"

#define FIM_MEDICOES_COMPLETAS 2
#define FIM_LATA_FICOU_AUSENTE -1
#define CONTINUA_MEDICAO_INVALIDA 0
#define CONTINUA_MEDICAO_VALIDA 1

#define TAMANHO_PACOTE 2 //em bytes

#define PACOTE_DIFERENTE 1
#define PACOTE_REPETIU 0

#define LATA_AUSENTE BIT14
#define LATA_SEM_MEDICOES_VALIDAS BIT0 | BIT14

#define NUMERO_MEDICOES_NECESSARIAS 20	

typedef struct lat
{
	// canal_temperatura guarda o bit correspondente ao canal utilizado para cada medicao
	// tempfinal armazena a temperatura apos o tratamento da amostra de 20 medicoes, armazenada em amostra[20]
	// medic_feitas guarda o numero de medicoes feitas ate o momento e presenca_flag indica se a lata esta presente
	// durante todo o processo de medicao (condicao para a amostra ser tratada)
	// porta_presenca guarda o numero da porta utilizada para verificar a presenca (ex. 21 se refere a porta 2_1)
	unsigned int canal_temperatura, canal_presenca;
	unsigned int tempfinal;
	int medic_feitas, ficou_ausente; // ficou_ausente = 1 <=> lata ficou ausente durante a medicao
	unsigned int id; // so 2 bit
	unsigned char ultimo_TX[TAMANHO_PACOTE + 1]; // guarda string da ultima mensagem enviada
	unsigned int amostra[NUMERO_MEDICOES_NECESSARIAS];									
} lt;

#include "app.h"

int LATA_PegarTemp (lt* lp);

void LATA_SalvarMedicoes (lt* lp);

int LATA_MedicaoValida (unsigned int a);

void LATA_Resetar (lt *lp);

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, lt *lp, unsigned int identific);

int LATA_MontarPacote (lt* lp, BYTE* s);

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[]);

unsigned int LATA_PegarCanaisTemp (lt* lp);

int LATA_EstaPresente (lt* lp);

int LATA_PosicaoVetor (unsigned int a);

WORD LATA_Paridade (unsigned int a, unsigned int b);

void LATA_Bitmap (unsigned char *mem, unsigned int id, unsigned int temp);

#endif
