/* ************************ Lab0.c *************************************
 * File name:   Lab0.c
 * Author:	Richard W. Wall
 * Date: 	August 19, 2013
 
 * This program is designed to provide a simple platform for exploring the
 * software instrumentation and debugging tools available with the MPLAB X
 * Integrated Development Environment (IDE). C code examples are provided
 * that demonstrate passing data by value and by reference between two
 * functions .
 * ********************************************************************* */

/*
 Note:  Local project header files are written between "  ".
        System header files are bracketed between < >.  */

#include "CerebotMX7cK.h"   /* Cerebot MX7cK board definitions */
#include "lab0.h"           /* Lab 0 definitions and function prototypes */

/* Global variable list - normally, only variables that are required
 * to pass data to or from interrupt service routines (ISR) are required
 * to be declared as global variables. Processor Special Function Register
 * (SFR) are global by the system. */

/* Constant data types cannot be modified at execution time and appear
 * as protected variables in the MPLAB watch window.  */

const int ac = A;   /* Constants assigned values that are defined */
const int bc = B;

/* --------------------  Start of program code ------------------------------ */
int main(void)
{
/* Variable that are declared as static can be viewed in the MPLAB watch
 * window but values for these type of variables will be shown when the program
 * is executing code in the function in which they are declared. Static local
 * variables do not automatically appear in the variables window.
 */

static int a;    /* Static local variables have permanent  memory allocation*/
static int b;
int c;           /* auto local variables have temporary memory allocation */

const int d = 3;    /* Variables that are declared as a data type "const"
                    are assigned a value when they are declared.  The
                    "const" variable cannot appear on the left side of the
                    equal (=) sign in any statement. For example, the
                    statement, d = 34*a;, will generate a compiler error */

    Cerebot_mx7cK_setup();  /* Initialize Cerebot MX7ck on board resources */
    
    //a = ac;             /* Initialize variables a and b. */
    //b = bc;

    while(1)                /* Infinite application loop */
    {
        a = ac;             /* Initialize variables a and b. */
        b = bc;
        no_swap(a, b);      /* Call swapping functions */
        c = swap(&a, &b);
        a = c - d;          /* Math statement */
    }

    //bc main is int ret type
    return(1);  /* A program developed for the embedded system should never 
                   be able to execute this return instruction. - bc inf loop */
}  /* End of main */

/* no_swap FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          void no_swap(int x, int y)
 * KEYWORDS:        swap, exchange, pass by value
 * DESCRIPTION:     This function swaps the values in variables x and y. Since
 *                  the variables are passed by value, the results are lost once
 *                  the program execution returns from this function.
 * PARAMETER1:      first integer value
 * PARAMETER2:      second integer value
 * RETURN VALUE:    None.
 *
 * NOTES:           Variables passed by value in the argument list are local
 *                  considered to be variables. Variables declared inside the
 *                  function are also local variables provided they are not
 *                  declared as static.
 * END DESCRIPTION **********************************************************/
void no_swap(int x, int y)
{
int z;          /* Temporary local variable */
    z = x;      /* Execute the exchange of values*/
    x = y;
    y = z;
}  /* End of no_swap funbction */

/* swap FUNCTION DESCRIPTION ************************************************
 * SYNTAX:          void swap(int ^x, int *y)
 * KEYWORDS:        swap, exchange, pass by reference
 * DESCRIPTION:     This function swaps the values of variables whose address are
 *                  passed as x and y. Since the variables are passed by reference,
 *                  the results are not lost once the function is done.
 * PARAMETER1:      first integer value
 * PARAMETER2:      second integer value
 * RETURN VALUE:    Sum of x and y.
 *
 * NOTES:       Variables that are passed by reference appear in the MPLAB
 *              locals window. Local variables that are declared as static
 *              can be viewed in the watch window.
 * END DESCRIPTION **********************************************************/
int swap(int *x, int *y)
{
int z;              /* Temporary local variable */
    z = *x;         /* Execute the exchange of values */
    *x = *y;
    *y = z;
    z = *x +*y;
    return z;
} /* End of swap function */

/* End of Lab0.c */
