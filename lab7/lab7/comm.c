/******************************  comm.c  *********************************
 * Author:      Seth Cram
 * Date:        October 19, 2021 - Original
 *
 * This code also uses the "printf" function on UART Serial Port 1
 *
 * Cerebot MX7cK requires UART crossover cable if connecting with PmodRS232
 * Connector Pin    Pmod Pin    Function
 * 	2		4	MX7 Rx
 * 	3		3	MX7 Tx
 * 	5		5	Gnd
 * 	6		6	Vcc
 *
 * Notes:
 * 	'U1' suffix corresponds to w/ using UART1 in function
***************************************************************************/

#include <plib.h>
#include <stdio.h>              /* Required for printf */
#include "comm.h"
#include "CerebotMX7cK.h"	/* Has info regarding the PB clock */

/* initialize_comm FUNCTION DESCRIPTION *************************************
 * SYNTAX:          void initialize_comm(unsigned int baud, int parity);
 * KEYWORDS:        UART, initialization, parity
 * DESCRIPTION:     Initializes UART1 comm port for specified baud rate using
 *                  the assigned parity
 * PARAMETER 1:     integer Baud rate
 * PARAMETER 1:     integer (parity, NO_PARITY, ODD_PARITY, or EVEN_PARITY)
 * RETURN VALUE:    None
 *
 * NOTES:           9 bit mode MARK or SPACE parity is not supported
 * END DESCRIPTION **********************************************************/
void initialize_uart1(unsigned int baud, int parity)
{
unsigned int BRG;

	//should subtraction be an add?
    BRG=(unsigned short)(((float)FPB / ((float)4 * (float) baud))-(float)0.5);
    switch(parity)
    {
        case NO_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_NO_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
        case ODD_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_ODD_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
        case EVEN_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_EVEN_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
    }
    printf("\n\nCerebot MX7ck Serial Port 1 ready\r\n");
}

/* _mon_putc FUNCTION DESCRIPTION ******************************************
 * SYNTAX:          void _mon_putc(char c);
 * KEYWORDS:        printf, console, monitor
 * DESCRIPTION:     Sets up serial port to function as console for printf.
 *                  Used only by system.
 * PARAMETER 1:     Character to send to monitor
 * RETURN VALUE:    None
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
void _mon_putc(char c)
{
    while(BusyUART1());
    WriteUART1((unsigned int) c);
} /* End of _mon_putc */

/* putcU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int putcU1( int c);
 * KEYWORDS:        UART, character
 * DESCRIPTION:     Waits while UART1 is busy (buffer full) and then sends a
 *                  single byte to UART1
 * PARAMETER 1:     character to send
 * RETURN VALUE:    character sent
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
int putcU1( int c)
{
    while(BusyUART1());
    WriteUART1((unsigned int) c);
    return c;
} /* End of putU1 */

/* getcU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int getcU1( char *ch);
 * KEYWORDS:        character, get
 * DESCRIPTION:     Checks for a new character to arrive to the UART1 serial port.
 * PARAMETER 1:     character pointer to character variable
 * RETURN VALUE:    TRUE = new character received
 *                  FALSE = No new character
 * NOTES:           This function does not block for no character received
 * END DESCRIPTION ********************************************************/
int getcU1( char *ch)
{
    if( !DataRdyUART1())	/* wait for new char to arrive */
        return FALSE;           /* Return new data not available flag */
    else
    {
        *ch = ReadUART1();	/* read the char from receive buffer */
        return TRUE;		/* Return new data available flag */
    }
}/* End of getU1 */

/* putsU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int putsU1( const char *s);
 * KEYWORDS:        UART, string
 * DESCRIPTION:     Sends a NULL terminates text string to UART1 with
 *                  CR and LF appended
 * PARAMETER 1:     pointer to text string
 * RETURN VALUE:    Logical TRUE
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
int putsU1( const char *s)
{
    putsUART1(s);
    putcUART1( '\r');
    putcUART1( '\n');
    return 1;
} /* End of putsU1 */

/* getstrU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int getstrU1( char *s, unsigned int len );
 * KEYWORDS:        string, get, UART
 * DESCRIPTION:     This function assembles a line of text until the number of
 *                  characters assembled exceed the buffer length or an ASCII
 *                  CR control character is received.  This function echo each
 *                  received character back to the UART. It also implements a
 *                  destructive backspace. ASCII LF control characters are
 *                  filtered out.  The returned string has the CR character
 *                  removed and a NULL character appended to terminate the text
 *                  string. BACKSPACE defined in comm.h header file.
 * PARAMETER 1:     character pointer to string
 * PARAMETER 2:     integer maximum string length
 * RETURN VALUE:    TRUE = EOL signaled by receiving return character
 *                  FALSE = waiting for end of line
 * NOTES:           It is presumed that the buffer pointer or the buffer length
 *                  does not change after the initial call asking to
 *                  receive a new line of text.  This function does not block
 *                  for no character received. A timeout can be added to this
 *                  to free up resource. There is no way to restart the function
 *                  after the first call until a EOL has been received.  Hence
 *                  this function has denial of service security risks.
 * END DESCRIPTION **********************************************************/
int getstrU1( char *s, unsigned int len )
{
static int eol = TRUE;  /* End of input string flag*/
static unsigned int buf_len;
static char *p;        /* copy of the buffer pointer */
char ch;                /* Received new character */

    if(eol)                 /* Start of new line?               */
    {			          
        p = s;              /* Copy pointer for backspacing     */
        eol = FALSE;	   
	buf_len = len - 1;  /* Save max buffer length with room for NULL */
    }

    if(!(getcU1(&ch)))      /*  Check for character received */
    {
        return FALSE;       /* Bail out if not */
    }
    else
    {
        *p = ch;       /* Save new character in string buffer */
   	putcU1( *p);   /* echo character */
	switch(ch)	/* Test for control characters */
	{
            case BACKSPACE:         // defined as 0x08 (cntl-H) in comm.h
                if ( p>s)           /* prevent backing up past the start! */
                {
                    putcU1( ' ');     /* overwrite the last character */
                    putcU1( BACKSPACE);
                    buf_len++;
                    p--;            /* back off the pointer */
                }
                break;
            case '\r':              /* carriage return */
                putcU1( '\r');      /* echo character */
                eol = TRUE;         /* Set end of line */
            	break;
            case '\n':              /* newline (line feed), not EOL */
                putcU1('\n');       // ignore but echo anyway
                break;              // PuTTY only sends \r for Enter Key
            default:
                p++;               /* increment buffer pointer */
                buf_len--;          /* decrement length counter */
        } // end of switch
    } // end of else

if( buf_len == 0 || eol)    /* Check for buffer full or end of line */
    {
        *p = '\0';             /* add null terminate the string */
        eol = TRUE;
        return TRUE;            /* Set EOL flag */
    }
    return FALSE;               /* Not EOL */

	} /* End of getstr */

/* End of comm.c */
