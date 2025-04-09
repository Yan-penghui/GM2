
#ifndef __UART4_H
#define __UART4_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"


#define uart4_rx_len_MAX  100  //�����ֽڼ������ֵ����һ�ν��յ��������������ֽڸ������ᱻDMA���´���ʼ��ַ��ʼת��
#define uart4_tx_len_MAX  100 

extern u8
    uart4_tx_package[100],
    uart4_rx_package[100];       //�����ֽڴ�ŵ����ݰ�

extern u16 uart4_rx_len;


void uart4_printf(char* fmt,...);
void uart4dma_init(unsigned int bound);

#endif


