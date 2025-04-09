
/*-------------------------------------------------*/
/*                                                 */
/*          	 WIFI（ESP8266）源文件             */
/*                                                 */
/*-------------------------------------------------*/

// 硬件连接：
// PA2 RX
// PA3 TX
// PA4 复位

#include "stm32f10x.h"  //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "delay.h"	    //包含需要的头文件
#include "usart1.h"	    //包含需要的头文件
#include "stdio.h"      //包含需要的头文件
#include "string.h"	    //包含需要的头文件
//#include <stdio.h>	

/*-------------------------------------------------*/
/*函数名：WiFi发送设置指令                         */
/*参  数：cmd：指令                                */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter = 0;                           	//WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF, 0, WiFi_RXBUFF_SIZE);     	//清空WiFi接收缓冲区 
	WiFi_printf("%s\r\n", cmd);                  	//发送指令
	
	
	while(timeout--)								//等待超时时间到0
	{     
		delay_ms(100);								//延时100ms 
		if(strstr(WiFi_RX_BUF, "OK"))              	//如果接收到OK表示指令成功
		   break;  									//主动跳出while循环
		printf("%d ", timeout);                 	//串口输出现在的超时时间
	}			
	printf("\r\n");                          			   
	if(timeout <= 0)return 1;                       //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else return 0;		         					//反之，表示正确，说明收到OK，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：断开MQTT服务器                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char MQTT_Reset(char *cmd,int timeout)
{
	WiFi_RxCounter = 0;                           	//WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF, 0, WiFi_RXBUFF_SIZE);     	//清空WiFi接收缓冲区 
	WiFi_printf("%s\r\n", cmd);                  	//发送指令
	
	
	while(timeout--)								//等待超时时间到0
	{     
		delay_ms(100);								//延时100ms 
		if(strstr(WiFi_RX_BUF, "OK")||strstr(WiFi_RX_BUF, "ERROR"))              	//如果接收到OK表示指令成功
		   break;  									//主动跳出while循环
		printf("%d ", timeout);                 	//串口输出现在的超时时间
	}			
	printf("\r\n");                          			   
	if(timeout <= 0)return 1;                       //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else return 0;		         					//反之，表示正确，说明收到OK，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter = 0;                                    //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF, 0, WiFi_RXBUFF_SIZE);              //清空WiFi接收缓冲区 
	WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASS); //发送指令	
	while(timeout--)									   //等待超时时间到0
	{                                   
		delay_ms(1000);                             	   //延时1s
		if(strstr(WiFi_RX_BUF, "WIFI GOT IP\r\n\r\nOK"))   //如果接收到WIFI GOT IP表示成功
			break;       						           //主动跳出while循环
		printf("%d ", timeout);                            //串口输出现在的超时时间
	}
	printf("\r\n");                             	       //串口输出信息
	if(timeout <= 0)return 1;                              //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                              //正确，返回0
}


