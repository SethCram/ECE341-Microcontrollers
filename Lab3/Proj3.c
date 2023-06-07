/******************************** Project 3 ****************************
*
* File: Proj3.c
* Author name: Seth Cram
* Rev. Date: 9/21/2021
*
* Project Description: 
* 
*
* Notes:
*  'sm' = "stepper motor"
*  'FS' = "full step"
*   CW = down table 
*   Where are 'CW/CCW', 'FS/HS' defined?
*   SM1-SM4 = stepper motor outputs (other LEDs in same port so read-modify-write)
*
*	
**********************************************************************/
#include <plib.h>
#include "CerebotMX7cK.h"
#include "Proj3.h"

int main()
{
 system_init (); /* Setup system Hardware. */
 
 unsigned int btns, dir, mode, sm_code, step_delay;

 //set btn1 and btn2 to digital inputs:
 PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2);

 //set LEDA, LEDB to digital ouputs? 
 // Conflict with SM1-4 so read-modify-write?:
 

 while(1)
 {
	btns = read_buttons();

	decode_buttons( btns, &step_delay, &dir, &mode );

	sm_code = sw_fsm( dir, mode );

	output_sm_code( sm_code );
    
    LATBINV = LEDC; //flip LEDC to see how long methods take

	//sw_msDelay( step_delay ); /* Software only delay */
 }
 return 0; 
}	 

//Sets up the configuration for Port B to control LEDA:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
	PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);/* Set PmodSTEP LEDs A-H outputs */
	LATBCLR = SM_LEDS; /* Turn off LEDA through LEDH */
}

//reads the status of BTN1 and BTN2:
int read_buttons(void)
{
	//return if btn1 and btn2 are set or not:
	return PORTReadBits( IOPORT_G, BTN1 | BTN2 );
}

//determines the values of step direction, step mode, and
// step delay using the rules specified in Table 2:
void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode )
{
	switch( buttons ) 
	{
		case BTN1: 
			*dir = CW;
			*mode = HS;
			*step_delay = 20; //HS = 20ms delay
		break;
		
		case BTN2: 
			*dir = CCW;
			*mode = HS;
			*step_delay = 20;
		break;

		case BTN1 | BTN2: //both btns pressed 
			*dir = CCW;
			*mode = FS;
			*step_delay = 40; //FS = 40ms delay
		break;

		default: //neither btn pressed, or more than 2 pressed
			*dir = CW;
			*mode = FS;
			*step_delay = 40;
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
			else //full stepping
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
	/* You cannot simply write the stepper motor control code to 
           Port B using the instruction, “PORTB = code;” Doing so will also
           clear out other PORT B bits used for instrumentation.
	*/
	//the bit state for LEDA and LEDB must be preserved when setting 
        // SM1 through SM4 (requires a read–modify-write)


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

	/* or 
	    temp = LATB & ~mask; // Read all 16 port B bits and clear mask bits
            LATG = temp | (new_data & mask;) // Write the modified bits to port B 
	*/
}

//software delay for the specified number of ms:
void sw_msDelay (unsigned int mS)
{
	int i; //for loop cnting variable
 
	while(mS --) // SW Stop breakpoint
	{
		for (i = 0; i< COUNTS_PER_MS; i++) // 1 ms delay loop
		{
			// do nothing
		}

	LATBINV = LEDA; // Toggle LEDA each ms for instrumentation
	}

	LATBINV = LEDB; /* Toggle LEDB each delay period */

} //SW Stop breakpoint