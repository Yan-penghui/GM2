
#ifndef __WIFI_H
#define __WIFI_H

#include "uart4.h"	   	

#define RESET_IO(x)    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)x)  //PA4控制WiFi的复位

#define WiFi_printf       u4_printf           //串口4控制 WiFi
#define WiFi_RxCounter    Usart4_RxCounter    //串口4控制 WiFi
#define WiFi_RX_BUF       Usart4_RxBuff       //串口4控制 WiFi
#define WiFi_RXBUFF_SIZE  USART4_RXBUFF_SIZE  //串口4控制 WiFi

extern const char SSID[];                     //路由器名称，在main.c中设置
extern const char PASS[];                     //路由器密码，在main.c中设置

extern int ServerPort;
extern char ServerIP[128];                    //存放服务器IP或是域名

void wifi_reset_io_init(void);
char WiFi_SendCmd(char *cmd, int timeout);
char WiFi_Reset(int timeout);
char WiFi_JoinAP(int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_Smartconfig(int timeout);
char WiFi_WaitAP(int timeout);
char WiFi_Connect_IoTServer(void);


#endif


