/******************************** Project 8 ****************************
*
* File: I2C_EEPROM_LIB.c
* Author name: Seth Cram
* Rev. Date: 11/02/2021
*
* Project Description: 
* 
*
* Notes:
*	OpenI2C2() = brg computed value for the baud rate generator. The value is 
*				  calculated as follws: BRG = (Fpb / 2 / baudrate) - 2
**********************************************************************/

#include <plib.h>
#include "CerebotMX7cK.h"
#include "I2C_EEPROM_LIB.h"

//#include "LCDlib.h" //needed for LCD_puts()

/*
//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
}
*/

//Initialize the I2C bus and the EEPROM such that after this function is called, the user of your
// library can immediately begin using the read and write functions
void init_EEPROM()
{
	//init_I2C2(); //init I2C bus
	
	//anything else?
}

//Initializes the I2C2 port for the requested speed (baud rate):
void init_I2C2( int SCK_FREQ ) 
{
	//open I2C port 2 by enabling it and giving it the baud rate
	//OpenI2C2( I2C_EN, BRG_VAL);
	OpenI2C2( I2C_EN, ( FPB / 2 / SCK_FREQ ) - 2 );
}

//simply looping a single byte read or a single byte write will not suffice for the
// arbitrary length read and write functions

/*
– Read len bytes from the EEPROM starting from EEPROM memory address mem addr into the
	buffer pointed to by i2cdata.
– Return zero on success or a non-zero value if there is an error. Examples of errors might include
	an invalid memory address passed into the function, a NULL pointer for i2cData, an invalid len
	argument, or an I2C bus error. Each error you handle should return a different non-zero value.
– The function will not return until either len bytes were read into i2cData or there was error.
*/
int eeprom_read(int mem_addr, char *i2c_data, int len)
{
	int index = 0;
	unsigned char write_err = 0;
	int upper_addr = mem_addr & 0xFF00; //mask off lower byte
	int lower_addr = mem_addr & 0x00FF; //mask off upper byte
	
	//check if mem addr valid (how?)
	
	if( len < 1 )
	{
		return -1; //faulty length error
	}
	
	if( i2c_data == NULL )
	{
		return 1; //Null pntr error
	}
	
	StartI2C2(); //start condition
	IdleI2C2(); //idle for write

	//write cntrl byte:
	write_err |= MasterWriteI2C2( ( EEPROM_I2C_ADDY << 1 ) | WRITE );
	
	//write mem addy:
	write_err |= MasterWriteI2C2( upper_addr );
	write_err |= MasterWriteI2C2( lower_addr );
	
	RestartI2C2(); //reverse I2c Bus Dir to read
	IdleI2C2(); //idle for writing a read?
	
	//initiate read from EEPROM:
	MasterWriteI2C2( ( EEPROM_I2C_ADDY << 1 ) | READ ); //1 for read
	
	while( len-- )
	{
		if( i2c_data[ len ] == NULL )
		{
			return 2; // array too small error
		}
		
		i2c_data[ index++ ] = MasterReadI2C2(); //read a byte from I2C
		
		AckI2C2(); //ack to keep reading bytes
		
		IdleI2C2(); //idle for next read or NACK
		
		//need to take into account if reached end of page
	}
	
	//will ack then NACK cause error?
	
	NotAckI2C2(); //end read with a NACK
	
	IdleI2C2(); //idle to stop
	StopI2C2(); //stop condition
	IdleI2C2(); //idle for readiness
	
	//if write_err != 0:
	if( write_err )
	{
		return 3; //problem during write
	}
	
	return 0; //no errors
	
}

//int single_eeprom_read( 

/*
– Write len bytes into the EEPROM starting at memory address mem addr from the buffer i2cData.
– The function should not return until either the write is complete or an error has occurred. 
	hint: use a function like int wait i2c xfer(int SlaveAddress) to poll the device until it isn’t busy anymore before
		returning from the function
	See the eeprom read specification for examples of errors.
*/	
int eeprom_write( int mem_addr, char *i2c_data, int len)
{
	//check if mem addr valid (how?)
	
	int index = 0;
	unsigned char write_err = 0;
	int upper_addr = mem_addr & 0xFF00; //mask off lower byte
	int lower_addr = mem_addr & 0x00FF; //mask off upper byte
	
	if( len < 1 )
	{
		return -1; //faulty length error
	}
	
	if( i2c_data == NULL )
	{
		return 1; //Null pntr error
	}
	
	//write cntrl byte:
	write_err |= MasterWriteI2C2( ( EEPROM_I2C_ADDY << 1 ) | WRITE );
	
	//write mem addy:
	write_err |= MasterWriteI2C2( mem_addr );
	
	//write data:
	while( len-- )
	{
		if( i2c_data[ index ] == NULL )
		{
			return 2; // array too small error
		}
		
		write_err |= MasterWriteI2C2( i2c_data[ index++ ] );
	}
	
	StopI2C2();
	IdleI2C2();
	
	//if write_err != 0:
	if( write_err )
	{
		return 1; //problem during write
	}
	
	//poll EEPROM for Write Completion:
	StartI2C2();
	IdleI2C2();
	
	wait_i2c_xfer();
	
	//if write_err != 0:
	if( write_err )
	{
		return 3; //problem during write
	}
	
	return 0; //no errors

}

//This is a blocking function until the slave device has completed the operation.
int wait_i2c_xfer()
{
	//Used by I2CWriteEEPROM to determine if the
	// device has completed the internal page write.
	
	//poll EEPROM for Write Completion:
	StartI2C2();
	IdleI2C2();
	
	//Write 1st data byte (cntrl byte):
	while( MasterWriteI2C2( ( EEPROM_I2C_ADDY << 1 ) | WRITE ) ) //rets 0 if no collision
	{
		//if write collision:
		
		RestartI2C2(); //try restart if no ack
		IdleI2C2();
	}
	
	StopI2C2(); //ACK received, so write complete
	IdleI2C2();
}

//check if slave is busy: (non-blocking)
char BusyI2C2(void) //rets non-zero if I2C Cntrllr busy 
{
	return( I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN ||
			I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT );
}