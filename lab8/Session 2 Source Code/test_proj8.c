/******************************** Project 8 ****************************
*
* File: test_proj8.c
* Author name: Seth Cram
* Rev. Date: 11/02/2021
*
* Project Description: 
* 	single byte read and single byte write 
*
* Notes:
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "I2C_EEPROM_LIB.h"

#include "LCDlib.h" //needed for LCD_puts()

//macros:
#define TEST_VAL 0x67 //trivial num
//#define LEN 200 // used as a macro for IDE debugging purposes

//prototypes:
void system_init();

    int main(){
	
	//main testing:
	//const unsigned int LEN = 256;
	//const unsigned int mem_addr = 0x0341;
	//const unsigned int mem_addr = 0x8012
	//const unsigned int mem_addr = 0x9464 //should throw upper bounds error
    
    //page boundary testing:
    //const unsigned int mem_addr = 0x0FFE; //should cross page boundary
    ///const unsigned int LEN = 3;
    
    //final test:
    //const unsigned int mem_addr = 0x04FB;
    //const unsigned int LEN = 200;
    
    //max byte write:
    const unsigned int mem_addr = 0x0000; //page offset is zero, so is page number
    const unsigned int LEN = 32768;

	char success[] = "TEST PASSED"; //dont do as pntrs if initialized
	char failure[] = "TEST FAILED";

	unsigned char i2c_data[ LEN ];
    unsigned char i2c_byte[ LEN ]; //store enough data to be returned
	unsigned int inCorrectEntries = 0; //0 for same data, non-zero for dif data

	//for returned errors:
	unsigned int write_err = 0;
	char writeErrStr[] = "WRITE ERROR";
	unsigned int read_err = 0;
	char readErrStr[] = "READ ERROR";
    
    int i; //cnting variable
    
    //timing vars:
    unsigned int startTick, elapsedTicks;
    double elapsedTime; //cant make unsigned 
	
	system_init();
	
	init_I2C2( Fsck ); //inits I2C

	initLCD();

	//fill array to write out:
	for( i = 0; i < LEN; i++ )
	{
		//fill each elly w/ test val:
		i2c_data[ i ] = TEST_VAL;
	}
	
    startTick = ReadCoreTimer();
    
	//Write 1 byte from i2c_data to EEPROM at mem_addr:
	write_err = eeprom_write( EEPROM_I2C_ADDY, mem_addr, i2c_data, LEN ); //pass MSB, LSB of addy, read 1 byte into i2c_data 
	
    elapsedTicks = ReadCoreTimer() - startTick; //elapsed time is in ticks
    
    elapsedTime = ( (double) elapsedTicks ) / ( (double) CORE_MS_TICK_RATE ); //elapsed time in ms
    
    startTick = ReadCoreTimer(); //new start tick for reading
    
	//read 1 byte from i2c_data at mem_addr:
	read_err = eeprom_read( EEPROM_I2C_ADDY, mem_addr, i2c_byte, LEN ); //read 1 byte in 
    
    elapsedTicks = ReadCoreTimer() - startTick; //elapsed time is in ticks
    
    elapsedTime = ( (double) elapsedTicks ) / ( (double) CORE_MS_TICK_RATE ); //elapsed time in ms
	
	//output to LCD
    
	//check array if any bytes not the same tween read+write:
	for( i = 0; i < LEN; i++ )
	{
		//if read in byte not same as og value:
		if( i2c_byte[ i ] != TEST_VAL )
		{
			//Data is different
			inCorrectEntries++;	
		}
	}

	//if write error:
	if( write_err )
	{
		LCD_puts( writeErrStr); //could also output error code
	}

	//if read error:
	if( read_err )
	{
		LCD_delay( 1000 ); //delay for half a second

		LCD_puts( readErrStr );	//could also output error code
	}
    
    LCD_delay( 1000 ); //delay for half a second
    
     if( inCorrectEntries == 0 ) //if same //END BREAKPOINT
    {
		LCD_puts( success );
	}
	else //if different
	{
		LCD_puts( failure );
	}
    
    //could also ouput number of incorrect entries
    
	while(1); //wait here
	
	return 1; //never reached
}