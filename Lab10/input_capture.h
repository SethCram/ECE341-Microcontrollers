/******************************** Project 10 ****************************
*
* File: input_capture.h
* Author name: Seth Cram
* Rev. Date: 11/30/2021
*
* Project Description: 
* 
*
* Notes:
*
**********************************************************************/

//PROTOTYPES:

void system_init(void);

int ic_init();

void t3_init(void);

//MACROS:

//DC motor consts: (future expansion)
//#define CW 1
//#define CCW 2

//curr DC motor consts:
#define CYCLE_FREQ 1000 //set for 1ms period
#define JIM_PR2 ( FPB / CYCLE_FREQ ) - 1 //should also mult cycle_freq by timer2 prescaler

//global var for motor speed:
extern float rsp;