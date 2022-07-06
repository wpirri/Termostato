/* *****************************************************************************
 * Termostato Cava
 * 10 a 20 °C
 * PIC12F675
 * *****************************************************************************
 * Pines:
 *  1 - Vdd (2.0 a 5.5)     - Vdd
 *  2 - GP5                 - Buzzer
 *  3 - GP4                 - Relee
 *  4 - GP3 / MCLR / Vpp    - Led Status
 *  5 - GP2 / INT           - Temp -
 *  6 - GP1 / ICSP CLK      - Temp +
 *  7 - GP0 / ICSP DAT      - NTC
 *  8 - Vss                 - Vss
 * Clock:
 *  Interno 4 MHz
 * ************************************************************************** */
#pragma config CPD = OFF
#pragma config CP = OFF
#pragma config BOREN = OFF
#pragma config MCLRE = OFF
#pragma config PWRTE = ON
#pragma config WDTE = ON
#pragma config FOSC = INTRCIO
/* ************************************************************************** */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "iee.h"

__EEPROM_DATA( 0x80, 0x80, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00 );

/* Para funciones Delay de la librerïa __delay_ms() y __delay_us() */
#define _XTAL_FREQ 4000000

/* *************************************************************************** *
 * Definiciones
 */
#define BEEP_LARGO 200
#define BEEP_CORTO 50
#define BEEP_SILENCIO 100
#define BOTON_DELAY 500
#define REFERENCIA_MAX 700
#define REFERENCIA_MIN 500

/* *************************************************************************** *
 * Globales
 */
volatile unsigned char saveEE;
volatile unsigned int timer0Div;
volatile unsigned char flag2KHz;
volatile unsigned char flag2Hz;

unsigned char contBuzzer;
unsigned char Temperatura;

unsigned int botonUpFiltro;
unsigned int botonDownFiltro;

unsigned int referenciaMax;
unsigned int referenciaMin;

/* *************************************************************************** *
 * Interrupciones
 */
/* Interrupcion cada 256us (prescaler en WDT) */
/* Si se usa % dentro de interrupciïon da overflow el stack */
void Timer0Int( void )
{
    T0IF = 0;
    if( !T0IE) return;

    flag2KHz = 1;

    timer0Div++;
    if(timer0Div >= 1956)
    {
        timer0Div = 0;
        flag2Hz = 1;
    }
    
}
/*
void Timer1Int( void )
{
    TMR1IF = 0;
    if( !TMR1IE) return;

    
}
*/
/*
void ExtInt( void )
{
    INTF = 0;
    if( !INTE) return;

}
*/
/*
 * void GPInt( void )
{
    unsigned char a = GPIO;
    
    GPIF = 0;
    if( !GPIE) return;
    

}
*/

/* *************************************************************************** *
 * Punto de interrupcion
 */
void __interrupt() ISR(void)
{
//    GIE = 0;  << NO SE DEBE HACER >>
    if(T0IF)   { Timer0Int(); }
    /*if(TMR1IF) { Timer1Int(); }*/
    /*if(INTF)   { ExtInt();    }*/
    /*if(GPIF) { GPInt(); }*/
//    GIE = 1;  << NO SE DEBE HACER >>
}

void Beep(unsigned char duracion)
{
    if(contBuzzer < duracion) contBuzzer = duracion;
}

void BotonUp()
{
    
}

void BotonDown()
{
    
}


/* *************************************************************************** *
 * MAIN
 */
