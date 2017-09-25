# -*- coding: utf-8 -*-

from __future__ import unicode_literals
from django.http import HttpResponse
import global_var
from django.template import loader
from temp.externo.defines import *
from utility import getTemp, tempValida, putTemp, pegarInstrucao
import time
from iniciar import Radio

def index(request):
    template = loader.get_template('temp/index.html')

    #limpa o buffer
    while Radio.getLastPckt () != None:
        pass

    global_var.nova_mensagem_index = False
    Radio.send(bytearray.fromhex("0700"), dstPAN, dstSHORT,
               PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED,
               SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
    currTime = time.time()
    context = {}

    while True:
        if global_var.nova_mensagem_index is True:
            packt = global_var.pacote_index
            context['a'] = getTemp((packt.payload[1] << 4) | (packt.payload[2] >> 4))
            context['b'] = getTemp(((packt.payload[2] & 0b1111) << 8) | (packt.payload[3] & 0b11111111))
            context['c'] = getTemp((packt.payload[4] << 4) | (packt.payload[5] >> 4))
            break
        if abs(currTime - time.time()) > global_var.TIMEOUT_ESPERA:
            # deu ruim
            context['timeout'] = "A geladeira nao respondeu a tempo"
            break

    return HttpResponse(template.render(context, request))

def resposta_configuracao(request):
    template = loader.get_template('temp/resposta_configuracao.html')

    context = {}
    lata_a = request.POST.get('lataa')
    lata_b = request.POST.get('latab')
    lata_c = request.POST.get('latac')
    temp_desejada = request.POST.get('temperatura')

    if temp_desejada == "" or (lata_a is None and lata_b is None and lata_c is None) \
            or (not tempValida(temp_desejada)):
        if temp_desejada == "":
            context['setar_temp_status'] = "Voce nao escolheu nenhuma temperatura"
        elif lata_a is None and lata_b is None and lata_c is None:
            context['setar_temp_status'] = "Voce nao escolheu nenhuma lata"
        else:
            context['setar_temp_status'] = "Temperatura invalida"

        return HttpResponse(template.render(context, request))

    else:
        temp_codificada = putTemp(temp_desejada)

        global_var.nova_mensagem_resposta = False
        while Radio.getLastPckt() is not None:
            pass

        Radio.send(pegarInstrucao(lata_a, lata_b, lata_c, temp_codificada), dstPAN, dstSHORT,
                   PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED,
                   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

        currTime = time.time()

        while True:
            if global_var.nova_mensagem_resposta == True:
                if global_var.pacote_resposta.payload[0] & 0b1000000:
                    context['setar_temp_status'] = "Temp setada"
                else:
                    context['setar_temp_status'] = "Temp nao foi setada"
                break
            if abs(currTime - time.time()) > global_var.TIMEOUT_ESPERA:
                # deu ruim
                context['timeout'] = "A geladeira nao respondeu a tempo e a temp nao foi setada"
                break

        return HttpResponse(template.render(context, request))