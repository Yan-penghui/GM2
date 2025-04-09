
#ifndef __USART3_H
#define __USART3_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"


#define usart3_rx_len_MAX  100  //�����ֽڼ������ֵ����һ�ν��յ��������������ֽڸ������ᱻDMA���´���ʼ��ַ��ʼת��
#define usart3_tx_len_MAX  100

extern u8
    usart3_tx_package[usart3_tx_len_MAX],
    usart3_rx_package[usart3_rx_len_MAX];       //�����ֽڴ�ŵ����ݰ�

extern u16 usart3_rx_len;


void usart3_printf(char* fmt,...);
void usart3dma_init(unsigned int bound);

#endif


