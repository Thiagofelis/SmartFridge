import RPi.GPIO as GPIO
import time
import spidev
from mrf24j40_registers import register_fields, short_addr_registers, long_addr_registers

#lembre desses trem aqui
#GPIO.setmode(GPIO.BOARD)
#GPIO.setup (11, GPIO.OUT)
#GPIO.setup (13, GPIO.OUT)
#GPIO.setup (15, GPIO.IN)
#GPIO.output (11, 0)
#GPIO.output (13, 1)


	

class TX (object):
	def __init__ (self):
		self.seqNum = 0x80
	
	def AddrConfig (self, PANID, SHORT=0, LONG=0): # addr as strings, PAN=2bytes, SHORT=2bytes, LONG=8bytes
		# ex. PAN = "af23"
		self.dstPANID = bytearray.fromhex (PANID)
		self.dstSHORT = bytearray.fromhex (SHORT)
		self.dstLONG = bytearray.fromhex (LONG)
		
	def config (self, frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode):
		self.frameControl = bytearray (2)
		self.frameControl[0] = frameType | ackRequired | PANcomp
		self.frameControl[1] = noSequenceNum | dstAddrMode | srcAddrMode

	def payloadToBuffer (self, payload)
		self.payload = bytearray (payload, 'utf-8') #size = len(self.payload), payload is a string
		
	def transmit (self):
		
		if (Radio.TX_busy == 1):
			return FAIL

		currAddr = 0x02 # jump Header and Frame lenght, fill later

		# Write control frame
		currAddr = zig_ContiguousWrite (currAddr, self.frameControl, 2)

		currAddr = zig_ContiguousWrite (currAddr, self.seqNum, 1)

		if (self.seqNum == 0xff): # seqNum always >=0x80, loops from 0xff to 0x80
			self.seqNum = 0x00
		else:
			self.seqNum += 1
		
		if ((self.frameControl[1] & DST_ADDR_MODE) == DST_SHORT_ADDR):
			currAddr = Radio.ContiguousWrite (currAddr, self.dstPANID, 2)
			currAddr = Radio.ContiguousWrite (currAddr, self.dstSHORT, 2)
	
		else if ((self.frameControl[1] & DST_ADDR_MODE) == DST_LONG_ADDR): 
			currAddr = Radio.ContiguousWrite (currAddr, self.dstPANID, 2)
			currAddr = Radio.ContiguousWrite (currAddr, self.dstLONG, 8)

		if ( ((self.frameControl[1] & DST_ADDR_MODE) != DST_NO_ADDR)
		  && ((self.frameControl[1] & SRC_ADDR_MODE) != SRC_NO_ADDR)  
		  && ((self.frameControl[0] & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)): 
			currAddr = Radio.ContiguousWrite (currAddr, Radio.PANid, 2)

		if ((self.frameControl[1] & SRC_ADDR_MODE) == SRC_SHORT_ADDR):
			currAddr = Radio.ContiguousWrite (currAddr, Radio.addrShort, 2)
		
		else if ((self.frameControl[1] & SRC_ADDR_MODE) == SRC_LONG_ADDR): 
			currAddr = Radio.ContiguousWrite (currAddr, Radio.addrLong, 8)

		Radio.setRegister (0x00, currAddr - 2) # minus the lenght of the first 2 bytes
		currAddr = Radio.ContiguousWrite (currAddr, Tx.payload, Tx.payloadSize);
		Radio.setRegister (0x01, currAddr - 2)

		Radio.setRegister ("TXNCON", 0b101 if ((self.frameControl[0] & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED) else 0b1) # start transmission

		if ((self.frameControl[0] & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED):
			Radio.TX_awatingAck = 1
		
		Radio.TX_busy = 1

		return SUCCESS;

	
class RX (object):
		

class Rd (object):
	
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

	def readRegister(self, reg):
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

	def __init__ (self, channel, srcAddrLong, srcAddrShort, srcPANid):
				
		self.startSPI ()
		
		self.channel = channel
		self.seqNum = 0x80;
		self.TX_busy = 0;
		self.TX_awatingAck = 0;
		self.TX_lastPackFail = 0;
		self.TX_busyChannel = 0;
		
		#talvez inicializar TX e RX aqui
		
		self.RstSoft ()
		self.SetShort ("RXFLUSH", 0x01)
		self.SetShort ("PACON2", 0x98)
		self.SetShort ("TXSTBL", 0x95)
		self.SetLong ("RFCTRL0", 0x03)
		self.SetLong ("RFCTRL1", 0x02)
		self.SetLong ("RFCTRL2", 0x80)
		self.SetLong ("RFCTRL6", 0x90)
		self.SetLong ("RFCTRL7", 0x80)
		self.SetLong ("RFCTRL8", 0x10)
		self.SetLong ("SCLKDIV", 0x21)
		self.SetShort ("BBREG2", 0x80)
		self.SetShort ("CCAEDTH", 0x60)
		self.SetShort ("BBREG6", 0x40)
		self.SetShort ("INTCON", 0b11110110) # ativa int de rx e tx
		self.SelChannel (channel) # Seleciona canal 11
		self.RstRF ()
		self.SetShort ("RXMCR", 0x00) 
		self.SetLong ("RFCTRL3", 0x00) # Seleciona potencia maxima
		self.GetShort ("INTSTAT") # Limpa int pendentes
		self.SetShort ("BBREG1", 0x00)
		self.configRadioAddr (srcAddrLong, srcAddrShort, srcPANid) 
		self.RstRF ()
		
		
	#olha aqui tbm, tem q muduar os tipo pra byte
	def configRadioAddr (self, PANID, SHORT, LONG): #addr must be string

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
		time.delay (0.0003) #300 μs, recomendado é 192 μs

	def RstSoft (self):
		self.setRegister ("SOFTRST", 0x07) # SOFTWARE RESET
		while ((self.readRegister ("SOFTRST") & 0x07) != 0x00):
			pass
		time.delay () #decidir dpois
		
	def SelChannel (self, channel):
		self.setRegister ("RFCTRL0", ((channel - 11) << 4) | 0x03)
		self.RstRF ()
	
	# arrumar as contiguous
	def ContiguousWrite (self, addr, mem, count):
		for i in range (0, count):
			self.SetLong (addr, mem[i])
			addr += 1
		return addr
	
	def ContiguousRead (self, addr, mem, count):
		mem = bytearray (count)
		for i in range (0, count):
			mem[i] = self.GetLong (addr)
			addr += 1
		return addr