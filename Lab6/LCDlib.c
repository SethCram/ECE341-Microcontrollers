/******************************** Project 6 ****************************
*
* File: LCDlib.c
* Author name: Seth Cram
* Rev. Date: 10/12/2021
*
* Project Description: 
* 
*
* Notes:
*  Mostly new project
*  Can you apply int bitmask to a char?? yes in C
*  EOL = end of line
*  IR is for busy flag, screen addresses, and setup
*  DR is for writing chars
*  When setting a new screen address, you must add the DDRAM_CNTRL_BIT
*  \r = carriage return 
*	
**********************************************************************/
#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDlib.h"

int main()
{
    
 system_init(); /* Setup system Hardware and SW for this lab. */

 //init the LCD:
 initLCD();

 //test strings to write to LCD display:
 char string1[] = "Does Dr J prefer PIC32 or FPGA??";
 char string2[] = "Answer: \116\145\151\164\150\145\162\041";

 //test: LCD_puts( string1 );
 
 while(1)
 {
	//alternate display of the test strings: (task 1)
	LCD_puts( string1 );

	LCD_delay( 5000 ); //delays 2s, should be 5s for testing

	LCD_puts( string2 );

	LCD_delay( 5000 ); //delays 2s, should be 5s for testing
     
	//repeatedly write 'D' to LCD using "writeLCD()": (task 2)
	//writeLCD( DR, string1[0] );

 }

 return 0; 
}	 

//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
}

//init the PMP for the LCD interface:
void initPMP()
{
	int cfg1 = PMP_ON|PMP_READ_WRITE_EN|PMP_READ_POL_HI|PMP_WRITE_POL_HI;
	int cfg2 = PMP_DATA_BUS_8 | PMP_MODE_MASTER1 |
 			PMP_WAIT_BEG_4 | PMP_WAIT_MID_15 | PMP_WAIT_END_4; //changed because of Dr. J recommendation
	int cfg3 = PMP_PEN_0; // only PMA0 enabled
	int cfg4 = PMP_INT_OFF; // no interrupts used
	mPMPOpen(cfg1, cfg2, cfg3, cfg4);
}

//init the LCD using the PMP:
void initLCD()
{
	//init the PMP:
 	initPMP();

	//dont use 'writeLCD()' here bc it checks busy flag 
	// and busy flag not yet avaliable

	LCD_delay( 50 );

	PMPSetAddress( IR );

	//Set Function = 8 bit data, 2 line display, 5X8 dot font:
	PMPMasterWrite( 0x38 );

	LCD_delay( 50 );

	//Set Display = Display on, Cursor on, Blink Cursor on:
	PMPMasterWrite( 0x0f );

	LCD_delay( 50 );

	//Clear Display: 
	clearLCD();

	LCD_delay( 5 );	//should be 5 (test w/ 500)	
}

//delay some, then clear the LCD display:
void clearLCD()
{   
    PMPSetAddress( IR ); //set to IR
    
    //Clear Display: 
	PMPMasterWrite( 0x01 );
}

//LCD write function using the PMP:
void writeLCD(int addr, char c)
{
 	while(busyLCD()); // Wait for LCD to be ready

 	PMPSetAddress(addr); // Set LCD RS control

 	PMPMasterWrite(c); // initiate write sequence

} // End of writeLCD

//check if the busy flag is set:
int busyLCD() 
{
	char readContents;
	//int busyFlagMask = 0b1000 0000; //mask to just find the busy flag
				          // int bitmask?? w/ char? ye
	int busyFlagMask = 0x80;

	//read the IR to get the busy flag and addy cntr:
	readContents = readLCD( IR );

	//mask away the addy cntr bits:
	readContents = readContents & busyFlagMask;		
    
	//if busy flag not set:
	if( readContents == 0x00 )//== busyFlagMask )
	{
		return 0; //not busy
	}
	else
	{
		return 1; //busy
	}

}

//LCD read function using the PMP:
char readLCD(int addr)
{
 	PMPSetAddress(addr); // Set LCD RS control

 	mPMPMasterReadByte(); // initiate dummy read sequence

 	return mPMPMasterReadByte();// read actual data

} // End of readLCD

//Function to write a text string to the LCD:
void LCD_puts(char *char_string)
{
    //clear LCD before outputting whole string:
    clearLCD();
    
	while(*char_string) // Look for end of string NULL character
	{
		LCD_putc(*char_string); // Write character to LCD

		char_string++; // Increment string pointer
 	}

} //End of LCD_puts

//Write character to LCD screen:
void LCD_putc( char currChar )
{
	char readContents;
	
	//int addyMask = 0111 1111;
	int addyMask = 0x7F; //mask to get rid of msb

	//loop until the LCD cntrller isnt busy:
	while( busyLCD() );

	//GET ADDY:

	//read from the instruction register at curr address:
	readContents = readLCD( IR ); //7 lsb's are addy of cursor
	
	readContents = readContents & addyMask; //store only addy in 'readLCD'

	
	//PAST EOL?:
	
	//if curr addy tween last addy of 1st line and 1st addy of 2nd line:  
	         //if( LINE_1_END < readContents < LINE_2_START ) //cant do in C
    if( (LINE_1_END < readContents) && (readContents < LINE_2_START) )
	{
		//set cursor to start of line 2:	
		writeLCD( IR, (LINE_2_START | DDRAM_CNTRL_BIT) );
	} 

	//if curr addy past last addy of 2nd line:
	if( LINE_2_END < readContents )
	{
		//set cursor to start of line 1:	
		writeLCD( IR, (LINE_1_START | DDRAM_CNTRL_BIT) );
	}

	//CONTROL CHAR?: (check busy flag again before write??) 
	//               (no bc writeLCD() does for us)
	
	switch( currChar ) //use switch for future expansion
	{
		case '\r': //move cursor to beginning of line

			//if cursor on first line:
			if( (LINE_1_START <= readContents) && (readContents < LINE_2_START) )
			{
				//set cursor to start of line 1:
				writeLCD( IR, (LINE_1_START | DDRAM_CNTRL_BIT) );	
			}

			//if cursor on 2nd line:
			if( LINE_2_START <= readContents )
			{
				//set cursor to start of line 2:
				writeLCD( IR, (LINE_2_START | DDRAM_CNTRL_BIT) );
			}

		break;

		case '\n': //move cursor to start of next line
			
			//if cursor on first line:
			if( (LINE_1_START <= readContents) && (readContents < LINE_2_START) )
			{
				//set cursor to start of line 2:
				writeLCD( IR, (LINE_2_START | DDRAM_CNTRL_BIT) );	
			}

			//if cursor on 2nd line:
			if( LINE_2_START <= readContents )
			{
				//set cursor to start of line 1:
				writeLCD( IR, (LINE_1_START | DDRAM_CNTRL_BIT) );
			}

		break;

		default: //normal char
		
			//WRITE CHAR:
			writeLCD( DR, currChar );

		break;
	}	
}

//software delay for the specified number of ms:
void LCD_delay (unsigned int mS)
{
	int i; //for loop cnting variable
 
	while(mS --) // SW Stop breakpoint
	{
		for (i = 0; i< COUNTS_PER_MS; i++) // 1 ms delay loop
		{
			// do nothing
		}

	//LATBINV = LEDA; // Toggle LEDA each ms for instrumentation

	}

} //SW Stop breakpoint

