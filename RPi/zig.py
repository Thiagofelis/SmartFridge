import RPi.GPIO as GPIO
import time
import spidev
import sys
from defines import *
from app import *

# os objetos RdStatus e Packt teram instancias entregues ao usuario para informa-lo sobre o status do
# radio e para entrega-lo o pacote. As instancias serao uma especie de "read-only", so servem para
# passar a informacao ao usuario e sao incapazes de fazer qualquer acao sobre o Rd
class RdStatus (object):
	def __init__ (self, channel, TX_busy, TX_awatingAck, TX_lastPackFail, TX_busyChannel,
		      RX_buffEmpty, RX_buffFull, RX_buffOverflow):
		self.channel = channel
		self.TX_busy = TX_busy
		self.TX_awatingAck = TX_awatingAck
		self.TX_lastPackFail = TX_lastPackFail
		self.TX_busyChannel = TX_busyChannel
		self.RX_buffEmpty = RX_buffEmpty
		self.RX_buffFull = RX_buffFull
		self.RX_buffOverflow = RX_buffOverflow
		
class Packt (object):
	def __init__ (self, payload, frameControl, seqNum, srcPANid, srcAddr, fcs, lqi, rssi):
		self.payload = payload
		self.seqNum = seqNum
		self.srcPANid = srcPANid
		self.srcAddr = srcAddr
		self.fcs = fcs
		self.lqi = lqi
		self.rssi = rssi
		self.packetType = frameControl[0] & PACKET_TYPE
		self.security = (frameControl[0] & SECURITY_FIELD) == SECURITY_ENABLED
		self.ack = (frameControl[0] & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED
		self.panidComp = (frameControl[0] & PAN_ID_COMP_FIELD) == PAN_ID_COMP_ENABLED
		self.seqNumComp = (frameControl[1] & SEQUENCE_NUM_SUP_FIELD) == SEQUENCE_NUM_SUP_ENABLED 
		      
# os objetos das classes TX e RX nao sao vistos nem manipulados pelo usuario,
# eles existem para tornar a informacao manipulada pelo Rd mais organizada		      
class TX (object): 
	def __init__ (self):
		self.seqNum = 0x00
	
	def addrConfig (self, PANID, SHORT="00", LONG="00"): # addr as strings, PAN=2bytes, SHORT=2bytes, LONG=8bytes
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
	
class RX (object):
	def __init__ (self):
		self.frameControl = bytearray (2)
		self.seqNum = 0
		self.dstPANid = bytearray (2)
		self.dstAddr = bytearray (1)
		self.srcPANid = bytearray (2)
		self.srcAddr = bytearray (1)
		self.payload = bytearray (1)
		self.fcs = bytearray (2)		
		self.lqi = bytearray (1)
		self.rssi = bytearray (1)
	def getPack (self):
		packet = Packt (self.payload, self.frameControl, self.seqNum, self.srcPANid, self.srcAddr, self.fcs, self.lqi, self.rssi)
		return packet
		
class Rd (object):
	def _receive (self):
		if self.RX_buffFull == True: #buffer cheio, sobrescreve o pacote mais velho
			self.RX_buffOverflow = True		
			self.RX_bufferFront = self.RX_bufferFront + 1 if (self.RX_bufferFront + 1 != self.RX_buffSize) else 0
		
		self.RX_buffEmpty = False
		
		#lembrando q nesse caso vc o dst e quem mandou a src (obvio, mas confunde as vezes)
		currAddr = 0x301
		
		currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].frameControl, 2)
	
		if (self.RXbuff[self.RX_bufferRear].frameControl[1] & SEQUENCE_NUM_SUP_FIELD) == SEQUENCE_NUM_SUP_DISABLED:
			self.RXbuff[self.RX_bufferRear].seqNum = self.getRegister (currAddr)
			currAddr += 1
		
		#acks n tem panid
		#n entendo mt bem essa parte, o zigbee em si ja faz o tratamento de ack, entao
		#acho q eu nunca chegaria a ler um frame de ack
		if (self.RXbuff[self.RX_bufferRear].frameControl[0] & PACKET_TYPE) != PACKET_TYPE_ACK:
			currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].dstPANid, 2)
		
		if (self.RXbuff[self.RX_bufferRear].frameControl[1] & DST_ADDR_MODE) == DST_SHORT_ADDR:
			self.RXbuff[self.RX_bufferRear].dstAddr = bytearray (2)
			currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].dstAddr, 2)
		
		elif (self.RXbuff[self.RX_bufferRear].frameControl[1] & DST_ADDR_MODE) == DST_LONG_ADDR:
			self.RXbuff[self.RX_bufferRear].dstAddr = bytearray (8)
			currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].dstAddr, 8)
			
		if (((self.RXbuff[self.RX_bufferRear].frameControl[1] & DST_ADDR_MODE) != DST_NO_ADDR)
		  and ((self.RXbuff[self.RX_bufferRear].frameControl[1] & SRC_ADDR_MODE) != SRC_NO_ADDR)  
		  and ((self.RXbuff[self.RX_bufferRear].frameControl[0] & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)): 
			currAddr = self.ContiguousWrite (currAddr, self.RXbuff[self.RX_bufferRear].srcPANid, 2)
			
		if (self.RXbuff[self.RX_bufferRear].frameControl[1] & SRC_ADDR_MODE) == SRC_SHORT_ADDR:
			self.RXbuff[self.RX_bufferRear].srcAddr = bytearray (2)
			currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].srcAddr, 2)
		
		elif (self.RXbuff[self.RX_bufferRear].frameControl[1] & SRC_ADDR_MODE) == SRC_LONG_ADDR:
			self.RXbuff[self.RX_bufferRear].srcAddr = bytearray (8)
			currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].srcAddr, 8)
		
		payloadSize = self.getRegister (0x300) - (currAddr - 0x301) - 2
		self.RXbuff[self.RX_bufferRear].payload = bytearray (payloadSize)
		currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].payload, payloadSize)
		
		currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].fcs, 2)
		
		currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].lqi, 1)
			
		currAddr = self.ContiguousRead (currAddr, self.RXbuff[self.RX_bufferRear].rssi, 1)
		
		self.RX_bufferRear = self.RX_bufferRear + 1 if (self.RX_bufferRear + 1 != self.RX_buffSize) else 0
		if self.RX_bufferRear == self.RX_bufferFront: 
			self.RX_buffFull = True
			
	def _transmit (self):

		currAddr = 0x02 # jump Header and Frame lenght, fill later

		# Write control frame
		currAddr = self.ContiguousWrite (currAddr, self.TXbuff.frameControl, 2)
		
		self.setRegister (currAddr, self.TXbuff.seqNum)
