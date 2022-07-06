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
#include "iee.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xc.h>

//This function Writes data to given address in internal EEPROM of PIC MCU
void iEEputc(unsigned char address, unsigned char data)
{
    unsigned char INTCON_SAVE;

    EEADR = address;
    EEDATA = data;

#ifdef EEPGD
    EEPGD = 0; // 0 = Access data EEPROM memory
#endif 
#ifdef CFGS 
    CFGS = 0; // 0 = Access Flash program or DATA EEPROM memory
#endif
    WREN = 1; // enable writes to internal EEPROM

    INTCON_SAVE = INTCON; // Save INTCON register contants
    INTCON = 0;             // Disable interrupts, Next two lines SHOULD run without interrupts

    EECON2 = 0x55;        // Required sequence for write to internal EEPROM
    EECON2 = 0xAA;        // Required sequence for write to internal EEPROM

    WR = 1;    // begin write to internal EEPROM
    INTCON = INTCON_SAVE; //Now we can safely enable interrupts if previously used

    NOP();

    while(EEIF == 0)//Wait till write operation complete
    {
        CLRWDT();
        NOP();
    }

    WREN = 0; // Disable writes to EEPROM on write complete (EEIF flag on set PIR2 )
    EEIF = 0;   //Clear EEPROM write complete flag. (must be cleared in software. So we do it here)

}

// This function reads data from address given in internal EEPROM of PIC
unsigned char iEEgetc(unsigned char address)
{
    EEADR = address;
#ifdef EEPGD
    EEPGD = 0; // 0 = Access data EEPROM memory
#endif
#ifdef CFGS
    CFGS = 0;  // 0 = Access Flash program or DATA EEPROM memory
#endif
    EECON1bits.RD = 1;    // EEPROM Read
    return EEDATA;        // return data
}
