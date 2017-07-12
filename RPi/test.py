import RPi.GPIO as GPIO
import sys
from defines import *

from app import *
from zig import *


srcPAN = "ef4d"
srcLONG = "3ed18a092adc68ff"
srcSHORT = "0c4f"

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"

s = bytearray.fromhex("f1a56791")

boardConfig ()

def foi():
	print "foi"

Radio = Rd (17, srcSHORT, srcLONG, srcPAN) #canal 17


  
	
Radio.send (s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED, SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

time.sleep (4)

RT = Radio.getRdStatus ()
print RT.TX_lastPackFail
print RT.TX_awatingAck

a = Radio.getLastPckt()
print a.payload[0]
print a.payload[1]
print a.payload[2]
print a.payload[3]

while True:
	pass

#while True:
#	if GPIO.input (15):
#		print "a"
#	else: 
#		print "b"
#		print Radio.getRegister ("INTSTAT")
#		print Radio.getRegister ("TXSTAT")
#	time.sleep (0.5)
GPIO.cleanup() 


