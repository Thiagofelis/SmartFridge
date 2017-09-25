from temp.externo.zig import *
from utility import em_pacote_recebido, board_config
global Radio


def init():
    board_config()
    global Radio
    Radio = Rd(17, srcSHORT, srcLONG, srcPAN, em_pacote_recebido) #canal 17
