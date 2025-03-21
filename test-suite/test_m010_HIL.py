# test_m011_HIL.py

# pi@raspberrypi:
	# pytest -s test_m011_HIL.py  --capture sys -rP --capture sys -rF --disable-pytest-warnings --html=m011-Test-Report.html

import serial, time
import pytest
import logging
import sys

stdWait = 5 # milliseconds

LOG = logging.getLogger(__name__)
LOG.setLevel('INFO')

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


# @pytest.mark.skip
def test_Keepalive():
	assert "parameter set" == dutSetGet("Keepalive:period 2", 10 * stdWait)
	assert "Keepalive started" == dutSetGet("Keepalive on", stdWait)
	time.sleep(1.95)
	assert "" == dutGet(stdWait)
	time.sleep(0.05)
	assert "Keepalive overflow" == dutGet(stdWait)

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
