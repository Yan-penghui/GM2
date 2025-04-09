
/*---------------------------------------------------------------*/
/*函数名：void my_runcode_task(void *pvParameters)   		    	  */
/*功  能：运行程序任务       																		 */
/*参  数：无                          			   									 */
/*返回值：无                                       					     */
/*其  他：                                                       */
/*---------------------------------------------------------------*/

#include "delay.h"
#include "runcode.h"
#include "instruct.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "switch.h"
#include "Thermostat.h"
#include "door.h"
#include "led.h"
#include "power.h"
#include "sys.h"
#include "stdlib.h"

TaskHandle_t Runcode_Task_Handler;
QueueHandle_t runcode_xQueue;

extern TaskHandle_t Showhmi_Task_Handler,GetCT_Task_Handler;

void my_runcode_task(void *pvParameters)
{
  int i,j,nowtem,progress_num,progress_time;
  float percent;
  u8 rehot = 0,atuodoor = 0,fan =0,all_p_num;
  u16 rehottem = 0 ;
  int rehottime = 0,atuodoor_downTem=0;
  u16 runcode[40];
  u16 Program_data[18][2];
  u8 LED=0;
  LED=3;					//YELLOW_LED = 1;RED_LED= 2;GREEN_LED= 3;
  xQueueSendToBack( LED_xQueue,&LED,0);	//写队列
  Program_data[17][0]=0;
  while(1)
    {
      //printf("创建任务------Runcode------\r\n");

      if (pdPASS == xQueueReceive( runcode_xQueue,&runcode,portMAX_DELAY ) )    	//读队列
        {


          for(i=0; i<17; i+=1)
            {

              if(runcode[2*i]!=0)
                {
                  Program_data[i][0]	=	runcode[2*i]+TEMchange-reTEMchange;
                }
              else
                {
                  Program_data[i][0]	=	0;
                }
              Program_data[i][1]	=	runcode[2*i+1];
              uart4_printf("Program_data[%d][0] = %d     [%d][1] = %d\r\n", i,Program_data[i][0],i,Program_data[i][1]);

            }

          fan  							=	runcode[34];
          atuodoor					=	runcode[35] ;
          atuodoor_downTem	=	runcode[36]+TEMchange-reTEMchange;
          rehot							=	runcode[37] ;
          rehottem					=	runcode[38]+TEMchange-reTEMchange ;
          rehottime					=	runcode[39] ;	//nin
          uart4_printf("fan  = %d\r\n"								,fan);
          uart4_printf("atuodoor  = %d\r\n"						,atuodoor);
          uart4_printf("atuodoor_downTem  = %d\r\n"		,atuodoor_downTem);
          uart4_printf("rehot  = %d\r\n"							,rehot);
          uart4_printf("rehottem  = %d\r\n"						,rehottem);
          uart4_printf("rehottime  = %d\r\n"					,rehottime);

          //防止传递屏幕数据错乱，挂起温控器与开关电源通信任务
          vTaskSuspend(Showhmi_Task_Handler); 	//任务挂起（停止）


          //炉门开启，需手动关门
          if(!LIM1)
            {
              printf("page yunxing\xff\xff\xff");		//跳转至运行界面
              printf("page yunxing\xff\xff\xff");		//跳转至运行界面
              printf("page guanbilumen\xff\xff\xff");
              isRunning_Runcode=0;

              //任务恢复为就绪
              vTaskResume(Showhmi_Task_Handler);  //任务就绪（运行）

              vTaskDelete(NULL);
            }

          //正常启动

          printf("page yunxing\xff\xff\xff");
          printf("page yunxing\xff\xff\xff");

          printf("caozuojilu.yunxingjilu.insert(beizhu.yunxingxinxi.txt)\xff\xff\xff");		//write run log
          printf("vis b3,0\xff\xff\xff");			//隐藏 开始 按钮
          printf("vis doorup,0\xff\xff\xff");			//隐藏 上下 按钮
          printf("vis b0,0\xff\xff\xff");			//显示 程序 按钮
          printf("vis b5,0\xff\xff\xff");			//显示 菜单 按钮
          printf("vis doordown,0\xff\xff\xff");   //隐藏 上下 按钮
          printf("jindu.val=0\xff\xff\xff");  //进度归零

          //开关电源继电器吸和
          POWER=0;
          duandianjixv=0;

          //开启屏保
          printf("yunxing.tm0.en=1\xff\xff\xff");
          printf("yunxing.tm0.en=1\xff\xff\xff");

          delay_ms(500);
          //寻找当前温度对应的起点温度段-温度点
          nowtem=PVTem;

          //程序理论总时长
          Alltime=0 ;
          for(j=0; j<16; j++)
            {
              Alltime=Alltime + Program_data[j][1] ;
            }

          uart4_printf("当前程序运行总时长预计 %02d：%02d\r\n",Alltime/60,Alltime%60);

          //程序理论总段数
          all_p_num=0 ;
          for(j=1; j<16; j++)
            {

              if(Program_data[j][0]==0)
                {
                  all_p_num=j-1;
                  break;
                }
            }
          uart4_printf("当前程序运行总段数：%d\r\n",all_p_num);

          //寻找加热起始段序号
          for(i=0 ; i<17 ; i++)
            {

              if(nowtem<Program_data[i+1][0]&&nowtem>=Program_data[i][0])
                {

                  progress_num = i;

//                  printf("yunxing.dangqianduan.val=%d",i/2);
//									printf("yunxing.zongduan.val=%d",);
                  uart4_printf("当前在升温段，温度%d，进入第%d段\r\n",nowtem,progress_num);
                  percent=1-(float)(nowtem-Program_data[i][0])/(Program_data[i+1][0]-Program_data[i][0]);

                  //计算已运行时长
                  runingtime=0;
                  for(j=0; j<i; j++)
                    {
                      runingtime=runingtime + Program_data[j][1] ;
                    }

                  runingtime = runingtime + Program_data[i][1] * (1-percent);

                  uart4_printf("总时长%ds,当前已运行%ds\xff\xff\xff",Alltime,runingtime);

                  printf("jindu.val=%d\xff\xff\xff",runingtime*100/Alltime);


                  progress_time=Program_data[i][1]*percent;//获取当前段总时长
                  heating(i,all_p_num,nowtem,Program_data[i+1][0],(int)(Program_data[i][1]*percent),fan);	 //当前温度   目标温度   时间s

                  break;
                }

            }

          uart4_printf("首段结束，开始进行剩余加热段\r\n");

          //剩余加热段
          i=i+1;
          for(i=i; i<17 ; i++)
            {
              if(break_Runcode)
                {

                  uart4_printf("手动结束，当前等待至设定温度\n");
                  break ;
                }

              progress_time=Program_data[i][1];		//获取当前段总时长

              uart4_printf("进入第%d段，温度%d->%d，本段时长：%02d：%02d\n",
                           i,Program_data[i][0],Program_data[i+1][0],progress_time/60,progress_time%60);



              //遇见0数据退出
              if(Program_data[i+1][0]==0)
                {
                  uart4_printf("加热程序运行结束\r\n");
                  break ;
                }

              heating(i,all_p_num,(float)Program_data[i][0],(float)Program_data[i+1][0],progress_time,fan);

            }


          //正常结束或结束时温度高于1400，烧结次数+1
          if((!break_Runcode)||(PVTem-TEMchange+reTEMchange)>1400)
            {

              printf("wepo yunxing.shaojiecishu.val,272\xff\xff\xff");
              printf("yunxing.shaojiecishu.val=yunxing.shaojiecishu.val+1\xff\xff\xff");
              printf("wepo yunxing.shaojiecishu.val,272\xff\xff\xff");

            }
          //关闭屏保
          printf("yunxing.tm0.en=0\xff\xff\xff");
          printf("yunxing.tm0.en=0\xff\xff\xff");


          //开关电源继电器断开
          POWER=1;
          Send_Thermostat('w',0x1b,1);				//1b-Srun： 0-run 1-stop 2-hold
          delay_ms(100);
          vTaskResume(Showhmi_Task_Handler);  //Showhmi_Task转为就绪态，运行

          //若开启自动开门，降温到atuodoor_downTem，开启一条缝
          if(atuodoor)
            {
              while(PVTem-TEMchange+reTEMchange >= atuodoor_downTem )
                {
                  if(break_Runcode)
                    {

                      uart4_printf("手动结束，当前等待降温至%d\n",atuodoor_downTem);
                      break ;
                    }

                  delay_ms(1000);
                  if (debug)
                    {
                      uart4_printf("等待降温到%d，开启一条缝\r\n",atuodoor_downTem);
                    }
                }

            }

          //防止传递屏幕数据错乱，挂起温控器与开关电源通信任务
          vTaskSuspend(Showhmi_Task_Handler); 	//任务挂起（停止）

          printf("zhuangtai.val=0\xff\xff\xff");		//0-待机 1-自动开门中 2-运行 3-故障
          printf("page yunxing\xff\xff\xff");
          printf("zhuangtai.val=0\xff\xff\xff");		//0-待机 1-预烘干 2-运行 3-故障
          printf("page yunxing\xff\xff\xff");
          printf("jindu.val=100\xff\xff\xff");

          printf("zongshengyu.txt=\"00:00:00\"\xff\xff\xff");

          LED=1;					//YELLOW_LED = 1;RED_LED= 2;GREEN_LED= 3;
          xQueueSendToBack( LED_xQueue,&LED,0);	//写队列

          printf("vis b3,1\xff\xff\xff");			//显示 开始 按钮
          printf("vis b3,1\xff\xff\xff");			//显示 开始 按钮
          printf("vis doorup,1\xff\xff\xff");			//显示 上下 按钮
          printf("vis doordown,1\xff\xff\xff");
          printf("vis b0,1\xff\xff\xff");			//显示 程序 按钮
          printf("vis b5,1\xff\xff\xff");			//显示 菜单 按钮
          //正常结束显示页面
          if(!break_Runcode)
            {
              printf("page jieshu\xff\xff\xff");
              BUZZER=1;
              delay_ms(50);
              BUZZER=0;
              delay_ms(50);
              BUZZER=1;
              delay_ms(50);
              BUZZER=0;

            }
          else
            {
              uart4_printf("手动结束热程序！\r\n");
            }

          if(atuodoor&&!break_Runcode)
            {
              door_flag=2;
            }
          break_Runcode=0;

          isRunning_Runcode=0;
          vTaskResume(Showhmi_Task_Handler);  //转为就绪态，运行
          //删除runcode任务
          vTaskDelete(NULL);
        }
    }
}


