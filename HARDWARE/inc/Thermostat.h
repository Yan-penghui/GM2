#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#ifndef __THERMOSTAT_H
#define  __THERMOSTAT_H


#define Thermostat_RX_BUF       usart3_rx_package       //����3���� �¿���
#define Thermostat_RXBUFF_SIZE  usart3_rx_len_MAX  			//����3���� �¿���

#define reTEMchange             0					//�����¶�����ֵ

extern int PVTem ,SVTem,RunTime  ,Pno,TEMchange;
extern volatile int Srun; 
extern volatile int Output;
void Thermostat_init(void)	;	
int Send_Thermostat(char operate,u8 name,u16 w_data);
extern void errormessage(int error);
void Serial_SendArray(u8 *Array, int Length,int usart);

#endif



