/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autor: Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.40
    
Programa: pic para motores de proyecto 1 de PCB
Hardware: PIC16F887
    
Creado: 10 de septiembre de 2021    
Descripcion: 
------------------------------------------------------------------------------*/

#pragma config FOSC = INTRC_NOCLKOUT   //configuracion de oscilador interno
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


/*-----------------------------------------------------------------------------
 ----------------------------LIBRERIAS-----------------------------------------
 -----------------------------------------------------------------------------*/
#include <xc.h>
#include <stdint.h>
#include "Osc_config.h"

/*-----------------------------------------------------------------------------
 ----------------------- VARIABLES A IMPLEMTENTAR------------------------------
 -----------------------------------------------------------------------------*/
//-------DIRECTIVAS DEL COMPILADOR
#define _XTAL_FREQ 8000000
//-------VARIABLES DE PROGRAMA
unsigned char antirrebote1, antirrebote2;
unsigned char encendido;
/*-----------------------------------------------------------------------------
 ------------------------ PROTOTIPOS DE FUNCIONES ------------------------------
 -----------------------------------------------------------------------------*/
void setup(void);
void motor_encendido(void);
void motor_apagado(void);
void motor_retorno(void);
/*-----------------------------------------------------------------------------
 --------------------------- INTERRUPCIONES -----------------------------------
 -----------------------------------------------------------------------------*/
void __interrupt() isr(void) //funcion de interrupciones
{ 
    if (INTCONbits.RBIF)
    {
        switch(PORTB)
        {
            /*default:
                antirrebote1=0;
                antirrebote2=0;
                break;*/
            case(0b11111101):       //caso interrupcion encendido o apagado
                antirrebote1=1;
                break;
            case(0b11111011):       //caso retorno de motor
                antirrebote2=1;
                break;
        }
        INTCONbits.RBIF=0;
    }

}

/*-----------------------------------------------------------------------------
 ----------------------------- MAIN LOOP --------------------------------------
 -----------------------------------------------------------------------------*/
void main(void) 
{
    setup();
    while(1)
    {
        if (antirrebote1==1 && PORTBbits.RB1==0 && PORTBbits.RB2==1)
        {
            antirrebote1=0;
            motor_encendido();            
        }
        //--
        if (antirrebote2==1 && PORTBbits.RB2==0 && PORTBbits.RB1==1)
        {
            antirrebote2=0;
            motor_retorno();
        }
    }
}
/*-----------------------------------------------------------------------------
 ---------------------------------- SET UP -----------------------------------
 -----------------------------------------------------------------------------*/
void setup(void)
{
    //-------CONFIGURACION ENTRADAS ANALOGICAS
    ANSEL=0;
    ANSELH=0;
    //-------CONFIGURACION DE ENTRADAS Y SALIDAS
    TRISBbits.TRISB1=1;        //entrada para boton de retorno
    TRISBbits.TRISB2=1;        //entrada para boton de retorno
    TRISD=0;                   //salida para pines de motor
    TRISE=0;                   //salida para pines de motor
    //-------LIMPIEZA DE PUERTOS
    PORTE=0;
    PORTB=0;
    PORTD=0;
    //-------CONFIGURACION DE RELOJ
    osc_config(8);
    //-------CONFIGURACION DE PULL UPS
    OPTION_REGbits.nRBPU=0;             //se activan WPUB
    WPUBbits.WPUB1=1;                   //RB1, boton encendido
    WPUBbits.WPUB2=1;                   //RB2, boton retorno
    //-------CONFIGURACION DE INTERRUPCIONES
    INTCONbits.GIE=1;                   //se habilita interrupciones globales
    INTCONbits.PEIE = 1;                //habilitan interrupciones por perifericos
    INTCONbits.RBIE=1;                  //se  habilita IntOnChange B
    INTCONbits.RBIF=0;                  //se  apaga bandera IntOnChange B
    IOCBbits.IOCB1=1;                   //habilita IOCB RB1
    IOCBbits.IOCB2=1;                   //habilita IOCB RB2
    
}
/*-----------------------------------------------------------------------------
 --------------------------------- FUNCIONES ----------------------------------
 -----------------------------------------------------------------------------*/
//-------
void motor_encendido(void)
{
    for(int i; i<1000;i++)
    {
        //paso 1
        PORTDbits.RD7=1;
        PORTDbits.RD6=0;
        PORTDbits.RD5=0;
        PORTDbits.RD4=1;
        __delay_ms(2);
        //paso 2
        PORTDbits.RD7=1;
        PORTDbits.RD6=1;
        PORTDbits.RD5=0;
        PORTDbits.RD4=0;
        __delay_ms(2);
        //paso 3
        PORTDbits.RD7=0;
        PORTDbits.RD6=1;
        PORTDbits.RD5=1;
        PORTDbits.RD4=0;
        __delay_ms(2);
        //paso 4
        PORTDbits.RD7=0;
        PORTDbits.RD6=0;
        PORTDbits.RD5=1;
        PORTDbits.RD4=1;
        __delay_ms(2);  
    }
}
//-------
void motor_apagado(void)
{
    PORTD=0x00;
}
//-------
void motor_retorno(void)
{
    for(int j;j<1000;j++)
    {
        //paso 1 reversa
        PORTDbits.RD7=0;
        PORTDbits.RD6=0;
        PORTDbits.RD5=1;
        PORTDbits.RD4=1;
        __delay_ms(2);
        //paso 2 reversa
        PORTDbits.RD7=0;
        PORTDbits.RD6=1;
        PORTDbits.RD5=1;
        PORTDbits.RD4=0;
        __delay_ms(2);
        //paso 3 reversa
        PORTDbits.RD7=1;
        PORTDbits.RD6=1;
        PORTDbits.RD5=0;
        PORTDbits.RD4=0;
        __delay_ms(2);
        //paso 4 reversa
        PORTDbits.RD7=1;
        PORTDbits.RD6=0;
        PORTDbits.RD5=0;
        PORTDbits.RD4=1;
        __delay_ms(2);
    }
}