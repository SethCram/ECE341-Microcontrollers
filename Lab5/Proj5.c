/******************************** Project 5 ****************************
*
* File: Proj5.c
* Author name: Seth Cram
* Rev. Date: 10/05/2021
*
* Project Description: 
* 
*
* Notes:
*   CW = down table 
*   SM1-SM4 = stepper motor outputs (other LEDs in same port so read-modify-write)
*   now set step_delay based on speed + mode in decode_btns()
*   Timer1 ISR (sm) is checked once every 1ms bc of PR1 value
*  
*   Are vars local to ISR deleted as soon as one is finished??
*   Change motor_cntr to static var??
*	
**********************************************************************/
#include <plib.h>
#include "CerebotMX7cK.h"
#include "Proj5.h"

int main()
{
    int btns;
    
 system_init(); /* Setup system Hardware and SW for this lab. */
	
 //setup timer1:
 t1_intr_init();

 //setup change notice intrs for btn1 and btn2:
 cn_intr_init();
 
 //read and decode the btns so 'step_delay' set:
 btns = read_buttons();
 decode_buttons( btns, &step_delay, &dir, &mode );
 
 //Timer1_ISR(); //tried calling ISR from code

 while(1)
 {
	//empty while(1) loop
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
    
	//interrupt setup:
    	INTEnableSystemMultiVectoredInt(); 
    	INTEnableInterrupts(); 
}

//init timer 1 for interrupts:
void t1_intr_init(void)
{
	//configure Timer 1 with internal clock, 1:1 prescale, PR1 for 1 ms period
 	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, T1_TICK-1);
	
	// set up the timer interrupt with a priority of 2, sub priority 0
	mT1SetIntPriority(2); // Group priority range: 1 to 7
	mT1SetIntSubPriority(0); // Subgroup priority range: 0 to 3
	mT1IntEnable(1); // Enable T1 interrupts
		
	// Global interrupts must enabled to complete the initialization.
}

//ISR for timer1: (triggered once every 1ms bc PR1 val)
void __ISR(_TIMER_1_VECTOR, IPL2) Timer1_ISR(void)
{
    static int motor_cntr = 0; //declared static bc only needed in this ISR, zero so initialized below
    
	LATBINV = LEDA; //toggle LEDA every ms
    
	unsigned int sm_code;	//local var

	motor_cntr = motor_cntr - 1; //decrement sm cntr	

	if( motor_cntr <= 0 ) //if time to set motor
	{
		sm_code = sw_fsm( dir, mode );

		output_sm_code( sm_code );

		LATBINV = LEDB; //toggle LEDB every motor write

		motor_cntr = step_delay; //reset cntr
	}

	mT1ClearIntFlag(); //has to be at the very end?
}

/* Initialization of CN peripheral for interrupt level 1 */
void cn_intr_init(void) 
{
	unsigned int dummy; // used to hold PORT read value

	// BTN1 and BTN2 pins set for input by Cerebot header file
	// PORTSetPinsDigitalIn(IOPORT_G, BIT_6 | BIT7); //
	
	// Enable CN for BTN1 and BTN2
	mCNOpen(CN_ON,(CN8_ENABLE | CN9_ENABLE), 0);

	// Set CN interrupts priority level 1 sub priority level 0
	mCNSetIntPriority(1); // Group priority (1 to 7)
	mCNSetIntSubPriority(0); // Subgroup priority (0 to 3)

	// read port to clear difference
	dummy = PORTReadBits(IOPORT_G, BTN1 | BTN2);
	mCNClearIntFlag(); // Clear CN interrupt flag
	mCNIntEnable(1); // Enable CN interrupts

	// Global interrupts must enabled to complete the initialization.
}

void __ISR(_CHANGE_NOTICE_VECTOR, IPL1) CN_ISR(void)
{
	LATBSET = LEDC; //start of CN ISR

	unsigned int btns; //local var 
    
    //debounce the btns for 20 ms:
    sw_msDelay( DEBOUNCE_TIME );
	
	btns = read_buttons();

	decode_buttons( btns, &step_delay, &dir, &mode );

	LATBCLR = LEDC; //end of CN ISR

	mCNClearIntFlag(); //has to be at the very end?
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

	//LATBINV = LEDA; // Toggle LEDA each ms for instrumentation
        //already flipped in T1 ISR
	}

} //SW Stop breakpoint

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
			*step_delay = 40; //rpm = 15, FS
		break;
		
		case BTN2: 
			*dir = CCW;
			*mode = HS; 
			*step_delay = 30; //rpm = 10, HS
		break;

		case BTN1 | BTN2: //both btns pressed 
			*dir = CCW;
			*mode = FS;
			*step_delay = 24; //rpm = 25, FS
		break;

		default: //neither btn pressed, or more than 2 pressed
			*dir = CW;
			*mode = HS;
			*step_delay = 20;  //rpm = 15, HS
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


