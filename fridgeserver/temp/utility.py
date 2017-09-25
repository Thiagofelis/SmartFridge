import ctypes
from temp.externo.zig import *
global Radio
global num
global byte_unit
from channels import Group
import json
import global_var

def em_pacote_recebido(packt):
    if packt.srcAddr == bytearray.fromhex(dstSHORT):
        if len(packt.payload) == 6:
            global_var.pacote_index = packt
            global_var.nova_mensagem_index = True
        if len(packt.payload) == 1:
            global_var.pacote_resposta = packt
            global_var.nova_mensagem_resposta = True

    if packt.payload[0] & 0b111:
        # espera estabelecer conexao com websocket
        time.sleep(1.5)

        lata_gelata = {'lta': 0, 'ltb': 0, 'ltc': 0}
        if packt.payload[0] & 0b001:
            lata_gelata['lta'] = 1
        if packt.payload[0] & 0b010:
            lata_gelata['ltb'] = 1
        if packt.payload[0] & 0b100:
            lata_gelata['ltc'] = 1

        Group("chat").send({
            "text": json.dumps(lata_gelata),
        })


def getTemp(var):
    if var == 0b1111111111:
        return "ausente"
    print var
    return str(float(var - 400) / 10)


def putTemp(var):
    return ctypes.c_uint16(int(float(var) * 10 + 400)).value


def tempValida(var):
    var = float(var)
    return -40 < var < 40


def pegarInstrucao(lata_a, lata_b, lata_c, temp_codificada):
    # caso com as tres latas
    if lata_a is not None and lata_b is not None and lata_c is not None:
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000) | (temp_codificada >> 8)
        p[2] = temp_codificada & 0b11111111
        p[3] = temp_codificada >> 4
        p[4] = ((temp_codificada << 4) & 0b11110000)
        comando = bytearray.fromhex("0007")
        return comando + p
    # caso com duas latas
    if lata_a is not None and lata_b is not None:  # a e b
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000) | (temp_codificada >> 8)
        p[2] = temp_codificada & 0b11111111
        comando = bytearray.fromhex("0003")
        return comando + p
    if lata_a is not None and lata_c is not None:  # a e c
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000) | (temp_codificada >> 8)
        p[2] = temp_codificada & 0b11111111
        comando = bytearray.fromhex("0005")
        return comando + p
    if lata_c is not None and lata_b is not None:  # c e b
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000) | (temp_codificada >> 8)
        p[2] = temp_codificada & 0b11111111
        comando = bytearray.fromhex("0006")
        return comando + p
    # casos com uma lata
    if lata_a is not None:  # a
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000)
        comando = bytearray.fromhex("0001")
        return comando + p
    if lata_b is not None:  # b
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000)
        comando = bytearray.fromhex("0002")
        return comando + p
    if lata_c is not None:  # c
        p = bytearray(5)
        p[0] = temp_codificada >> 4
        p[1] = ((temp_codificada << 4) & 0b11110000)
        comando = bytearray.fromhex("0004")
        return comando + p


def board_config():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup (11, GPIO.OUT)
    GPIO.setup (13, GPIO.OUT)
    GPIO.setup (15, GPIO.IN)
    GPIO.output (11, 0)
    GPIO.output (13, 1)


def board_clean():
    GPIO.cleanup()
