from zig import *
import ctypes

srcPAN = "ef4d"
srcLONG = "3ed18a092adc68ff"
srcSHORT = "0c4f"

dstPAN = "f25a"
dstLONG = "112904397c2214ae"
dstSHORT = "1abc"
boardConfig ()


def putTemp (var):
    return ctypes.c_uint16(int(float(var) * 10 + 400)).value

def getTemp (var):
    return float(var - 400) / 10

def intFunction(pack):
    if pack.payload[0] & 0b1:
        print "lata 0 atingiu temp"
    if pack.payload[0] & 0b10:
        print "lata 1 atingiu temp"
    if pack.payload[0] & 0b100:
        print "lata 2 atingiu temp"
    if pack.payload[0] & 0b1000:
	if (pack.payload[1] << 4 | pack.payload[2] >> 4) == 0b111111111111:
	    print "a lata esta ausente ou a geladeira ta com problema"
	else: 
            print "a temp da lata 0 e " + str(getTemp(pack.payload[1] << 4 | pack.payload[2] >> 4))
    if pack.payload[0] & 0b10000:
        if (pack.payload[1] << 4 | pack.payload[2] >> 4) == 0b111111111111:
            print "a lata esta ausente ou a geladeira ta com problema"
        else:
            print "a temp da lata 1 e " + str(getTemp(pack.payload[1] << 4 | pack.payload[2] >> 4))
    if pack.payload[0] & 0b100000:
        if (pack.payload[1] << 4 | pack.payload[2] >> 4) == 0b111111111111:
            print "a lata esta ausente ou a geladeira ta com problema"
        else:
            print "a temp da lata 2 e " + str(getTemp(pack.payload[1] << 4 | pack.payload[2] >> 4))
    if pack.payload[0] & 0b1000000:
        print "temp setada"
    if pack.payload[0] & 0b10000000:
        print "pong"
Radio = Rd(17, srcSHORT, srcLONG, srcPAN, intFunction) #canal 17


while True:
    comando = raw_input()
    comando = comando.split(" ")
    if comando[0] == "gettemp":
        if comando[1] == "0":
            s = bytearray.fromhex("0100")
        if comando[1] == "1":
            s = bytearray.fromhex("0200")
        if comando[1] == "2":
            s = bytearray.fromhex("0400")
        Radio.send(s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED,
                   PAN_ID_COMP_DISABLED, SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
        continue
    if comando[0] == "ping":
        s = bytearray.fromhex("4000")
        Radio.send(s, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED,
                   PAN_ID_COMP_DISABLED, SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
        continue
    if comando[0] == "settemp":
        if comando[1] == "0":
            s = bytearray.fromhex("0001")
        if comando[1] == "1":
            s = bytearray.fromhex("0002")
        if comando[1] == "2":
            s = bytearray.fromhex("0004")
        k = putTemp(comando[2])
        p = bytearray(2)
        p[0] = k >> 4
        p[1] = (k << 4) & 0b11110000

        Radio.send(s + p, dstPAN, dstSHORT, PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED,
                   PAN_ID_COMP_DISABLED, SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
        continue
