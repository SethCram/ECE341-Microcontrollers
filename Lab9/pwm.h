/******************************** Project 9 ****************************
*
* File: pwm.h
* Author name: Seth Cram
* Rev. Date: 11/08/2021
*
* Project Description: 
* 
*
* Notes:
*
**********************************************************************/

//PROTOTYPES:

void system_init(void);

int pwm_init(int dutyCycle, int cycleFrequency);

int pwm_set(int dutyCycle);

void t2_intr_init(void);

void cn_intr_init(void);

int read_buttons(void);

void decode_buttons( unsigned int buttons, 
        unsigned int *dir, unsigned int *dutyCycle);

void sw_msDelay(unsigned int mS);

//unsigned int sw_fsm( unsigned int dir, unsigned int mode );
//void output_sm_code( unsigned int sm_code );

//MACROS:

//stepper motor constants:
//#define HS 1
//#define FS 2

//DC motor consts: (future expansion)
#define CW 1
#define CCW 2

/* Software timer const */
#define COUNTS_PER_MS 8889 

//Debounce btns const:
#define DEBOUNCE_TIME 20 //debounce btns for 20ms

//curr DC motor consts:
#define CYCLE_FREQ 1000 //set for 1ms period
#define JIM_PR2 ( FPB / CYCLE_FREQ ) - 1 //should also mult cycle_freq by timer2 prescaler