#		currAddr = self.ContiguousWrite (currAddr, self.TXbuff.seqNum, 1)
		currAddr += 1

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
		  and ((self.TXbuff.frameControl[1] & SRC_ADDR_MODE) != SRC_NO_ADDR)  
		  and ((self.TXbuff.frameControl[0] & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)): 
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
		self.TX_lastTXtime = time.time()

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

	def __init__ (self, channel, srcAddrShort, srcAddrLong, srcPANid, intfunc = None): #funcao para o usuario (na inicializacao)
		
		
		self.startSPI ()
		
		self.channel = channel
		self.TX_busy = False
		self.TX_awatingAck = False
		self.TX_lastPackFail = False
		self.TX_busyChannel = False
		self.RX_bufferRear = 0
		self.RX_bufferFront = 0
		self.RX_buffEmpty = True
		self.RX_buffFull = False
		self.RX_buffOverflow = False
		self.RX_buffSize = 8
		self.TX_lastTXtime = 0
		self.intFunc = intfunc
	
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

		GPIO.setmode (GPIO.BOARD)
		GPIO.setup (INTPIN, GPIO.IN)
		global ignoreCalls
		ignoreCalls = False
		GPIO.add_event_detect(INTPIN, GPIO.FALLING, callback=self.intHandle)  
		if GPIO.input(INTPIN) == 0: #int ja aconteceu
			self.intHandle(INTPIN)		

	def reset (self):
		GPIO.remove_event_detect(INTPIN)
		self.TX_busy = False
		self.TX_awatingAck = False
		self.TX_lastPackFail = True
		self.TX_busyChannel = False
		
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
		self.selChannel (self.channel) # Seleciona canal 11
		self.RstRF ()
		self.setRegister ("RXMCR", 0x00) 
		self.setRegister ("RFCON3", 0x00) # Seleciona potencia maxima
		self.getRegister ("INTSTAT") # Limpa int pendentes
		self.setRegister ("BBREG1", 0x00)
		self.RstRF ()
		
		GPIO.add_event_detect(INTPIN, GPIO.FALLING, callback=self.intHandle)  
		
	def intHandle (self, channel):
		global ignoreCalls
		if ignoreCalls == True:
			return		
		if channel != INTPIN:
			return
		ignoreCalls = True
		#GPIO.remove_event_detect(INTPIN)
		info = self.getRegister ("INTSTAT")
		
		if info & 0b1 == 0b1: #int TX
			self.TX_busy = False
			if self.TX_awatingAck == True:
				self.TX_awatingAck = False
				txInfo = self.getRegister ("TXSTAT")
				if txInfo & 0b1 == 0b1: #fail
					self.TX_lastPackFail = True
					if txInfo & 0b100000 == 0b100000:
						self.TX_busyChannel = True
					else:
						self.TX_busyChannel = False
				else:
					self.TX_lastPackFail = False
             
	                if GPIO.input(INTPIN) == 0: #int aconteceu durante a funcao
        	                self.intHandle(INTPIN)

			#GPIO.add_event_detect(INTPIN, GPIO.FALLING, callback=self.intHandle)
			ignoreCalls = False
		
		if info & 0b1000 == 0b1000: # int RX
			self.setRegister ("BBREG1", 0b100)			
			self._receive ()
			self.setRegister ("RXFLUSH", 0x01)  
			if self.intFunc != None: 
				packt = self.getLastPckt()
				self.intFunc (packt)		
			#GPIO.add_event_detect(INTPIN, GPIO.FALLING, callback=self.intHandle)
			self.setRegister ("BBREG1", 0x00)
			ignoreCalls = False
	
		return
		
	def waitOrReset (self):
		currTime = time.time()
		while (self.TX_busy == True):
			if (abs (currTime - self.TX_lastTXtime) < 0.100): #100ms => TIMEOUT_TIME
				currTime = time.time()
			else:
				self.reset ()
				return
	
	def getLastPckt (self): #funcao para o usuario
		if self.RX_buffEmpty == True:
			return
		
		self.RX_buffFull = False
		temp = self.RXbuff[self.RX_bufferFront].getPack ()
		self.RX_bufferFront = self.RX_bufferFront + 1 if (self.RX_bufferFront + 1 != self.RX_buffSize) else 0		
	
		if self.RX_bufferRear == self.RX_bufferFront:
			self.RX_buffEmpty = True
		
		return temp

