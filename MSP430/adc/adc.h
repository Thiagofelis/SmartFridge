#ifndef ADC_H
#define ADC_H

#include "msp430g2553.h"
#include <stdio.h>
#include <stdlib.h>

#define NUMERO_DE_CANAIS_DO_ADC 8

void ADC_Medir (unsigned int *medicoes);

void ADC_Configurar (unsigned int *medicoes, unsigned int canais_set);

#endif