# pinocchIO
	firmware for an IO controller with USB interface


### uC	STM32F103RET6
	 * 32BIT ARM-MCU 
	 * ... FLASH
	 * max. ... IO-channels
	 * max. clock speed ...


### Concept
	 * VOUTA = PA4 -> DAC_OUT1
	 * VOUTB = PA5 -> DAC_OUT2

### actual Config
	 * ... Hz SYCLK
	 * ... Hz ABP1 (Timers)
	 * ... Hz ABP2 (Periphs)
	 * std=gnu11 ... C11-Standard with gnu extensions

### flashing from CMD in Win
	 * make all
	 * st-link_cli -P m011.bin 0x08000000 && st-link_cli -Rst

### flashing from linux
	~/dow/stlink/build/Release/bin/st-flash
	sudo ./st-flash write /media/100GB/Messungen/script/HIL/m011.bin  0x8000000
	 * st-flash write m011.bin  0x8000000

### powerup m011-HW
	* GND lift switch
	* short both resistors 'Ros'
	* BNC/SMA sockets
	* close Voff
	* trim Voff to 1.65
	* Solder on USB socket
	* Resolder Relay
	* flash firmware
