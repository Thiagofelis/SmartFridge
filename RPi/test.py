import RPi.GPIO as GPIO
import sys
from defines import *

from app import *
from zig import *


def hello(channel):
	a = Radio.getRegister ("INTSTAT")
	print a
	print Radio.getRegister ("TXSTAT")
	if a & 8:
		print "RX"
		print Radio.getRegister (0x300)
                print Radio.getRegister (0x301)
                print Radio.getRegister (0x302)
                print Radio.getRegister (0x303)
                print Radio.getRegister (0x304)
                print Radio.getRegister (0x305)
                print Radio.getRegister (0x306)
                print Radio.getRegister (0x307)
                print Radio.getRegister (0x308)
                print Radio.getRegister (0x309)
                print Radio.getRegister (0x30a)
                print Radio.getRegister (0x30b)
                print Radio.getRegister (0x30c)
                print Radio.getRegister (0x30d)
                print Radio.getRegister (0x30e)
                print Radio.getRegister (0x30f)
                print Radio.getRegister (0x310)
		print Radio.getRegister (0x313)
srcPAN = "ef4d"
srcLONG = "3ed18a092adc68ff"
srcSHORT = "0c4f"

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"

s = bytearray.fromhex("00000f111111")

boardConfig ()

Radio = Rd (17, srcSHORT, srcLONG, srcPAN) #canal 17


GPIO.add_event_detect(15, GPIO.FALLING, callback=hello)  
	
Radio.send (s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_DISABLED, PAN_ID_COMP_DISABLED, 
				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
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


