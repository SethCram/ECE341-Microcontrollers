/******************************** Project 7 ****************************
*
* File: Btns_ISR.c
* Author name: Seth Cram
* Rev. Date: 10/19/2021
*
* Project Description: 
* 
*
* Notes:
*	
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "Btns_ISR.h"

#include "comm.h" //needed for putsU1()
#include "LCDlib.h" //needed for LCD_puts()

//use extern to access global variables that are declared in a 
// different C file

extern unsigned int step_delay, dir, mode;

/*
//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
	
	PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);// Set PmodSTEP LEDs A-H outputs 
	
	LATBCLR = SM_LEDS; // Turn off LEDA through LEDH 
	
	//set btn1 and btn2 to digital inputs:
 	PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2);
    
	//interrupt setup:
    	INTEnableSystemMultiVectoredInt(); 
    	INTEnableInterrupts(); 
}
*/

/* Initialization of CN peripheral for interrupt level 1 */
void cn_intr_init(void) 
{
	unsigned int dummy; // used to hold PORT read value
	int btns; //for step_delay init

	//read and decode the btns so 'step_delay' init'd:
 	btns = read_buttons();
 	decode_buttons( btns, &step_delay, &dir, &mode );

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


/* //decode btns w/o LCD or UART communication
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

	//output new SM vars to LCD screen and UART?
}
*/

//decode btns w/ LCD and UART communication:
void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode)
{
//const int LINE_SIZE = 15; //also set in test file (combine?)
//char outputStr[LINE_SIZE]; //store output string
char *outputStr; //have to init size?

	switch( buttons ) 
	{
		case BTN1: 
			*dir = CW;
			*mode = FS;
			*step_delay = 40; //rpm = 15, FS
			outputStr = "CW FULL 15"; 
		break;
		
		case BTN2: 
			*dir = CCW;
			*mode = HS; 
			*step_delay = 30; //rpm = 10, HS
			outputStr = "CWW HALF 10";
		break;

		case BTN1 | BTN2: //both btns pressed 
			*dir = CCW;
			*mode = FS;
			*step_delay = 24; //rpm = 25, FS
			outputStr = "CWW FULL 25";
		break;

		default: //neither btn pressed, or more than 2 pressed
			*dir = CW;
			*mode = HS;
			*step_delay = 20;  //rpm = 15, HS
			outputStr = "CW HALF 15";
		break;
	}
	
    putsU1("\n"); //jump to next line
        //putsU1("Echoed String:"); //output to UART (failed formatting)
	putsU1(outputStr); //output to UART
	LCD_puts(outputStr); //output to LCD on cleared screen
}


//seperate function to compute step delay based on mode and speed:
int ComputeStepDelay( int mode, int speed ) //speed should be in RPMs
					    //FS = 1, HS = 2
{
	int T_delay;
	T_delay = 60000 / (speed * 100 * mode);
	return T_delay;
}

