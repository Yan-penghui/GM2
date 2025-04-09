#ifndef __SWITCH_H
#define __SWITCH_H

#include "stm32f10x.h" 

#define LIM1 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) //限位开关1 上
#define LIM2 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) //限位开关2 下
#define UP PBout(13)// 电机向上
#define DOWN PBout(12)// 电机向下
#define FAN_DOWN PBout(14)//  下风扇启停
#define FAN_UP PBout(15)//  上风扇启停 

#define BUZZER PCout(13)//蜂鸣器
#define POWER PAout(8)// 负载通断
#define RS485 PCout(4)
#define RS485_POWER PCout(5)

void Switch_Init(void);

#endif
