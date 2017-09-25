MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
DEPS = lata.h
INC = -I../def -I../spi -I../zig -I../adc -I../app -I../lata

lata.o: lata.c lata.h ../app/app.h ../def/def.h
	$(MCC) $(MSP) -c  lata.c $(CFLAGS) $(INC) -o ../build/lata.o

