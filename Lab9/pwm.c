/******************************** Project 9 ****************************
*
* File: pwm.c
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
#include "LCDlib.h" //needed for LCD_puts()
//#include <string.h> //for strcat() and sprintf()

//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
	
	//set btn1 and btn2 to digital inputs:
 	PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2);
	
    unsigned int LEDAtoD = LEDA | LEDB | LEDC | LEDD;
    
	//set LEDA-D as outputs:
	PORTSetPinsDigitalOut(IOPORT_B, LEDAtoD );
	
	LATBCLR = LEDAtoD ; /* Turn off LEDA through LEDH */
    
    
    //setup for next proj:
    
    PORTSetPinsDigitalOut(IOPORT_D, BIT_7 ); //for RD1 (dir)
    
    LATDCLR = BIT_7; 
    
    PORTSetPinsDigitalIn(IOPORT_B, BIT_9 | BIT_10 ); //for RD9 and 10 (tachometer inputs)
    
    //dont have to setup enable pin
    
	//interrupt setup:
    INTEnableSystemMultiVectoredInt(); 
    INTEnableInterrupts(); 
}

/*
This function should include all the code required to initialize the PWM output to a specific duty cycle
(a number in the range [0, 100]), as well as set the PWM cycle frequency (specified in Hz). For this lab,
the PWM should be initialized with a 40% duty cycle and a cycle frequency of 1 kHz. The function
should return zero on success, or a non-zero value otherwise.
*/
int pwm_init( int dutyCycle, int cycleFrequency) 
{
	//if duty cycle isnt 0 to 100:
	if( dutyCycle < 0 || 100 < dutyCycle )
	{
		return 1; //failure
	}
	//if cycle freq is negative:
	else if( cycleFrequency <= 0 )
	{
		return -1; //failure
	}
	
    //supposed to multiply cycle freq by timer 2 prescaler, but throws error
	unsigned int pr2 = ( FPB / cycleFrequency ) - 1; //should be 9999 for first run
	unsigned int oc3r = dutyCycle * ( pr2 + 1 ) / 100;
	
	//not necessary: ?
	mOC3ClearIntFlag(); // Clear output compare interrupt flag (not using this interrupt)
	
	//open timer2:
	OpenTimer2( ( T2_ON | T2_SOURCE_INT | T2_PS_1_1 ) , pr2 );
    
	
	//open OC3R: (settings), (OCxRS), (OCxR)
	OpenOC3( ( OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE ), oc3r, oc3r ) ;
	
	//sets duty cycle (OC3R) and cycle freq (PR2) 
	
	return 0; //success
}

/*
This function should set the PWM duty cycle to the value specified by dutyCycle, where dutyCycle
is a number in the range [0, 100] representing the percent duty cycle the PWM should be set to on the
next PWM cycle. The function returns zero on success and non-zero otherwise
*/
int pwm_set(int dutyCycle)  //this neads access to PR2 to set OC3R
{ 
	//if duty cycle isnt 0 to 100:
	if( dutyCycle < 0 || 100 < dutyCycle )
	{
		return 1; //failure
	}
	
	//only needa set once, pr should change:
	//static unsigned int pr2 = ( FPB / CYCLE_FREQ ) - 1;
	
	unsigned int oc3rs = dutyCycle * ( JIM_PR2 + 1 ) / 100; //forgot '/100'
	
	//set new PWM duty cycle:
    SetDCOC3PWM( oc3rs );
    
	return 0; //success
}

//init timer 2 for interrupts:
void t2_intr_init(void)
{
	// set up the timer interrupt with a priority of 2, sub priority 0
	mT2SetIntPriority(2); // Group priority range: 1 to 7
	mT2SetIntSubPriority(0); // Subgroup priority range: 0 to 3
	mT2IntEnable(1); // Enable T2 interrupts
		
	// Global interrupts must enabled to complete the initialization.
}

//ISR for timer2: (triggered once every 1ms bc PR2 val)
void __ISR(_TIMER_2_VECTOR, IPL2) Timer2_ISR(void)
{
	
	LATBINV = LEDA; //toggle LEDA every ms
	
	/*
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
	*/
	
	mT2ClearIntFlag(); //has to be at the very end?
}

/* Initialization of CN peripheral for interrupt level 1 */
void cn_intr_init(void) 
{
	unsigned int dummy; // used to hold PORT read value
	
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
	LATBSET = LEDB; //start of CN ISR
	
	//char wholeCycleMsg[15]; //could be 12, 15 to be safe
	//char cycleMsg1[] = "PWM = "; 
	//char cycleMsg2[] = "%";
	//char dataCycleStr[5]; //could be 4, 5 just in case
	//char dutyCycleStr[5]; //could be 4
	char *errorMsg = "Error Setting PWM";

	unsigned int btns, dir, dutyCycle; //local vars 
    
    //debounce the btns for 20 ms:
    sw_msDelay( DEBOUNCE_TIME );
	
	btns = read_buttons();

	//decode btns into correct duty cycle for PWM:
	decode_buttons( btns, &dir, &dutyCycle );

    int errorCode = pwm_set( dutyCycle );
    
	//set PWM to desired duty cycle:
	if( errorCode != 0 ) //error setting duty cycle
	{
		LCD_puts( errorMsg ); //if error, output
	}
	else //no error in setting duty cycle
	{	
        
		//combine to form LCD message:
        /*
		strcat( wholeCycleMsg, cycleMsg1 );
        itoa( dutyCycle, dutyCycleStr, 10);
		strcat( wholeCycleMsg, dutyCycleStr );
		strcat( wholeCycleMsg, cycleMsg2 );
		*/
        
		//put it on LCD:
		//LCD_puts( wholeCycleMsg );
        //LCD_puts( "new duty cycle set");
	}

	LATBCLR = LEDB; //end of CN ISR

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

//decodes buttons for desired duty cycle
// according to lab handout table:
void decode_buttons( unsigned int buttons, 
			unsigned int *dir, unsigned int *dutyCycle)
{

	switch( buttons ) 
	{
		case BTN1: 
			*dir = CW;
			*dutyCycle = 65;
            LCD_puts("duty cycle: 65");
		break;
		
		case BTN2: 
			*dir = CW;
			*dutyCycle = 80;
            LCD_puts("duty cycle: 80");
		break;

		case BTN1 | BTN2: //both btns pressed 
			*dir = CW;
			*dutyCycle = 95;
			LCD_puts("duty cycle: 95");
		break;

		default: //neither btn pressed, or more than 2 pressed
			*dir = CW;
			*dutyCycle = 40;
			LCD_puts("duty cycle: 40");
		break;
	}
}

//determines the new output code for the stepper motor:
/*
unsigned int sw_fsm( unsigned int dir, unsigned int mode )
{
	enum { S0 = 0, S0_5, S1, S1_5, S2, S2_5, S3, S3_5 }; // Declaration of states 
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
	return sm_code[ presState ]; // Return next state 
} 
*/

//sends the four bit code to the stepper motor IO pins:
/*
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
*/