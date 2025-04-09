
#ifndef __USART2_H
#define __USART2_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"

#define usart2_rx_len_MAX  100  //接收字节计数最大值，若一次接收到超过此数量的字节个数，会被DMA重新从起始地址开始转运
#define usart2_tx_len_MAX  100

extern u8
    usart2_tx_package[usart2_tx_len_MAX],
    usart2_rx_package[usart2_rx_len_MAX];       //连续字节存放的数据包

extern u16 usart2_rx_len;  


void usart2dma_init(unsigned int bound);
void u2_printf(char*,...) ;          
void u2_TxData(unsigned char *data);

#endif
