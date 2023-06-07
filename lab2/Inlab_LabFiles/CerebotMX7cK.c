/***************************  CerebotMX7cK.c  ******************************
**
**  Author:     Richard Wall
**  Date:       August 15, 2013
**  
**  Revised: 17Jan2014 (JFF)
**  
**  Configures the Crerbot MX7cK processor board for:
**      Sets LED1 through LED4 for output
**      Sets BTN1, BTN2 and BTN3 for input
**
** **************************************************************************/

/* The config_bits.h must be included once and only once in any project */
#include "config_bits.h"    /* Processor configuration definitions */

#include <plib.h>           /* PIC32 Perihperal Library header file */
#include "CerebotMX7cK.h"   /* Cerebot MX7cK pin definitions */

/* Cerebot_mx7ck_setup FUNCTION DESCRIPTION ********************************
 * SYNTAX:          void Cerebot_mx7ck_setup(void);
 * KEYWORDS:        Cerebot MX7cK, PIC32, setup
 * Parameters:      None
 * Return:          None
 * Notes:           None
 * END DESCRIPTION ********************************************************/
void Cerebot_mx7cK_setup(void)
{
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Statement configures cache and wait states for maximum performance
 * Given the options, this function will change the flash wait states, RAM
 * wait state and enable prefetch cache but will not change the PBDIV.
 * The PBDIV value is set via pragma FPBDIV in config_bits.h.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    DDPCONbits.JTAGEN = 0;  // Statement is required to use Pin RA0 as IO
    PORTSetPinsDigitalIn(IOPORT_A, BTN3);	 /* Set BTN3 as input */

    PORTSetPinsDigitalIn(IOPORT_G, BTN1 | BTN2); /* Set BTN1 & BTN2 as inputs */
    PORTSetPinsDigitalOut(IOPORT_G, BRD_LEDS); 	 /* Set BRD LEDs as output */
    LATGCLR = BRD_LEDS;			/* Turn off LED1 through LED4 */
} /* End of Cerebot_mx7cK_setup */

/* End of CerebotMX7cK.c */
