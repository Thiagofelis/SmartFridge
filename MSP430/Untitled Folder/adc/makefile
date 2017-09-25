MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall

zig.o: adc.c adc.h
	$(MCC) $(MSP) -c  adc.c $(CFLAGS) -o ../build/adc.o

