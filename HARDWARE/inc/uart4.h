
#ifndef __UART4_H
#define __UART4_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"


#define uart4_rx_len_MAX  100  //接收字节计数最大值，若一次接收到超过此数量的字节个数，会被DMA重新从起始地址开始转运
#define uart4_tx_len_MAX  100 

extern u8
    uart4_tx_package[100],
    uart4_rx_package[100];       //连续字节存放的数据包

extern u16 uart4_rx_len;


void uart4_printf(char* fmt,...);
void uart4dma_init(unsigned int bound);

#endif


