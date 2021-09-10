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

// CONFIG1
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
//#include <stdint.h>
#include "Osc_config.h"
/*-----------------------------------------------------------------------------
 ----------------------- VARIABLES A IMPLEMTENTAR------------------------------
 -----------------------------------------------------------------------------*/
//-------DIRECTIVAS DEL COMPILADOR
#define _XTAL_FREQ 4000000
//-------VARIABLES DE PROGRAMA
unsigned char antirrebote1;  //para boton de inicio
unsigned char antirrebote2;  //para boton de parar
unsigned char antirrebote3;  //para boton de retorno
unsigned char bot_encendido, bot_apagado, bot_retorno;
unsigned char encendido[8] = {0b1000,       //paso 1
                                0b1100,     //paso 2
                                0b0100,     //paso 3
                                0b0110,     //paso 4
                                0b0010,     //paso 5
                                0b0011,     //paso 6
                                0b0001,     //paso 7 
                                0b1001      //paso 8
                                };    
unsigned char apagado = 0b0000;             //se apaga el motor
unsigned char retorno[8] = {0b1001,         //paso 8
                            0b0001,         //paso 7
                            0b0011,         //paso 6
                            0b0010,         //paso 5
                            0b0110,         //paso 4
                            0b0100,         //paso 3
                            0b1100,         //paso 2
                            0b1000          //paso 1
                            };
/*-----------------------------------------------------------------------------
 ------------------------ PROTOTIPOS DE FUNCIONES ------------------------------
 -----------------------------------------------------------------------------*/
void setup(void);
void antirrebotes(void);
void motor_encendido(void);
void motor_apagado(void);
void retorno_motor(void);
/*-----------------------------------------------------------------------------
 --------------------------- INTERRUPCIONES -----------------------------------
 -----------------------------------------------------------------------------*/
