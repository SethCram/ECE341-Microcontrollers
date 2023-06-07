/********************** Project 3 *********************************
*
* File: Proj3.h
* Author: Seth Cram
* Date: 9/21/2021
*
*/

/* Software timer definition */
#define COUNTS_PER_MS 8889 /* Exact value is to be determined */

//Seth defined constants:
#define HS 1
#define FS 2
#define CW 1
#define CCW 2

/* Function Prototypes */
void system_init (void); /* hardware initialization */

int read_buttons(void);

void decode_buttons( unsigned int buttons, unsigned int *step_delay, 
			unsigned int *dir, unsigned int *mode );

unsigned int sw_fsm( unsigned int dir, unsigned int mode );

void output_sm_code( unsigned int sm_code );

void sw_msDelay(unsigned int mS); 