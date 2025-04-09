
#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"
#include "stdio.h"     
#include "stdarg.h"			
#include "string.h"    

#define usart1_rx_len_MAX  100  //�����ֽڼ������ֵ����һ�ν��յ��������������ֽڸ������ᱻDMA���´���ʼ��ַ��ʼת��
#define usart1_tx_len_MAX  100 

extern u8
    usart1_tx_package[usart1_tx_len_MAX],
    usart1_rx_package[usart1_rx_len_MAX];       //�����ֽڴ�ŵ����ݰ�
extern u16 usart1_rx_len;

void usart1dma_init(unsigned int bound);

#endif


