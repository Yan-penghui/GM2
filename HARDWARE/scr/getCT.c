
/*-------------------------------------------------*/
/*                                                 */
/*          	获取AHT20温湿度                      */
/*                                                 */
/*-------------------------------------------------*/

#include "delay.h"
#include "getCT.h"
#include "AHT20.h"


void my_getCT_task(void *pvParameters)
{
  Init_I2C_Sensor_Port(); 							//IIC初始化
  delay_ms(500);
  if((AHT20_Read_Status()&0x18)!=0x18)
    {
      AHT20_Start_Init();
      Delay_1ms(10);
    }
  delay_ms(4000);
  while(1)
    {
      delay_ms(3000);
      //读取温湿度
      getCT();
    }
}

