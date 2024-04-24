/*
 * shell.c
 *
 *  Created on: 26.10.2010
 *      Author: Admin
 */
#include "shell.h"
#include <stdarg.h>
#include <main.h>
#include <stdio.h>

//------------------------------------------------------------------------------
/// Outputs a character on the DBGU line.
/// \note This function is synchronous (i.e. uses polling).
/// \param c  Character to send.
//------------------------------------------------------------------------------
void DBGU_PutChar(unsigned char c)
{
    // Wait for the transmitter to be ready
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0);

    // Send character
    AT91C_BASE_DBGU->DBGU_THR = c;

    // Wait for the transfer to complete
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0);
}

//------------------------------------------------------------------------------
/// Return 1 if a character can be read in DBGU
//------------------------------------------------------------------------------
unsigned int DBGU_IsRxReady()
{
    return (AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY);
}

//------------------------------------------------------------------------------
/// Reads and returns a character from the DBGU.
/// \note This function is synchronous (i.e. uses polling).
/// \return Character received.
//------------------------------------------------------------------------------
unsigned char DBGU_GetChar(void)
{
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY) == 0);
    return AT91C_BASE_DBGU->DBGU_RHR;
}

//#ifndef NOFPUT


//------------------------------------------------------------------------------
/// \exclude
/// Implementation of fputc using the DBGU as the standard output. Required
/// for printf().
/// \param c  Character to write.
/// \param pStream  Output stream.
/// \param The character written if successful, or -1 if the output stream is
/// not stdout or stderr.
//------------------------------------------------------------------------------
signed int fputc(signed int c, FILE *pStream)
{
    if ((pStream == stdout) || (pStream == stderr)) {

        DBGU_PutChar(c);
        return c;
    }
    else {

        return EOF;
    }
}

//------------------------------------------------------------------------------
/// \exclude
/// Implementation of fputs using the DBGU as the standard output. Required
/// for printf(). Does NOT currently use the PDC.
/// \param pStr  String to write.
/// \param pStream  Output stream.
/// \return Number of characters written if successful, or -1 if the output
/// stream is not stdout or stderr.
//------------------------------------------------------------------------------
signed int fputs(const char *pStr, FILE *pStream)
{
    signed int num = 0;

    while (*pStr != 0) {

        if (fputc(*pStr, pStream) == -1) {

            return -1;
        }
        num++;
        pStr++;
    }

    return num;
}

//#undef putchar

//------------------------------------------------------------------------------
/// \exclude
/// Outputs a character on the DBGU.
/// \param c  Character to output.
/// \return The character that was output.
//------------------------------------------------------------------------------
signed int putchar(signed int c)
{
    return fputc(c, stdout);
}
//------------------------------------------------------------------------------
void irq_shell (void)
{

}
//------------------------------------------------------------------------------
