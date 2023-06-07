/******************************** Project 8 ****************************
*
* File: I2C_EEPROM_LIB.c
* Author name: Seth Cram
* Rev. Date: 11/02/2021
*
* File Description: 
*   Source code for writing an arbitrary number of bytes to a specified EEPROM from the PIC32. 
*   Essentially, an EEPROM device driver. 
*   Tested in test_proj8.c.
*
* Notes:
*	OpenI2C2() = brg computed value for the baud rate generator. The value is 
*				  calculated as follws: BRG = (Fpb / 2 / baudrate) - 2
*       Always have to idle after a stop, start, restart, ack, or nack
*
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

#include <plib.h> //peripheral lib for PIC32 functs
#include "CerebotMX7cK.h" //to setup cerebot board
#include "I2C_EEPROM_LIB.h" //macros and prototypes

//Sets up the configuration:
void system_init(void)
{
	// Setup processor board
	Cerebot_mx7cK_setup();
}

//Initializes the I2C bus and the EEPROM such that after this function is called, the user of the
// library can immediately begin using the read and write functions
// Initializes the I2C2 port for the requested speed (baud rate):
void init_I2C2( int Sck_Freq ) 
{
	//open I2C port 2 by enabling it and giving it the baud rate
	//if macro: OpenI2C2( I2C_EN, BRG_VAL);
	OpenI2C2( I2C_EN, ( FPB / 2 / Sck_Freq ) - 2 );
}

//simply looping a single byte read or a single byte write will not suffice for the
// arbitrary length read and write functions

/*
Read len bytes from the EEPROM starting from EEPROM memory address mem addr into the
	buffer pointed to by i2c_byte.
Return zero on success or a non-zero value if there is an error. Examples of errors might include
	an invalid memory address passed into the function, a NULL pointer for i2c_byte, an invalid len
	argument, or an I2C bus error. Each error handled should return a different non-zero value.
The function will not return until either len bytes were read into i2c_byte or there was error.
*/
int eeprom_read( int slaveAddy, int mem_addr, char *i2c_byte, int len )
{
	unsigned int index = 0;
	unsigned char write_err = 0;
	unsigned int upper_addr = mem_addr >> SHIFT_BYTE; //shift MSB to LSB
	unsigned int lower_addr = mem_addr & 0x00FF; //mask off upper byte
	unsigned int memAddyCntr = mem_addr;
	
    
	//check if mem addr valid:
    if( memAddyCntr > MAX_BYTES )
	{
		//cant address it, so error
		return MEM_UPPER_BOUNDS_ERR; //mem addy too high
	}
	
	if( len < 1 )
	{
		return FAULTY_LEN_ERR; //faulty length error
	}
	
	if( i2c_byte == NULL )
	{
		return NULL_PNTR_ERR; //Null pntr error
	}
	
	I2C2_Start_Condition();

	//write cntrl byte:
	write_err |= MasterWriteI2C2( ( slaveAddy << SLAVE_SHIFT ) | WRITE );
	
	//write mem addy:
	write_err |= MasterWriteI2C2( upper_addr );
	write_err |= MasterWriteI2C2( lower_addr );
	
	I2C2_Restart();
	
	//initiate read from EEPROM:
	write_err |= MasterWriteI2C2( ( slaveAddy << SLAVE_SHIFT ) | READ ); //1 for read
	
	//while len != 0, decrement len then run code:
	while( len-- )
	{
        
		//if curr loc in array is NULL:
		if( i2c_byte[ len ] == NULL ) //BREAKPOINT
		{
			I2C2_Nack(); //end read with a NACK

			I2C2_Stop_Condition(); 		

			return ARRAY_SMALL_ERR | write_err; // array too small error
		}
		
		i2c_byte[ index++ ] = MasterReadI2C2(); //read a byte from I2C
	
		memAddyCntr++; //incr mem addy cntr
        
		//if next byte written is out of bounds:
        if( memAddyCntr > MAX_BYTES )
		{
			//stop reading and ret
			
			I2C2_Nack();

			I2C2_Stop_Condition();

			return MEM_UPPER_BOUNDS_ERR | write_err; //mem addy too high 
		}
        
		//if len not 0:
		if( len != 0)
		{
			//ack to keep reading bytes
			I2C2_Ack(); 
		}
		
		//dont need to take into account if reached end of page
	}
	
	I2C2_Nack();

	I2C2_Stop_Condition();
	
	//if write_err != 0:
	if( write_err ) //BREAKPOINT
	{
		return write_err; //problem during write
	}
	
	return NO_ERR; //no errors
	
}

