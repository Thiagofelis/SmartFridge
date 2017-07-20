#ifndef DEF_H
#define DEF_H

#include <stdio.h>
#include <stdlib.h>

// defines genericos

#define BIT14 0b100000000000000

#define true 1
#define false 0

#define P1_X 0
#define P2_X BIT8
#define PX_X BIT8

//define para intType
#define TIMER BIT0
#define RX BIT1
#define LATA BIT2
#define TIMER_IS_ON BIT7
#define SLEEPING BIT6

//define para mensagens

//primera mesagem de controle RPi>MSP
#define TEMP_LATA_0 BIT0
#define TEMP_LATA_1 BIT1
#define TEMP_LATA_2 BIT2
#define TEMP_LATA_TODAS 0b111
#define SETAR_TEMP_DE_0 BIT3 
#define SETAR_TEMP_DE_1 BIT4
#define SETAR_TEMP_DE_2 BIT5
#define PING BIT6
//segunda mesagem de controle RPi>MSP
#define PREVISAO_LATA_0 BIT0
#define PREVISAO_LATA_1 BIT1
#define PREVISAO_LATA_2 BIT2
//mensagem de controle MSP>RPi
#define LATA_0_ATINGIU_TEMP BIT0
#define LATA_1_ATINGIU_TEMP BIT1
#define LATA_2_ATINGIU_TEMP BIT2
#define LATA_TODAS_ATINGIU_TEMP 0b111

// quando tiver que mandar algo, a ordem sera
// TEMP_LATA 0 a 2, PREVISAO_LATA 0 a 2

//define de pino
#define ZIG_INTPIN BIT1

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char* BYTEPNT;
#endif