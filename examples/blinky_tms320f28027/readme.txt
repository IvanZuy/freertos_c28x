Author: 		rosalesr (rosales.r@gmail.com)
Date: 			2016/11/20

Description:	blinky_f28027 is an example project utilizing IvanZuy's FreeRTOS port to the 
				TMS320F2x processors. This example is compatible with the F28027 Piccolo MCU 
				on the C2000 Launchpad. 
				
				Just took the blinky_f28034 from IvanZuy and changed out the library paths to 
				point to the appropriate F28027 headers included in the TI ControlSuite library. 
				
				Loads the blink LED example into the F28027 flash memory so you have to make sure
				the launchpad is setup to boot from flash:
				
					Dipswitch S1.1 - high
					Dipswitch S1.2 - high
					Dipswitch S1.3 - high (JTAG_TRST routed to F28027 TRST pin for debugger support)
				
				Tested on the following hardware: 
				TI ControlSuite v3.4.3 (November 07, 2016)
				C2000 Launchpad
