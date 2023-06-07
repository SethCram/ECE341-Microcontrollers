/******************************** Project 7 ****************************
*
* File: Btns_ISR.h
* Author name: Seth Cram
* Rev. Date: 10/19/2021
*
* Project Description: 
* 
*
* Notes:
*	
**********************************************************************/

/* Software timer const */
#define COUNTS_PER_MS 8889 /* Exact value is to be determined */

//Debounce btns const:
#define DEBOUNCE_TIME 20 //debounce btns for 20ms

//stepper motor constants:
#define HS 2
#define FS 1
#define CW 1
#define CCW 2

//PROTOTYPES:
//void system_init (void); /* hardware initialization */

void cn_intr_init(void); 

int read_buttons(void);

void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode );

void sw_msDelay(unsigned int mS); 

int ComputeStepDelay( int mode, int speed );