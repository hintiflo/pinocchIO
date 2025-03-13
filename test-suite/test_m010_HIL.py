# test_m011_HIL.py

# pi@raspberrypi:
	# cd /media/100GB/Messungen/script/HIL
	# pytest -s test_m011_HIL.py  --capture sys -rP --capture sys -rF --disable-pytest-warnings --html=m011-Test-Report.html

import serial, time
import pytest
import logging
import sys
import platform

hil = True
try:
	import RPi.GPIO as GPIO
	dut = serial.Serial("/dev/ttyACM0")
except ImportError:
	print("no GPIO package available")
	hil = False
	dut = serial.Serial("COM4")

stdWait = 5 # milliseconds

LOG = logging.getLogger(__name__)
LOG.setLevel('INFO')

# dut.close()

''' (connection to) the device under test
''' 
# test_Identification()
# test_PortB(1)
# test_Relay(1)


''' pyvisa instead of serial
	import pyvisa
	rm = pyvisa.ResourceManager('@py')
	deviceAddress = "TCPIP0::" + ip + "::inst0::INSTR" 
	inst = self.rm.open_resource(deviceAddress)

'''


'''	test-fixture 
'''


@pytest.fixture(scope="session", autouse=True)
def init_session(request):	
	# print("before a session")
	if "Linux" == platform.system():
		hil = True
		print("Linux")
	if "Windows" == platform.system():
		hil = False
		print("Windows")
	hilSetup()
	request.addfinalizer(finalize_session)

def finalize_session():		
	# print("after a session")
	dut.close()
	hilTeardown()


@pytest.fixture(scope="function", autouse=True)
def init_function(request):	
	# print("before every case")
	request.addfinalizer(finalize_function)

def finalize_function():	
	# print("after every case")
	print()


''' helper funcs
''' 

def hilSetup():
	if hil:
		GPIO.setmode(GPIO.BCM)

def hilTeardown():
	if hil:
		GPIO.cleanup()

def hilSetAsInput(hilPin):
	if hil:
		GPIO.setup(hilPin, GPIO.IN)

def hilReadInput(hilPin):
	if hil:
		pinState =  GPIO.input(hilPin)
		LOG.info("hilRead: " + str(hilPin) + ": " + str(pinState) )
		return pinState
	else:
		return None 
	
def hilSetAsOutput(hilPin):
	if hil:
		GPIO.setup(hilPin, GPIO.OUT)

def hilSetOutputHigh(hilPin):
	if hil:
		LOG.info("hilSet: " + str(hilPin) + " HIGH")
		GPIO.output(hilPin, GPIO.HIGH)
		
def hilSetOutputLow(hilPin):
	if hil:
		LOG.info("hilSet: " + str(hilPin) + " LOW")
		GPIO.output(hilPin, GPIO.LOW)

def dutSetGet(cmd, waitTimeMs):
	msg = cmd + '\n'
	msg = msg.encode("utf-8")
	dut.write(msg)
	time.sleep(waitTimeMs/1000.0)
	response = dut.read(dut.inWaiting()).decode("utf-8").strip()
	LOG.info("cmd: " + "\"" + cmd + "\"" + " -> reply: " + "\"" + response + "\"")
	return response

def dutGet(waitTimeMs):
	time.sleep(waitTimeMs/1000.0)
	response = dut.read(dut.inWaiting()).decode("utf-8").strip()
	LOG.info("reply: " + "\"" + response + "\"")
	return response

''' test cases
'''
# @pytest.mark.skip
def test_Identification():
	dutSetGet("*RST", stdWait)
	assert "smartIO, v0.3" in dutSetGet("*IDN?", stdWait)

@pytest.mark.skip
def test_Debounce():
	assert "nothing" == "happened, yet"
	"route 3V3 via a relay to a debounced input and check wheter only one IRQ occurs"

