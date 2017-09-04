import RPi.GPIO as GPIO
import sys
from defines import *
from app import *

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"

def intFunct (packt):
	print "oi"
	if packt.srcAddr != bytearray.fromhex(dstLONG) and packt.srcAddr != bytearray.fromhex(dstSHORT):
		print "i"
		return
	if len(packt.payload) == 1:
		print "o"
		return
	num_mesg = len(packt.payload)/2
	for i in range(num_mesg):
		print packt.payload[i]
		if (packt.payload[i] & 0b110000) == 0:
			temp = ((packt.payload[i] & 0b11) << 8) | packt.payload[i + 1]
		elif (packt.payload[i] & 0b110000) == 0b010000:
			temp = "lata ausente"
		else:
			temp = "medicao invalida"
		id = (packt.payload[i] & 0b11000000) >> 6
		print "id: {0}, {1}".format(str(id), str(temp))
	
def boardConfig ():
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup (11, GPIO.OUT)
	GPIO.setup (13, GPIO.OUT)
	GPIO.setup (15, GPIO.IN)
	GPIO.output (11, 0)
	GPIO.output (13, 1)

def boardFinish ():
	GPIO.cleanup()

