import numpy as np


class Lt(object):
    def __init__(self, id):
        self.id = id

    @staticmethod
    def getTemp(val):
        val = (float(33 * val) / (1023 - val))
        xp = [5.24, 8.01, 10.0, 12.56, 20.24, 33.63, 57.67, 102.3, 188.2, 360.9]
        fp = [400, 300, 250, 200, 100, 0, -100, -200, -300, -400]
        return np.interp(val, xp, fp)

    @staticmethod
    def bitmap(a, b):
        a ^= a >> 8
        a ^= a >> 4
        a ^= a >> 2
        a ^= a >> 1
        a &= 1

        b ^= b >> 8
        b ^= b >> 4
        b ^= b >> 2
        b ^= b >> 1
        b &= 1

        return (a << 2) | (b << 3)
