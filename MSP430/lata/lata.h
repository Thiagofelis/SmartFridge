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

#define LATA_AUSENTE 1024
#define LATA_SEM_MEDICOES_VALIDAS 1025

#define NUMERO_MEDICOES_NECESSARIAS 10	

#define PONTOS_A_SEREM_INTERPOLADOS 4

#define TEMP_INVALIDA 0b111111111111

#define NAO_TEM 900

#define PAROU 0xff

typedef struct lat
{
	// canal_temperatura guarda o bit correspondente ao canal utilizado para cada medicao
	// tempfinal armazena a temperatura apos o tratamento da amostra de 20 medicoes, armazenada em amostra[20]
	// medic_feitas guarda o numero de medicoes feitas ate o momento e presenca_flag indica se a lata esta presente
	// durante todo o processo de medicao (condicao para a amostra ser tratada)
	// porta_presenca guarda o numero da porta utilizada para verificar a presenca (ex. 21 se refere a porta 2_1)
	unsigned int canal_temperatura : 8;
	unsigned int canal_presenca    : 9;	
	unsigned int tempfinal         :10;
	unsigned int temp_desejada     :10;
	unsigned int medic_feitas      : 4; // ATENCAO, precisa mudar se NUMERO_MEDICOES_NECESSARIAS mudar
	unsigned int ficou_ausente     : 1; // ficou_ausente = 1 <=> lata ficou ausente durante a medicao
	unsigned int id                : 2;
//	unsigned char ultimo_TX[TAMANHO_PACOTE + 1]; // guarda string da ultima mensagem enviada
	unsigned char medicoes_x[PONTOS_A_SEREM_INTERPOLADOS];
	unsigned int medicoes_y[PONTOS_A_SEREM_INTERPOLADOS];	
	unsigned char ultimo_x; // soma 1 a cada minuto q passa
	unsigned int amostra[NUMERO_MEDICOES_NECESSARIAS];									
} lt;

#include "app.h"

void LATA_SetarTempDesejada (lt* lp, unsigned int temp);

unsigned char LATA_AtingiuTemp (lt* lp);

void LATA_SalvarMedicoes (lt *lp);

unsigned int LATA_converterParaTemp (unsigned int x);

unsigned int LATA_PegarTemp (lt* lp);

void LATA_ConverterMedicoesEmTemp (lt* lp);

int LATA_MedicaoValida (unsigned int a);

void LATA_Resetar (lt *lp);

void LATA_Iniciar (unsigned int tempcanal, unsigned int prescanal, lt *lp, unsigned int identific);

//int LATA_MontarPacote (lt* lp, BYTE* s);

int LATA_CarregarMedicoes (lt* lp, unsigned int medicoes[]);

unsigned int LATA_PegarCanaisTemp (lt* lp);

int LATA_EstaPresente (lt* lp);

int LATA_PosicaoVetor (unsigned int a);

WORD LATA_Paridade (unsigned int a, unsigned int b);

void LATA_Bitmap (unsigned char *mem, unsigned int id, unsigned int temp);

unsigned int LATA_tempMedia (unsigned int vec[]);

unsigned int _round (float i); //math.h ta dando problema 

#endif