void main(void)
{
    /*
    bit 7 GPPU: GPIO Pull-up Enable bit
        1 = GPIO pull-ups are disabled
        0 = GPIO pull-ups are enabled by individual port latch values
    bit 6 INTEDG: Interrupt Edge Select bit
        1 = Interrupt on rising edge of GP2/INT pin
        0 = Interrupt on falling edge of GP2/INT pin
    bit 5 T0CS: TMR0 Clock Source Select bit
        1 = Transition on GP2/T0CKI pin
        0 = Internal instruction cycle clock (CLKOUT)
    bit 4 T0SE: TMR0 Source Edge Select bit
        1 = Increment on high-to-low transition on GP2/T0CKI pin
        0 = Increment on low-to-high transition on GP2/T0CKI pin
    bit 3 PSA: Prescaler Assignment bit
        1 = Prescaler is assigned to the WDT
        0 = Prescaler is assigned to the TIMER0 module
    bit 2-0 PS2:PS0: Prescaler Rate Select bits
        Bit Value   TMR0 Rate WDT Rate
        000         1:2         1:1
        001         1:4         1:2
        010         1:8         1:4
        011         1:16        1:8
        100         1:32        1:16
        101         1:64        1:32
        110         1:128       1:64
        111         1:256       1:128
     */
    OPTION_REG = 0b00001101;    /* PSA -> WDT 1:32 = 576 ms */
    /**/
    GPIO = 0x00;
    /* Control del comparador */
    CMCON = 0x07;     /* Solamente necesario para el 675 */
    ANSEL = 0x00;
    /* Configuracion de I/O */
    TRISIO = 0b00000111;
    /* Pull-up */
    WPU = 0b00000110;
 
    /*
        bit 7 Unimplemented: Read as ?0?
        bit 6 TMR1GE: Timer1 Gate Enable bit
            If TMR1ON = 0:
            This bit is ignored
            If TMR1ON = 1:
            1 = Timer1 is on if T1G pin is low
            0 = Timer1 is on
        bit 5-4 T1CKPS1:T1CKPS0: Timer1 Input Clock Prescale Select bits
            11 = 1:8 Prescale Value
            10 = 1:4 Prescale Value
            01 = 1:2 Prescale Value
            00 = 1:1 Prescale Value
        bit 3 T1OSCEN: LP Oscillator Enable Control bit
            If INTOSC without CLKOUT oscillator is active:
            1 = LP oscillator is enabled for Timer1 clock
            0 = LP oscillator is off
            Else:
            This bit is ignored
        bit 2 T1SYNC: Timer1 External Clock Input Synchronization Control bit
            TMR1CS = 1:
            1 = Do not synchronize external clock input
            0 = Synchronize external clock input
            TMR1CS = 0:
            This bit is ignored. Timer1 uses the internal clock.
        bit 1 TMR1CS: Timer1 Clock Source Select bit
            1 = External clock from T1OSO/T1CKI pin (on the rising edge)
            0 = Internal clock (FOSC/4)
        bit 0 TMR1ON: Timer1 On bit
            1 = Enables Timer1
            0 = Stops Timer1
     */
    T1CON = 0b00000001;

    /*
        bit 7 EEIE: EE Write Complete Interrupt Enable bit
            1 = Enables the EE write complete interrupt
            0 = Disables the EE write complete interrupt
        bit 6 ADIE: A/D Converter Interrupt Enable bit (PIC12F675 only)
            1 = Enables the A/D converter interrupt
            0 = Disables the A/D converter interrupt
        bit 5-4 Unimplemented: Read as ?0?
        bit 3 CMIE: Comparator Interrupt Enable bit
            1 = Enables the comparator interrupt
            0 = Disables the comparator interrupt
        bit 2-1 Unimplemented: Read as ?0?
        bit 0 TMR1IE: TMR1 Overflow Interrupt Enable bit
            1 = Enables the TMR1 overflow interrupt
            0 = Disables the TMR1 overflow interrupt
     */
    PIE1 = 0b00000001;

    /*
    bit 7 GIE: Global Interrupt Enable bit
        1 = Enables all unmasked interrupts
        0 = Disables all interrupts
    bit 6 PEIE: Peripheral Interrupt Enable bit
        1 = Enables all unmasked peripheral interrupts
        0 = Disables all peripheral interrupts
    bit 5 T0IE: TMR0 Overflow Interrupt Enable bit
        1 = Enables the TMR0 interrupt
        0 = Disables the TMR0 interrupt
    bit 4 INTE: GP2/INT External Interrupt Enable bit
        1 = Enables the GP2/INT external interrupt
        0 = Disables the GP2/INT external interrupt
    bit 3 GPIE: Port Change Interrupt Enable bit (1)
        1 = Enables the GPIO port change interrupt
        0 = Disables the GPIO port change interrupt
    bit 2 T0IF: TMR0 Overflow Interrupt Flag bit (2)
        1 = TMR0 register has overflowed (must be cleared in software)
        0 = TMR0 register did not overflow
    bit 1 INTF: GP2/INT External Interrupt Flag bit
        1 = The GP2/INT external interrupt occurred (must be cleared in software)
        0 = The GP2/INT external interrupt did not occur
    bit 0 GPIF: Port Change Interrupt Flag bit
        1 = When at least one of the GP5:GP0 pins changed state (must be cleared in software)
        0 = None of the GP5:GP0 pins have changed state
     */
    INTCON = 0b00100000;

    /*
     * Port Change Interrupt
     */
    IOC = 0b00011000;

    
    saveEE = 0;
    timer0Div = 0;
    flag2KHz = 0;
    flag2Hz = 0;
    contBuzzer = 0;
    botonUpFiltro = BOTON_DELAY / 2;
    botonDownFiltro = BOTON_DELAY / 2;
 
    Temperatura = iEEgetc(0x00);

    if(Temperatura > 20) Temperatura = 10;
    if(Temperatura < 10) Temperatura = 10;

    referenciaMax = (Temperatura * 100)
    
    GIE = 1;    /* Habilito las interrupciones */

    while(1)
    {
        CLRWDT();

        /* Boton UP */  
        if(GPIO & 0b00000010)
        {
            if(botonUpFiltro) botonUpFiltro--;
        }
        else
        {
            if(botonUpFiltro < BOTON_DELAY)
            {
               botonUpFiltro++;
               if(botonUpFiltro >= BOTON_DELAY)
               {
                   BotonUp();
               }
            }
        }
        /* Boton DOWN */
        if(GPIO & 0b00000010)
        {
            if(botonDownFiltro) botonDownFiltro--;
        }
        else
        {
            if(botonDownFiltro < BOTON_DELAY)
            {
               botonDownFiltro++;
               if(botonDownFiltro >= BOTON_DELAY)
               {
                   BotonDown();
               }
            }
        }        
        
        if(flag2Hz)
        {
            flag2Hz = 0;
            GPIO ^= 0b00001000; /* Led Status */
        }
        
        if(flag2KHz)
        {
            flag2KHz = 0;
            if(contBuzzer)
            {
                contBuzzer--;
                GPIO ^= 0b00100000; /* Buzzer */
            }
        }

        if(saveEE)
        {
            saveEE = 0;
            iEEputc(0x00, Temperatura);
        }
        
    }
}

