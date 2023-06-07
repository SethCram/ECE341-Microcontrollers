/******************************** Project 10 ****************************
*
* File: test_proj10.c
* Author name: Seth Cram
* Rev. Date: 11/30/2021
*
* Project Description: 
* 
*
* Notes:
*
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "input_capture.h"
#include "LCDlib.h"
#include "pwm.h"
#include <string.h> //for sprintf()

extern float rps; //already in ic header file but needed here too?

int main()
{
	//VARS:
	
	int dutyCycle = 40; //should be 40
	char errorMsg[] = "Error Init PWM";
	int msgDelay = 100; //in ms
	char rpsStr[16]; //should only need 14, but 16 just to be safe
				//if doesnt output on LCD correct, decr
	
	//INIT:
	
	system_init();
    
    	initLCD(); //for instrumentation
	
	//For this lab, the PWM should be initialized with a 40% 
	// duty cycle and a cycle frequency of 1 kHz.
    int errorCode = pwm_init( dutyCycle, CYCLE_FREQ );
    
	//if error initing pwm:
	if( errorCode != 0 )
	{
		//disable CN intr?
		mCNIntEnable( 0 );

		LCD_puts( errorMsg ); //if error, output

		//enable CN intr?
		mCNIntEnable( 1 );
	}

	ic_init(); //init input compare module
	
	t2_intr_init();

	t3_init();

	cn_intr_init();

	/*
	Updates the motor speed measurement (RPS) on line 2 of the LCD
		display at the rate of once each 100ms
	Clear only line 2 of the LCD and display the measured revolutions per
		second as RPS = xxx.xx
	LCD updates must be protected from CN interrupts
	*/
	while( 1 )
	{
		//disable CN intr:
		mCNIntEnable( 0 );

		//clear line 2 and change cursor pos to start of line 2	
		ClearLCDline( 2 );
		
		//display the measured revolutions per second as RPS = xxx.xx

		//format string and put into rpsStr:
		sprintf( rpsStr, "RPS = %.2f", rps ); //could remove 0 if dont want leading zeros
			//6 = min num of tot digits
			//2 = num of digits after deci

		//output formatted string:
		LCD_puts( rpsStr );

		//enable CN intr:
		mCNIntEnable( 1 );

		//delay till next output:
		LCD_delay( msgDelay );
	}
}
