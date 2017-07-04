#ifndef GEN2_H
#define GEN2_H

#include <stdio.h>
#include <stdlib.h>
#include "msp430g2553.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char* BYTEPNT;

#define RED 1
#define GREEN 0

void Blink (int i);

void BlinkBinary (BYTE data);

#endif