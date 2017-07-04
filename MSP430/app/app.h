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

/* Utilizado para adormecer */
unsigned int globalSeg;

int App_algumaLataPresente (lt *lata, int numero_latas);

void App_ativaIntPres (WORD canais_presenca);

void App_desativaIntPres (WORD canais_presenca);

void App_rstLatas (lt *lata, int numero_latas);

void App_medirLatas (lt *lata, WORD medicoes[], int numero_latas);

void App_attLedLatas (lt *lata, int numero_latas);

void App_enviaMed (lt *lata, int numero_latas);

void App_sleep10seg (unsigned int times);

int _round (float i); //math.h ta dando problema 

int App_numDig (int a);

int App_tempMedia (int vec[]);

int App_pegarTemp (int x);

int App_lerCanal (int canal);

void App_bitmap (unsigned char *mem, unsigned int id, int temp);

WORD App_paridade (unsigned int a, unsigned int b);

void App_configuraMSP ();

void App_configuraRadio ();

void App_configurarADC (WORD *medicoes, WORD canais_presenca);

WORD App_pegarCanaisPresenca (lt *lata, int numero_latas);

#endif
