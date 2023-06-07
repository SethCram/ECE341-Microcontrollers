/********************** Project 4 *********************************
*
* File: Proj4.h
* Author: Seth Cram
* Date: 9/28/2021
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
#define T1_TICK (FPB/T1_PRESCALE/TOGGLES_PER_SEC) //what's FPB?

/* Function Prototypes */
void system_init (void); /* hardware initialization */

int read_buttons(void);

void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode );

unsigned int sw_fsm( unsigned int dir, unsigned int mode );

void output_sm_code( unsigned int sm_code );

void Timer1_delay( unsigned int *btn_cntr, unsigned int *motor_cntr);