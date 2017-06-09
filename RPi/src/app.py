import RPi.GPIO as GPIO

def boardConfig ()
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup (11, GPIO.OUT)
	GPIO.setup (13, GPIO.OUT)
	GPIO.setup (15, GPIO.IN)
	GPIO.output (11, 0)
	GPIO.output (13, 1)
