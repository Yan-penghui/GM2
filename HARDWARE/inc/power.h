#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#ifndef __POWER_H
#define __POWER_H

#define power_RxCounter  		  usart2_rx_len    //����2���� �¿���
#define power_RX_BUF       		usart2_rx_package       //����2���� �¿���
#define power_RXBUFF_SIZE  		usart2_rx_len_MAX  //����2���� �¿���





extern float v_opt ,i_opt ,v_ipt ;


void power_init(void)	;	
int Send_power(char operate,u8 name,u16 w_data);
extern void errormessage(int error);


#endif



