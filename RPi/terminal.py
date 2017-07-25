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

Radio = Rd(17, srcSHORT, srcLONG, srcPAN) #canal 17


def waitFor(header):
    ctime = time.time()
    header = bytearray.fromhex(header)
    while True:
        if abs(time.time() - ctime) >= 0.6:
            return None
        else:
            pkt = Rd.getLastPckt()
            if pkt != None and pkt.payload[0] == header[0]:
                return pkt

s = bytearray.fromhex(0040)
Radio.send(s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED,
            PAN_ID_COMP_DISABLED, SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

ctime = time.time()

while True:
    if abs(time.time() - ctime) >= 0.6:
        print "Conexao falhou"
        break
    else:
        pck = Rd.getLastPckt()
        if pck != None and pck.payload == bytearray.fromhex("80"):
            print "Geladeira conectada com sucesso"

while True:
    comando = raw_input()
    comando = comando.split(" ")
    if comando[0] == "gettemp":
        if len(comando) != 2:
            print "ERRO: comando gettemp toma somente um argumento, que e o id da lata"
        else:
            s = bytearray.fromhex(0001)