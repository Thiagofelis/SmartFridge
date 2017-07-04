import RPi.GPIO as GPIO
import sys
from defines import *
from app import *

def intFunc (packt):
	print packt.payload
	
def boardConfig ():
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup (11, GPIO.OUT)
	GPIO.setup (13, GPIO.OUT)
	GPIO.setup (15, GPIO.IN)
	GPIO.output (11, 0)
	GPIO.output (13, 1)

def boardFinish ():
	GPIO.cleanup() 