/* 
 * File:   Struct_Def.h
 * Author: JACKY.SJ.SIA
 *
 * Created on 2021?5?25?, ?? 10:25
 */
#ifndef STRUCT_DEF_H
#define	STRUCT_DEF_H

#include "Configure.h"

typedef struct {
    uint8_t w_index;
    uint8_t r_index;
    int queue[50];
}QUEUE_DATA;

enum TASK
{
  EVENT_10MS_TIME_TASK = 0,
  EVENT_1S_TIME_TASK,
};

typedef struct {
    unsigned int counter1;
    unsigned int result2;
    unsigned int data[60];
    unsigned char data_process_flag :1;
    unsigned char data_process_done :1;
    unsigned char data_printf_flag :1;
    unsigned char wait_for_repeat_command_flag :1;
    unsigned char address;
    unsigned char _address;
    unsigned char command;
    unsigned char _command;
    
    unsigned int error_counter;
    unsigned int reset_repeat_counter;
    
    unsigned int T3_value;
}IR_CONTROL;

typedef struct{

    unsigned char wait_to_print :1;

}SYS_FLAG;

typedef union {
	unsigned int lt;
	unsigned char bt[2];
}EDGE;

extern EDGE EDGE_O;
extern EDGE EDGE_N;

extern IR_CONTROL IR_control;
extern SYS_FLAG Sys_Flag;
extern QUEUE_DATA Queue_Data;

//typedef enum{
//    10MS_TASK = 0,
//    1S_TASK
//};


#endif	/* STRUCT_DEF_H */

