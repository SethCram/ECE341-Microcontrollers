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
*  RestartI2C2() = generates I2C Bus Restart condition.
*  StartI2C2() = Generates I2C Bus Start condition
*  IdleI2C2() = This function generates Wait condition until I2C bus is Idle.
*			  - blocking function till I2C bus idle
*  			  - This function will be in a wait state until Start Condition Enable bit, Stop 
*				 Condition Enable bit, Receive Enable bit, Acknowledge Sequence 
*				 Enable bit of I2C Control register and Transmit Status bit I
*				 2C Status register are clear.
* 			  - I2C peripheral must be in Idle state before an I
*				 2C operation can be initiated 
*				 or write collision will be generated.
*  StopI2C2() = Generates I2C Bus Stop condition
*  NotAckI2C2() = Generates I2C bus Not Acknowledge condition
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "I2C_EEPROM_LIB.h"

#include "LCDlib.h" //needed for LCD_puts()

//macros:
#define TEST_VAL 0x67 //trivial #

//prototypes:
void system_init();

int main()
{
	unsigned char SlaveAddy, i2c_byte, write_err = 0;
	char i2c_data[10] = {0,0,0,0,0,0,0,0,0,0};
	int datasz, index; 
	
	char success[] = "TEST PASSED"; //dont do as pntrs if init
	char failure[] = "TEST FAILED";
	//int mem_addr = 0x0341;
	//char i2c_data[] = TEST_VAL;
	
	system_init();
	
	init_I2C2( Fsck ); //inits I2C

    /*
	//if I2C for port 2 is busy:
	if( BusyI2C2() )
	{
		return 1; //I2C is busy
	}
     */
	initLCD();
	
	//Write 1 byte from i2c_data to EEPROM at mem_addr:
	//eeprom_write( mem_addr, i2c_data, 1 ); //pass MSB, LSB of addy, read 1 byte into i2c_data 
	
	//how to combo, then decode LSB and MSB of addy?
	
	//read 1 byte from i2c_data at mem_addr:
	//eeprom_read( mem_addr, i2c_data, 1 ); //read 1 byte in 
	
	//ouput to LCD
	
	//Create I2C Frame:
	SlaveAddy = 0x50; //SlaveAddy = 0b 0101 0000, should be 0b 1010 0000
	i2c_data[0] = ( ( SlaveAddy << 1 ) | 0 ); //cntrl byte, 0 for write
	i2c_data[1] = 0x03; // Mem addy (MSB)
	i2c_data[2] = 0x41; // Mem addy (LSB)
	i2c_data[3] = TEST_VAL; // single data byte 
							//  true bc a byte is 8 bits
    //i2c_data[3] = 0x10; //false data
	
    //WRITE:
    
	datasz = 4;
	index = 0;
	StartI2C2(); //non-blocking //START BREAKPOINT
	IdleI2C2(); //blocking
	
	//write the first 4 bytes of i2c_data (only last byte actually written):
	while( datasz-- )
	{
		write_err |= MasterWriteI2C2( i2c_data[ index++ ] );
	}
	
	StopI2C2();
	IdleI2C2();
	
	//if write_err != 0:
	if( write_err )
	{
		return 3; //problem during write
	}
	
	//poll EEPROM for Write Completion:
	StartI2C2();
	IdleI2C2();
	
	//Write 1st data byte (cntrl byte):
	while( MasterWriteI2C2( i2c_data[ 0 ] ) ) //rets 0 if no collision
	{
		//if write collision:
		
		RestartI2C2(); //try restart if no ack
		IdleI2C2();
	}
	
	StopI2C2(); //ACK received, so write complete
	IdleI2C2();
	
	//Read from EEPROM:
	datasz = 3;
	index = 0;
	
	StartI2C2();
	IdleI2C2();
	
	//write cntrl byte and mem addy bytes:
	while( datasz-- )
	{
		MasterWriteI2C2( i2c_data[ index++ ] ); //didnt check ack?
	}
	
	RestartI2C2(); //reverse I2c Bus Dir to read
	IdleI2C2();
	
	//initiate read from EEPROM:
	MasterWriteI2C2( ( SlaveAddy << 1 ) | 1 ); //1 for read
	
	i2c_byte = MasterReadI2C2(); //read a byte from I2C
	
	NotAckI2C2(); //end read with a NACK
	
	IdleI2C2();
	StopI2C2();
	IdleI2C2();
	
	//compare written data byte w/ read data byte:
	if( i2c_byte == TEST_VAL ) //if same //END BREAKPOINT
	{
		//PORTSetBits( IOPORT_G, LED4 ); //set LED4 high
		
		LCD_puts( success );
	}
	else //if different
	{
		//PORTSetBits( IOPORT_G, LED1 ); //set LED1 high
		
		LCD_puts( failure );
	}
	
	while(1); //wait here
	
	return 1; //never reached
}

//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
}