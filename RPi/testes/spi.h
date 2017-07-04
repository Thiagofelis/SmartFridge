#ifndef SPI_H
#define SPI_H

#include <stdio.h>
#include <stdlib.h>
#include "msp430g2553.h"
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char* BYTEPNT;


void SPI_StartSlave ();

void SPI_StartMaster ();

BYTE SPI_Send (BYTE data);

void SPI_SlaveRst ();

#endif
