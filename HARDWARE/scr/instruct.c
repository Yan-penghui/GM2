
/*---------------------------------------------------------------*/
/*��������void my_instruct_task(void *pvParameters)   		       */
/*��  �ܣ�������ָ�����״̬����       							             */
/*��  ������                          			   			          	 */
/*����ֵ����                                       			         */
/*��  ���������� U1_xQueue                                       */
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
      if (pdPASS == xQueueReceive( U1_xQueue,&instruct_RxBuff,portMAX_DELAY ))    	//������
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

//������01 ���м��ȳ���
          if(instruct == 0x01)
            {

 //����runcode����
              if(!isRunning_Runcode)
                {
                  xTaskCreate(my_runcode_task, "my_runcode_task", 128*2, NULL, 4, &Runcode_Task_Handler);
                  uart4_printf("���ȳ���Runcode�����ɹ�!\r\n");
                }
              else if(isRunning_Runcode)
                {
                  uart4_printf("����Runcode�����У����ȳ���Runcode����ʧ�ܣ�\r\n");
                }

              isRunning_Runcode=1;
 //����������� Program_data[17][2]:[�¶�][ʱ��]
              for(i=2; i<17; i+=2)
                {
                  TEM_data[(i-2)/2]		=(instruct_RxBuff[i+1]<<8|instruct_RxBuff[i]);          //�¶ȣ����϶�
                  V_data[(i-2)/2]		=instruct_RxBuff[i+1+16]<<8|instruct_RxBuff[i+16];		//���ʣ����϶�/s
                  H_data[(i-2)/2]		=instruct_RxBuff[i+1+32]<<8|instruct_RxBuff[i+32];		//����ʱ�䣺����

                }

              myProgram_data[0][0]	= 45;													//��һ���¶ȵ㣺45���϶�
              hotting_time_s=1 ;                                //��һ���¶ȵ㣺1s
              // myProgram_data[0][1]	= hotting_time_s;
              myProgram_data[0][1]	= abs(TEM_data[1]-TEM_data[0])*60.0/(float)V_data[0];

              for(i=0; i<8; i+=1)
                {
                  myProgram_data[2*i+1][0]	= TEM_data[i];						//�¶�
                  myProgram_data[2*i+1][1]	= H_data[i]*60;						//ʱ��s

                  myProgram_data[2*i+2][0]	= TEM_data[i];						//�¶�
                  hotting_time_s=abs(TEM_data[i+1]-TEM_data[i])*60.0/(float)V_data[i+1];
                  myProgram_data[2*i+2][1]	= (int)hotting_time_s;
                  if(i>=7)
                    {
                      myProgram_data[2*i+2][1]	= 0;
                    }
                }

//���ԣ���ʾ���ɳ�����������
              for(i=0; i<17; i+=1)
                {

                  runcode[2*i] = myProgram_data[i][0];
                  runcode[2*i+1] = myProgram_data[i][1];
//if (debug)
//{
//	uart4_printf("runcode[%d] =%d��runcode[%d+1] =%d s\r\n",i,myProgram_data[i][0],i,myProgram_data[i][1]);
//}
                }

//�߼����ò���д��
              runcode[34] = instruct_RxBuff[51]<<8|instruct_RxBuff[50] ;			//fan
              runcode[35] = instruct_RxBuff[53]<<8|instruct_RxBuff[52];				//atuodoor
              runcode[36] = instruct_RxBuff[55]<<8|instruct_RxBuff[54];				//atuodoor_downTem
              runcode[37] = instruct_RxBuff[57]<<8|instruct_RxBuff[56];				//rehot
              runcode[38] = instruct_RxBuff[59]<<8|instruct_RxBuff[58];				//rehottem
              runcode[39] = instruct_RxBuff[61]<<8|instruct_RxBuff[60] ;			//rehottime

//д���и�runcode����
              xQueueSendToBack( runcode_xQueue,&runcode,100);	//д����

            }

//������02 �޸��¶�
          else if(instruct == 0x02)
            {
              TEMchange=instruct_RxBuff[2];      //tem change
              if(instruct_RxBuff[2]>100)
                {
                  TEMchange=instruct_RxBuff[2]-256;
                }
              uart4_printf("��ǰ�¿�����%d�桰����Ļ��ʾ�¶�:%d���û�����ֵ��%d����������ֵ��%d\r\n",

                           PVTem,PVTem-TEMchange+reTEMchange,TEMchange,reTEMchange);

            }

//������10 ¯������
          else if(instruct == 0x10)
            {
              switch (door_flag)
                {
                //��ǰֹͣ״̬
                case 0:
                  door_flag=1;      //0 ֹͣ��1 up ,2 down
                  break;
                //��ǰ����״̬��תΪֹͣ
                case 1:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //��ǰ�½�״̬��תΪֹͣ
                case 2:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //δ֪״̬
                default:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                }

            }
//������11 ¯��ֹͣ
          else if(instruct == 0x11)
            {

              door_flag=0;      //0 ֹͣ��1 up ,2 down
              DOWN=1;
              UP = 1; 		/*�͵�ƽ��Ч*/
              printf("doorup.pic=0\xff\xff\xff");
              printf("doordown.pic=2\xff\xff\xff");

            }

//������12 ¯���½�
          else if(instruct == 0x12)
            {

              switch (door_flag)
                {
                //��ǰֹͣ״̬
                case 0:
                  door_flag=2;      //0 ֹͣ��1 up ,2 down
                  break;
                //��ǰ����״̬��תΪֹͣ
                case 1:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //��ǰ�½�״̬��תΪֹͣ
                case 2:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                //δ֪״̬
                default:
                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  DOWN=1;
                  UP = 1; 		/*�͵�ƽ��Ч*/
                  printf("doorup.pic=0\xff\xff\xff");
                  printf("doordown.pic=2\xff\xff\xff");
                  break;
                }
            }

//������14 �����������еļ��ȳ���
          else if(instruct == 0x14)
            {

              printf("vis b3,1\xff\xff\xff");			//��ʾ ��ʼ ��ť
              printf("vis b3,1\xff\xff\xff");			//��ʾ ��ʼ ��ť
              printf("zhuangtai.val=0\xff\xff\xff");		//0-���� 1-Ԥ��� 2-���� 3-����
              printf("vis b0,1\xff\xff\xff");			//��ʾ ���� ��ť
              printf("vis b5,1\xff\xff\xff");			//��ʾ �˵� ��ť
              printf("vis doorup,1\xff\xff\xff");			//��ʾ ���� ��ť
              printf("vis doordown,1\xff\xff\xff");


              if(isRunning_Runcode)
                {
                  //vTaskSuspend(Runcode_Task_Handler); 	//�������ֹͣ��
                  //ɾ��runcode����

                  door_flag=0;      //0 ֹͣ��1 up ,2 down
                  break_Runcode=1;

                }
              else
                {
                  uart4_printf("�޼��ȳ����������У�\r\n");
                }
            }

//������15 �ϵ����
          else if(instruct == 0x15)
            {
              //�ϵ����
              duandianjixv=1;
            }
        }
//��մ���1���ջ�����
      memset(usart1_rx_package, 0, usart1_rx_len_MAX);
    }
}


