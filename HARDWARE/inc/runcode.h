
#ifndef __runcode_H
#define __runcode_H

#include "FreeRTOS.h"	 //FreeRTOS配置头文件
#include "queue.h"		 //队列   	
#include "task.h"

#define rehoting_doorrun_alltime 25*10			//单位：0.1s


extern QueueHandle_t runcode_xQueue;
extern TaskHandle_t Runcode_Task_Handler;

void my_runcode_task(void *pvParameters);
void heating(u8 run_p_num,u8 all_p_num,float starttem,float endtem,u16 time,u8 fan);
void errormessage(volatile int error);
#endif


