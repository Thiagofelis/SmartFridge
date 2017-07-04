MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall
INC = -I../def -I../spi

zig.o: zig.c zig.h zigaddr.h ../spi/spi.h ../def/def.h
	$(MCC) $(MSP) -c  zig.c $(CFLAGS) $(INC) -o ../build/zig.o

