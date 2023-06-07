/******************************** Project 9 ****************************
*
* File: test_proj9.c
* Author name: Seth Cram
* Rev. Date: 11/08/2021
*
* Project Description: 
* 
*
* Notes:
*
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "pwm.h"
#include "LCDlib.h"

int main()
{
	//VARS:
	
	int dutyCycle = 40;
	//int cycleFreq = 1000; //changed to a macro const so accessible everywhere
	char errorMsg[] = "Error Init PWM";
	
	
	//INIT:
	
	system_init();
    
    initLCD(); //for instrumentation
	
	//For this lab, the PWM should be initialized with a 40% 
	// duty cycle and a cycle frequency of 1 kHz.
    int errorCode = pwm_init( dutyCycle, CYCLE_FREQ );
    
	if( errorCode != 0 )
	{
		LCD_puts( errorMsg ); //if error, output
	}
	
	t2_intr_init();

	cn_intr_init();

	
	while( 1 )
	{
		//empty, no fg tasks
	}
}
