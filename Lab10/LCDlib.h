/********************** Project 7 *********************************
*
* File: LCDlib.h
* Author: Seth Cram
* Date: 10/19/2021
*
*/

//?should turn macros into local consts?

/* Software timer const */
#define COUNTS_PER_MS 8889 

//consts for data and instr register:
#define IR 0
#define DR 1

//LCD screen consts:
#define LINE_1_START 0x00
#define LINE_1_END 0x0F
#define LINE_2_START 0x40
#define LINE_2_END 0x4F

//const for changing the LCD address/cursor location:
#define DDRAM_CNTRL_BIT 0x80

/* Function Prototypes */
//void system_init (void); /* hardware initialization */

void initPMP(); //called by initLCD()

void initLCD();

void clearLCD();

int busyLCD();

void writeLCD(int addr, char c);

char readLCD(int addr);

void LCD_puts(char *char_string);

void LCD_putc( char currChar );

void LCD_delay(unsigned int mS); 

int ChangeCursorLoc( int lineNum );

int ClearLCDline( int lineNum );