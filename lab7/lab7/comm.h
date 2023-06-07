/*********************************  comm.h **********************************
 * Author:      Seth Cram
 * Date:        October 19, 2021
*****************************************************************************/

#ifndef __MX7COMM_H__

    #define __MX7COMM_H__
    #define _UART1
#endif

#ifndef __COMM_H__

    #define __COMM_M__
    #define BACKSPACE       0x08
    #define NO_PARITY       0
    #define ODD_PARITY      1
    #define EVEN_PARITY     2

#endif

void initialize_uart1(unsigned int baud, int parity);
void _mon_putc(char c); /* Called by system to implement "printf" functions */
int putcU1( int c);      /* Send single character to UART */
int getcU1( char *ch);   /* Get single character from UART */
int putsU1( const char *s);     /* Send string to UART1 */
int getstrU1( char *s, unsigned int len ); /* Get CR terminated string */

/* End of comm.h */