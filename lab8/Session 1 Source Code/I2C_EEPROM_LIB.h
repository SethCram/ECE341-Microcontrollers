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
*	
**********************************************************************/

//void system_init(void);

void init_EEPROM();

void init_I2C2(int SCK_FREQ); 

int eeprom_read(int mem_addr, char *i2c_data, int len);

int eeprom_write( int mem_addr, char *i2c_data, int len);

int wait_i2c_xfer();
//aren't wait and busy functs the same? 1 blocking & other not?
char BusyI2C2(void);

//#define EEPROM_I2C_ADDY 0b10100000 //bc only have 1 slave dev attached
#define EEPROM_I2C_ADDY 0xA0 //cntrl bit 

#define PAGE_LENGTH 64 //bc each page is 64 bytes, 512 bits

//#define FPB 10000000 //freq of periph bus clk (10MHz) //already def'd elsewhere

#define Fsck 400000 //desired baud rate

//#define BRG_VAL ( ( FPB / 2 / Fsck ) - 2 ) //speed to open I2C with

#define WRITE 0

#define READ 1