/*
Write len bytes into the EEPROM starting at memory address mem addr from the buffer i2c_data.
The function should not return until either the write is complete or an error has occurred. 
So, poll the device until it isn't busy anymore before returning from the function.
*/	
int eeprom_write( int slaveAddy, int mem_addr, char *i2c_data, int len)
{
	
	unsigned int index = 0;
	unsigned char write_err = 0;
	unsigned char poll_err = 0;
	unsigned int upper_addr = mem_addr >> SHIFT_BYTE; //move upper byte to lower byte to write
	unsigned int lower_addr = mem_addr & LOWER_BYTE; //mask off upper byte
	
	unsigned int memAddyCntr = mem_addr;

	unsigned int page_number = mem_addr >> SHIFT_PAGE_NUMS; //shift over page nums 
	
	//check if mem addr valid:
    if( memAddyCntr > MAX_BYTES )
	{
		//cant address it, so error
		return MEM_UPPER_BOUNDS_ERR; //mem addy too high
	}
	
	if( len < 1 )
	{
		return FAULTY_LEN_ERR; //faulty length error
	}
	
	if( i2c_data == NULL )
	{
		return NULL_PNTR_ERR; //Null pntr error
	}

	I2C2_Start_Condition(); //start writing
	
	//write cntrl byte:
	write_err |= MasterWriteI2C2( ( slaveAddy << SLAVE_SHIFT ) | WRITE );
	
	//write mem addy:
	write_err |= MasterWriteI2C2( upper_addr );
        write_err |= MasterWriteI2C2( lower_addr );
	
	//write data:
	while( len-- )
	{
		//if curr loc in array is NULL:
		if( i2c_data[ index ] == NULL )
		{
			I2C2_Stop_Condition();
			
			//poll until data committed to mem:
			poll_err = wait_i2c_xfer( slaveAddy );

			// array too small error and possible polling/write error:
			return (ARRAY_SMALL_ERR | poll_err | write_err); //ERRORS here??
		}

		//write data out, incr to next loc:
		write_err |= MasterWriteI2C2( i2c_data[ index++ ] );

		memAddyCntr++; //incr mem cntr to next loc

		//if mem addy crossed over into another page:
		if( memAddyCntr % PAGE_LENGTH == 0 )
		{
			//dont have to write a byte here before committing
			// data to mem bc mem addy's start at 0, so last
			// byte already committed to page latches

			//stop transfer
			I2C2_Stop_Condition();
		
			//poll until data committed to mem:
			poll_err = wait_i2c_xfer( slaveAddy );

			//have to rewrite write cntrl byte + new mem loc?
			//yes bc beginning on new data transfer
			// (mem loc not 100% required)

			//if next byte written is out of bounds:
            if( memAddyCntr > MAX_BYTES )
			{
				return (MEM_UPPER_BOUNDS_ERR | poll_err | write_err); //mem addy too high BREAKPOINT
			}

			I2C2_Start_Condition();
	
			//write cntrl byte:
			write_err |= MasterWriteI2C2( ( slaveAddy << SLAVE_SHIFT ) | WRITE );
	
			//write mem addy:
			write_err |= MasterWriteI2C2( memAddyCntr >> SHIFT_BYTE ); //upper byte
        	write_err |= MasterWriteI2C2( memAddyCntr & LOWER_BYTE ); //lower byte
		}	
	}
	
	I2C2_Stop_Condition();
	
	//poll EEPROM for Write Completion:
	poll_err = wait_i2c_xfer( slaveAddy );
	
	//if problem polling or writing: 
	if( write_err || poll_err) //BREAKPOINT 
	{
		return (write_err | poll_err); //problem during write/polling
	}
	
	return NO_ERR; //no errors

}

/*
	This is a blocking function until the slave device has completed 
         the operation.
	Used by I2CWriteEEPROM to determine if the device has completed the
         internal page write.
	Also a helper funct.
*/
int wait_i2c_xfer( int slaveAddy )
{
	const unsigned int maxWrites = 1000;
	unsigned int currWrite = 0;	

	//poll EEPROM for Write Completion:
	I2C2_Start_Condition();
	
	//Write 1st data byte (cntrl byte):
	while( MasterWriteI2C2( ( slaveAddy << SLAVE_SHIFT ) | WRITE ) ) //rets 0 if no collision
	{
		//if write collision:

		//if too many writes attempted:
		if( currWrite++ >= maxWrites )
		{
			//timed out, so write complete
			I2C2_Stop_Condition();

			return TIMEOUT_ERR; //timeout error	
		}
		
		//try restart if no ack
		I2C2_Restart();
	}
	
	//ACK received, so write complete
	I2C2_Stop_Condition();

	return NO_ERR; //success
}

//MICRO HELPER FUNCTS:

void I2C2_Start_Condition()
{
	StartI2C2();
	IdleI2C2();
}

void I2C2_Stop_Condition()
{
	StopI2C2(); //ACK received, so write complete
	IdleI2C2();
}

void I2C2_Nack()
{
	NotAckI2C2(); 
	IdleI2C2(); //idle after nacks too
}

void I2C2_Ack()
{
	AckI2C2(); 
	IdleI2C2(); //idle for next read 
}

void I2C2_Restart()
{
	RestartI2C2(); 
	IdleI2C2();
}