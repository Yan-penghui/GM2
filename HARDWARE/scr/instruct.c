
/*---------------------------------------------------------------*/
/*函数名：void my_instruct_task(void *pvParameters)   		       */
/*功  能：串口屏指令解析状态任务       							             */
/*参  数：无                          			   			          	 */
/*返回值：无                                       			         */
/*其  他：读队列 U1_xQueue                                       */
/*---------------------------------------------------------------*/

#include "delay.h"
#include "instruct.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "switch.h"
#include "stdlib.h"
#include "Thermostat.h"
#include "door.h"
#include "led.h"
#include "runcode.h"
#include "usart1.h"


volatile u8 break_Runcode=0, duandianjixv=0,isRunning_Runcode=0,debug=0;

int runingtime=0,Alltime=0;
int TEMchange=0;

extern QueueHandle_t U1_xQueue;
extern TaskHandle_t Showhmi_Task_Handler;

void my_instruct_task(void *pvParameters)
{
  int i,instruct;
  u16 TEM_data[8];
  u16 V_data[8];
  u16 H_data[8];
  u16 myProgram_data[17][2];
  u16 runcode[41];
  u8 instruct_RxBuff[70];

  float hotting_time_s=0;
  while(1)
    {
      //printf("----------my_instruct_task----------\r\n");
      if (pdPASS == xQueueReceive( U1_xQueue,&instruct_RxBuff,portMAX_DELAY ))    	//读队列
        {

          if (debug)
            {
              uart4_printf("U1_xQueue=");
              for(i=0; i<usart1_rx_len; i++)
                {
                  uart4_printf("%02x ",instruct_RxBuff[i]);
                }
              uart4_printf("\r\n");
            }

          instruct=instruct_RxBuff[1];

//功能码01 运行加热程序
          if(instruct == 0x01)
            {

 //创建runcode任务
              if(!isRunning_Runcode)
                {
                  xTaskCreate(my_runcode_task, "my_runcode_task", 128*2, NULL, 4, &Runcode_Task_Handler);
                  uart4_printf("加热程序Runcode创建成功!\r\n");
                }
              else if(isRunning_Runcode)
                {
                  uart4_printf("已有Runcode在运行，加热程序Runcode创建失败！\r\n");
                }

              isRunning_Runcode=1;
 //程序参数整理 Program_data[17][2]:[温度][时间]
              for(i=2; i<17; i+=2)
                {
                  TEM_data[(i-2)/2]		=(instruct_RxBuff[i+1]<<8|instruct_RxBuff[i]);          //温度：摄氏度
                  V_data[(i-2)/2]		=instruct_RxBuff[i+1+16]<<8|instruct_RxBuff[i+16];		//速率：摄氏度/s
                  H_data[(i-2)/2]		=instruct_RxBuff[i+1+32]<<8|instruct_RxBuff[i+32];		//保持时间：分钟

                }

              myProgram_data[0][0]	= 45;													//第一个温度点：45摄氏度
              hotting_time_s=1 ;                                //第一个温度点：1s
              // myProgram_data[0][1]	= hotting_time_s;
              myProgram_data[0][1]	= abs(TEM_data[1]-TEM_data[0])*60.0/(float)V_data[0];

              for(i=0; i<8; i+=1)
                {
                  myProgram_data[2*i+1][0]	= TEM_data[i];						//温度
                  myProgram_data[2*i+1][1]	= H_data[i]*60;						//时间s

                  myProgram_data[2*i+2][0]	= TEM_data[i];						//温度
                  hotting_time_s=abs(TEM_data[i+1]-TEM_data[i])*60.0/(float)V_data[i+1];
                  myProgram_data[2*i+2][1]	= (int)hotting_time_s;
                  if(i>=7)
                    {
                      myProgram_data[2*i+2][1]	= 0;
                    }
                }

//测试：显示构成程序的数组参数
              for(i=0; i<17; i+=1)
                {

                  runcode[2*i] = myProgram_data[i][0];
                  runcode[2*i+1] = myProgram_data[i][1];
//if (debug)
//{
//	uart4_printf("runcode[%d] =%d，runcode[%d+1] =%d s\r\n",i,myProgram_data[i][0],i,myProgram_data[i][1]);
//}
                }

//高级设置参数写入
              runcode[34] = instruct_RxBuff[51]<<8|instruct_RxBuff[50] ;			//fan
              runcode[35] = instruct_RxBuff[53]<<8|instruct_RxBuff[52];				//atuodoor
              runcode[36] = instruct_RxBuff[55]<<8|instruct_RxBuff[54];				//atuodoor_downTem
              runcode[37] = instruct_RxBuff[57]<<8|instruct_RxBuff[56];				//rehot
              runcode[38] = instruct_RxBuff[59]<<8|instruct_RxBuff[58];				//rehottem
              runcode[39] = instruct_RxBuff[61]<<8|instruct_RxBuff[60] ;			//rehottime

//写队列给runcode任务
              xQueueSendToBack( runcode_xQueue,&runcode,100);	//写队列

            }

//功能码02 修改温度
          else if(instruct == 0x02)
            {
              TEMchange=instruct_RxBuff[2];      //tem change
              if(instruct_RxBuff[2]>100)
                {
                  TEMchange=instruct_RxBuff[2]-256;
                }
              uart4_printf("当前温控器：%d℃“，屏幕显示温度:%d，用户修正值：%d，程序修正值：%d\r\n",

                           PVTem,PVTem-TEMchange+reTEMchange,TEMchange,reTEMchange);

            }

//功能码10 炉门上升
          else if(instruct == 0x10)
            {
              switch (door_flag)
                {
                //当前停止状态
                case 0:
                  door_flag=1;      //0 停止，1 up ,2 down
                  break;
                //当前上升状态，转为停止
                case 1:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //当前下降状态，转为停止
                case 2:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //未知状态
                default:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                }

            }
//功能码11 炉门停止
          else if(instruct == 0x11)
            {

              door_flag=0;      //0 停止，1 up ,2 down
              DOWN=1;
              UP = 1; 		/*低电平有效*/
              printf("doorup.pic=0\xff\xff\xff");
              printf("doordown.pic=2\xff\xff\xff");

            }

//功能码12 炉门下降
          else if(instruct == 0x12)
            {

              switch (door_flag)
                {
                //当前停止状态
                case 0:
                  door_flag=2;      //0 停止，1 up ,2 down
                  break;
                //当前上升状态，转为停止
                case 1:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //当前下降状态，转为停止
                case 2:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //未知状态
                default:
                  door_flag=0;      //0 停止，1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*低电平有效*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                }
            }

//功能码14 结束正在运行的加热程序
          else if(instruct == 0x14)
            {

              printf("vis b3,1\xff\xff\xff");			//显示 开始 按钮
              printf("vis b3,1\xff\xff\xff");			//显示 开始 按钮
              printf("zhuangtai.val=0\xff\xff\xff");		//0-待机 1-预烘干 2-运行 3-故障
              printf("vis b0,1\xff\xff\xff");			//显示 程序 按钮
              printf("vis b5,1\xff\xff\xff");			//显示 菜单 按钮
              printf("vis doorup,1\xff\xff\xff");			//显示 上下 按钮
              printf("vis doordown,1\xff\xff\xff");


              if(isRunning_Runcode)
                {
                  //vTaskSuspend(Runcode_Task_Handler); 	//任务挂起（停止）
                  //删除runcode任务

                  door_flag=0;      //0 停止，1 up ,2 down
                  break_Runcode=1;

                }
              else
                {
                  uart4_printf("无加热程序正在运行！\r\n");
                }
            }

//功能码15 断电继续
          else if(instruct == 0x15)
            {
              //断电继续
              duandianjixv=1;
            }
        }
//清空串口1接收缓冲区
      memset(usart1_rx_package, 0, usart1_rx_len_MAX);
    }
}


