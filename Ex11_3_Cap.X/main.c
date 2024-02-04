//**********************************************************
//*              CCP2 for IR Remote use
//              2020 09 08
/*****************Author RJ******************/
/*Version S1.00B01*/
//**********************************************************
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
#include    "Main_App.h"
#include    "Struct_Def.h"

#pragma 	config	OSC=HS, BOREN=OFF, BORV = 2, PWRT=ON, WDT=OFF, LVP=OFF 
#pragma		config	CCP2MX = PORTBE	
#define OSC_CLOCK 10
/*Each count is 3.2us*/
/*Start Signal is 13.5ms*/
/*logic 0 take 1.12ms, logic 1 take 2.25ms */
/*Each command cycle without repeat is 125ms, include start and end*/

void CCP2_isr(void);	
void Rx_isr (void);
#define FORWARD             0x2A
#define BACKWARD            0x34
#define RIGHT_ROTATE        0x32
#define LEFT_ROTATE         0x2C
#define STOP                0x00

#define BUTTON_CH_MINUS         0xA2
#define BUTTON_CH               0x62
#define BUTTON_CH_PLUS          0xE2
#define BUTTON_PAUSE            0xC2
#define BUTTON_PLUSE            0xA8
#define BUTTON_MINUS            0xE0
#define BUTTON_PREV             0x22
#define BUTTON_NEXT             0x02
#define BUTTON_EQ               0x90
#define BUTTON_100_P            0x98
#define BUTTON_200_P            0xB0
#define BUTTON_0                0x68
#define BUTTON_1                0x30
#define BUTTON_2                0x18
#define BUTTON_3                0x7A
#define BUTTON_4                0x10
#define BUTTON_5                0x38
#define BUTTON_6                0x5A
#define BUTTON_7                0x42
#define BUTTON_8                0x4A
#define BUTTON_9                0x52

void delay_ms(long A);
//void delay_10us(long A);
void Delay_16us(long A);
unsigned int pwm_duty = 0;
unsigned int error_cnt = 0;
unsigned long time_cnt = 0;

#pragma code low_vector=0x18
void low_interrupt (void)
{
_asm GOTO Rx_isr _endasm
}
#pragma code
#pragma interruptlow Rx_isr
void Rx_isr (void)
{
    if(PIR1bits.RCIF == 1)
    {
        unsigned char RX_Temp;
        PIR1bits.RCIF = 0;		
        RX_Temp=ReadUSART();		
        if(RX_Temp=='c')
        {
            PORTA = 0x2a;
            Sys_Flag.wait_to_print = TRUE;
        }
        if(RX_Temp=='p')
        {
            PORTA = 0x34;
        }
        if(RX_Temp=='s')
        {
            PORTA = 0x00;
        }
    }
    if(INTCONbits.TMR0IF == 1)
    {
        /*10ms base, period = 1/(10MZ/4) = 0.4us*/
        /*0.01s/period = 25000*/
        INTCONbits.TMR0IF = 0;
        WriteTimer0(TMR0_VAL);
        time_cnt++;
        
        EnQueue(EVENT_10MS_TIME_TASK);
        
        if(time_cnt >= (long)100)
        {
            time_cnt = 0;
            EnQueue(EVENT_1S_TIME_TASK);
        }
    }
    if(INTCON3bits.INT2IF == 1)
    {
        INTCON3bits.INT2IF = 0;
        
        if(INTCON2bits.INTEDG2==0)
        {
            OpenTimer3( TIMER_INT_OFF &		
                T3_16BIT_RW &
                T3_SOURCE_INT &
                T3_PS_1_1 &
                T3_SYNC_EXT_ON  );
             WriteTimer3(0);
        }
        else
        {
            //CloseTimer3();
            IR_control.T3_value = ReadTimer3();
        }
        INTCON2bits.INTEDG2 ^= 1;

    }
}


