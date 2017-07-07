import RPi.GPIO as GPIO
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

boardConfig()

Radio = Rd(17, srcSHORT, srcLONG, srcPAN, intFunc)

Lata = [Lt(i) for i in range(3)]

while True:
    pass
