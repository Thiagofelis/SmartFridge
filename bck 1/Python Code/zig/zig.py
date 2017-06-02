import RPi.GPIO as GPIO
import time


class Rd (object):
	def __init__ (self, channel, srcAddrLong, srcAddrShort, srcPANid):
		self.channel = channel
		self.seqNum = 0x80;
		self.TX_busy = 0;
		self.TX_awatingAck = 0;
		self.TX_lastPackFail = 0;
		self.TX_busyChannel = 0;
		
		self.RstSoft ()
		self.SetShort (RXFLUSH, 0x01)
		self.SetShort (PACON2, 0x98)
		self.SetShort (TXSTBL, 0x95)
		self.SetLong (RFCTRL0, 0x03)
		self.SetLong (RFCTRL1, 0x02)
		self.SetLong (RFCTRL2, 0x80)
		self.SetLong (RFCTRL6, 0x90)
		self.SetLong (RFCTRL7, 0x80)
		self.SetLong (RFCTRL8, 0x10)
		self.SetLong (SCLKDIV, 0x21)
		self.SetShort (BBREG2, 0x80)
		self.SetShort (CCAEDTH, 0x60)
		self.SetShort (BBREG6, 0x40)
		self.SetShort (INTCON, 0b11110110) # ativa int de rx e tx
		self.SelChannel (channel) # Seleciona canal 11
		self.RstRF ()
		self.SetShort (RXMCR, 0x00) 
		self.SetLong (RFCTRL3, 0x00) # Seleciona potencia maxima
		self.GetShort (INTSTAT) # Limpa int pendentes
		self.SetShort (BBREG1, 0x00)
		self.configRadioAddr (srcAddrLong, srcAddrShort, srcPANid) 
		self.RstRF ()
		
	def configRadioAddr (self, srcAddrLong, srcAddrShort, srcPANid):
		self.AddrLong = srcAddrLong
		self.AddrShort = srcAddrShort
		self.PANid = srcPANid		

	def RstRF (self):
		self.SetShort (RFCTL, 0x04)
		self.SetShort (RFCTL, 0x00)
		time.delay (0.0003) #300 μs, recomendado é 192 μs

	def RstSoft (self):
		self.SetShort (SOFTRST, 0x07) # SOFTWARE RESET
		while ((self.GetShort (SOFTRST) & 0x07) != 0x00)
		time.delay () #decidir dpois
	
	def SetShort (self, address, value):
		GPIO.output (x, 1)
		spi_Send (((address << 1) & 0b01111111) | 0x01)
		spi_Send (value)
		GPIO.output (x, 0)
		
	def GetShort (self, address):
		#P2OUT &= ~BIT3
		SPI_Send ((address << 1) & 0b01111110)
		toReturn = SPI_Send (0x00)
		#P2OUT |= BIT3
		return toReturn	

	def GetLong (self, address):
		#P2OUT &= ~BIT3;
		SPI_Send (((address >> 3) & 0b01111111) | 0x80)
		SPI_Send (((address << 5) & 0b11100000))
		toReturn = SPI_Send (0x00)
		#P2OUT |= BIT3;
		return toReturn

	def SetLong (self, address, value):
		#P2OUT &= ~BIT3;
		SPI_Send ((((BYTE)(address >> 3)) & 0b01111111) | 0x80)
		SPI_Send ((((BYTE)(address << 5)) & 0b11100000) | 0x10)
		SPI_Send (value)
		#P2OUT |= BIT3;
		
	def SelChannel (self, channel):
		self.SetLong (RFCTRL0, ((channel - 11) << 4) | 0x03)
		self.RstRF ()
		
	def ContiguousWrite (self, addr, mem, count):	
		for i in range (0, count)
			self.SetLong (addr++, mem[i])
		return addr
	
	def ContiguousRead (self, addr, mem, count):
		for i in range (0, count)
			mem[i] = self.GetLong (addr++)
		return addr