TickType_t xLastWakeTime;

void heating(u8 run_p_num,u8 all_p_num,float starttem,float endtem,u16 time,u8 fan)
{



  volatile int thisP_run_time=0;
  volatile int wait_time=0,endtem_int,pvtem;
  volatile float prograss_time;
  volatile float v=(endtem - starttem)/time;
  xLastWakeTime = xTaskGetTickCount();
  endtem_int=(int)endtem;
  prograss_time=time;

  uart4_printf("heating(%d/%d,starttem:%02f,endtem:%02f,time:%ds)\r\n",
               run_p_num,all_p_num,starttem,endtem,time);
  printf("zhuangtai.val=2\xff\xff\xff");		//0-待机 1-预烘干 2-运行 3-故障

  Send_Thermostat('w',0x1b,0);			//1b-Srun： 0-run 1-stop 2-hold
  delay_ms(200);
  Send_Thermostat('w', 0x50, starttem*10 );
  delay_ms(200);
  //设置输出电流寄存器地址0x0001   0x0000：0A
  Send_power('w',0x01,Output*46+200);
  delay_ms(200);
  //读状态
  Send_power('r',0x07,0x0005);


  uart4_printf("(int)starttem=%d   (int)endtem=%d\r\n",(int)starttem,(int)endtem);
  while(time--)
    {

      if(v==0)
        {
          pvtem =endtem_int;
        }
      else
        {
          pvtem = (int)(starttem +	v*thisP_run_time );
        }


      if (debug)
        {
          uart4_printf("本段:%.2f-> %.2f ( %.2f ℃/s ),运行:%ds|%ds, 当前设定(实际)：%d ( %d )\r\n",
                       starttem,endtem,v,thisP_run_time,(int)prograss_time,pvtem,PVTem);
        }

      Send_Thermostat('w', 0x50, pvtem*10 );
      vTaskDelayUntil(&xLastWakeTime,1000/4);
      //设置输出电流寄存器地址0x0001   0x0000：0A
      Send_power('w',0x01,Output*46+200);
      vTaskDelayUntil(&xLastWakeTime,1000/4);
      //读状态
      Send_power('r',0x07,0x0005);
      vTaskDelayUntil(&xLastWakeTime,1000/4);

      thisP_run_time=thisP_run_time+1;
      runingtime = runingtime + 1;

      printf("yunxing.zongduan.val=%d\xff\xff\xff",all_p_num/2);
      printf("yunxing.dangqianduan.val=%d\xff\xff\xff",run_p_num/2+1);

      printf("duanshengyu.txt=\"%02d:%02d:%02d\"\xff\xff\xff",
             ((int)prograss_time-thisP_run_time)/3600,
             ((int)prograss_time-thisP_run_time)%3600/60,
             ((int)prograss_time-thisP_run_time+1)%3600%60);

      printf("zongshengyu.txt=\"%02d:%02d:%02d\"\xff\xff\xff",
             (Alltime-runingtime)/3600,
             (Alltime-runingtime)%3600/60,
             (Alltime-runingtime)%3600%60);

      printf("jindu.val=%d\xff\xff\xff",runingtime*100/Alltime);
      vTaskDelayUntil(&xLastWakeTime,1000/4);

      if(break_Runcode)
        {

          uart4_printf("手动结束，当前运行heatingr\n");
          return;
        }

    }
  pvtem =endtem_int;
  Send_Thermostat('w', 0x50, endtem_int*10 );

  //终了温度未到设定温度5内，进入等待
  if( PVTem < endtem-5  )
    {
      uart4_printf("运行时间结束，未到设定温度5内，开始等待升温r\n");
    }
  wait_time=0;
  while(PVTem < endtem-5 && endtem!=0 )
    {

      Send_Thermostat('r',0x4a,0);              	//4a-PV：测量温度
      delay_ms(1000/4);
      //设置输出电流寄存器地址0x0001   0x0000：0A
      Send_power('w',0x01,Output*46+200);
      delay_ms(1000/4);
      //读状态
      Send_power('r',0x07,0x0005);
      delay_ms(1000/4);
      delay_ms(1000/4);

      wait_time=wait_time+1;
      if (debug)
        {

          uart4_printf("等待设定温度：%d->%d  ,已等待时间%d s\r\n",PVTem,endtem_int,wait_time);
        }
      if(break_Runcode)
        {

          uart4_printf("手动结束，当前等待至设定温度\n");
          return;
        }

      //error4:炉膛密封差    半小时未达设定温度(heating)
      if(wait_time>1800&&endtem - starttem)
        {
          uart4_printf("半小时未达设定温度，error4:炉膛密封差\r\n");
          errormessage(4);
        }
    }

  uart4_printf("达到设定值，结束等待\r\n");

}



