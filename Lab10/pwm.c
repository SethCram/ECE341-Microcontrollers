/******************************** Project 10 ****************************
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

/*
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
	
	LATBCLR = LEDAtoD ; // Turn off LEDA through LEDH
    
    
    //setup for next proj:
    
    PORTSetPinsDigitalOut(IOPORT_D, BIT_7 ); //for RD1 (dir)
    
    LATDCLR = BIT_7; 
    
    //dont have to setup enable pin
    
	//interrupt setup:
    INTEnableSystemMultiVectoredInt(); 
    INTEnableInterrupts(); 
}
*/

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
	mT2SetIntSubPriority(1); // is set to 0 in lab 9
	mT2IntEnable(1); // Enable T2 interrupts
		
	// Global interrupts must enabled to complete the initialization.
}

//ISR for timer2: (triggered once every 1ms bc PR2 val)
void __ISR(_TIMER_2_VECTOR, IPL2) Timer2_ISR(void)
{
	
	LATBINV = LEDA; //toggle LEDA every ms
	
	mT2ClearIntFlag(); 
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
	//clear line 1 of LCD:
	ClearLCDline( 1 );

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

	//Should only clear and write line 1 of LCD
	//Dont needa disable CN ISR bc we're in it rn

}