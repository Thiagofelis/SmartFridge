# -*- coding: utf-8 -*-

from __future__ import unicode_literals
from django.http import HttpResponse
import var
from django.template import loader
from temp.externo.defines import *
from temp.externo.func import getTemp, tempValida, putTemp
from django.shortcuts import render
import time, random
from iniciar import Radio, num, byte_unit

def index(request):
    template = loader.get_template('temp/index.html')

    #limpa o buffer
    while Radio.getLastPckt () != None:
        pass
#   num[0] = num[0] + byte_unit[0] if num[0] != 255 else 0
    # ATENCAO: fazer com que no inicio de cada mensagem seja mandado um numero entre 00 e 09
    # e que o msp retorne esse numero no inicio da mensagem, para proposito de indentificacao

    var.varia = False
    Radio.send(bytearray.fromhex("0300"), dstPAN, dstSHORT,
               PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED,
               SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
    currTime = time.time()
    context = {}

    while True:
        if var.varia == True:
            context['a'] = getTemp((var.pacote.payload[1] << 4) | (var.pacote.payload[2] >> 4))
            context['b'] = getTemp(((var.pacote.payload[2] & 0b1111) << 8) | (var.pacote.payload[3] & 0b11111111))
            break
        if abs(currTime - time.time()) > 3:
            # deu ruim
            context['timeout'] = "A geladeira nao respondeu a tempo"
            break

        print vars(Radio.getRdStatus())
        time.sleep(0.8)

    return HttpResponse(template.render(context, request))

def configura(request):
    template = loader.get_template('temp/index.html')
    context = {}
    lata_a = request.POST.get('lataa')
    lata_b = request.POST.get('latab')
    temp_desejada = request.POST.get('temperatura')

    if temp_desejada == "" or (lata_a == None and lata_b == None) or (not tempValida(temp_desejada)):
        if temp_desejada == "":
            context['setar_temp_status'] = "Voce nao escolheu nenhuma temperatura"
        elif lata_a == None and lata_b == None:
            context['setar_temp_status'] = "Voce nao escolheu nenhuma lata"
        else:
            context['setar_temp_status'] = "Temperatura invalida"
        # limpa o buffer
        while Radio.getLastPckt() != None:
            pass
        Radio.send(bytearray.fromhex("0300"), dstPAN, dstSHORT,
                   PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED,
                   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)
        currTime = time.time()
        while True:
            packt = Radio.getLastPckt()
            if packt != None:
                if len(packt.payload) == 4 and packt.srcAddr == bytearray.fromhex(dstSHORT):
                    context['a'] = getTemp((packt.payload[1] << 4) | (packt.payload[2] >> 4))
                    context['b'] = getTemp(((packt.payload[2] & 0b1111) << 8) | (packt.payload[3] & 0b11111111))
                    break
            if abs(currTime - time.time()) > 3:
                # deu ruim
                context['timeout'] = "A geladeira nao respondeu a tempo"
                break

        return HttpResponse(template.render(context, request))
    else:
        k = putTemp(temp_desejada)
        if lata_a != None and lata_b != None:# lataA e lataB
            p = bytearray(3)
            p[0] = k >> 4
            p[1] = ((k << 4) & 0b11110000) | (k >> 8)
            p[2] = k & 0b11111111
            comando = bytearray.fromhex("0303")
        elif lata_a != None and lata_b == None:# lataA
            p = bytearray(2)
            p[0] = k >> 4
            p[1] = (k << 4) & 0b11110000
            comando = bytearray.fromhex("0301")
        else:#                                   lataB
            p = bytearray(2)
            p[0] = k >> 4
            p[1] = (k << 4) & 0b11110000
            comando = bytearray.fromhex("0302")
        var.varia = False
        Radio.send(comando + p, dstPAN, dstSHORT,
                   PACKET_TYPE_DATA, ACK_REQUIRED_ENABLED, PAN_ID_COMP_DISABLED,
                   SEQUENCE_NUM_SUP_DISABLED, DST_SHORT_ADDR, SRC_SHORT_ADDR)

    currTime = time.time()

    while True:
        if var.varia == True:
            context['a'] = getTemp((var.pacote.payload[1] << 4) | (var.pacote.payload[2] >> 4))
            context['b'] = getTemp(((var.pacote.payload[2] & 0b1111) << 8) | (var.pacote.payload[3] & 0b11111111))
            if var.pacote.payload[0] & 0b1000000:
                context['setar_temp_status'] = "Temp setada"
                print "a"
            else:
                context['setar_temp_status'] = "Temp nao foi setada"
            break
        if abs(currTime - time.time()) > 3:
            # deu ruim
            context['timeout'] = "A geladeira nao respondeu a tempo e a temp nao foi setada"
            break

        print vars(Radio.getRdStatus())
        time.sleep(0.8)

    return HttpResponse(template.render(context, request))