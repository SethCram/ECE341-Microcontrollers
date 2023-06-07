/******************************** Project 4 ****************************
*
* File: Proj4.c
* Author name: Seth Cram
* Rev. Date: 9/21/2021
*
* Project Description: 
* 
*
* Notes:
*   CW = down table 
*   SM1-SM4 = stepper motor outputs (other LEDs in same port so read-modify-write)
*   now set step_delay based on speed + mode in decode_btns()
*	
**********************************************************************/
#include <plib.h>
#include "CerebotMX7cK.h"
#include "Proj4.h"

int main()
{
 system_init (); /* Setup system Hardware and SW for this lab. */
 
 //stepper motor vars:
 unsigned int btns, dir, mode, speed, sm_code, step_delay; 
 
 //timer vars:
 int btn_cntr, motor_cntr;

 //cnters init'd to zero so reset them properly first iteration:
 btn_cntr = 0;
 motor_cntr = 0;

 while(1)
 {
	if( btn_cntr <= 0 ) //if time to read btns
	{
		btns = read_buttons();

		decode_buttons( btns, &step_delay, &dir, &mode );
	
		LATBINV = LEDB; //toggle LEDB every btn check

		//reset cntr to check btns every 100ms:
		btn_cntr = 100; 		
	}	

	if( motor_cntr <= 0 ) //if time to set motor
	{
		sm_code = sw_fsm( dir, mode );

		output_sm_code( sm_code );

		LATBINV = LEDC; //toggle LEDC every motor write

		motor_cntr = step_delay; //reset cntr
	}
	
	//waits for 1ms + decrements cntrs:
	Timer1_delay( &btn_cntr, &motor_cntr); 
 }

 return 0; 
}	 

//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
	
	PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);/* Set PmodSTEP LEDs A-H outputs */
	
	LATBCLR = SM_LEDS; /* Turn off LEDA through LEDH */
	
	//set btn1 and btn2 to digital inputs:
 	PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2);
    
    INTEnable( INT_T1, 1); //enable T1 interrupt
 
    OpenTimer1( (T1_ON | T1_PS_1_1), T1_TICK-1 ); //turn on timer1 w/ prescale1
                                                 // load PR1 w/ 9999 bc period of 
                                                 // 1ms reqs it 
                                                 //consts def'd in proj4 header
}

//reads the status of BTN1 and BTN2:
int read_buttons(void)
{
	//return if btn1 and btn2 are set or not:
	return PORTReadBits( IOPORT_G, BTN1 | BTN2 );
}

//determines the values of motor direction, motor mode, and
// step delay using the rules specified in Table 2:
void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode)
{

//new step_delay calced w/: 
//T_delay (ms/step) = 60000 (ms/min) / (X rev/min * 100 steps/rev * MODE)

	switch( buttons ) 
	{
		case BTN1: 
			*dir = CW;
			*mode = FS;
			*step_delay = 40; //rpm = 15
		break;
		
		case BTN2: 
			*dir = CCW;
			*mode = HS; 
			*step_delay = 30; //rpm = 10
		break;

		case BTN1 | BTN2: //both btns pressed 
			*dir = CCW;
			*mode = FS;
			*step_delay = 24; //rpm = 25
		break;

		default: //neither btn pressed, or more than 2 pressed
			*dir = CW;
			*mode = HS;
			*step_delay = 20;  //rpm = 15
		break;
	}
}

//determines the new output code for the stepper motor:
unsigned int sw_fsm( unsigned int dir, unsigned int mode )
{
	enum { S0 = 0, S0_5, S1, S1_5, S2, S2_5, S3, S3_5 }; /* Declaration of states */
	static unsigned int presState; 
	const unsigned int sm_code[] = { 0x02, 0x0A, 0x08, 0x09, 0x01, 0x05, 0x04, 0x06 };
	
	switch ( presState ) //NSL
	{
		case S0:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S0_5;
			}
			else //full stepping
			{
				presState = S1;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S3_5;
			}
			else //full stepping
			{
				presState = S3;
			}
		}
		break;
		
		case S0_5:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S1;
			}
			else //full stepping
			{
				presState = S1_5;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S0;
			}
			else //full stepping
			{
				presState = S3_5;
			}
		}
		break;

		case S1:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S1_5;
			}
			else //full stepping
			{
				presState = S2;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S0_5;
			}
			else //full stepping
			{
				presState = S0;
			}
		}
		break;

		case S1_5:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S2;
			}
			else //full stepping
			{
				presState = S2_5;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S1;
			}
			else //full stepping
			{
				presState = S0_5;
			}
		}
		break;
	
		case S2:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S2_5;
			}
			else //full motorping
			{
				presState = S3;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S1_5;
			}
			else //full stepping
			{
				presState = S1;
			}
		}
		break;
		
		case S2_5:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S3;
			}
			else //full stepping
			{
				presState = S3_5;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S2;
			}
			else //full stepping
			{
				presState = S1_5;
			}
		}
		break;

		case S3:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S3_5;
			}
			else //full stepping
			{
				presState = S0;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S2_5;
			}
			else //full stepping
			{
				presState = S2;
			}
		}
		break;

		case S3_5:
		if ( dir == CW ) //if rotting CW
		{
			 if( mode == HS )
			{
				presState = S0;
			}
			else //full stepping
			{
				presState = S0_5;
			}
		}
		else //CCW
		{
			 if( mode == HS )
			{
				presState = S3;
			}
			else //full stepping
			{
				presState = S2_5;
			}
		}
		break;
	}
	return sm_code[ presState ]; /* Return next state */
} 

//sends the four bit code to the stepper motor IO pins:
void output_sm_code( unsigned int sm_code )
{
	//READ-WRITE-MODIFY: ( LEDA-D which are Port B pins 2-6 kept same )

	int temp, new_data, mask;

	mask = SM_COILS; //SM_COILS = (SM1 | SM2 | SM3 | SM4)
	new_data = (sm_code << 7); //our new data is the step motor code;
				   // we shift it 7 bits to left since 
				   // motor code is pins 7-10 
				   // bc sm_code = 1st 4 bits

	temp = LATB; //read all 16 port B bits
	temp = temp & ~mask; //clear all bits that need to be set by our stepper motor code
	
	new_data = new_data & mask; //clear all non SM1-4 bits
	
	temp = temp | new_data; //recombine curr bits with new bits

	LATB = temp;
}

//waits for 1ms + decrements cntrs:
void Timer1_delay( unsigned int *btn_cntr, unsigned int *motor_cntr)
{
	//wait for intr flag to be set
	//while( !mT1GetIntFlag() ); //should be 1ms
    while( !( INTGetFlag( INT_T1 ) ));
	
	//mT1ClearIntFlag(); //clear intr flag
    INTClearFlag( INT_T1 );

	LATBINV = LEDA; //toggle LEDA every 1ms

	//decrement cntr vars bc 1ms passed:
	*btn_cntr = *btn_cntr - 1;
	*motor_cntr = *motor_cntr - 1;	
}	
