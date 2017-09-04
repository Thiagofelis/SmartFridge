import var
from temp.externo.zig import *
global Radio
global num
global byte_unit
from channels import Group

def foi(packt):
    print "foi"
    if len(packt.payload) == 4 and packt.srcAddr == bytearray.fromhex(dstSHORT):
        var.pacote = packt
        var.varia = True
    if packt.payload[0] & 0b111:
        Group("chat").send({
        "text": "ola",
        }, immediately=True)

def init():
    boardConfig()
    global Radio
    Radio = Rd(17, srcSHORT, srcLONG, srcPAN, foi) #canal 17
    global num
    num = bytearray(1)
    num[0] = 0
    global byte_unit
    byte_unit = bytearray(1)
    byte_unit[0] = 1