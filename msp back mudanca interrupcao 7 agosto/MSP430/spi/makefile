MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
INC = -I../def
spi.o: spi.c spi.h ../def/def.h
	$(MCC) $(MSP) -c  spi.c $(CFLAGS) $(INC) -o ../build/spi.o
