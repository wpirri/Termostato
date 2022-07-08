/* *****************************************************************************
 * Termostato Cava
 * 10 a 20 °C
 * PIC12F675
 * *****************************************************************************
 * Pines:
 *  1 - Vdd (2.0 a 5.5)           - Vdd
 *  2 - GP5                 OUT   - Buzzer
 *  3 - GP4                 OUT   - Relee
 *  4 - GP3 / MCLR / Vpp    No puede ser OUT
 *  5 - GP2 / INT           ANA   - NTC
 *  6 - GP1 / ICSP CLK      IN PU - Temp +
 *  7 - GP0 / ICSP DAT      IN PU - Temp -
 *  8 - Vss                       - Vss
 * Clock:
 *  Interno 4 MHz
 * ADC: 10°C +/- 700
 *      20°C +/- 500
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
#define BEEP_LARGO 1000
#define BEEP_CORTO 50
#define BEEP_SILENCIO 100
#define BOTON_DELAY 500
#define TEMP_MAX 20
#define TEMP_MIN 10
#define SAVE_EE_DELAY 20
#define DELTA_RELAY 10
#define DELTA_ADC 15

/* *************************************************************************** *
 * Globales
 */
volatile unsigned char saveEE;
volatile unsigned int timer0Div;
volatile unsigned char flag2KHz;
volatile unsigned char flag2Hz;

unsigned int contBuzzer;
unsigned char contLed;
unsigned char contRelay;
unsigned char Temperatura;
unsigned int Referencia;

unsigned int botonUpFiltro;
unsigned int botonDownFiltro;
unsigned char botonUpRebote;
unsigned char botonDownRebote;

unsigned int referenciaMax;
unsigned int referenciaMin;

unsigned char medicionPendiente;

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

void CalcularReferencias()
{
    referenciaMin = ((Temperatura - 10) * 20) + 500 - DELTA_ADC;
    referenciaMax = ((Temperatura - 10) * 20) + 500 + DELTA_ADC;
}

void Beep(unsigned int duracion)
{
    if(contBuzzer < duracion) contBuzzer = duracion;
}

void BotonUp()
{
    if(Temperatura < TEMP_MAX)
    {
        Temperatura++;
        Beep(BEEP_CORTO);
        CalcularReferencias();
        saveEE = SAVE_EE_DELAY;
    }
    else
    {
        Temperatura = TEMP_MAX;
        Beep(BEEP_LARGO);
    }
}

void BotonDown()
{
    if(Temperatura > TEMP_MIN)
    {
        Temperatura--;
        Beep(BEEP_CORTO);
        CalcularReferencias();
        saveEE = SAVE_EE_DELAY;
    }
    else
    {
        Temperatura = TEMP_MIN;
        Beep(BEEP_LARGO);
    }
}

void MedicionStart()
{
    if(medicionPendiente == 1) return;
    ADCON0bits.GO = 1;
    medicionPendiente = 1;
}

void MedicionCheck()
{
    if(medicionPendiente == 0) return;
    if(ADCON0bits.GO) return;
    Referencia = (unsigned int)((ADRESH << 8) + ADRESL);
    medicionPendiente = 0;
    
    /* */
    
    if(Referencia > referenciaMax)
    {
        /* Temperatura por debajo del umbral */
        if(contRelay < DELTA_RELAY) contRelay++;
        if( !GPIObits.GP4 && (contRelay == DELTA_RELAY)) GPIObits.GP4 = 1;
    }
    else if(Referencia < referenciaMin)
    {
        /* Temperatura sobre el umbral */
        if(contRelay) contRelay--;
        if(GPIObits.GP4 && (contRelay == 0)) GPIObits.GP4 = 0;
        
    }
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

    /* Configuracion de I/O */
    TRISIO = 0b00000111;
    /* Pull-up */
    WPU = 0b00000011;
 
    /*
     ADC
    bit 7 ADFM: A/D Result Formed Select bit
        1 = Right justified
        0 = Left justified
    bit 6 VCFG: Voltage Reference bit
        1 = VREF pin
        0 = VDD
    bit 5-4 Unimplemented: Read as zero
    bit 3-2 CHS1:CHS0: Analog Channel Select bits
        00 = Channel 00 (AN0)
        01 = Channel 01 (AN1)
        10 = Channel 02 (AN2)
        11 = Channel 03 (AN3)
    bit 1 GO/DONE: A/D Conversion STATUS bit
        1 = A/D conversion cycle in progress. Setting this bit starts an A/D conversion cycle.
         This bit is automatically cleared by hardware when the A/D conversion has completed.
        0 = A/D conversion completed/not in progress
    bit 0 ADON: A/D Conversion STATUS bit
        1 = A/D converter module is operating
        0 = A/D converter is shut-off and consumes no operating current
    */
    ADCON0 = 0b10001001;
    ANSEL = 0b00000100;

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
    T1CON = 0b00000000;

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
    PIE1 = 0b00000000;

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
    IOC = 0b00000000;

    
    saveEE = 0;
    timer0Div = 0;
    flag2KHz = 0;
    flag2Hz = 0;
    contBuzzer = 0;
    contLed = 0;
    botonUpFiltro = BOTON_DELAY / 2;
    botonDownFiltro = BOTON_DELAY / 2;
    botonUpRebote = 0;
    botonDownRebote = 0;
    Referencia = 0;
    medicionPendiente = 0;
    contRelay = 0;
 
    Temperatura = iEEgetc(0x00);

    if(Temperatura > TEMP_MAX || Temperatura < TEMP_MIN) Temperatura = (TEMP_MAX + TEMP_MIN) / 2;

    CalcularReferencias();
    
    GIE = 1;    /* Habilito las interrupciones */

    while(1)
    {
        CLRWDT();

        MedicionCheck();
        
        /* Boton UP */  
        if(GPIObits.GP0 == 1)
        {
            if(botonUpFiltro)
            {
                botonUpFiltro--;
                if(botonUpFiltro == 0)
                {
                    botonUpRebote = 0;
                }
            }
        }
        else
        {
            if(botonUpFiltro < BOTON_DELAY)
            {
               botonUpFiltro++;
               if(botonUpFiltro >= BOTON_DELAY && botonUpRebote == 0)
               {
                   botonUpRebote = 1;
                   BotonUp();
               }
            }
        }
        /* Boton DOWN */
        if(GPIObits.GP1 == 1)
        {
            if(botonDownFiltro)
            {
                botonDownFiltro--;
                if(botonDownFiltro == 0)
                {
                    botonDownRebote = 0;
                }
            }
        }
        else
        {
            if(botonDownFiltro < BOTON_DELAY)
            {
               botonDownFiltro++;
               if(botonDownFiltro >= BOTON_DELAY && botonDownRebote == 0)
               {
                   botonDownRebote = 1;
                   BotonDown();
               }
            }
        }
        
        if(flag2Hz)
        {
            flag2Hz = 0;
            contLed++;
//            GPIObits.GP3 = (contLed &1);
            
            MedicionStart();
        }
        
        if(flag2KHz)
        {
            flag2KHz = 0;
            if(contBuzzer)
            {
                contBuzzer--;
                GPIObits.GP5 ^= 1; /* Buzzer */
            }

            if(saveEE)
            {
                saveEE--;
                if(saveEE == 0) iEEputc(0x00, Temperatura);
            }


        }

        
    }
}

