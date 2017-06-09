import RPi.GPIO as GPIO
import time
import spidev
import sys
sys.path.append('../inc')
from defines import *
	

class TX (object):
	def __init__ (self):
		self.seqNum = 0x00
	
	def addrConfig (self, PANID, SHORT=0, LONG=0): # addr as strings, PAN=2bytes, SHORT=2bytes, LONG=8bytes
		# ex. PAN = "af23"
		self.dstPANID = bytearray.fromhex (PANID)
		self.dstSHORT = bytearray.fromhex (SHORT)
		self.dstLONG = bytearray.fromhex (LONG)
		
	def config (self, frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode):
		self.frameControl = bytearray (2)
		self.frameControl[0] = frameType | ackRequired | PANcomp
		self.frameControl[1] = noSequenceNum | dstAddrMode | srcAddrMode

	def payloadToBuffer (self, payload):
		self.payload = payload #tipo do argumento da funcao tem de ser bytearray
		#self.payload = bytearray (payload, 'utf-8') #size = len(self.payload), payload is a string
	
class RX (object):
	def __init__ (self):

class Rd (object):
	def _receive (self):
		if self.RX_bufferRear == self.RX_bufferFront and self.RX_empty == False: #buffer cheio
			#decidir dpois o q fazer			
			return

		self.setRegister ("BBREG1", 0b100) # stop rx fifo from receive packets
	
		currAddr = 0x300
	