''' TIMERS

@pytest.mark.skip
@pytest.mark.parametrize("timer", ["TimerA","TimerB","TimerC"])
@pytest.mark.parametrize("freq,expect1, expect2", [
	(0,  	"parameter invalid", None),
	(1,  	"parameter set",1),
	(10, 	"parameter set",10),
	(100,	"parameter set",100),
	(1001,	"parameter set",1001),
	(10000,	"parameter set",10000),
	(10001,	"parameter set",1.004e+04),
	(1e5,	"parameter invalid", None),
	])
def test_Timer_Freq(timer, freq, expect1, expect2):
	assert expect1 == dutSetGet(timer+":Frequency " +str(freq), 100)
	response = dutSetGet(timer+":Frequency?", 100)
	if expect2 != None:
		expect2str = timer + " Frequency "+  "{:.3e}".format(expect2)
		assert expect2str.lower() in response.lower()
	print(response)


@pytest.mark.skip
@pytest.mark.parametrize("timer", ["TimerA","TimerB","TimerC"])
@pytest.mark.parametrize("peri,expect1, expect2", [
	(0,  		"parameter invalid", None),
	(1,  		"parameter set", 1.000e+00),
	(1/10, 		"parameter set", 1.000e-01),
	(1/100,		"parameter set", 1.000e-02),
	(1/1001,	"parameter set", 9.987e-04),
	(1/10000,	"parameter set", 1/10000),
	(1/10001,	"parameter set",  9.958e-05),
	(1/1e5,		"parameter invalid", None),
	])
def test_Timer_Period(timer, peri, expect1, expect2):
	assert expect1 == dutSetGet( timer+":Period " +str(peri), 10)
	response = dutSetGet( timer+":Period?", 10)
	if expect2 != None:
		assert ( timer+" Period "+ "{:.3e}".format(expect2) ).lower() in response.lower()
		# print(timer+" Peri = "+str(expect2), '???' , response)


@pytest.mark.skip
@pytest.mark.parametrize("pin", ["0", "1"])
def test_PortC(pin):
	cmdPrefix = "PortC:" + str(pin)
	for idx in range(8):
		dutSetGet(cmdPrefix + " off", 1)
		assert cmdPrefix + " OFF" == dutSetGet( cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " on", 400/(idx+1) )
		assert cmdPrefix + " ON" == dutSetGet(cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " off", 400/(idx+1) )
		assert cmdPrefix + " OFF" == dutSetGet(cmdPrefix + "?", stdWait)

@pytest.mark.skip
@pytest.mark.parametrize("pin", ["0", "1"])
def test_PortC(pin):
	cmdPrefix = "PortC:" + str(pin)
	for idx in range(8):
		dutSetGet(cmdPrefix + " off", 1)
		assert cmdPrefix + " OFF" == dutSetGet( cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " on", 400/(idx+1) )
		assert cmdPrefix + " ON" == dutSetGet(cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " off", 400/(idx+1) )
		assert cmdPrefix + " OFF" == dutSetGet(cmdPrefix + "?", stdWait)



'''


''' procedure: test_PortB_InOut.pdf
'''

