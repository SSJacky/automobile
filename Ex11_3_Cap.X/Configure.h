/* 
 * File:   Configure.h
 * Author: JACKY.SJ.SIA
 *
 * Created on 2021?5?25?, ?? 10:59
 */

#ifndef CONFIGURE_H
#define	CONFIGURE_H

typedef char                  int8_t;
typedef unsigned char         uint8_t;
typedef int                   int16_t;
typedef unsigned int          uint16_t;
typedef short long            int24_t;
typedef unsigned short long   uint24_t;
typedef long                  int32_t;
typedef unsigned long         uint32_t;

#define BIT0_TIME_INTERVAL_MAX 400
#define BIT0_TIME_INTERVAL_MIN 340
#define BIT1_TIME_INTERVAL_MAX 740
#define BIT1_TIME_INTERVAL_MIN 680
#define IR_LEADER_TIME_MAX 4300
#define IR_LEADER_TIME_MIN 4200
#define IR_REPEAT_COMMAND_H_MIN 0x7400
#define IR_REPEAT_COMMAND_H_MAX 0x7700
#define IR_REPEAT_COMMAND_L_MIN 0x0D00
#define IR_REPEAT_COMMAND_L_MAX 0x0E00
#define MAX_DATA_BUFFER 36
#define TRUE 1
#define FALSE 0
#define TMR0_VAL 40536
//#define TMR0_VAL 65510

#define QUEUE_MAX_TASK_NUM 50

#define DISABLE_INTERRUPT()       {INTCONbits.GIEL = 0;INTCONbits.GIEH = 0;}
#define ENABLE_INTERRUPT()        {INTCONbits.GIEL = 1;INTCONbits.GIEH = 1;}
#endif	/* CONFIGURE_H */

