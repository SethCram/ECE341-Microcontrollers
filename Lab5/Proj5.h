/********************** Project 5 *********************************
*
* File: Proj5.h
* Author: Seth Cram
* Date: 10/05/2021
*
*/

//stepper motor constants:
#define HS 1
#define FS 2
#define CW 1
#define CCW 2

//Timer1 consts:
#define T1_PRESCALE 1
#define TOGGLES_PER_SEC 1000
#define T1_TICK (FPB/T1_PRESCALE/TOGGLES_PER_SEC) //FPB = periph bus clk

/* Software timer const */
#define COUNTS_PER_MS 8889 /* Exact value is to be determined */

//Debounce btns const:
#define DEBOUNCE_TIME 20 //debounce btns for 20ms

//global variables:
unsigned int step_delay, dir, mode;

/* Function Prototypes */
void system_init (void); /* hardware initialization */

void t1_intr_init(void);

void cn_intr_init(void); 

int read_buttons(void);

void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode );

unsigned int sw_fsm( unsigned int dir, unsigned int mode );

void output_sm_code( unsigned int sm_code );

void sw_msDelay(unsigned int mS); 