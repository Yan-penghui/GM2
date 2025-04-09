#ifndef __LED_H
#define __LED_H

#include "FreeRTOS.h"	 //FreeRTOS配置头文件
#include "queue.h"		 //队列

#define YELLOW_LED PBout(4)
#define RED_LED PBout(5)
#define GREEN_LED PBout(8)

extern QueueHandle_t LED_xQueue;

void led_init(void);               //初始化	

#endif
