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

s = bytearray.fromhex("00000f111111")

boardConfig ()

Radio = Rd (17, srcSHORT, srcLONG, srcPAN) #canal 17


  
	
#Radio.send (s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_DISABLED, PAN_ID_COMP_DISABLED, 
#				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

time.sleep (3)
a = Radio.getLastPckt()
print a
print a[0][0]
print a[0][1]
print a[0][2]
print a[0][3]
print a[1][0]
print a[1][1]

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


