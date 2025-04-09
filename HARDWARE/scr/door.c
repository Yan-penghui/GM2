#include "stm32f10x.h"
#include "door.h"
#include "usart1.h"
#include "delay.h"
#include "FreeRTOS.h"	 //FreeRTOS配置头文件
#include "queue.h"		 //队列
#include "switch.h"
#include "timers.h"
#include "Thermostat.h"      //包含需要的头文件 
#include "uart4.h"


void my_door_task()
{
  while(1)
    {
      u8 runtime=100;
      //电机关闭100s
      //printf("----------my_door_task----------\r\n");
      if (door_flag == 1)
        {
          printf("doorup.pic=1\xff\xff\xff");
          printf("doordown.pic=2\xff\xff\xff");
          updoor(runtime);
          printf("doorup.pic=0\xff\xff\xff");
          printf("doordown.pic=2\xff\xff\xff");
          printf("doorup.pic=0\xff\xff\xff");
          printf("doordown.pic=2\xff\xff\xff");
        }

      if (door_flag != 1||door_flag != 2)
        {

          DOWN=1;
          UP = 1; /*低电平有效*/
        }

      if (door_flag == 2)
        {

          downdoor(runtime);

        }
      delay_ms(200);
      //tem-re<500 turnoff_fan
      if(PVTem-TEMchange+reTEMchange<=500)
        {
          FAN_DOWN =1;
          FAN_UP=1;
        }
      else
        {
          FAN_DOWN =0;//  风扇启停
          FAN_UP=0;
        }


    }
}


void updoor(u8 run_time)//上升炉门  （超时报错：秒）
{
  int timeout = door_outtime*10;
  int runtime = run_time * 10;			/*（动作时间：0.1秒,超时时间：0.1秒)*/

  DOWN=1;
  UP = 1; //低电平有效




  if(run_time==0)
    {
      runtime =door_outtime*10;
    }

  if(LIM1 !=1 )
    {
      DOWN=1;
      UP = 0; //低电平有效
      while(timeout--)
        {
          uart4_printf("关门中······\r\n");
          delay_ms(100);
          runtime = runtime -1;



          //运行到下限位退出
          if(LIM1==1)
            {
              uart4_printf("运行到下限位退出\r\n");
              break;
            }

          //接收到停止指令：手动退出
          if(door_flag==0)
            {
              uart4_printf("接收到停止指令：手动退出\r\n");
              break;
            }

          //运行到达设定时间退出
          if(runtime <= 0)
            {
              uart4_printf("运行到达设定时间退出\r\n");
              break;
            }

        }

      //超时
      if(timeout<=0)
        {
          uart4_printf("炉门上升超时故障\r\n");
          //error6:炉门上升超时故障
          errormessage(6);
          stopdoor();
        }

    }
  DOWN=1;
  UP = 1; /*低电平有效*/
  door_flag=0;
  printf("yunxing.doorup.pic=0\xff\xff\xff");
  printf("yunxing.doordown.pic=2\xff\xff\xff");
  printf("yunxing.doorup.pic=0\xff\xff\xff");
  printf("yunxing.doordown.pic=2\xff\xff\xff");
  uart4_printf("关门结束······\r\n");
}

void downdoor(u8 run_time)//下降炉门  （动作时间：秒,超时时间：秒）
{
  int timeout = door_outtime*10;
  int runtime = run_time * 10;			/*（动作时间：0.1秒,超时时间：0.1秒)*/
  int i;

  DOWN=1;
  UP = 1; //低电平有效

  //1000度以上不开门
  if( PVTem-TEMchange+reTEMchange > 1000 )
    {
      /*page luwenguogao*/
      //printf("炉温过高\r\n");
      printf("page wenduguogao\xff\xff\xff");
      DOWN=1;
      UP = 1; /*低电平有效*/
      door_flag=0;
      return;
    }
  printf("zhuangtai.val=1\xff\xff\xff");		//0-待机 1-自动开门中 2-运行 3-故障
  printf("zhuangtai.val=1\xff\xff\xff");		//0-待机 1-自动开门中 2-运行 3-故障
  //温度600-1200，分步下降
  if(  (PVTem-TEMchange+reTEMchange)> 600 && 1000 >=( PVTem-TEMchange+reTEMchange) )
    {

      uart4_printf("600-1000，分步下降\r\n");
      //炉门运行总时长22s
      //隐藏上下按键
      printf("vis doorup,0\xff\xff\xff");
      printf("vis doordown,0\xff\xff\xff");
      printf("vis doorup,0\xff\xff\xff");
      printf("vis doordown,0\xff\xff\xff");

#define tem_step 40

      for(i=1; i<=10; i++)
        {
          //每40度下降1.5s，共十步
          DOWN=0;
          UP = 1; //低电平有效
          delay_ms(1500);
          DOWN=1;
          UP = 1; //低电平有效

          while (PVTem-TEMchange+reTEMchange > 1000-i*tem_step)
            {

              delay_ms(1000);
              uart4_printf("等待温度降至%d\r\n",1000-i*tem_step);
            }

        }
    }


  //下降按键变灰
  printf("doorup.pic=0\xff\xff\xff");
  printf("doordown.pic=3\xff\xff\xff");

  if(run_time==0)
    {
      runtime =door_outtime*10;
    }


  if(LIM2 != 1 )
    {
      uart4_printf("低于600度，直接打开\r\n");
      DOWN=0;
      UP = 1; //低电平有效
      while(timeout--)
        {
          delay_ms(100);
          runtime = runtime -1;

          //运行到达设定时间退出
          if(runtime <= 0 )
            {
              uart4_printf("运行到达设定时间退出\r\n");
              break;
            }

          //运行到下限位退出
          if(LIM2==1)
            {
              uart4_printf("运行到下限位退出");
              break;
            }

          //接收到停止指令：手动退出
          if(door_flag==0)
            {
              uart4_printf("接收到停止指令：手动退出\r\n");
              break;
            }

        }

      //超时故障
      if(timeout<=0)
        {
          uart4_printf("炉门下降超时故障\r\n");
          //error7:炉门下降超时故障
          errormessage(7);
          stopdoor();
        }
    }

  DOWN=1;
  UP = 1; /*低电平有效*/
  door_flag=0;
  //显示上下按键、变白色
  printf("vis doorup,1\xff\xff\xff");
  printf("vis doordown,1\xff\xff\xff");
  printf("vis doorup,1\xff\xff\xff");
  printf("vis doordown,1\xff\xff\xff");
  printf("yunxing.doorup.pic=0\xff\xff\xff");
  printf("yunxing.doordown.pic=2\xff\xff\xff");
  printf("yunxing.doorup.pic=0\xff\xff\xff");
  printf("yunxing.doordown.pic=2\xff\xff\xff");
  printf("zhuangtai.val=0\xff\xff\xff");		//0-待机 1-自动开门中 2-运行 3-故障
}


void stopdoor()
{

  while(1)
    {
      DOWN=1;
      UP = 1; //低电平有效

    }
}