void __interrupt() isr(void) //funcion de interrupciones
{   
    //-------INTERRUPCION POR BOTONAZO
    if (INTCONbits.RBIF)
    {
        switch(PORTB)
        {
            default:
                antirrebote1=0;
                antirrebote2=0;
                antirrebote3=0;
                break;
            case(0b11111110):       //caso interrupcion encendido
                antirrebote1=1;
                break;
            case(0b11111101):       //caso interrupcion apagado
                antirrebote2=1;
                break;
            case(0b11111100):       //caso interrupcion returno
                antirrebote3=1;
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
        //-------FUNCION PARA 3 ANTIRREBOTES
        antirrebotes();
        //-------EVALUACION SI SE PRENDE
        if (bot_encendido==1 && bot_apagado==0 && bot_retorno==0)
        {
            for(int i=0; i<1000;i++)
            {
                motor_encendido();
            }
        }
        //-------EVALUACION SI SE PIDE RETORNO
        else if (bot_encendido==0 && bot_apagado==0 && bot_retorno==1)
        {
            for(int i=0; i<1000;i++)
            {
                retorno_motor();
            }
        }
        //-------EVALUACION SI SE APAGA
        else if(bot_encendido==0 && bot_apagado==1 && bot_retorno==0)
            motor_apagado();
        
        //-------EVALUACION SI NO SE HACE NADA
        else
            motor_apagado();
        
        
    }
    return;
}
/*-----------------------------------------------------------------------------
 ---------------------------------- SET UP -----------------------------------
 -----------------------------------------------------------------------------*/
void setup(void)
{
    //-------CONFIGURACION ENTRADAS ANALOGICAS
    ANSEL=0;
    ANSELH=0;
    //-------CONFIGURACION DE PUERTOS
    TRISA=0;        //salida para los pines de los motores
    TRISB=1;        //entrada para los botones de modos
    TRISD=0;        //entrada para los botones de modos
    //-------LIMPIEZA DE PUERTOS
    PORTA=0;        //se limpia puerto A
    PORTB=0;        //se limpia puerto B
    PORTD=0;        //se limpia puerto B
    //-------CONFIGURACION DE RELOJ
    osc_config(4);
    //-------CONFIGURACION DE WPUB
    OPTION_REGbits.nRBPU=0;             //se activan WPUB
    //WPUBbits.WPUB0=1;                   //RB0, boton encendido
    WPUBbits.WPUB1=1;                   //RB1, boton apagado
    //WPUBbits.WPUB2=1;                   //RB2, boton retorno
    //-------CONFIGURACION DE INTERRUPCIONES
    INTCONbits.GIE=1;                   //se habilita interrupciones globales
    INTCONbits.PEIE = 1;                //habilitan interrupciones por perifericos
    INTCONbits.RBIE=1;                  //se  habilita IntOnChange B
    INTCONbits.RBIF=0;                  //se  apaga bandera IntOnChange B
    IOCBbits.IOCB0=1;                   //habilita IOCB RB0
    IOCBbits.IOCB1=1;                   //habilita IOCB RB1
    IOCBbits.IOCB2=1;                   //habilita IOCB RB2
}
/*-----------------------------------------------------------------------------
 --------------------------------- FUNCIONES ----------------------------------
 -----------------------------------------------------------------------------*/
//-------FUNCION PARA 3 ANTIRREBOTES
void antirrebotes(void)
{
    //-------ANTIRREBOTE DE BOTON ENCENDIDO
    if (antirrebote1==1 && PORTBbits.RB0==0)
    {
        antirrebote1=0;
        PORTD++;
        bot_encendido++;
        if (bot_encendido>=2)
            bot_encendido=0;
    }
    else
        bot_encendido=0;
    //-------ANTIRREBOTE DE BOTON APAGADO
    if (antirrebote2==1 && PORTBbits.RB1==0)
    {
        antirrebote2=0;
        bot_apagado++;
        if(bot_apagado>=2)
            bot_apagado=0;
    }
    else
        bot_apagado=0;
    //-------ANTIRREBOTE DE BOTON RETORNO
    if (antirrebote3==1 && PORTBbits.RB2==0)
    {
        antirrebote3=0;
        bot_retorno++;
        if(bot_retorno>=2)
            bot_retorno=0;
    }
    else
        bot_retorno=0;
    return;
}

//-------FUNCION PARA MOTOR ENCENDIDO
void motor_encendido(void)
{
    PORTA=encendido[1];     //pines en paso 1
    __delay_ms(1);
    PORTA=encendido[2];     //pines en paso 2
    __delay_ms(1);
    PORTA=encendido[3];     //pines en paso 3
    __delay_ms(1);
    PORTA=encendido[4];     //pines en paso 4
    __delay_ms(1);
    PORTA=encendido[5];     //pines en paso 5
    __delay_ms(1);
    PORTA=encendido[6];     //pines en paso 6
    __delay_ms(1);
    PORTA=encendido[7];     //pines en paso 7
    __delay_ms(1);
    PORTA=encendido[8];     //pines en paso 8
    __delay_ms(1);
    
}

//-------FUNCION PARA MOTOR APAGADO
void motor_apagado(void)
{
    PORTA=apagado;          //pines en 0, se apaga el motor
}

//-------FUNCION PARA RETORNO DE MOTOR
void retorno_motor(void)
{
    PORTA=retorno[8];       //pines en paso 8
    __delay_ms(1);
    PORTA=retorno[7];       //pines en paso 7
    __delay_ms(1);
    PORTA=retorno[6];       //pines en paso 6
    __delay_ms(1);
    PORTA=retorno[5];       //pines en paso 5
    __delay_ms(1);
    PORTA=retorno[4];       //pines en paso 4
    __delay_ms(1);
    PORTA=retorno[3];       //pines en paso 3
    __delay_ms(1);
    PORTA=retorno[2];       //pines en paso 2
    __delay_ms(1);
    PORTA=retorno[1];       //pines en paso 1
    __delay_ms(1);
}