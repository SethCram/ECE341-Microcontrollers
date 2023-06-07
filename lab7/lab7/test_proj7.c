/******************************** Project 7 ****************************
*
* File: test_proj7.c
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

#include <stdio.h> //for sscanf()
#include <strings.h> //for strcasecmp()

//include all 4 header files for the 4 .c files: (for their functs and interrupts)
#include "Btns_ISR.h"
#include "LCDlib.h" 
#include "SM_ISR.h"
#include "comm.h"

//prototypes:
void system_init();

//global vars:
extern unsigned int step_delay, dir, mode;

int main()
{
	const int LINE_SIZE = 15; //bc "CCW HALF 30" is 11 (over to be safe)
				  // should be 12
	const int DIR_SIZE = 5; //should be 4
	const int MODE_SIZE = 6; //should be 5
	//const int SPEED_SIZE = 4; //should be 3 //dont need

	//UART var:
	unsigned int baud = 19200; //19200 is bit rate and not baud rate?
    
    //seth vars:
    char line[LINE_SIZE]; //to hold max expected string
	char currDir[DIR_SIZE], currMode[MODE_SIZE];
	int currSpeed;

	//INIT:

	system_init(); //init syst from ISR lab

	initialize_uart1(baud, ODD_PARITY); 

	//init the LCD: 
	initLCD();

	//LCD and uart init have to be above cn init bc used in it

	//setup timer1:
 	t1_intr_init();

 	//setup change notice intrs for btn1 and btn2: (step_delay init'd here)
 	cn_intr_init();


	// UART TESTING:

	//Write string to UART (2 methods)
	//Equivalent to printf(‚ÄúHello World\r\n‚Äù); twice 
	//putsU1("Hello World"); // Constant text string
	//char str[32]; // Size of the character array must anticipate
			// longest possible string plus 1
	//sprintf(str, "Hello World"Äù); // Variable str set to ‚ÄúHello World‚Äù
	//putsU1(str); 


	//Read string from UART:
	//char str[20]; // Buffer size set to hold maximum expected
				// number of characters per line plus 1
	//while(!getstrU1(str, sizeof(str))); //store string in str


	//Read words from UART string:
	//The additional space characters are ignored
	//Using the string scanf function:
	//char str[21]; // Buffer size set to hold max expected
		      //  number of chars per line plus NULL terminator
	//int x1, x2;
	//char s1[5], s2[5];
	//while(!getstrU1(str, sizeof(str))); //read UART string into str
	//sscanf(str, "%i %i %s %s"Äù, &x1, &x2, s1, s2);


	while(1) //BG TASKS
	{
		//SETH CODE:
        
		//read UART string
		while(!getstrU1(line, sizeof(line))); 
        
            //putsU1("Echoed String:"); //output to UART (failed formatting)
        
        putsU1("\n"); //jump next line
        
        putsU1(line); //output to UART

		mCNIntEnable(0); //when invoke LCD
        
		//put read-in line on LCD: (clears before outputting)
		LCD_puts(line);
		
		//parse:
		sscanf(line, "%s %s %d", currDir, currMode, &currSpeed);

		//if currDir is CW:
		if( strcasecmp( currDir, "CW" ) == 0 ) //IGNORE THESE ERRORS
		{
			dir = CW; //set global to it
		}
		//if its CCW
		else if( strcasecmp( currDir, "CCW" ) == 0 ) 
		{
			dir = CCW; //set global to it
		}

		//if currMode is HALF:
		else if( strcasecmp( currMode, "HALF" ) == 0 )
		{
			mode = HS; //set global to it
		}
		//if currMode is FULL:
		else if( strcasecmp( currMode, "FULL" ) == 0 )
		{
			mode = FS; //set global to it
		}

		//if currSpeed in the valid range:
		if ( 1 <= currSpeed && currSpeed <= 30 )
		{
			//set global to calculated step_delay:
			step_delay = ComputeStepDelay( mode, currSpeed );
		}

		mCNIntEnable(1); //when finish invoking LCD
 
	}


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
