/******************************** Project 7 ****************************
*
* File: SM_ISR.h
* Author name: Seth Cram
* Rev. Date: 10/19/2021
*
* Project Description: 
* 
*
* Notes:
*	
**********************************************************************/

//stepper motor constants:
#define HS 2
#define FS 1
#define CW 1
#define CCW 2

//Timer1 consts:
#define T1_PRESCALE 1
#define TOGGLES_PER_SEC 1000
#define T1_TICK (FPB/T1_PRESCALE/TOGGLES_PER_SEC) //FPB = periph bus clk

//global variables:
unsigned int step_delay, dir, mode;

//PROTOTYPES:
//void system_init (void); /* hardware initialization */

void t1_intr_init(void);

unsigned int sw_fsm( unsigned int dir, unsigned int mode );

void output_sm_code( unsigned int sm_code ); 