# @pytest.mark.skip
@pytest.mark.parametrize("pin", ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"])
def test_PortA(pin):
	cmdPrefix = "PortA:" + str(pin)
	for idx in range(8):
		dutSetGet(cmdPrefix + " off", stdWait)
		assert cmdPrefix + " off" == dutSetGet( cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " on", stdWait )
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
		dutSetGet(cmdPrefix + " off", stdWait )
		assert cmdPrefix + " off" == dutSetGet(cmdPrefix + "?", stdWait)




# @pytest.mark.skip
def test_Keepalive():
	assert "parameter set" == dutSetGet("Keepalive:period 2", 10 * stdWait)
	assert "Keepalive started" == dutSetGet("Keepalive on", stdWait)
	time.sleep(1.95)
	assert "" == dutGet(stdWait)
	time.sleep(0.05)
	assert "Keepalive overflow" == dutGet(stdWait)


# @pytest.mark.skip
@pytest.mark.parametrize("pin", [0, 1])
def test_PortB_InOut(pin):
	cmdPrefix = "PortB:" + str(pin)
	# print(type(pin))
	# GPIO2 -> PB:0
	# GPIO3 -> PB:1
	if 0 == pin:
		hilPin = 4
	if 1 == pin:
		hilPin = 5
	hilSetAsInput(hilPin)

	dutSetGet("*RST", stdWait)
	
	dutSetGet( cmdPrefix + " output", stdWait)
	# assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " state?", stdWait)
	assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " off"    == dutSetGet( cmdPrefix + "?", stdWait)
	assert 0 == hilReadInput(hilPin)
	
	for idx in range(4):
		dutSetGet(cmdPrefix + " on", stdWait )
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
		assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
		assert 1 == hilReadInput(hilPin)
		dutSetGet(cmdPrefix + " off", stdWait)
		assert cmdPrefix + " off" == dutSetGet( cmdPrefix + "?", stdWait)
		assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
		assert 0 == hilReadInput(hilPin)

	dutSetGet(cmdPrefix + " Input", stdWait )
	assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " direction?", stdWait)
	hilSetAsOutput(hilPin)

	for idx in range(4):
		hilSetOutputHigh(hilPin)
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
		assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " direction?", stdWait)

		hilSetOutputLow(hilPin)
		assert cmdPrefix + " off" == dutSetGet( cmdPrefix + "?", stdWait)
		assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " direction?", stdWait)

	dutSetGet(cmdPrefix + " Output", stdWait )
	assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " noIRQ" == dutSetGet( cmdPrefix + " irq?", stdWait)
	dutSetGet(cmdPrefix + " on", stdWait )
	assert 1 == hilReadInput(hilPin)
	dutSetGet(cmdPrefix + " off", stdWait )
	assert 0 == hilReadInput(hilPin)

''' procedure: test_PortB_IRQ.pdf
'''
# @pytest.mark.skip
@pytest.mark.parametrize("pin", [0, 1])
def test_PortB_IRQ(pin):
	cmdPrefix = "PortB:" + str(pin)
	# GPIO2 -> PB:0
	# GPIO3 -> PB:1
	if 0 == pin:
		hilPin = 4
		extiLine = 9
	if 1 == pin:
		hilPin = 5
		extiLine = 10
	hilSetAsOutput(hilPin)
	hilSetOutputLow(hilPin)
	
	dutSetGet("*RST", stdWait)
	dutSetGet(cmdPrefix + " Input", stdWait )
	# assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " state?", stdWait)
	assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " noIRQ" == dutSetGet( cmdPrefix + " irq?", stdWait)
	dutSetGet( cmdPrefix + " RisingEdge", stdWait)
	assert cmdPrefix + " RisingEdge" == dutSetGet( cmdPrefix + " irq?", stdWait)

	for idx in range(4):
		hilSetOutputHigh(hilPin)
		assert "RisingEdge on ExtIrq " + str(extiLine) == dutGet(500)
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
	
		hilSetOutputLow(hilPin)
		assert "" == dutGet(500)
		assert cmdPrefix + " off" == dutSetGet(cmdPrefix + "?", stdWait)
	
	dutSetGet( cmdPrefix + " FallingEdge", stdWait)
	assert cmdPrefix + " Rising&FallingEdge" == dutSetGet( cmdPrefix + " irq?", stdWait)
	
	for idx in range(4):
		hilSetOutputHigh(hilPin)
		assert "RisingEdge on ExtIrq " + str(extiLine) == dutGet(500)
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
	
		hilSetOutputLow(hilPin)
		assert "FallingEdge on ExtIrq " + str(extiLine) == dutGet(500)
		assert cmdPrefix + " off" == dutSetGet(cmdPrefix + "?", stdWait)
	
	dutSetGet(cmdPrefix + " Output", stdWait )
	assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " noIRQ" == dutSetGet( cmdPrefix + " irq?", stdWait)
	dutSetGet(cmdPrefix + " on", stdWait )
	assert 1 == hilReadInput(hilPin)
	dutSetGet(cmdPrefix + " off", stdWait )
	assert 0 == hilReadInput(hilPin)


