system:
	cd spi && $(MAKE) && cd .. && cd zig && $(MAKE) && cd .. && cd lata && $(MAKE) && cd .. && cd adc && $(MAKE) && cd .. && cd app && $(MAKE) && cd .. && cd main && $(MAKE) && cd .. && cd testes && $(MAKE) && cd .. && cd nmain && $(MAKE) 

BUILD = build/spi.o build/zig.o build/lata.o build/adc.o build/app.o build/main.o

BUILDTEST = build/spi.o build/zig.o build/lata.o build/adc.o build/app.o build/teste.o

BUILDN = build/spi.o build/zig.o build/lata.o build/adc.o build/app.o build/nmain.o

MCC = msp430-gcc
MSP = -0s -mmcu=msp430g2553
CFLAGS = -Wall -lm

soft: $(BUILD)
	$(MCC) $(MSP) $(BUILD) $(CFLAGS) -o soft.elf

test: $(BUILDTEST)
	$(MCC) $(MSP) $(BUILDTEST) $(CFLAGS) -o test.elf
	
nmain: $(BUILDN)
	$(MCC) $(MSP) $(BUILDN) $(CFLAGS) -o nmain.elf
