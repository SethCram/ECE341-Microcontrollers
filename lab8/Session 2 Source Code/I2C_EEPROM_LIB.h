/******************************** Project 8 ****************************
*
* File: I2C_EEPROM_LIB.h
* Author name: Seth Cram
* Rev. Date: 11/02/2021
*
* File Description: 
*   Function prototypes and macros needed for I2C_EEPROM_LIB.c
*   and to use the I2C_EEPROM library.
* 
**********************************************************************/

//PROTOTYPES:

void system_init(void); //Sets up the hardware

void init_I2C2(int SCK_FREQ); //Initializes the I2C bus and the EEPROM

//Read len bytes from the EEPROM into i2c_byte:
int eeprom_read( int slaveAddy, int mem_addr, char *i2c_byte, int len );

//Write len bytes to the EEPROM from i2c_data:
int eeprom_write( int slaveAddy, int mem_addr, char *i2c_data, int len );

//blocking function that waits until the slave device has completed its operation:
int wait_i2c_xfer(int slaveAddy);

void I2C2_Start_Condition();

void I2C2_Stop_Condition();

void I2C2_Nack();

void I2C2_Ack();

void I2C2_Restart();

//MACROS:

//#define EEPROM_I2C_ADDY 0b10100000 //bc only have 1 slave dev attached (after shift)
#define EEPROM_I2C_ADDY 0x50 //cntrl byte shifted right once 

#define SLAVE_SHIFT 1 //however much we shift slave addy left by

#define PAGE_LENGTH 64 //bc each page is 64 bytes, 512 bits, which is 2^6 (max offset in a page)

//#define PAGE_OFFSET_BITS 0x003F //6 lsb
//#define PAGE_NUMBER_BITS 0xFFC0 //9 msb

#define SHIFT_BYTE 8 //amt to shift a byte to switch it to the MSB or LSB
#define SHIFT_PAGE_NUMS 6 //amt to shift page numbers over to get rid of offset

//#define MAX_PAGES 512 //not 1024 bc msb of upper mem addy not used (2^9 not 2^10)
#define MAX_BYTES 32768 //this is 2^15 bc the 16th bit of addy not used

#define Fsck 400000 //desired baud rate

//#define BRG_VAL ( ( FPB / 2 / Fsck ) - 2 ) //speed to open I2C with

#define WRITE 0

#define READ 1

#define LOWER_BYTE 0x00FF
//#define UPPER_BYTE 0xFF00

//error codes:
#define NO_ERR 0
#define TIMEOUT_ERR 2
#define WRITE_ERR 4
#define NULL_PNTR_ERR 8
#define ARRAY_SMALL_ERR 16
#define FAULTY_LEN_ERR 32
#define MEM_UPPER_BOUNDS_ERR 64