/*-------------------------------------------------*/
/*函数名：配置MQTT用户属性				            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char MQTT_Cfg_Server(int timeout)
{	
	WiFi_RxCounter = 0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 
	WiFi_printf("AT+MQTTUSERCFG=0,2,\"TAN\",\"yph\",\"1032979127\",0,0,\"\"\r\n"); //发送配置指令
	
	while(timeout--)								  //等待超时与否
	{                           
		delay_ms(100);                             	  //延时100ms	
		if(strstr(WiFi_RX_BUF, "OK"))            //如果接受到OK表示配置成功
			break;                                    //跳出while循环
		if(strstr(WiFi_RX_BUF, "ERROR"))             //如果接受到ERROR表示配置失败
		{WiFi_printf("AT+MQTTUSERCFG=ERROR"); //发送配置指令
			return 1; }                                //配置失败返回1
		printf("%d ", timeout);                       //串口输出现在的超时时间  
	}
	printf("\r\n");                                   
	if(timeout <= 0)return 2;                         //超时错误，返回2
	
	return 0;	                                      //成功返回0	
}



/*-------------------------------------------------*/
/*函数名：连接MQTT服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char MQTT_Connect_Server(int timeout)
{	
	WiFi_RxCounter = 0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 
 
	WiFi_printf("AT+MQTTCONN=0,\"%s\",%d,1\r\n", ServerIP, ServerPort);//发送连接服务器指令
	
	while(timeout--)								  //等待超时与否
	{                           
		delay_ms(100);                             	  //延时100ms	
		if(strstr(WiFi_RX_BUF, "MQTTCONNECTED"))      //如果接受到MQTTCONNECTED表示连接成功
			break;                                    //跳出while循环
		if(strstr(WiFi_RX_BUF, "ERROR"))             //如果接受到CLOSED表示服务器未开启
		{
			printf("AT+MQTTCONN=ERROR\r\n");
			return 1;                                 //服务器未开启返回1
	
		}

		printf("%d ", timeout);                       //串口输出现在的超时时间  
	}
	printf("\r\n");                                   
	if(timeout <= 0)return 3;                         //超时错误，返回3

	return 0;	                                      //成功返回0	
}
/*-------------------------------------------------*/
/*函数名：WiFi_Smartconfig                         */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{
	
	WiFi_RxCounter = 0;                           		//WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     		//清空WiFi接收缓冲区     
	while(timeout--)									//等待超时时间到0
	{                           		
		delay_ms(1000);                         		//延时1s
		if(strstr(WiFi_RX_BUF, "connected"))    	 	//如果串口接受到connected表示成功
			break;                                  	//跳出while循环  
		printf("%d ", timeout);                 		//串口输出现在的超时时间  
	}	
	printf("\r\n");                          			
	if(timeout <= 0)return 1;                     		//超时错误，返回1
	return 0;                                   		//正确返回0
}
/*-------------------------------------------------*/
/*函数名：等待加入路由器                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //等待超时时间到0
		delay_ms(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF, "WIFI GOT IP"))      //如果接收到WIFI GOT IP表示成功
			break;       						 
		printf("%d ", timeout);                     //串口输出现在的超时时间
	}
	printf("\r\n");                             	//串口输出信息
	if(timeout <= 0)return 1;                       //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：WiFi连接服务器                           */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Connect_IoTServer(void)
{	
	
	
	printf("准备设置STA模式\r\n");                
	if(WiFi_SendCmd("AT+CWMODE=1",20))			  //设置STA模式，100ms超时单位，总计2s超时时间
	{             
		printf("设置STA模式失败，准备重启\r\n");  //返回非0值，进入if
		return 2;                                 //返回2
	}else printf("设置STA模式成功\r\n");          
	                            
//	printf("准备取消自动连接\r\n");            	  
//	if(WiFi_SendCmd("AT+CWAUTOCONN=0",50))		  //取消自动连接，100ms超时单位，总计5s超时时间
//	{       
//		printf("取消自动连接失败，准备重启\r\n"); //返回非0值，进入if
//		return 3;                                 //返回3
//	}else printf("取消自动连接成功\r\n");         
			
	printf("准备连接路由器\r\n");                 	
	if(WiFi_JoinAP(10))							  //连接路由器,1s超时单位，总计10s超时时间
	{                          
		printf("连接路由器失败，准备重启\r\n");   //返回非0值，进入if
		return 4;                                 //返回4	
	}else printf("连接路由器成功\r\n");       		

//	printf("准备设置透传\r\n");                    
//	if(WiFi_SendCmd("AT+CIPMODE=1",50)) 		  //设置透传，100ms超时单位，总计5s超时时间
//	{           
//		printf("设置透传失败，准备重启\r\n");     //返回非0值，进入if
//		return 8;                                 //返回8
//	}else printf("设置透传成功\r\n");              
	
//	printf("准备关闭多路连接\r\n");               
//	if(WiFi_SendCmd("AT+CIPMUX=0",50)) 		      //关闭多路连接，100ms超时单位，总计5s超时时间
//	{            
//		printf("关闭多路连接失败，准备重启\r\n"); //返回非0值，进入if
//		return 9;                                 //返回9
//	}else printf("关闭多路连接成功\r\n");  

	printf("MQTT服务器断开连接\r\n");                   
	if(MQTT_Reset("AT+MQTTCLEAN=0",100))							  //复位，100ms超时单位，总计5s超时时间
	{                             
		printf("断开MQTT服务器失败，准备重启\r\n");	      //返回非0值，进入if
		return 1;                                 //返回1
	}else printf("断开MQTT服务器成功\r\n");                 


	
	printf("准备配置MQTT用户属性\r\n"); 
	
	if(MQTT_Cfg_Server(100))      			  //连接服务器，100ms超时单位，总计10s超时时间
	{            
		printf("配置MQTT用户属性失败，准备重启\r\n");   //返回非0值，进入if
		return 9;                                //返回10
	}else printf("配置MQTT用户属性成功\r\n");     
	
	
	printf("准备连接服务器\r\n");                 
	if(MQTT_Connect_Server(150))      			  //连接服务器，100ms超时单位，总计10s超时时间
	{            
		printf("连接服务器失败，准备重启\r\n");   //返回非0值，进入if
		return 10;                                //返回10
	}else printf("连接服务器成功\r\n");           
	return 0;                                     //正确返回0
}
	