#	essa funcao provavelmente ta obsoleta, agr q fiz getRdStatus toda a informacao sobre o Rd pode ser obtida de uma vez	
	def RXoverflow (self): # funcao para o usuario
		temp = self.RX_buffOverflow
		self.RX_buffOverflow = False
		return temp
	
	def getRdStatus (self): # funcao para o usuario
		temp = RdStatus (self.channel, self.TX_busy, self.TX_awatingAck, self.TX_lastPackFail, 
		      self.TX_busyChannel, self.RX_buffEmpty, self.RX_buffFull, self.RX_buffOverflow)
		return temp
	
	def send (self, payload, PANID, ADDR, frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode): #funcao para o usuario

		# verificacao de erros (so verifica erro de endereco. eu talvez tire a verificacao pq n da pra fazer verificacao para
		# td tipo de erro, daria trabalho e o codigo ia ficar mt grande)
		if len(ADDR) != 4 and len(ADDR) != 16:
			return ADDR_ERROR

		if (len(ADDR) == 4 and dstAddrMode != DST_SHORT_ADDR):
			return ADDR_ERROR			
	
		if (len(ADDR) == 16 and dstAddrMode != DST_LONG_ADDR):
			return ADDR_ERROR

		if self.TX_busy == True:
			self.waitOrReset ()
		
		if dstAddrMode == DST_SHORT_ADDR:
			self.TXbuff.addrConfig (PANID, SHORT = ADDR)
		else:
			self.TXbuff.addrConfig (PANID, LONG = ADDR)

		self.TXbuff.config (frameType, ackRequired, PANcomp, noSequenceNum, dstAddrMode, srcAddrMode)

		self.TXbuff.payloadToBuffer (payload)

		self._transmit ()
		return SUCCESS
		
	def configRadioAddr (self, SHORT, LONG, PANID):  #funcao para o usuario
		#addr must be string 
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
		
	def selChannel (self, channel): #funcao para o usuario
		self.setRegister ("RFCON0", ((channel - 11) << 4) | 0x03)
		self.RstRF ()
	
	def ContiguousWrite (self, addr, mem, count):
		for i in range (0, count):
			self.setRegister (addr, mem[i])
			addr += 1
		return addr
	
	def ContiguousRead (self, addr, mem, count):
		for i in range (0, count):
			mem[i] = self.getRegister (addr)
			addr += 1
		return addr	

