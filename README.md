### uC	STM32F103RET6
 * 32BIT ARM-MCU 
 * 512kb FLASH
 * max. 51 IO-channels
 * max. clock speed 72MHz

### Analog oputputs
 * VOUTA = PA4 -> DAC_OUT1
 * VOUTB = PA5 -> DAC_OUT2

### actual Config
 * 48Hz SYCLK
 * 24Hz ABP1 (Timers)
 * 24Hz ABP2 (Periphs)
 * std=gnu11 ... C11-Standard with gnu extensions

### flashing from CMD in Win
 * make all
 * st-link_cli -P m011.bin 0x08000000 && st-link_cli -Rst

### flashing from linux
	~/dow/stlink/build/Release/bin/st-flash
	sudo ./st-flash write /media/100GB/Messungen/script/HIL/m011.bin  0x8000000
	 * st-flash write m011.bin  0x8000000

### basics on F103
 * http://stefanfrings.de/stm32/stm32f1.html

