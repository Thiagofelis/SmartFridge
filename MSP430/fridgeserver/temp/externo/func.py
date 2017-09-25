from zig import *
from app import *
from defines import *
import ctypes

def getTemp(var):
    if var == 0b1111111111:
        return "ausente"
    print var
    return str(float(var - 400) / 10)

def putTemp (var):
    return ctypes.c_uint16(int(float(var) * 10 + 400)).value

def tempValida (var):
    var = float(var)
    return -40 < var < 40
