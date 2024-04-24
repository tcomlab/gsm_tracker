/*
 * shell.h
 *
 *  Created on: 26.10.2010
 *      Author: Admin
 */

#ifndef SHELL_H_
#define SHELL_H_



extern unsigned char DBGU_GetChar(void);
extern void DBGU_PutChar(unsigned char c);
extern unsigned int DBGU_IsRxReady(void);
void irq_shell (void);

#endif /* SHELL_H_ */
