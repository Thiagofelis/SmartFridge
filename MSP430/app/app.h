#ifndef APP_H
#define APP_H

#include "msp430g2553.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "def.h"
#include "adc.h"
#include "zig.h"
#include "lata.h"

#define MAX_TENTATIVAS_MEDICAO 40

#define FIM_MEDICOES_COMPLETAS 2
#define FIM_LATA_FICOU_AUSENTE -1
#define CONTINUA_MEDICAO_INVALIDA 0
#define CONTINUA_MEDICAO_VALIDA 1

#define TAMANHO_PACOTE 2 //em bytes

#define PACOTE_DIFERENTE 1
#define PACOTE_REPETIU 0

#define NUMERO_MEDICOES_NECESSARIAS 20	

/* Utilizado para adormecer */
unsigned int globalSeg;

int App_algumaLataPresente (lt *lata, int numero_latas);

void App_ativaIntPres (unsigned int sinais_presenca1, unsigned int sinais_presenca2);

void App_desativaIntPres (unsigned int sinais_presenca1, unsigned int sinais_presenca2);

void App_rstLatas (lt *lata, int numero_latas);

void App_medirLatas (lt *lata, WORD medicoes[], int numero_latas);

void App_attLedLatas (lt *lata, int numero_latas);

void App_enviaMed (lt *lata, int numero_latas);

void App_sleep10seg (unsigned int times);

unsigned int _round (float i); //math.h ta dando problema 

unsigned int App_tempMedia (unsigned int vec[]);

//int App_pegarTemp (int x);

unsigned int App_lerCanal (unsigned int pino);

void App_configuraMSP ();

void App_configuraRadio ();

void App_configurarADC (WORD *medicoes, WORD canais_presenca);

WORD App_pegarCanaisTemp (lt *lata, int numero_latas);

#endif
