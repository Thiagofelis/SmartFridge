#ifndef ADC_H
#define ADC_H

#include "msp430g2553.h"
#include <stdio.h>
#include <stdlib.h>

void ADC_Medir (int *medicoes);

void ADC_Configurar (int *medicoes, unsigned int canais_set);

#endif