void errormessage(volatile int error)
{
  int i = 0;
  u8 LED;

  //防止传递屏幕数据错乱，挂起温控器与开关电源通信任务
  vTaskSuspend(Showhmi_Task_Handler); 	//任务挂起（停止）

  vTaskSuspend(GetCT_Task_Handler); 	//任务挂起（停止）
  while(1)
    {
      //not v error
      if (error!=5&&error!=0)
        {
          //run->stop
          if(error!=8&&error!=1&&error!=2)
            {
              Send_Thermostat('w',0x1b,1);
            } //1b-Srun： 0-run 1-stop 2-hold
          //motor ->stop
          door_flag=0;      //0 停止，1 up ,2 down
          //开关电源继电器断开
          POWER=1;

          //led->red
          LED=2;					//YELLOW_LED = 1;RED_LED= 2;GREEN_LED= 3;
          xQueueSendToBack( LED_xQueue,&LED,0);	//写队列

          //write error log into hmi page_guzhang
          printf("page pingbao\xff\xff\xff");
          printf("page pingbao\xff\xff\xff");
          delay_ms(2000);
          printf("page guzhang\xff\xff\xff");
          printf("page guzhang\xff\xff\xff");
          printf("pingbao.riqi.val=0\xff\xff\xff");
          printf("spstr pingbao.riqi.txt,beizhu.shijian.txt,\"^\",0\xff\xff\xff");
          printf("guzhang.timetemp.txt=beizhu.shijian.txt+\"^\"\+\"%d\"\xff\xff\xff",error);
          printf("guzhang.guzhangjilu.insert(guzhang.timetemp.txt)\xff\xff\xff");
          //BUZZER run write error log only

          for(;;)
            {
              BUZZER=1;
            }
        }
      //if error=5
      if (error==5)
        {

          //write error log into hmi page_guzhang
          printf("page pingbao\xff\xff\xff");
          printf("page pingbao\xff\xff\xff");
          delay_ms(2000);
          printf("page guzhang\xff\xff\xff");
          printf("page guzhang\xff\xff\xff");
          printf("pingbao.riqi.val=0\xff\xff\xff");
          printf("guzhang.timetemp.txt=pingbao.riqi.txt+\"^\"\+\"%d\"\xff\xff\xff",error);
          printf("guzhang.guzhangjilu.insert(timetemp.txt)\xff\xff\xff");
          printf("page yunxing\xff\xff\xff");


          vTaskResume(Showhmi_Task_Handler);  //转为就绪态，运行
          vTaskResume(GetCT_Task_Handler);

          for(i = 0; i<10; i++)
            {
              BUZZER=1;
              delay_ms(50);
              BUZZER=0;
              delay_ms(50);
            }
          error=0;
          delay_ms(1000);
          break;
        }

    }
}







