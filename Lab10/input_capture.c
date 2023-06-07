/******************************** Project 10 ****************************
*
* File: input_capture.c
* Author name: Seth Cram
* Rev. Date: 11/30/2021
*
* Project Objective: 
* 	Implement a motor speed tachometer by measuring the
*         frequency of the motor shaft sensor
*
* Notes:
*
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "input_capture.h"
#include "LCDlib.h" //needed for LCD_puts()

//to pass the revs per sec out of OC ISR:
float rps = 0;

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
    
    
    //setup for lab10:
    
    PORTSetPinsDigitalOut(IOPORT_D, BIT_7 ); //for RD1 (dir)
    
    LATDCLR = BIT_7; 
    
    //PORTSetPinsDigitalIn(IOPORT_D, BIT_9 | BIT_10 ); //for RD3 and 12 (tachometer inputs)
    
    //dont have to setup enable pin
    
	//interrupt setup:
    INTEnableSystemMultiVectoredInt(); 
    INTEnableInterrupts(); 
}

//init input capture module
int ic_init()
{
	//for hall effect sensors:
	const unsigned int MTR_SA = BIT_9; //bit 3?
	const unsigned int MTR_SB = BIT_10; //bit 12?

	//Hall effect sensors SA (RD3) and SB (RD12) as inputs:
	PORTSetPinsDigitalIn( IOPORT_D, (MTR_SA | MTR_SB) );	

	//clear input capture intrs:
	mIC5ClearIntFlag();

	//open ouput compare module:
	OpenCapture5( IC_ON | IC_IDLE_STOP | IC_FEDGE_FALL | IC_CAP_16BIT 
			| IC_TIMER3_SRC | IC_INT_1CAPTURE  
			| IC_EVERY_FALL_EDGE); 

	//config input capture intr:
	ConfigIntCapture5( IC_INT_ON | IC_INT_PRIOR_3 | IC_INT_SUB_PRIOR_0 );

	return 0; //success
}

//input capture ISR:
void __ISR( _INPUT_CAPTURE_5_VECTOR, ipl3 ) Capture5_ISR(void)
{
	//given vars:
	static unsigned short t_old = 0; //hold prev val of timer
	unsigned int fifo_vals[4]; // 4 vals max in fifo to read out
	unsigned short t_new; //bc 16-bit timer so short (need as static?)
	unsigned short t_diff; //need as static?
	float speed;
	//seth's vars:
	static unsigned int captureNum = 0; //cnting var for curr capture
	static unsigned int index = 0; //cnting var for array
	static const int minCaps = 16; //min caps to start averaging
	static unsigned short int arrTdiff[ 16 ]; //array to store periods (minCaps)
	//static unsigned short int *oldestTdiff = arrTdiff[ 0 ]; //pntr to oldest t_diff (dont need?)

	LATBINV = LEDD; //toggle LEDD

	//read fifo using fifo_vals:
	ReadCapture5( fifo_vals ); //destructive read 

	t_new = fifo_vals[0]; //save time of event
				//unsigned truncated to unsigned short

	t_diff = t_new - t_old; //time elapsed in ticks

	t_old = t_new; //update prev tick 

	//store t_diff in static array (circular buffer):
	arrTdiff[ index ] = t_diff; 

	//Moving average of 16 t_diff's (store them in an array)
	if( captureNum >= minCaps - 1 ) //file 15 vals
	{
		//Compute motor speed in RPS (revolutions per sec) 
		// (should just be the frequency of the t_diff period)	
		//Use floats so not alot truncation
		//Dont div by zero 	
		//Save as global var (called rps)
		
		int sum = 0; //sum of T_diffs
        int i; //for-loop cnting var

		//loop thru array:
		for( i = 0; i < minCaps; i++) //should be minCaps + 1?
		{
			sum += arrTdiff[ i ]; //sum all Tdiff's	
		}

		float avg = ((float) sum) / ( (float) minCaps); //avg Tdiff, curr in seconds/tick

        //convert avg ticks/s to avg s / ticks: 
        avg = avg * (256.0 / 10000000.0); // (ticks/s) * (s/tick / ticks/s) = s / ticks
                                            // avg and T_diff are both in ticks, need to convert to the time domain for revs per sec
        
		//avg freq of caps: (in floating pnt)
		rps = 1.0 / avg; // ticks/s
        
		//display rps to LCD in main()
	}

    index++; //incr array index
    
	//if index maxed out:
	if( index >= minCaps ) //fill 15 then rollover
	{
		index = 0; //reset array index
	}

	captureNum++; //a capture taken
	
	mIC5ClearIntFlag(); //clear intr flag
}

//init timer 3 for interrupts:
void t3_init(void)
{
	//Configure Timer 3 to use the PBCLK and a pre-scale value of 256. 
	//Set Timer 3 PR3 register to 0xFFFF for maximum interval.
	OpenTimer3( T3_ON | T3_PS_1_256 | T3_SOURCE_INT, 0xFFFF );

	// set up the timer interrupt with a priority of 2, sub priority 0
	mT3SetIntPriority(2); // same as timer 2
	mT3SetIntSubPriority(2); // higher than timer 2
	mT3IntEnable(1); // Enable T3 interrupts
		
	// Global interrupts must enabled to complete the initialization.
}

//ISR for timer3: (triggered once every long time bc PR3 val)
void __ISR(_TIMER_3_VECTOR, IPL2) Timer3_ISR(void) //shouldnt be IPL3?
{
	LATBINV = LEDC; //toggle LEDC every entry

	mT3ClearIntFlag(); 
}