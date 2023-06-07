/********************** Project 2 *********************************
*
* File: Project2.h
* Author: Seth Cram
* Date: 9/14/2021
*
*/

/* Software timer definition */
#define COUNTS_PER_MS 8889 /* Exact value is to be determined */

/* Function Prototypes */
void system_init (void); /* hardware initialization */
void sw_msDelay (unsigned int mS); /* Software only delay */
void hw_msDelay(unsigned int mS); /* Hardware-assisted delay */