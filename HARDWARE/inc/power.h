#include "stm32f10x.h"  //包含需要的头文件
#ifndef __POWER_H
#define __POWER_H

#define power_RxCounter  		  usart2_rx_len    //串口2控制 温控器
#define power_RX_BUF       		usart2_rx_package       //串口2控制 温控器
#define power_RXBUFF_SIZE  		usart2_rx_len_MAX  //串口2控制 温控器





extern float v_opt ,i_opt ,v_ipt ;


void power_init(void)	;	
int Send_power(char operate,u8 name,u16 w_data);
extern void errormessage(int error);


#endif



