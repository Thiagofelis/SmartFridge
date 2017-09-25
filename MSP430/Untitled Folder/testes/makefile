MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
INC = -I../def -I../spi -I../zig -I../adc -I../app -I../lata

main.o: teste.c ../app/app.h
	$(MCC) $(MSP) -c  teste.c $(CFLAGS) $(INC) -o ../build/teste.o

