
#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"
#include "stdio.h"     
#include "stdarg.h"			
#include "string.h"    

#define usart1_rx_len_MAX  100  //接收字节计数最大值，若一次接收到超过此数量的字节个数，会被DMA重新从起始地址开始转运
#define usart1_tx_len_MAX  100 

extern u8
    usart1_tx_package[usart1_tx_len_MAX],
    usart1_rx_package[usart1_rx_len_MAX];       //连续字节存放的数据包
extern u16 usart1_rx_len;

void usart1dma_init(unsigned int bound);

#endif


