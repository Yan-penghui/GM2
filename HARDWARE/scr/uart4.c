
/*-------------------------------------------------*/
/*                                                 */
/*          	       串口4               	   */
/*                调试端口                         */
/*-------------------------------------------------*/

#include "FreeRTOS.h"	 //FreeRTOS配置头文件
#include "queue.h"		 //队
#include "uart4.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "sys.h"
#include "instruct.h"
#include "Thermostat.h"


u16 uart4_rx_len = 0;     //接收字节计数
u8
uart4_tx_package[uart4_tx_len_MAX],
                 uart4_rx_package[uart4_rx_len_MAX];       //连续字节存放的数据包

volatile bool is_uart4_receive_ok = false;    //串口是否已接收完毕

void UART4_IRQHandler(void)
{

  if( USART_GetITStatus(UART4, USART_IT_IDLE) != RESET )
    {
      UART4 -> SR;  //访问一下SR寄存器
      UART4 -> DR;  //访问一下DR寄存器

      DMA_Cmd(DMA2_Channel3, DISABLE);
      uart4_rx_len = uart4_rx_len_MAX - DMA_GetCurrDataCounter(DMA2_Channel3);
      DMA_SetCurrDataCounter( DMA2_Channel3, sizeof(uart4_rx_package) );
      DMA_Cmd(DMA2_Channel3, ENABLE);

      uart4_rx_package[uart4_rx_len] = '\0';          //给最后一位补上结束符
      is_uart4_receive_ok = true;                //标记一帧数据已接收完成

      USART_ClearITPendingBit(UART4, USART_IT_IDLE);  //清除IDLE中断标志位

      //功能码98 开启调试
      if(uart4_rx_package[1] == 0x98)
        {
          //断电继续
          debug=1;

        }
//功能码99 关闭调试
      else if(uart4_rx_package[1] == 0x99)
        {
          //断电继续
          debug =0;

        }
//功能码97 温度赋值
      else if(uart4_rx_package[1] == 0x97)
        {

          PVTem =uart4_rx_package[2]*256+uart4_rx_package[3];

        }
    }
}



/* @brief 获取接收完毕的标志位，查看是否已接收完成。
 * @retval 1: 接收完成
 *         0: 未接收完成 */
u8 get_uart4_TC_flag(void)
{
  if(is_uart4_receive_ok == true)
    {
      is_uart4_receive_ok = false;
      return 1;//如果接收已完成，返回1
    }
  else
    return 0;//接收未完成，返回0
}

/* @brief 获取接收到的数据包。
 * @retval  接收到的数据包，是一个长度为256的u8数组指针，有结束符'\0'。 */
u8* get_uart4_rx_package(void)
{
  return uart4_rx_package;
}

/* @brief 获取接收到的数据包长度。
 * @retval 接收完一帧数据的长度 */
u16 get_uart4_rx_len(void)
{
  return uart4_rx_len;
}

/* @brief 清空数据包，实际是把第一位用结束符替代。*/
void clear_uart4_rx_package(void)
{
  uart4_rx_package[0] = '\0';
}



/* @brief 阻塞式地发送一个字节。*/
void uart4_send_byte_blocking(u8 data)
{
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  USART_SendData(UART4, data);
}

/* @brief uart_ch340发送函数，printf的参数格式*/
void uart4_printf(char* fmt,...)
{
  va_list ap;
  va_start(ap, fmt);
  vsprintf((char*)uart4_tx_package, fmt, ap);
  va_end(ap);
  u16 len = strlen( (char*)uart4_tx_package );
  for(u16 i=0; i<len; i++)
    uart4_send_byte_blocking(uart4_tx_package[i]);
}



/* @brief 初始化ch340所连接MCU的串口外设。*/
static void uart4_init(unsigned int bound)
{
  /*初始化发送接收引脚，PC10(uart4_TX)，Pc11(uart4_RX)。*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /*初始化uart4外设。*/
  USART_DeInit(UART4);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_BaudRate = bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(UART4, &USART_InitStructure);
  USART_Cmd(UART4, ENABLE);
}

/* @brief 初始化ch340所连接MCU串口的空闲中断*/
static void uart4_idle_interrupt_init(void)
{
  USART_ClearFlag(UART4, USART_IT_IDLE);
  USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  /*已在主函数中设置优先级分组。*/
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* @brief 初始化ch340所连接MCU的串口的接收寄存器DMA转运。*/
static void uart4_rx_dma_init(void)
{
  /*开启uart4的接收寄存器的DMA通道，查参考手册，是DMA1通道3。*/
  DMA_DeInit(DMA2_Channel3);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
  DMA_InitTypeDef DMA_InitStructure =
  {
    .DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR,
    .DMA_MemoryBaseAddr = (uint32_t)uart4_rx_package,
    .DMA_DIR = DMA_DIR_PeripheralSRC,
    .DMA_BufferSize = sizeof(uart4_rx_package),
    .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
    .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
    .DMA_Mode = DMA_Mode_Circular,
    .DMA_Priority = DMA_Priority_Medium,
    .DMA_M2M = DMA_M2M_Disable,
  };
  DMA_Init(DMA2_Channel3, &DMA_InitStructure);
  DMA_Cmd(DMA2_Channel3, ENABLE);
}

/* @brief 总初始化*/
void uart4dma_init(unsigned int bound)
{
  uart4_init(bound);                  //初始化串口外设
  uart4_idle_interrupt_init();   //初始化串口的空闲中断
  uart4_rx_dma_init();           //初始化接收寄存器的DMA通道
}

