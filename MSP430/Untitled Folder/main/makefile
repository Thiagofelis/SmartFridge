MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
INC = -I../def -I../spi -I../zig -I../adc -I../app -I../lata

main.o: main.c ../app/app.h
	$(MCC) $(MSP) -c  main.c $(CFLAGS) $(INC) -o ../build/main.o

