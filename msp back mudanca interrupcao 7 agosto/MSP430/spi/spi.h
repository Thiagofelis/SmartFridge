#ifndef SPI_H
#define SPI_H

#include <stdio.h>
#include <stdlib.h>
#include "msp430g2553.h"
#include "def.h"

void SPI_StartSlave ();

void SPI_StartMaster ();

BYTE SPI_Send (BYTE data);

//void SPI_SlaveRst ();

#endif
