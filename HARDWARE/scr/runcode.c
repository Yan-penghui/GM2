
/*---------------------------------------------------------------*/
/*��������void my_runcode_task(void *pvParameters)   		    	  */
/*��  �ܣ����г�������       																		 */
/*��  ������                          			   									 */
/*����ֵ����                                       					     */
/*��  ����                                                       */
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
  xQueueSendToBack( LED_xQueue,&LED,0);	//д����
  Program_data[17][0]=0;
  while(1)
    {
      //printf("��������------Runcode------\r\n");

      if (pdPASS == xQueueReceive( runcode_xQueue,&runcode,portMAX_DELAY ) )    	//������
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

          //��ֹ������Ļ���ݴ��ң������¿����뿪�ص�Դͨ������
          vTaskSuspend(Showhmi_Task_Handler); 	//�������ֹͣ��


          //¯�ſ��������ֶ�����
          if(!LIM1)
            {
              printf("page yunxing\xff\xff\xff");		//��ת�����н���
              printf("page yunxing\xff\xff\xff");		//��ת�����н���
              printf("page guanbilumen\xff\xff\xff");
              isRunning_Runcode=0;

              //����ָ�Ϊ����
              vTaskResume(Showhmi_Task_Handler);  //������������У�

              vTaskDelete(NULL);
            }

          //��������

          printf("page yunxing\xff\xff\xff");
          printf("page yunxing\xff\xff\xff");

          printf("caozuojilu.yunxingjilu.insert(beizhu.yunxingxinxi.txt)\xff\xff\xff");		//write run log
          printf("vis b3,0\xff\xff\xff");			//���� ��ʼ ��ť
          printf("vis doorup,0\xff\xff\xff");			//���� ���� ��ť
          printf("vis b0,0\xff\xff\xff");			//��ʾ ���� ��ť
          printf("vis b5,0\xff\xff\xff");			//��ʾ �˵� ��ť
          printf("vis doordown,0\xff\xff\xff");   //���� ���� ��ť
          printf("jindu.val=0\xff\xff\xff");  //���ȹ���

          //���ص�Դ�̵�������
          POWER=0;
          duandianjixv=0;

          //��������
          printf("yunxing.tm0.en=1\xff\xff\xff");
          printf("yunxing.tm0.en=1\xff\xff\xff");

          delay_ms(500);
          //Ѱ�ҵ�ǰ�¶ȶ�Ӧ������¶ȶ�-�¶ȵ�
          nowtem=PVTem;

          //����������ʱ��
          Alltime=0 ;
          for(j=0; j<16; j++)
            {
              Alltime=Alltime + Program_data[j][1] ;
            }

          uart4_printf("��ǰ����������ʱ��Ԥ�� %02d��%02d\r\n",Alltime/60,Alltime%60);

          //���������ܶ���
          all_p_num=0 ;
          for(j=1; j<16; j++)
            {

              if(Program_data[j][0]==0)
                {
                  all_p_num=j-1;
                  break;
                }
            }
          uart4_printf("��ǰ���������ܶ�����%d\r\n",all_p_num);

          //Ѱ�Ҽ�����ʼ�����
          for(i=0 ; i<17 ; i++)
            {

              if(nowtem<Program_data[i+1][0]&&nowtem>=Program_data[i][0])
                {

                  progress_num = i;

//                  printf("yunxing.dangqianduan.val=%d",i/2);
//									printf("yunxing.zongduan.val=%d",);
                  uart4_printf("��ǰ�����¶Σ��¶�%d�������%d��\r\n",nowtem,progress_num);
                  percent=1-(float)(nowtem-Program_data[i][0])/(Program_data[i+1][0]-Program_data[i][0]);

                  //����������ʱ��
                  runingtime=0;
                  for(j=0; j<i; j++)
                    {
                      runingtime=runingtime + Program_data[j][1] ;
                    }

                  runingtime = runingtime + Program_data[i][1] * (1-percent);

                  uart4_printf("��ʱ��%ds,��ǰ������%ds\xff\xff\xff",Alltime,runingtime);

                  printf("jindu.val=%d\xff\xff\xff",runingtime*100/Alltime);


                  progress_time=Program_data[i][1]*percent;//��ȡ��ǰ����ʱ��
                  heating(i,all_p_num,nowtem,Program_data[i+1][0],(int)(Program_data[i][1]*percent),fan);	 //��ǰ�¶�   Ŀ���¶�   ʱ��s

                  break;
                }

            }

          uart4_printf("�׶ν�������ʼ����ʣ����ȶ�\r\n");

          //ʣ����ȶ�
          i=i+1;
          for(i=i; i<17 ; i++)
            {
              if(break_Runcode)
                {

                  uart4_printf("�ֶ���������ǰ�ȴ����趨�¶�\n");
                  break ;
                }

              progress_time=Program_data[i][1];		//��ȡ��ǰ����ʱ��

              uart4_printf("�����%d�Σ��¶�%d->%d������ʱ����%02d��%02d\n",
                           i,Program_data[i][0],Program_data[i+1][0],progress_time/60,progress_time%60);



              //����0�����˳�
              if(Program_data[i+1][0]==0)
                {
                  uart4_printf("���ȳ������н���\r\n");
                  break ;
                }

              heating(i,all_p_num,(float)Program_data[i][0],(float)Program_data[i+1][0],progress_time,fan);

            }


          //�������������ʱ�¶ȸ���1400���ս����+1
          if((!break_Runcode)||(PVTem-TEMchange+reTEMchange)>1400)
            {

              printf("wepo yunxing.shaojiecishu.val,272\xff\xff\xff");
              printf("yunxing.shaojiecishu.val=yunxing.shaojiecishu.val+1\xff\xff\xff");
              printf("wepo yunxing.shaojiecishu.val,272\xff\xff\xff");

            }
          //�ر�����
          printf("yunxing.tm0.en=0\xff\xff\xff");
          printf("yunxing.tm0.en=0\xff\xff\xff");


          //���ص�Դ�̵����Ͽ�
          POWER=1;
          Send_Thermostat('w',0x1b,1);				//1b-Srun�� 0-run 1-stop 2-hold
          delay_ms(100);
          vTaskResume(Showhmi_Task_Handler);  //Showhmi_TaskתΪ����̬������

          //�������Զ����ţ����µ�atuodoor_downTem������һ����
          if(atuodoor)
            {
              while(PVTem-TEMchange+reTEMchange >= atuodoor_downTem )
                {
                  if(break_Runcode)
                    {

                      uart4_printf("�ֶ���������ǰ�ȴ�������%d\n",atuodoor_downTem);
                      break ;
                    }

                  delay_ms(1000);
                  if (debug)
                    {
                      uart4_printf("�ȴ����µ�%d������һ����\r\n",atuodoor_downTem);
                    }
                }

            }

          //��ֹ������Ļ���ݴ��ң������¿����뿪�ص�Դͨ������
          vTaskSuspend(Showhmi_Task_Handler); 	//�������ֹͣ��

          printf("zhuangtai.val=0\xff\xff\xff");		//0-���� 1-�Զ������� 2-���� 3-����
          printf("page yunxing\xff\xff\xff");
          printf("zhuangtai.val=0\xff\xff\xff");		//0-���� 1-Ԥ��� 2-���� 3-����
          printf("page yunxing\xff\xff\xff");
          printf("jindu.val=100\xff\xff\xff");

          printf("zongshengyu.txt=\"00:00:00\"\xff\xff\xff");

          LED=1;					//YELLOW_LED = 1;RED_LED= 2;GREEN_LED= 3;
          xQueueSendToBack( LED_xQueue,&LED,0);	//д����

          printf("vis b3,1\xff\xff\xff");			//��ʾ ��ʼ ��ť
          printf("vis b3,1\xff\xff\xff");			//��ʾ ��ʼ ��ť
          printf("vis doorup,1\xff\xff\xff");			//��ʾ ���� ��ť
          printf("vis doordown,1\xff\xff\xff");
          printf("vis b0,1\xff\xff\xff");			//��ʾ ���� ��ť
          printf("vis b5,1\xff\xff\xff");			//��ʾ �˵� ��ť
          //����������ʾҳ��
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
              uart4_printf("�ֶ������ȳ���\r\n");
            }

          if(atuodoor&&!break_Runcode)
            {
              door_flag=2;
            }
          break_Runcode=0;

          isRunning_Runcode=0;
          vTaskResume(Showhmi_Task_Handler);  //תΪ����̬������
          //ɾ��runcode����
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
  printf("zhuangtai.val=2\xff\xff\xff");		//0-���� 1-Ԥ��� 2-���� 3-����

  Send_Thermostat('w',0x1b,0);			//1b-Srun�� 0-run 1-stop 2-hold
  delay_ms(200);
  Send_Thermostat('w', 0x50, starttem*10 );
  delay_ms(200);
  //������������Ĵ�����ַ0x0001   0x0000��0A
  Send_power('w',0x01,Output*46+200);
  delay_ms(200);
  //��״̬
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
          uart4_printf("����:%.2f-> %.2f ( %.2f ��/s ),����:%ds|%ds, ��ǰ�趨(ʵ��)��%d ( %d )\r\n",
                       starttem,endtem,v,thisP_run_time,(int)prograss_time,pvtem,PVTem);
        }

      Send_Thermostat('w', 0x50, pvtem*10 );
      vTaskDelayUntil(&xLastWakeTime,1000/4);
      //������������Ĵ�����ַ0x0001   0x0000��0A
      Send_power('w',0x01,Output*46+200);
      vTaskDelayUntil(&xLastWakeTime,1000/4);
      //��״̬
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

          uart4_printf("�ֶ���������ǰ����heatingr\n");
          return;
        }

    }
  pvtem =endtem_int;
  Send_Thermostat('w', 0x50, endtem_int*10 );

  //�����¶�δ���趨�¶�5�ڣ�����ȴ�
  if( PVTem < endtem-5  )
    {
      uart4_printf("����ʱ�������δ���趨�¶�5�ڣ���ʼ�ȴ�����r\n");
    }
  wait_time=0;
  while(PVTem < endtem-5 && endtem!=0 )
    {

      Send_Thermostat('r',0x4a,0);              	//4a-PV�������¶�
      delay_ms(1000/4);
      //������������Ĵ�����ַ0x0001   0x0000��0A
      Send_power('w',0x01,Output*46+200);
      delay_ms(1000/4);
      //��״̬
      Send_power('r',0x07,0x0005);
      delay_ms(1000/4);
      delay_ms(1000/4);

      wait_time=wait_time+1;
      if (debug)
        {

          uart4_printf("�ȴ��趨�¶ȣ�%d->%d  ,�ѵȴ�ʱ��%d s\r\n",PVTem,endtem_int,wait_time);
        }
      if(break_Runcode)
        {

          uart4_printf("�ֶ���������ǰ�ȴ����趨�¶�\n");
          return;
        }

      //error4:¯���ܷ��    ��Сʱδ���趨�¶�(heating)
      if(wait_time>1800&&endtem - starttem)
        {
          uart4_printf("��Сʱδ���趨�¶ȣ�error4:¯���ܷ��\r\n");
          errormessage(4);
        }
    }

  uart4_printf("�ﵽ�趨ֵ�������ȴ�\r\n");

}



void errormessage(volatile int error)
{
  int i = 0;
  u8 LED;

  //��ֹ������Ļ���ݴ��ң������¿����뿪�ص�Դͨ������
  vTaskSuspend(Showhmi_Task_Handler); 	//�������ֹͣ��

  vTaskSuspend(GetCT_Task_Handler); 	//�������ֹͣ��
  while(1)
    {
      //not v error
      if (error!=5&&error!=0)
        {
          //run->stop
          if(error!=8&&error!=1&&error!=2)
            {
              Send_Thermostat('w',0x1b,1);
            } //1b-Srun�� 0-run 1-stop 2-hold
          //motor ->stop
          door_flag=0;      //0 ֹͣ��1 up ,2 down
          //���ص�Դ�̵����Ͽ�
          POWER=1;

          //led->red
          LED=2;					//YELLOW_LED = 1;RED_LED= 2;GREEN_LED= 3;
          xQueueSendToBack( LED_xQueue,&LED,0);	//д����

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


          vTaskResume(Showhmi_Task_Handler);  //תΪ����̬������
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







