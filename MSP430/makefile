system:
	cd spi && $(MAKE) && cd .. && cd zig && $(MAKE) && cd .. && cd lata && $(MAKE) && cd .. && cd adc && $(MAKE) && cd .. && cd app && $(MAKE) && cd .. && cd main && $(MAKE) 

BUILD = build/spi.o build/zig.o build/lata.o build/adc.o build/app.o build/main.o 

MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall -lm

soft: $(BUILD)
	$(MCC) $(MSP) $(BUILD) $(CFLAGS) -o soft.elf
