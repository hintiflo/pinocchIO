### uC	STM32F103RET6
 * 32BIT ARM-MCU 
 * ... FLASH
 * max. ... IO-channels
 * max. clock speed ...


### Concept
 * mostly legacy code from m010, extended by analogue outs
 * PA:4 = PC4
 * PA:5 = PC5
 * VOUTA = PA4 -> DAC_OUT1
 * VOUTB = PA5 -> DAC_OUT2
 * try to replace startup code with startup_stm32f103retx.s
	* C:\Users\r.hif\REC\#arc\CHASE-Rohrsensor\LFP-Treiber_src\Startup\startup_stm32f103c8tx.s
	* C:\Users\r.hif\REC\HISAMP\MOPAv01\firmware\Startup\startup_stm32f103retx.s

### actual Config
 * ...Hz SYCLK
 * ...Hz ABP1 (Timers)
 * ...Hz ABP2 (Periphs)
 * std=gnu11 ... C11-Standard with gnu extensions


### allocation of resources:
 * DAC-CH1: VOUT1
 * DAC-CH2: VOUT2
 * TIM8 : PulseC
 * 

### flashing from CMD in Win
 * make all
 * st-link_cli -P m011.bin 0x08000000 && st-link_cli -Rst

### flashing from linux
	~/dow/stlink/build/Release/bin/st-flash
	sudo ./st-flash write /media/100GB/Messungen/script/HIL/m011.bin  0x8000000
	 * st-flash write m011.bin  0x8000000

### basics on F103
 * http://stefanfrings.de/stm32/stm32f1.html

### generate a makefile from Cube-Rpoject
 * https://github.com/baoshi/CubeMX2Makefile
 * nur informativ: https://github.com/prof7bit/bare_metal_stm32f401xe
 
### SPI, 16Bit:
 *	https://www.mikrocontroller.net/topic/414289

powerup m011
	v GND-Lift Schalter
	v schliessen von beiden Ros
	BNC/SMA Buchsen
	v schliessen von Voff
	Voff korrigieren auf 1.65
	v USB Buchse auflöten
	Relais nachlöten
	fläääsh
	