
/*-------------------------------------------------*/
/*                                                 */
/*          	       ����2                  	   */
/*                 SMPS���ص�Դmodbusͨ��        */
/*-------------------------------------------------*/

#include "stm32f10x.h"
#include "usart2.h"
#include "Thermostat.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "usart2.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "sys.h"

extern QueueHandle_t U2_xQueue;

u16 usart2_rx_len = 0;     //�����ֽڼ���
u8	usart2_tx_package[usart2_tx_len_MAX],
 usart2_rx_package[usart2_rx_len_MAX];       //�����ֽڴ�ŵ����ݰ�

volatile bool is_usart2_receive_ok = false;    //�����Ƿ��ѽ������

void USART2_IRQHandler(void)
{

  if( USART_GetITStatus(USART2, USART_IT_IDLE) != RESET )
    {
      USART2 -> SR;  //����һ��SR�Ĵ���
      USART2 -> DR;  //����һ��DR�Ĵ���

      DMA_Cmd(DMA1_Channel6, DISABLE);
      usart2_rx_len = usart2_rx_len_MAX - DMA_GetCurrDataCounter(DMA1_Channel6);
      DMA_SetCurrDataCounter( DMA1_Channel6, sizeof(usart2_rx_package) );
      DMA_Cmd(DMA1_Channel6, ENABLE);

      usart2_rx_package[usart2_rx_len] = '\0';          //�����һλ���Ͻ�����
      is_usart2_receive_ok = true;                //���һ֡�����ѽ������

      USART_ClearITPendingBit(USART2, USART_IT_IDLE);  //���IDLE�жϱ�־λ
      xQueueSendToBackFromISR( U2_xQueue,&usart2_rx_package,NULL);


    }
}



/* @brief ��ȡ������ϵı�־λ���鿴�Ƿ��ѽ�����ɡ�
 * @retval 1: �������
 *         0: δ������� */
u8 get_usart2_TC_flag(void)
{
  if(is_usart2_receive_ok == true)
    {
      is_usart2_receive_ok = false;
      return 1;//�����������ɣ�����1
    }
  else
    return 0;//����δ��ɣ�����0
}

/* @brief ��ȡ���յ������ݰ���
 * @retval  ���յ������ݰ�����һ������Ϊ256��u8����ָ�룬�н�����'\0'�� */
u8* get_usart2_rx_package(void)
{
  return usart2_rx_package;
}

/* @brief ��ȡ���յ������ݰ����ȡ�
 * @retval ������һ֡���ݵĳ��� */
u16 get_usart2_rx_len(void)
{
  return usart2_rx_len;
}

/* @brief ������ݰ���ʵ���ǰѵ�һλ�ý����������*/
void clear_usart2_rx_package(void)
{
  usart2_rx_package[0] = '\0';
}


/* @brief ����ʽ�ط���һ���ֽڡ�*/
void usart2_send_byte_blocking(u8 data)
{
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
  USART_SendData(USART2, data);
}

/* @brief uart_ch340���ͺ�����printf�Ĳ�����ʽ*/
void usart2_printf(char* fmt,...)
{
  va_list ap;
  va_start(ap, fmt);
  vsprintf((char*)usart2_tx_package, fmt, ap);
  va_end(ap);
  u16 len = strlen( (char*)usart2_tx_package );
  for(u16 i=0; i<len; i++)
    usart2_send_byte_blocking(usart2_tx_package[i]);
}



/* @brief ��ʼ��ch340������MCU�Ĵ������衣*/
static void usart2_init(unsigned int bound)
{
  /*��ʼ�����ͽ������ţ�PB10(usart2_TX)��PB11(usart2_RX)��*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*��ʼ��usart2���衣*/
  USART_DeInit(USART2);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_BaudRate =  bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART2, &USART_InitStructure);
  USART_Cmd(USART2, ENABLE);
}

/* @brief ��ʼ��ch340������MCU���ڵĿ����ж�*/
static void usart2_idle_interrupt_init(void)
{
  USART_ClearFlag(USART2, USART_IT_IDLE);
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  /*�������������������ȼ����顣*/
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* @brief ��ʼ��ch340������MCU�Ĵ��ڵĽ��ռĴ���DMAת�ˡ�*/
static void usart2_rx_dma_init(void)
{
  /*����usart2�Ľ��ռĴ�����DMAͨ������ο��ֲᣬ��DMA1ͨ��6��*/
  DMA_DeInit(DMA1_Channel6);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  DMA_InitTypeDef DMA_InitStructure =
  {
    .DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR,
    .DMA_MemoryBaseAddr = (uint32_t)usart2_rx_package,
    .DMA_DIR = DMA_DIR_PeripheralSRC,
    .DMA_BufferSize = sizeof(usart2_rx_package),
    .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
    .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
    .DMA_Mode = DMA_Mode_Circular,
    .DMA_Priority = DMA_Priority_Medium,
    .DMA_M2M = DMA_M2M_Disable,
  };
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel6, ENABLE);
}

/* @brief �ܳ�ʼ��*/
void usart2dma_init(unsigned int bound )
{
  usart2_init( bound);                  //��ʼ����������
  usart2_idle_interrupt_init();   //��ʼ�����ڵĿ����ж�
  usart2_rx_dma_init();           //��ʼ�����ռĴ�����DMAͨ��
}

