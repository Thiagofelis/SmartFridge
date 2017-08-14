MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall -lm
INC = -I../def -I../spi -I../zig -I../lata -I../adc -I../app
HEAD = ../zig/zig.h ../spi/spi.h ../def/def.h ../lata/lata.h ../adc/adc.h


app.o: app.c app.h $(HEAD) 
	$(MCC) $(MSP) -c  app.c $(CFLAGS) $(INC) -o ../build/app.o

