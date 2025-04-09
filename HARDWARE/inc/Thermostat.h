#include "stm32f10x.h"  //包含需要的头文件
#ifndef __THERMOSTAT_H
#define  __THERMOSTAT_H


#define Thermostat_RX_BUF       usart3_rx_package       //串口3控制 温控器
#define Thermostat_RXBUFF_SIZE  usart3_rx_len_MAX  			//串口3控制 温控器

#define reTEMchange             0					//出厂温度修正值

extern int PVTem ,SVTem,RunTime  ,Pno,TEMchange;
extern volatile int Srun; 
extern volatile int Output;
void Thermostat_init(void)	;	
int Send_Thermostat(char operate,u8 name,u16 w_data);
extern void errormessage(int error);
void Serial_SendArray(u8 *Array, int Length,int usart);

#endif



