import RPi.GPIO as GPIO
from zig import *
from defines import *
from 

srcPAN = "ef4d"
srcLONG = "3ed18a092adc68ff"
srcSHORT = "0c4f"

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"

s = "abcd efgh 12345678"



Radio = Rd (11, srcSHORT, srcLONG, srcPAN)
	
Radio.send (s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_ENABLED, 
				   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

print TX_buff.transmit()
