import RPi.GPIO as GPIO
import time
import sys
from defines import *
from app import *
from zig import *
from lata import *

srcPAN = "ef4d"
srcLONG = "3ed18a092adc68ff"
srcSHORT = "0c4f"

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"

ignoreCalls = False
boardConfig()
#print GPIO.input(INTPIN)

Radio = Rd(17, srcSHORT, srcLONG, srcPAN, intFunct)

Lata = [Lt(i) for i in range(3)]

#time.sleep(10)

#print Radio.getRegister ("INTSTAT")
#print GPIO.input(INTPIN)

#st = Radio.getLastPckt()
#print st
while True:
    pass

