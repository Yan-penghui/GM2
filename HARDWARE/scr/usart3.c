
/*-------------------------------------------------*/
/*                                                 */
/*          	       ����3               	   */
/*                �¿���modbusͨ��                         */
/*-------------------------------------------------*/

#include "FreeRTOS.h"
#include "queue.h"
#include "usart3.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "sys.h"
#include "task.h"


extern QueueHandle_t U3_xQueue;

u16 usart3_rx_len = 0;     //�����ֽڼ���

u8
usart3_tx_package[usart3_tx_len_MAX],
                  usart3_rx_package[usart3_rx_len_MAX];       //�����ֽڴ�ŵ����ݰ�

volatile bool is_usart3_receive_ok = false;    //�����Ƿ��ѽ������



void USART3_IRQHandler(void)
{

  if( USART_GetITStatus(USART3, USART_IT_IDLE) != RESET )
    {
      USART3 -> SR;  //����һ��SR�Ĵ���
      USART3 -> DR;  //����һ��DR�Ĵ���

      DMA_Cmd(DMA1_Channel3, DISABLE);
      usart3_rx_len = usart3_rx_len_MAX - DMA_GetCurrDataCounter(DMA1_Channel3);
      DMA_SetCurrDataCounter( DMA1_Channel3, sizeof(usart3_rx_package) );
      DMA_Cmd(DMA1_Channel3, ENABLE);

      usart3_rx_package[usart3_rx_len] = '\0';          //�����һλ���Ͻ�����
      is_usart3_receive_ok = true;                //���һ֡�����ѽ������

      USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //���IDLE�жϱ�־λ
      xQueueSendToBackFromISR( U3_xQueue,&usart3_rx_package,NULL);

    }
}





/* @brief ��ȡ������ϵı�־λ���鿴�Ƿ��ѽ�����ɡ�
 * @retval 1: �������
 *         0: δ������� */
u8 get_usart3_TC_flag(void)
{
  if(is_usart3_receive_ok == true)
    {
      is_usart3_receive_ok = false;
      return 1;//�����������ɣ�����1
    }
  else
    return 0;//����δ��ɣ�����0
}

/* @brief ��ȡ���յ������ݰ���
 * @retval  ���յ������ݰ�����һ������Ϊ256��u8����ָ�룬�н�����'\0'�� */
u8* get_usart3_rx_package(void)
{
  return usart3_rx_package;
}

/* @brief ��ȡ���յ������ݰ����ȡ�
 * @retval ������һ֡���ݵĳ��� */
u16 get_usart3_rx_len(void)
{
  return usart3_rx_len;
}

/* @brief ������ݰ���ʵ���ǰѵ�һλ�ý����������*/
void clear_usart3_rx_package(void)
{
  usart3_rx_package[0] = '\0';
}


/* @brief ����ʽ�ط���һ���ֽڡ�*/
void usart3_send_byte_blocking(u8 data)
{
  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
  USART_SendData(USART3, data);
}

/* @brief uart_ch340���ͺ�����printf�Ĳ�����ʽ*/
void usart3_printf(char* fmt,...)
{
  va_list ap;
  va_start(ap, fmt);
  vsprintf((char*)usart3_tx_package, fmt, ap);
  va_end(ap);
  u16 len = strlen( (char*)usart3_tx_package );
  for(u16 i=0; i<len; i++)
    usart3_send_byte_blocking(usart3_tx_package[i]);
}


/* @brief ��ʼ��ch340������MCU�Ĵ������衣*/
static void usart3_init(unsigned int bound)
{
  /*��ʼ�����ͽ������ţ�PB10(USART3_TX)��PB11(USART3_RX)��*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*��ʼ��USART3���衣*/
  USART_DeInit(USART3);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_BaudRate = bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART3, &USART_InitStructure);
  USART_Cmd(USART3, ENABLE);
}

/* @brief ��ʼ��ch340������MCU���ڵĿ����ж�*/
static void usart3_idle_interrupt_init(void)
{
  USART_ClearFlag(USART3, USART_IT_IDLE);
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  /*�������������������ȼ����顣*/
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* @brief ��ʼ��ch340������MCU�Ĵ��ڵĽ��ռĴ���DMAת�ˡ�*/
static void usart3_rx_dma_init(void)
{
  /*����USART3�Ľ��ռĴ�����DMAͨ������ο��ֲᣬ��DMA1ͨ��3��*/
  DMA_DeInit(DMA1_Channel3);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
  DMA_InitTypeDef DMA_InitStructure =
  {
    .DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR,
    .DMA_MemoryBaseAddr = (uint32_t)usart3_rx_package,
    .DMA_DIR = DMA_DIR_PeripheralSRC,
    .DMA_BufferSize = sizeof(usart3_rx_package),
    .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
    .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
    .DMA_Mode = DMA_Mode_Circular,
    .DMA_Priority = DMA_Priority_Medium,
    .DMA_M2M = DMA_M2M_Disable,
  };
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel3, ENABLE);
}

/* @brief �ܳ�ʼ��*/
void usart3dma_init(unsigned int bound)
{
  usart3_init(bound);                  //��ʼ����������
  usart3_idle_interrupt_init();   //��ʼ�����ڵĿ����ж�
  usart3_rx_dma_init();           //��ʼ�����ռĴ�����DMAͨ��
}

