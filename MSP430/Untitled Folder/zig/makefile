MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
INC = -I../def -I../spi -I../lata -I../adc -I../app -I../zig
HEAD = ../spi/spi.h ../def/def.h ../lata/lata.h ../adc/adc.h ../app/app.h

zig.o: zig.c zig.h zigaddr.h $(HEAD) 
	$(MCC) $(MSP) -c  zig.c $(CFLAGS) $(INC) -o ../build/zig.o

