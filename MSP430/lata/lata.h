#ifndef LATA_H
#define LATA_H


#include "msp430g2553.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"

typedef struct lat
{
	// canal_temperatura guarda o bit correspondente ao canal utilizado para cada medicao
	// tempfinal armazena a temperatura apos o tratamento da amostra de 20 medicoes, armazenada em amostra[20]
	// medic_feitas guarda o numero de medicoes feitas ate o momento e presenca_flag indica se a lata esta presente
	// durante todo o processo de medicao (condicao para a amostra ser tratada)
	// porta_presenca guarda o numero da porta utilizada para verificar a presenca (ex. 21 se refere a porta 2_1)
	unsigned int canal_temperatura, canal_presenca;
	unsigned int tempfinal;
	int medic_feitas, presenca_flag; // presenca_flag = 1 <=> lata ficou ausente durante a medicao
	unsigned int id; // so 2 bit
	unsigned char ultimo_TX[3]; // guarda string da ultima mensagem enviada
	unsigned int amostra[20];									
} lt;

#include "app.h"

int LATA_PegarTemp (lt* lp);

void LATA_SalvarMedicoes (lt* lp);

int LATA_MedicaoValida (unsigned int a);

void LATA_Resetar (lt *lp);

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, lt *lp, unsigned int identific);

int LATA_Enviar (lt* lp, BYTE* s);

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[]);

unsigned int LATA_PegarCanaisTemp (lt* lp);

int LATA_EstaPresente (lt* lp);

int LATA_PosicaoVetor (unsigned int a);


#endif
