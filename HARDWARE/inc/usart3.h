
#ifndef __USART3_H
#define __USART3_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"


#define usart3_rx_len_MAX  100  //接收字节计数最大值，若一次接收到超过此数量的字节个数，会被DMA重新从起始地址开始转运
#define usart3_tx_len_MAX  100

extern u8
    usart3_tx_package[usart3_tx_len_MAX],
    usart3_rx_package[usart3_rx_len_MAX];       //连续字节存放的数据包

extern u16 usart3_rx_len;


void usart3_printf(char* fmt,...);
void usart3dma_init(unsigned int bound);

#endif


