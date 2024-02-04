/*******************************************************************************
  @File     Main_App.c
  @Author  SJ 
  @Version  
  @Date    2021/05/26
  @Brief    
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
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

/* Public Functions Prototypes -----------------------------------------------*/
void System_Initial(void);
int IR_Data_Process(void);
void Shift_Right(unsigned char* data_result, unsigned char shift, unsigned char logic);
uint8_t EnQueue(uint8_t index);
uint8_t DeQueue(uint8_t* IPC);
void IPC_Proc(void);
void IPC_Event_Proc(uint8_t ipc_data);
void Event_Task_10ms(void);
void Event_Task_1s(void);
/* Private Function Prototypes -----------------------------------------------*/

/* Private Variables ---------------------------------------------------------*/

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
void System_Initial(void)
{
    PORTD = 0x00;			
	TRISD = 0; 				
    TRISA = 0; 
    PORTA = 0b00;
    PORTB = 0b00;
    TRISBbits.RB2 = 1;
    TRISBbits.RB1 = 0;
//  OpenADC(ADC_FOSC_32 & ADC_LEFT_JUST & ADC_20_TAD,
//          ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & 
//          ADC_VREFMINUS_VSS, 14);
    INTCON2bits.INTEDG2 = 0;	
    INTCON3bits.INT2IP = 0;
    INTCON3bits.INT2IE = 0;

    ADCON0=0x01;		
    ADCON1=0x0E;		
    ADCON2=0x3A;		
    /*use as printf*/
    OpenUSART( 	USART_TX_INT_OFF &	
                USART_RX_INT_ON &	
                USART_ASYNCH_MODE &	
                USART_EIGHT_BIT &
                USART_CONT_RX &
                USART_BRGH_HIGH,
                64 );
    
    /*Uart Receive flag*/
    PIR1bits.RCIF = 0;		
    /*Uart priority = low*/
	IPR1bits.RCIP = 0;		
    
    /*10Mhz system clock*/
    /*32.768kHZ only T1 use*/
    OpenTimer0( TIMER_INT_ON &		
				T0_16BIT &
				T0_SOURCE_INT &
				T0_PS_1_1 &
                T0_EDGE_FALL );
    WriteTimer0(TMR0_VAL);	
    
    INTCON2bits.TMR0IP = 0;
    /*CCP2 Use Timer1 */
    /*Sysclock = 10MHz  Resolution = 3.2us*/
    OpenTimer1( TIMER_INT_OFF &		
				T1_16BIT_RW &
				T1_SOURCE_INT &
				T1_PS_1_8 &
				T1_OSC1EN_OFF &
				T1_SYNC_EXT_ON );
    
	TRISCbits.TRISC2=0;		// CCP1?
	OpenPWM1(0x9B);   		// PR = 3
    SetDCPWM1(0x02);		// duty cycle is equal to PWM pulse width
    /*PWM Clock Timer2*/
	OpenTimer2(TIMER_INT_OFF&T2_PS_1_4&T2_POST_1_1);	
    /*Set CCP use pin as input*/
	TRISBbits.TRISB3=1;		
	/*Configure CCP2*/
    /*CCP Clock source sets at T3CON*/
    /*Now use Timer1 as CCP source*/
    memset(&IR_control,0,sizeof(IR_CONTROL));
	OpenCapture2(CAPTURE_INT_ON & C2_EVERY_FALL_EDGE);
    /*Set CCP2 as High priority interrupt*/
    IPR2bits.CCP2IP = 1;
    /*Interrupt priority enable*/
	RCONbits.IPEN = 1;		
    /*Enable Low priority interrupt*/
    INTCONbits.GIEL = 1;
    /*Enable High priority interrupt*/
	INTCONbits.GIEH = 1;		 

}
/***********************************************************************
  *Name         : IR_Data_Process
  *Description  : IR_Data_Process
  *Parameter    : none
  *Return       : none
  *Examples     :
                        @code

                        @endcode
***********************************************************************/
int IR_Data_Process(void)
{
    
    unsigned char i = 0;
    unsigned char custom_ptr = 0;
    unsigned char _custom_ptr = 0;
    unsigned char data_ptr,_data_ptr;
    
    if(IR_control.wait_for_repeat_command_flag == TRUE)
    {
        if((IR_control.data[0] < IR_REPEAT_COMMAND_H_MAX) && (IR_control.data[0] > IR_REPEAT_COMMAND_H_MIN))
        {
            if((IR_control.data[1] < IR_REPEAT_COMMAND_L_MAX) && (IR_control.data[1] > IR_REPEAT_COMMAND_L_MIN))
            {
                IR_control.data_process_done = TRUE;
                IR_control.data_process_flag = FALSE;
                IR_control.counter1 = 0;
                return 0; 
            }
            else
            {
                memset(&IR_control,0,sizeof(IR_CONTROL));
                memset(&EDGE_O,0,sizeof(EDGE_O));
                memset(&EDGE_N,0,sizeof(EDGE_N));
                return -1;
            }
        }
        else
        {
            memset(&IR_control,0,sizeof(IR_CONTROL));
            memset(&EDGE_O,0,sizeof(EDGE_O));
            memset(&EDGE_N,0,sizeof(EDGE_N));
            return -1;            
        }
    }
    if ((IR_control.data[1] < IR_LEADER_TIME_MIN) || (IR_control.data[1] > IR_LEADER_TIME_MAX))
    {
        IR_control.data_process_flag = FALSE;
        IR_control.data_process_done = FALSE;
        IR_control.counter1 = 0;
        return -1;
    } 
    for (i=0; i<8; i++)
    {
      // custom code
        custom_ptr = i+2;
        if ((IR_control.data[custom_ptr] >= BIT0_TIME_INTERVAL_MIN) && (IR_control.data[custom_ptr] <= BIT0_TIME_INTERVAL_MAX))
        {
            Shift_Right(&IR_control.address, 1, 0);
        }
        else  if ((IR_control.data[custom_ptr] >= BIT1_TIME_INTERVAL_MIN) && (IR_control.data[custom_ptr] <= BIT1_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control.address, 1, 1);
        }
      
        //custom' code
        _custom_ptr = i+10;
        if ((IR_control.data[_custom_ptr] >= BIT0_TIME_INTERVAL_MIN) && (IR_control.data[_custom_ptr] <= BIT0_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control._address, 1, 0);
        }
        else  if ((IR_control.data[_custom_ptr] >= BIT1_TIME_INTERVAL_MIN) && (IR_control.data[_custom_ptr] <= BIT1_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control._address, 1, 1);
        }
      
        // data code
        data_ptr = i+18;
        if ((IR_control.data[data_ptr] >= BIT0_TIME_INTERVAL_MIN) && (IR_control.data[data_ptr] <= BIT0_TIME_INTERVAL_MAX))
        {
            Shift_Right(&IR_control.command, 1, 0);
        }
        else  if ((IR_control.data[data_ptr] >= BIT1_TIME_INTERVAL_MIN) && (IR_control.data[data_ptr] <= BIT1_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control.command, 1, 1);
        }
      
        // data' code
        _data_ptr = i+26;
        if ((IR_control.data[_data_ptr] >= BIT0_TIME_INTERVAL_MIN) && (IR_control.data[_data_ptr] <= BIT0_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control._command, 1, 0);
        }
        else  if ((IR_control.data[_data_ptr] >= BIT1_TIME_INTERVAL_MIN) && (IR_control.data[_data_ptr] <= BIT1_TIME_INTERVAL_MAX)) 
        {
            Shift_Right(&IR_control._command, 1, 1);
        }
  }

  if ((IR_control.address != (IR_control._address^0xFF)) || (IR_control.command != (IR_control._command^0xFF)))
  {
    memset(&IR_control,0,sizeof(IR_CONTROL));
    memset(&EDGE_O,0,sizeof(EDGE_O));
    memset(&EDGE_N,0,sizeof(EDGE_N));
    return -1;
  }

  IR_control.data_process_flag = FALSE;
  IR_control.data_process_done = TRUE;
  IR_control.wait_for_repeat_command_flag = TRUE;
  IR_control.counter1 = 0;
  return 0;
    
}
/***********************************************************************
  *Name         : IR_Data_Task
  *Description  : IR_Data_Task
  *Parameter    : none
  *Return       : none
  *Examples     :
                        @code

                        @endcode
***********************************************************************/
//void IR_Data_Process(void)
//{
//
//    
//}
void Shift_Right(unsigned char* data_result, unsigned char shift, unsigned char logic)
{
    *data_result = (*data_result << shift) | logic;
}
uint8_t EnQueue(uint8_t index)
{
    static is_buff_full = 0;
    
    DISABLE_INTERRUPT();
    if((Queue_Data.w_index + 1) % QUEUE_MAX_TASK_NUM == Queue_Data.r_index)
    {
        is_buff_full = TRUE;
        ENABLE_INTERRUPT();
        return FALSE;
    }
    else
    {
        is_buff_full = FALSE;
        Queue_Data.w_index = (Queue_Data.w_index + 1) % QUEUE_MAX_TASK_NUM;
        Queue_Data.queue[Queue_Data.w_index] = index;
    }
    ENABLE_INTERRUPT();
    return TRUE;
}
uint8_t DeQueue(uint8_t* IPC)
{
    static is_buff_Empty = 0;
    
    DISABLE_INTERRUPT();
    if(Queue_Data.r_index == Queue_Data.w_index)
    {
        is_buff_Empty = TRUE;
        ENABLE_INTERRUPT();
        return FALSE;
    }
    else
    {
        is_buff_Empty = FALSE;
        *IPC =  Queue_Data.queue[Queue_Data.r_index];
        Queue_Data.r_index = (Queue_Data.r_index + 1) % QUEUE_MAX_TASK_NUM;
    }
    ENABLE_INTERRUPT();
    return TRUE;
    
}
void IPC_Proc(void)
{
  uint8_t ipc_data;

  if(DeQueue(&ipc_data) == TRUE)
  {
    IPC_Event_Proc(ipc_data);
  }
}
void IPC_Event_Proc(uint8_t ipc_data)
{
  switch(ipc_data)
  {
  case EVENT_10MS_TIME_TASK:
    Event_Task_10ms();
    break;
  case EVENT_1S_TIME_TASK:
    Event_Task_1s();
    break;

  default:
    break;
  }
}
void Event_Task_10ms(void)
{
    IR_control.T3_value = ReadTimer3();
    printf("T3_value = %d\n",IR_control.T3_value);
    if(IR_control.wait_for_repeat_command_flag == TRUE)
    {
        if(++IR_control.reset_repeat_counter>=15u)
        {
            IR_control.wait_for_repeat_command_flag = FALSE;
            IR_control.reset_repeat_counter = 0;
        }
    }
}
void Event_Task_1s(void)
{
    //IR_control.T3_value++;
    //IR_control.T3_value = ReadTimer3();
    //printf("T3_value = %d\n",IR_control.T3_value);
    PORTD++;
    if(IR_control.counter1 != 0)
    {
        IR_control.error_counter++;
        if(IR_control.error_counter >= (unsigned int)2)
        {
            IR_control.counter1 = 0;
            IR_control.error_counter = 0;
        }          
    }
    else
    {
        IR_control.error_counter = 0;
    }
}