#		currAddr = zig_ContiguousRead (currAddr, self.RXbuff[RX_bufferRear].frameLenght, 1);



	def _transmit (self):

		currAddr = 0x02 # jump Header and Frame lenght, fill later

		# Write control frame
		currAddr = self.ContiguousWrite (currAddr, self.TXbuff.frameControl, 2)

		currAddr = self.ContiguousWrite (currAddr, self.TXbuff.seqNum, 1)

		if self.TXbuff.seqNum == 0x7f: # seqNum always <0x80, loops from 0x7f to 0x00
			self.TXbuff.seqNum = 0x00
		else:
			self.TXbuff.seqNum += 1
		
		if (self.TXbuff.frameControl[1] & DST_ADDR_MODE) == DST_SHORT_ADDR:
			currAddr = self.ContiguousWrite (currAddr, self.TXbuff.dstPANID, 2)
			currAddr = self.ContiguousWrite (currAddr, self.TXbuff.dstSHORT, 2)
	
		elif (self.TXbuff.frameControl[1] & DST_ADDR_MODE) == DST_LONG_ADDR: 
			currAddr = self.ContiguousWrite (currAddr, self.TXbuff.dstPANID, 2)
			currAddr = self.ContiguousWrite (currAddr, self.TXbuff.dstLONG, 8)

		if (((self.TXbuff.frameControl[1] & DST_ADDR_MODE) != DST_NO_ADDR)
		  and ((self.frameControl[1] & SRC_ADDR_MODE) != SRC_NO_ADDR)  
		  and ((self.frameControl[0] & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)): 
			currAddr = self.ContiguousWrite (currAddr, self.PANid, 2)

		if (self.TXbuff.frameControl[1] & SRC_ADDR_MODE) == SRC_SHORT_ADDR:
			currAddr = self.ContiguousWrite (currAddr, self.AddrShort, 2)
		
		elif (self.TXbuff.frameControl[1] & SRC_ADDR_MODE) == SRC_LONG_ADDR: 
			currAddr = self.ContiguousWrite (currAddr, self.AddrLong, 8)

		self.setRegister (0x00, currAddr - 2) # minus the lenght of the first 2 bytes
		currAddr = self.ContiguousWrite (currAddr, self.TXbuff.payload, len(self.TXbuff.payload))
		self.setRegister (0x01, currAddr - 2)

		self.setRegister ("TXNCON", 0b101 if ((self.TXbuff.frameControl[0] & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED) else 0b1) # start transmission

		if (self.TXbuff.frameControl[0] & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED:
			self.TX_awatingAck = 1
		
		self.TX_busy = 1

	def startSPI (self):
		self.spi = spidev.SpiDev()
		self.spi.open (0,0)
		self.spi.mode = 0
		self.spi.max_speed_hz = 10000
		self.spi.cshigh = False
		
	def setRegister (self, reg, value):
		if reg in short_addr_registers:
			reg_addr = short_addr_registers[reg]
			bytes = [((reg_addr << 1) + 0x01), value]
		else:
			if reg in long_addr_registers:
				reg_addr = long_addr_registers[reg]
			else:
				reg_addr = reg
			bytes = []
			bytes.append((reg_addr >> 3) + 0x80)
			bytes.append(((reg_addr << 5) & 0xE0) + 0x10)
			bytes.append(value)
		self.spi.xfer2(bytes)

	def getRegister(self, reg):
		if reg in short_addr_registers:
			reg_addr = short_addr_registers[reg]
			bytes = [(reg_addr << 1), 0]
			result = self.spi.xfer2(bytes)
			return result[1]
		else:
			if reg in long_addr_registers:
				reg_addr = long_addr_registers[reg]
			else:
				reg_addr = reg
			bytes = []
			bytes.append((reg_addr >> 3) + 0x80)
			bytes.append((reg_addr << 5) & 0xE0)
			bytes.append(0)
			result = self.spi.xfer2(bytes)
			return result[2]		

	def __init__ (self, channel, srcAddrShort, srcAddrLong, srcPANid):
				
		self.startSPI ()
		
		self.channel = channel
		self.TX_busy = False
		self.TX_awatingAck = False
		self.TX_lastPackFail = False
		self.TX_busyChannel = False
		self.RX_bufferRear = 0
		self.RX_bufferFront = 0
		self.RX_empty = True # se rear == front, vai estar em false pra empty e true pra full
		
	
		# recomenda-se esperar 2ms apos o power on, eu espero 4ms		
		time.sleep (0.004)
		
		self.RstSoft ()
		self.setRegister ("RXFLUSH", 0x01)
		self.setRegister ("PACON2", 0x98)
		self.setRegister ("TXSTBL", 0x95)
		self.setRegister ("RFCON0", 0x03)
		self.setRegister ("RFCON1", 0x02)
		self.setRegister ("RFCON2", 0x80)
		self.setRegister ("RFCON6", 0x90)
		self.setRegister ("RFCON7", 0x80)
		self.setRegister ("RFCON8", 0x10)
		self.setRegister ("SLPCON1", 0x21)
		self.setRegister ("BBREG2", 0x80)
		self.setRegister ("CCAEDTH", 0x60)
		self.setRegister ("BBREG6", 0x40)
		self.setRegister ("INTCON", 0b11110110) # ativa int de rx e tx
		self.selChannel (channel) # Seleciona canal 11
		self.RstRF ()
		self.setRegister ("RXMCR", 0x00) 
		self.setRegister ("RFCON3", 0x00) # Seleciona potencia maxima
		self.getRegister ("INTSTAT") # Limpa int pendentes
		self.setRegister ("BBREG1", 0x00)
		self.configRadioAddr (srcAddrShort, srcAddrLong, srcPANid) 
		self.RstRF ()

		self.TXbuff = TX ()	
		self.RXbuff = [RX () for i in range (8)] #buffer size = 8

#		GPIO.setmode (GPIO.BOARD)
#		GPIO.setup (INTPIN, GPIO.IN)
#		GPIO.add_event_detect(INTPIN, GPIO.RISING, callback=self.intHandle)  

	def intHandle (self, channel)
		if channel != INTPIN:
			return
		

# ADAPTAR A FUNCAO PARA OLHAR O PINO DO INT		
#	def waitOrReset (self):
#		i = 50
#		while i != 0 and self.TX_buff == True:
#			time.sleep (0.001)
#			i -= 1
#		if self.TX_buff == True:
			#reset radio		

	def send (self, payload, PANID, ADDR, frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode):

		# verificacao de erros
		if len(ADDR) != 4 and len(ADDR) != 16:
			return ADDR_ERROR

		if (len(ADDR) == 4 and dstAddrMode != SRC_SHORT_ADDR):
			return ADDR_ERROR			
	
		if (len(ADDR) == 16 and dstAddrMode != SRC_LONG_ADDR):
			return ADDR_ERROR


		if self.TX_busy == True:
			self.waitOrReset ()
		
		if dstAddrMode == SRC_SHORT_ADDR:
			self.TXbuff.addrConfig (PANID, SHORT = ADDR)
		else
			self.TXbuff.addrConfig (PANID, LONG = ADDR)

		self.TXbuff.config (frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode)

		self.TXbuff.payloadToBuffer (payload)

		self._transmit ()
		

	#olha aqui tbm, tem q muduar os tipo pra byte
	def configRadioAddr (self, SHORT, LONG, PANID): #addr must be string

		self.PANid = bytearray.fromhex (PANID)
		self.setRegister ("PANIDL", self.PANid[0])
		self.setRegister ("PANIDH", self.PANid[1])

		self.AddrShort = bytearray.fromhex (SHORT)
		self.setRegister ("SADRL", self.AddrShort[0])
		self.setRegister ("SADRH", self.AddrShort[1])

		self.AddrLong = bytearray.fromhex (LONG)
		for i in range(0, 8):
			self.setRegister (0x05 + i, self.AddrLong[i]) #0x05 == EADR0

	def RstRF (self):
		self.setRegister ("RFCTL", 0x04)
		self.setRegister ("RFCTL", 0x00)
		time.sleep (0.0003)

	def RstSoft (self):
		self.setRegister ("SOFTRST", 0x07) # SOFTWARE RESET
		while ((self.getRegister ("SOFTRST") & 0x07) != 0x00):
			pass
		
	def selChannel (self, channel):
		self.setRegister ("RFCON0", ((channel - 11) << 4) | 0x03)
		self.RstRF ()
	
	def ContiguousWrite (self, addr, mem, count):
		for i in range (0, count):
			self.setRegister (addr, mem[i])
			addr += 1
		return addr

