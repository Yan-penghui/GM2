
/*-------------------------------------------------*/
/*                                                 */
/*   		    modbus   调压器                      */
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


float v_opt = 0,i_opt = 0,p_opt = 0;
int	power_state = 0,power_error = 0;


/*-----------------------------------------------------------*/
/*	函数名：power_init初始化 调压器         	   		          */
/*	参  数：																									*/
/*	                             													    */
/*-----------------------------------------------------------*/
void power_init(void)
{
	int ck=0;
	
  ck=ck+Send_power('w',0x0064,0x0002);		//1.00 启停设置         0端子-1键盘-2通信
	ck=ck+Send_power('w',0x0136,0x0000); 		//3.10 通信启停 				0停机 1启动 2复位  
	ck=ck+Send_power('w',0x0070,0x0002); 		//1.12移项设置      		0开环-1恒压-2恒流-3恒功率
  ck=ck+Send_power('w',0x0072,0x0000); 		//1.14限制值   24%      50A*0.24=12A  12*9.17=110A
  ck=ck+Send_power('w',0x00d8,0x0003); 		//2.16 负载断线         0不检测 1报警不停机 2报警并停机 3报警自复位
 
	
	//初始化失败
  if(ck!=5) 		
    // 严重故障：
    //error9:调压器通信线未连接
    {
      uart4_printf("调压器初始化异常！\r\n");
      errormessage(9);
    }
  else
    {
      uart4_printf("调压器初始化成功\r\n");
    }
}


/*-------------------------------------------------*/
/*函数名：开关电源发送设置指令                         */
/*参  数：operate：W/R  写/读                       */
/*参  数：name：寄存器地址        					 */
/*参  数：w_data：写入数据值       					  */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
int Send_power(char operate,u16 name,u16 w_data)
{
  u8 send_data[8];
  u8 re_data[20];
  u16 sendCRC=0;
  u8 revlen=0;
  int send_times=4;		//超时计数

  send_data[0] = 0x01;        //设备地址 01
  
  if(operate=='w')
    {
      send_data[1] = 0x06;        //写指令 06
			send_data[2] = name>>8; 				//寄存器地址高字节  0x00
			send_data[3] = name;        //寄存器地址低字节
      send_data[4] = w_data>>8;         //写入数据 （高八位）
      send_data[5] = w_data;            //写入数据  （低八位）
    }
  if(operate=='r')
    {
      send_data[1] = 0x03;         //读指令 03
			send_data[2] = 0x00; 				//寄存器地址高字节  0x00
      send_data[3] = 0x00;				//读出寄存器地址起始位  0x0000位
      send_data[4] = 0x00;        //读出寄存器宽度  8位（高八位）
      send_data[5] = 0x08;        //读出寄存器宽度  8位（低八位）
    }

  //crc16校验
  sendCRC = ModbusCRCCalc(send_data,6) ;
  send_data[6] = sendCRC>>8;
  send_data[7] = sendCRC;		
		
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
					
				
  if(send_data[1]==0x03)
    {
      //revlen=13;
			revlen=power_RX_BUF[2]+3;

    }
  if(send_data[1]==0x06)
    {
      //revlen=6;
			revlen=power_RX_BUF[2]*2+3;
    }


          //计算接收数据的校验
          modbusCRC = ModbusCRCCalc(re_data,revlen) ;

          //实际接收到的校验
          myCRC     = power_RX_BUF[revlen]*256+power_RX_BUF[revlen+1];

          //如果CRC校验通过
          if( myCRC == modbusCRC)
            {
              //读指令
              if(send_data[1]==0x03)
                {
                  v_opt				=	(power_RX_BUF[3]*256 + power_RX_BUF[4]);
                  i_opt				=	(power_RX_BUF[5]*256 + power_RX_BUF[6]);
									
                  p_opt				=	(power_RX_BUF[7]*256 + power_RX_BUF[8]);
									power_state	=	(power_RX_BUF[13]*256 + power_RX_BUF[14]);
                  power_error	=	(power_RX_BUF[17]*256 + power_RX_BUF[18]);

                  printf("optv.txt=\"%.2fV\"\xff\xff\xff",v_opt);
                  printf("opti.txt=\"%.2fA\"\xff\xff\xff",i_opt);

                  switch (power_state)
                    {
											//停机
                    case 0:                    
                      printf("   ");
                      break;
                   
										//运行
                    case 1:
                       printf("   ");
                      break;
										
										//故障
										case 2:
											uart4_printf("调压器异常！\r\n");
													switch (power_error)
														{
																											
															case 1:
																uart4_printf("电源：正常\r\n");
																break;
															case 2:
																uart4_printf("电源：关机\r\n");
																break;
																case 3:
																uart4_printf("电源：正常\r\n");
																break;
															case 4:
																uart4_printf("电源：关机\r\n");
																break;
																case 5:
																uart4_printf("电源：正常\r\n");
																break;
															case 6:
																uart4_printf("电源：关机\r\n");
																break;
																case 7:
																uart4_printf("电源：正常\r\n");
																break;
															case 8:
																uart4_printf("电源：关机\r\n");
																break;
																case 9:
																uart4_printf("电源：正常\r\n");
																break;
															case 10:
																uart4_printf("电源：关机\r\n");
																break;
																												
															
														}
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
                  uart4_printf("调压器读指令 CRC校验失败,myCRC:%x,modbusCRC:%x\r\n",myCRC,modbusCRC);
                }
              else if(revlen==6)
                {
                  uart4_printf("调压器写指令 CRC校验失败,myCRC:%x,modbusCRC:%x\r\n",myCRC,modbusCRC);
                }

            }
        }
      else
        {
          //uart4_printf("调压器%d次接收数据超时！\r\n",5-send_times);
        }

      send_times=send_times-1;
    }

  return 0;
}
