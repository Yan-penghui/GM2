#ifndef _DOOR_H_
#define _DOOR_H_

#include "stm32f10x.h"  
#define door_outtime   30  //30s

extern volatile u8 door_flag;


void updoor(u8 run_time);
void downdoor(u8 run_time);
void stopdoor(void);
void door(u8 instruct,u8 runtime,u8 outtime);
#endif

