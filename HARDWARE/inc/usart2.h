
#ifndef __USART2_H
#define __USART2_H

#include "stdio.h"      
#include "stdarg.h"		 
#include "string.h"    
#include "stm32f10x.h"

#define usart2_rx_len_MAX  100  //�����ֽڼ������ֵ����һ�ν��յ��������������ֽڸ������ᱻDMA���´���ʼ��ַ��ʼת��
#define usart2_tx_len_MAX  100

extern u8
    usart2_tx_package[usart2_tx_len_MAX],
    usart2_rx_package[usart2_rx_len_MAX];       //�����ֽڴ�ŵ����ݰ�

extern u16 usart2_rx_len;  


void usart2dma_init(unsigned int bound);
void u2_printf(char*,...) ;          
void u2_TxData(unsigned char *data);

#endif
