/************* Project 1 ***************
 * File name:	Project1.c
 *
 * Author:	Richard W. Wall
 * Date:	August 10, 2013    - Tested with MPLab X
 * Notes: 	This project converts the binary encoded buttons to
 *              set one of the four Cerebot MX7cK LEDs.
****************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "Project1.h"

int main()
{
    initialize_system();

    //BTN AND LED INITS:

   //set btn1 and btn2 to digital inputs:
   PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2);

   //if btn 3 was req'd: 
   //DDPCONbits.JTAGEN = 0; // Disable JTAB programming
   //PORTSetPinsDigitalIn(IOPORT_A, BTN3);
	
   //set LED1-LED4 to digital outputs: (clear bc dont know vals)
   PORTSetPinsDigitalOut(IOPORT_G, LED1 | LED2 | LED3 | LED4);
   PORTClearBits(IOPORT_G, LED1 | LED2 | LED3 | LED4);

    while(1)
    {
	//init vars:
	int btns_State;
	int leds_State;
	
	//read and store buttons state:
	btns_State = read_buttons();

	//read and store leds that should be lit based on btns combo:
	leds_State = decode_buttons( btns_State );

	//light leds from led state data:
	control_leds( leds_State );

    }

    return 1;
}

/* initialize_system Function Description *************************************
 * SYNTAX:          void initialize_system();
 * PARAMETER1:      No Parameters
 * KEYWORDS:        initialize
 * DESCRIPTION:     Sets Registers to default (0) to prevent non zero values 
 *                  from popping up.
 * RETURN VALUE:    None
 * END DESCRIPTION ************************************************************/
void initialize_system()
{
    Cerebot_mx7cK_setup(); // Initialize processor board
}

/* read_buttons Function Description *****************************************
 * SYNTAX:          int read_buttons(void);
 * KEYWORDS:        button, read, inputs
 * DESCRIPTION:     Reads the status of the input buttons.  Button status is
 *                  reported for button bit positions only. All other bits in the
 *                  returned value are set to zero as shown below:
 *
 *      Port G Bit position [15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0]
 *      Port G Bit value    [ 0| 0| 0| 0| 0| 0| 0| 0|B2|B1| 0| 0| 0| 0| 0| 0]
 *
 *      B1 will be 1 if BTN1 is pressed otherwise B1 will be zero
 *      B2 will be 1 if BTN2 is pressed otherwise B2 will be zero
 *
 * PARAMETERS:      None
 * RETURN VALUE:    int variable containing button status
 * NOTES:           BTN3 on the Cerebot MX7cK is used in this function.  Adding
 *                  BTN3 detection is an assignment left to the reader.
 *                  Refer to Cerebot MX7cK reference manual.
 * END DESCRIPTION ************************************************************/
int read_buttons(void)
{
	//return if btn1 and btn2 are set or not:
	return PORTReadBits( IOPORT_G, BTN1 | BTN2 );
}

/* decode_buttons Function Description ***************************************
 * SYNTAX:          int decode_buttons(int);
 * KEYWORDS:        button, decode, LEDs
 * DESCRIPTION:     Maps the button status to light one of four LEDS
 *
 *         Port G Bit position [15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0]
 *         Port G Bit value    [L4|L3|L2|L1| X| X| X| X| X| X| X| X| X| X| X| X]
 *
 *         LED1 = ON for L1 = 1, LED1 = OFF for L1 = 0
 *         LED2 = ON for L2 = 1, LED2 = OFF for L2 = 0
 *         LED3 = ON for L3 = 1, LED3 = OFF for L3 = 0
 *         LED4 = ON for L4 = 1, LED4 = OFF for L4 = 0
 *         X = Don't care
 * 
 * PARAMETER1:      button status
 * RETURN VALUE:    LED 1 through 4 ON/OFF control
 * NOTES:           Refer to Cerebot MX7cK data sheet for button and LED bit
 *                  positions
 * END DESCRIPTION ************************************************************/
int decode_buttons(int buttons)
{
	//init vars:
	int button1;
	int button2;

	//store state of individual btns:
	button1 = buttons & BTN1;
	button2 = buttons & BTN2;

	//reset val of button1 and button2 to 1 or 0 deping on whether set or not:
	if( button1 == BTN1)
	{
		button1 = 1;
	}
	else
	{
		button1 = 0;
	}

	if(button2 == BTN2)
	{
		button2 = 1;
	}
	else
	{
		button2 = 0;
	}

	//set 1st LED if btns both off:
	if( button1 == 0 && button2 == 0 )
	{
		return LED1;
	}

	//set 2nd LED if btn1 on and btn2 off:
	if( button1 == 1 && button2 == 0 )
	{
		return LED2;
	}

	//set 3rd LED if btn1 off and btn2 on:
	if( button1 == 0 && button2 == 1 )
	{
		return LED3;
	}

	//set 4th LED if btns both on:
	if( button1 == 1 && button2 == 1 )
	{
		return LED4;
	}
}

/* control_leds Function Description *****************************************
 * SYNTAX:          void control_leds(int leds);
 * KEYWORDS:        led, output, lights
 * DESCRIPTION:     lights the LEDs specified the bit positions in the input
 *                  argument.
 * PARAMETER1:      integer LED1 through LED4 ON/OFF control
 * RETURN VALUE:    None.
 * NOTES:	Only the bit positions related to LED1 through LED4 are modified
 *              by this function. The BRD_LEDS mask constant initially clears
 *              all bit LATG positions associated with LED1 through LED4.
 *              The BRD_LEDs mask is also used to limit the Port G bits that
 *              can be modified to thoes in the BRD_LEDS mask field.
 *              The read-modify-write instruction is not atomic.
 *              Refer to Cerebot MX7cK data sheet for LED bit positions
 * END DESCRIPTION **********************************************************/
void control_leds(int leds)
{
	//light LEDs: (BRD_LEDS mask used to limit the Port G bits set to only LEDs) 
	PORTSetBits( IOPORT_G, leds | BRD_LEDS );

	//are the BRD_LEDS mask all 1's in the LED bit pos's?
}		
	
/* End of Project1.c */