''' procedure: test_PortB_PullUpDown.pdf
'''
@pytest.mark.skip
@pytest.mark.parametrize("pin", [0, 1])
def test_PortB_PullUpDown(pin):
	cmdPrefix = "PortB:" + str(pin)
	# GPIO2 -> PB:0
	# GPIO3 -> PB:1
	if 0 == pin:
		hilPin = 4
		extiLine = 9
	if 1 == pin:
		hilPin = 5
		extiLine = 10

	hilSetAsOutput(hilPin)
	hilSetOutputLow(hilPin)

	hilSetAsInput(hilPin)
	dutSetGet("*RST", stdWait)
	dutSetGet(cmdPrefix + " Input", stdWait )
	# assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " state?", stdWait)
	assert cmdPrefix + " input" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " noPull" == dutSetGet( cmdPrefix + " pull?", stdWait)

	for idx in range(4):
		dutSetGet(cmdPrefix + " PullUp", stdWait )
		assert cmdPrefix + " PullUp" == dutSetGet( cmdPrefix + " pull?", stdWait)
		assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
		assert 1 == hilReadInput(hilPin)
		
		dutSetGet(cmdPrefix + " PullDown", stdWait )
		assert cmdPrefix + " PullDown" == dutSetGet( cmdPrefix + " pull?", stdWait)
		assert cmdPrefix + " off" == dutSetGet( cmdPrefix + "?", stdWait)
		assert 0 == hilReadInput(hilPin)

	dutSetGet( cmdPrefix + " nopull", stdWait)
	assert cmdPrefix + " noPull" == dutSetGet( cmdPrefix + " pull?", stdWait)
	dutSetGet(cmdPrefix + " Output", stdWait )
	assert cmdPrefix + " output" == dutSetGet( cmdPrefix + " direction?", stdWait)
	assert cmdPrefix + " noIRQ" == dutSetGet( cmdPrefix + " irq?", stdWait)
	print( dutSetGet( cmdPrefix + " pull?", stdWait) )
	dutSetGet(cmdPrefix + " on", stdWait )
	assert 1 == hilReadInput(hilPin)
	dutSetGet(cmdPrefix + " off", stdWait )
	assert 0 == hilReadInput(hilPin)



# @pytest.mark.skip
@pytest.mark.parametrize("pin", ["0", "1"])
def test_Relay(pin):
	relayWait = 200
	cmdPrefix = "Relay" + str(pin)
	for idx in range(2):
			dutSetGet(cmdPrefix + " off", relayWait)
			assert cmdPrefix + " off" == dutSetGet( cmdPrefix + "?", stdWait)
			dutSetGet(cmdPrefix + " on", relayWait )
			assert cmdPrefix + " on" == dutSetGet(cmdPrefix + "?", stdWait)
			dutSetGet(cmdPrefix + " off", relayWait )
			assert cmdPrefix + " off" == dutSetGet(cmdPrefix + "?", stdWait)


# read out a Rspis GPIO 2
#	GPIO.setmode(GPIO.BCM)
#	GPIO.setup(2, GPIO.IN)
#	pinState = GPIO.input(2)
#	print("GPIO 2 = ", pinState)


# toggle a Raspis GPIO 16 Pin
#	GPIO.setmode(GPIO.BCM)
#	GPIO.setup(16, GPIO.OUT)
#	GPIO.output(16, GPIO.HIGH)
#	time.sleep(2)
#	GPIO.output(16, GPIO.LOW)
#	time.sleep(2)
#	GPIO.output(16, GPIO.HIGH)
#	time.sleep(2)
#	GPIO.output(16, GPIO.LOW)
#	GPIO.cleanup()


''' call test run from within this code
# pytest -s --capture sys -rP --capture sys -rF -m single --disable-pytest-warnings --html=m011-HIL.html

args_str = "-s --capture sys -rP --capture sys -rF -m single --disable-pytest-warnings --html=m011-HIL.html "
args_str = "-s --capture sys -rP --capture sys -rF --disable-pytest-warnings --html=m011-HIL-Report.html "
args = args_str.split(" ")
pytest.main(args)

# args_str = "-s --capture sys -rP --capture sys -rF -m single --disable-pytest-warnings --html=m011.html "
# args_str = "-s --capture sys -rP --capture sys -rF --disable-pytest-warnings --html=m011.html "
# args = args_str.split(" ")
# pytest.main(args)
# pytest.main()
'''

