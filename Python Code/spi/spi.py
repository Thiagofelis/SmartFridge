

def spi_Start ():
	import spidev
	spi = spidev.SpiDev()
	spi.open(0, 0) # port 0, device 1

def spi_Send (byte):
	retrn = spi.xfer2(byte)
	return retrn