#pragma code high_vector=0x08	
void high_interrupt (void)
{
    _asm 
    GOTO CCP2_isr 
    _endasm
}
#pragma code
#pragma interrupt CCP2_isr

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
    if(IR_control.wait_for_repeat_command_flag == TRUE)
    {
        IR_control.reset_repeat_counter = 0;
        if(IR_control.counter1 >= 2u)
        {
            IR_control.data_process_flag = TRUE;
        }
    }
    else if(IR_control.counter1 >= MAX_DATA_BUFFER)
    {
        IR_control.data_process_flag = TRUE;
        IR_control.counter1 = MAX_DATA_BUFFER - 1;
    }

    /*Save last step to calculate period*/
	EDGE_O.lt=EDGE_N.lt;
    /*Clear CCP module flag*/
	PIR2bits.CCP2IF = 0;		
}


void main () {
   
    unsigned char print_cnt = 0;
    unsigned char *a;
    long int bb;
    long ii = 0;
    System_Initial();
    bb = 320;
    printf(" bb = %d\n",bb);
    bb = (long)25*32000;
    printf("size of bb = %d\n",sizeof(bb));
    a = &print_cnt;
    printf(" bb = %d\n",bb);
    
    PORTB = 0x02;
    Delay_16us(1);
    PORTB = 0;
	while(1) {
        
        IPC_Proc();
        
         if(Sys_Flag.wait_to_print == TRUE)
         {
            Sys_Flag.wait_to_print = FALSE;
            memset(&IR_control,0,sizeof(IR_CONTROL));
            memset(&EDGE_O,0,sizeof(EDGE_O));
            memset(&EDGE_N,0,sizeof(EDGE_N));
         }

        
        if(IR_control.data_process_flag)
        {
            IR_Data_Process();
        }
        
        if(IR_control.data_process_done)
        {
            switch(IR_control.command)
            {

                case BUTTON_PLUSE:/*+*/
                    PORTA = FORWARD;
//                    if(pwm_duty == 0)
//                    {
//                        pwm_duty = 0;
//                    }
//                    else
//                    {
//                        pwm_duty-=5u;
//                    }
//                    printf("PWM Duty = %d\n",pwm_duty);
                    printf("FORWORD\n");
                    break;
                case BUTTON_MINUS:/*-*/
                     PORTA = BACKWARD;
//                    pwm_duty+=5u;
//                    printf("PWM Duty = %d\n",pwm_duty);
                    break;
                case BUTTON_PAUSE:/*stop*/
                     PORTA = STOP;
//                    pwm_duty = 0u;
//                    printf("PWM Duty = %d\n",pwm_duty);
                    break;
                case BUTTON_PREV:
                    PORTA = LEFT_ROTATE;
                    delay_ms(320);
                    PORTA = STOP;
                    break;
                case BUTTON_NEXT:
                    PORTA = RIGHT_ROTATE;
                    delay_ms(320);
                    //Delay_us(20000);
                    PORTA = STOP;
                    break;
                case BUTTON_0:
                    PORTA = RIGHT_ROTATE;
                    delay_ms(640);
                    PORTA = STOP;
                    break;
                default:    
                    printf("command = %d\n",IR_control.command);
                    break;
                    
            }
            IR_control.data_process_done = FALSE;
        }

      SetDCPWM1((unsigned int)pwm_duty); 
   }
}
void delay_ms(long A) {
//This function is only good for OSC_CLOCK higher than 4MHz
	long i;
	int us2TCY;
	us2TCY=(10*OSC_CLOCK)>>2;
	for(i=0;i<A;i++) Delay100TCYx(us2TCY);		
}
//void delay_10us(long A) {
////This function is only good for OSC_CLOCK higher than 4MHz
//	long i;
//	long us2TCY;
//	us2TCY = 25;
//	for(i=0;i<A*25;i++) Delay1TCY();		
//}
void Delay_16us(long A)
{
    long i;
    for(i=0;i<A;i++)
    {
        Delay1TCY();
    }

}
