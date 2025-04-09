
/*-------------------------------------------------*/
/*                                                 */
/*   		    modbus   开关电源                      */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"
#include "power.h"
#include "delay.h"
#include "uart4.h"
#include "usart3.h"
#include "usart2.h"
#include "switch.h"
#include "stdio.h"
#include "string.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "crc.h"
#include "Thermostat.h"

TickType_t xStartTime_power, xEndTime_power;
extern QueueHandle_t U2_xQueue;


float v_opt = 0,i_opt = 0,v_ipt = 0;
int	power_state = 0;


/*-----------------------------------------------------------*/
/*	函数名：power_init初始化 开关电源         	             */
/*	参  数：																									*/
/*	R:	0x007 v_opt                                          */
/*			0x008 i_opt                                          */
/*			0x00a v_ipt                                          */
/*			0x00b power_state                                     */
/*-----------------------------------------------------------*/
void power_init(void)
{

  Send_power('w',0x06,0x0000);		//开关机寄存器地址0x0006         0开机-1关机
  Send_power('w',0x03,0x0001); 		//选用模式寄存器地址0x0003       0电位器调节，1modbus调节
  //Send_power('w',0x00,0x11c1); 		//设置输出电压寄存器地址0x0000   0x11c1：24V
  //Send_power('w',0x01,0x11c1); 		//设置输出电流寄存器地址0x0001   0x0000：0A
  Send_power('w',0x04,0x17ed); 		//设置过压保护寄存器地址0x0004   0x17ed：122%
  Send_power('w',0x05,0x17ed); 		//设置过流保护寄存器地址0x0005   0x17ed：122%

  if(Send_power('r',0x07,0x0005)==0) 		//读状态失败
    // 严重故障：
    //error9:开关电源通信线未连接
    {
      uart4_printf("开关电源通信异常！\r\n");
      errormessage(9);
    }
  else
    {
      uart4_printf("开关电源参数初始化成功\r\n");
    }
}


/*-------------------------------------------------*/
/*函数名：开关电源发送设置指令                         */
/*参  数：operate：W/R  写/读                       */
/*参  数：name：寄存器地址        					 */
/*参  数：w_data：写入数据值       					  */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
int Send_power(char operate,u8 name,u16 w_data)
{
  u8 send_data[8];
  u8 re_data[20];
  u16 sendCRC=0;
  u8 revlen=0;


  int send_times=4;		//超时计数

  send_data[0] = 0x01;        //设备地址 01
  send_data[2] = 0x00; 				//寄存器地址高字节  0x00
  send_data[3] = name;        //寄存器地址低字节

  if(operate=='w')
    {
      send_data[1] = 0x06;        //写指令 06
      send_data[4] = w_data>>8;         //写入数据 （高八位）
      send_data[5] = w_data;            //写入数据  （低八位）
    }
  if(operate=='r')
    {
      send_data[1] = 0x03;         //读指令 03
      send_data[3] = 0x07;				//读出寄存器地址起始位  0x0007位
      send_data[4] = 0x00;        //读出寄存器宽度  5位（高八位）
      send_data[5] = 0x05;        //读出寄存器宽度  5位（低八位）
    }


  //crc16校验

  sendCRC = ModbusCRCCalc(send_data,6) ;
  send_data[6] = sendCRC>>8;
  send_data[7] = sendCRC;


  if(send_data[1]==0x03)
    {
      revlen=13;

    }
  if(send_data[1]==0x06)
    {
      revlen=6;

    }
  //发送指令数据，发送失败重试次数：5
  while(send_times)
    {
      //使能发送1使能接收0
      RS485_POWER=1;
      //清空usart2接收缓冲区
      memset(usart2_rx_package, 0, usart2_rx_len);
      //发送指令数据Serial_SendArray(data, 字计数量,串口)
      Serial_SendArray(send_data, 9,2);

//			xStartTime_power = xTaskGetTickCount();
      //使能发送1使能接收0
      RS485_POWER=0;

      //100ms内接收到正确返回值
      if (pdPASS == xQueueReceive( U2_xQueue,&re_data,100 ))
        {

          u16 modbusCRC,myCRC;

//					 // 获取当前节拍计数
//          xEndTime_power = xTaskGetTickCount();
//          uart4_printf("开关电源数据接收time=%d\r\n",xEndTime_power-xStartTime_power);


//			printf("U2_xQueue=");
//			for(i=0;i<10;i++)
//			{
//			printf("%02x ",re_data[i]);
//		}
//			 printf("\r\n");


          //计算接收数据的校验
          modbusCRC = ModbusCRCCalc(re_data,revlen) ;

          //实际接收到的校验
          myCRC     = power_RX_BUF[revlen]*256+power_RX_BUF[revlen+1];

          //如果CRC校验通过
          if( myCRC == modbusCRC)
            {
              //读指令的返回数据revlen==13
              if(revlen==13)
                {
                  v_opt				=	(power_RX_BUF[3]*256 + power_RX_BUF[4])*26.4/5000.0;
                  i_opt				=	(power_RX_BUF[5]*256 + power_RX_BUF[6])*125.0/5000.0;
                  v_ipt				=	(power_RX_BUF[9]*256 + power_RX_BUF[10])/10.0;
                  power_state	=	(power_RX_BUF[11]*256 + power_RX_BUF[12]);

                  printf("optv.txt=\"%.2fV\"\xff\xff\xff",v_opt);
                  printf("opti.txt=\"%.2fA\"\xff\xff\xff",i_opt);

                  switch (power_state)
                    {
                    case 1:
                      errormessage(5);
                      uart4_printf("电源：输入电压异常=%.2fV\r\n",v_ipt);
                      break;
                    case 2:
                      uart4_printf("电源：关机\r\n");
                      break;
                    case 3:
//                      uart4_printf("电源：正常\r\n");
                      break;

                    case 4:
                      uart4_printf("电源：过温\r\n");
                      break;
                    case 5:
                      uart4_printf("电源：过压\r\n");
                      break;
                    case 6:
                      uart4_printf("电源：过流\r\n");
                      break;
                    case 7:
                      uart4_printf("电源：短路\r\n");

                      Send_power('w',0x06,0x0001);		//开关机寄存器地址0x0006         0开机-1关机
                      delay_ms(200);
                      Send_power('w',0x06,0x0000);		//开关机寄存器地址0x0006         0开机-1关机
                      delay_ms(2000);

                      break;
                    }
                }
//接收成功
              return 1;

            }
          else
            {
              if(revlen==13)
                {
                  uart4_printf("开关电源读指令 CRC校验失败,myCRC:%x,modbusCRC:%x\r\n",myCRC,modbusCRC);
                }
              else if(revlen==6)
                {
                  uart4_printf("开关电源写指令 CRC校验失败,myCRC:%x,modbusCRC:%x\r\n",myCRC,modbusCRC);
                }

            }
        }
      else
        {
          //uart4_printf("开关电源%d次接收数据超时！\r\n",5-send_times);
        }

      send_times=send_times-1;
    }

  return 0;
}

