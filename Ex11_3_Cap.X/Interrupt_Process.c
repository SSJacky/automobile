/*******************************************************************************
  @File     Main_App.c
  @Author  SJ 
  @Version  
  @Date    2021/05/26
  @Brief    
*******************************************************************************/
#include    "Main_App.h"
#include 	<p18f4520.h> 	
#include 	<delays.h> 		
#include 	<adc.h>			
#include 	<timers.h>		
#include 	<pwm.h>			
#include 	<capture.h>		
#include 	<usart.h>		
#include    <string.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    "Configure.h"
#include    "Struct_Def.h"
#include    "Interrupt_Process.h"


/* Public Functions Prototypes -----------------------------------------------*/
void CCP2_isr(void);	
void Rx_isr (void);
/* Private Function Prototypes -----------------------------------------------*/

/* Private Variables ---------------------------------------------------------*/
unsigned int time_cnt = 0;
/* Functions -----------------------------------------------------------------*/
/***********************************************************************
  *Name         : System_Initial
  *Description  : System Initial
  *Parameter    : none
  *Return       : none
  *Examples     :
                        @code

                        @endcode
***********************************************************************/
void Rx_isr (void)
{
    if(PIR1bits.RCIF == 1)
    {
        unsigned char RX_Temp;
        PIR1bits.RCIF = 0;		
        RX_Temp=ReadUSART();		
        if(RX_Temp=='c')
        {
            Sys_Flag.wait_to_print = TRUE;
        }
    }
    if(INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;
        WriteTimer0(TMR0_VAL);
        time_cnt++;
    }
    if(time_cnt>=(unsigned int)100)
    {
        time_cnt = 0;
        PORTD++;
    }
}
/***********************************************************************
  *Name         : System_Initial
  *Description  : System Initial
  *Parameter    : none
  *Return       : none
  *Examples     :
                        @code

                        @endcode
***********************************************************************/
void CCP2_isr (void)
{
    /*Read CCPR2L & CCPR2H value, which was counted by TMR1*/
	EDGE_N.lt=ReadCapture2();	
    
    /*Calculate period and avoid overflow*/
    if(EDGE_N.lt > EDGE_O.lt)
    {
        IR_control.result2 = EDGE_N.lt - EDGE_O.lt;
    }
    else
    {
        IR_control.result2 = (EDGE_N.lt - EDGE_O.lt) + (0xFFFFuL);
    }
    
    IR_control.data[IR_control.counter1] = IR_control.result2;
    IR_control.counter1++;
    IR_control.data_printf_flag = TRUE;
    if(IR_control.counter1 >= MAX_DATA_BUFFER)
    {
        IR_control.data_process_flag = TRUE;
        IR_control.counter1 = MAX_DATA_BUFFER - 1;
    }

    /*Save last step to calculate period*/
	EDGE_O.lt=EDGE_N.lt;
    /*Clear CCP module flag*/
	PIR2bits.CCP2IF = 0;		
}
