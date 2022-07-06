/* ************************************************************************** *
 *
 *  Filename:           iee.c
 *  Date:               27 de Junio de 2014
 *  File Version:       1.0
 *  Assembled using:    XC8
 *
 *  Author:             Walter Pirri
 *                      (walter***AT***pirri***DOT***com***DOT***ar)
 *  Company:            WGP
 *
 *************************************************************************** */
#ifndef _IEE_H_
#define	_IEE_H_

#define EE_CHECK_VALUE      0xA5
#define EE_CHECK_OFFSET     0

#define EE_TIMER_OFFSET     1
#define EE_PROGRAMM_OFFSET  2


void iEEputc(unsigned char address, unsigned char data);
unsigned char iEEgetc(unsigned char address);

#endif	/* _IEE_H_ */

