
/*-------------------------------------------------*/
/*                                                 */
/*          	       串口3               	   */
/*                温控器modbus通信                         */
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

u16 usart3_rx_len = 0;     //接收字节计数

u8
usart3_tx_package[usart3_tx_len_MAX],
                  usart3_rx_package[usart3_rx_len_MAX];       //连续字节存放的数据包

volatile bool is_usart3_receive_ok = false;    //串口是否已接收完毕



void USART3_IRQHandler(void)
{

  if( USART_GetITStatus(USART3, USART_IT_IDLE) != RESET )
    {
      USART3 -> SR;  //访问一下SR寄存器
      USART3 -> DR;  //访问一下DR寄存器

      DMA_Cmd(DMA1_Channel3, DISABLE);
      usart3_rx_len = usart3_rx_len_MAX - DMA_GetCurrDataCounter(DMA1_Channel3);
      DMA_SetCurrDataCounter( DMA1_Channel3, sizeof(usart3_rx_package) );
      DMA_Cmd(DMA1_Channel3, ENABLE);

      usart3_rx_package[usart3_rx_len] = '\0';          //给最后一位补上结束符
      is_usart3_receive_ok = true;                //标记一帧数据已接收完成

      USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //清除IDLE中断标志位
      xQueueSendToBackFromISR( U3_xQueue,&usart3_rx_package,NULL);

    }
}





/* @brief 获取接收完毕的标志位，查看是否已接收完成。
 * @retval 1: 接收完成
 *         0: 未接收完成 */
u8 get_usart3_TC_flag(void)
{
  if(is_usart3_receive_ok == true)
    {
      is_usart3_receive_ok = false;
      return 1;//如果接收已完成，返回1
    }
  else
    return 0;//接收未完成，返回0
}

/* @brief 获取接收到的数据包。
 * @retval  接收到的数据包，是一个长度为256的u8数组指针，有结束符'\0'。 */
u8* get_usart3_rx_package(void)
{
  return usart3_rx_package;
}

/* @brief 获取接收到的数据包长度。
 * @retval 接收完一帧数据的长度 */
u16 get_usart3_rx_len(void)
{
  return usart3_rx_len;
}

/* @brief 清空数据包，实际是把第一位用结束符替代。*/
void clear_usart3_rx_package(void)
{
  usart3_rx_package[0] = '\0';
}


/* @brief 阻塞式地发送一个字节。*/
void usart3_send_byte_blocking(u8 data)
{
  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
  USART_SendData(USART3, data);
}

/* @brief uart_ch340发送函数，printf的参数格式*/
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


/* @brief 初始化ch340所连接MCU的串口外设。*/
static void usart3_init(unsigned int bound)
{
  /*初始化发送接收引脚，PB10(USART3_TX)，PB11(USART3_RX)。*/
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

  /*初始化USART3外设。*/
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

/* @brief 初始化ch340所连接MCU串口的空闲中断*/
static void usart3_idle_interrupt_init(void)
{
  USART_ClearFlag(USART3, USART_IT_IDLE);
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  /*已在主函数中设置优先级分组。*/
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* @brief 初始化ch340所连接MCU的串口的接收寄存器DMA转运。*/
static void usart3_rx_dma_init(void)
{
  /*开启USART3的接收寄存器的DMA通道，查参考手册，是DMA1通道3。*/
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

/* @brief 总初始化*/
void usart3dma_init(unsigned int bound)
{
  usart3_init(bound);                  //初始化串口外设
  usart3_idle_interrupt_init();   //初始化串口的空闲中断
  usart3_rx_dma_init();           //初始化接收寄存器的DMA通道
}

