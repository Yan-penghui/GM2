
/*-------------------------------------------------*/
/*                                                 */
/*          		 定时器3                       */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：定时器3使能30s定时                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
//定时器3中断服务程序
//extern vu16 USART2_RX_STA;
void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//是更新中断
    {
      USART2_RX_STA|=1<<15;	//标记接收完成
      TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIM3更新中断标志
      TIM_Cmd(TIM3, DISABLE);  //关闭TIM3
    }
}

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM3_Int_Init(u16 arr,u16 psc)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//TIM3时钟使能

  //定时器TIM3初始化
  TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

  TIM_Cmd(TIM3,ENABLE);//开启定时器3

  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
  NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

}



//void TIM3_ENABLE_30S(void)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //定义一个设置定时器的变量
//	NVIC_InitTypeDef NVIC_InitStructure;                           //定义一个设置中断的变量
//
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);           //使能TIM3时钟
//	TIM_DeInit(TIM3);                                              //定时器3寄存器恢复默认值
//	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	           //设置自动重装载值
//	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;             //设置定时器预分频数
//	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
//	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1分频
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);            //设置TIM3
//
//	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);                    //清除溢出中断标志位
//	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                     //使能TIM3溢出中断
//	TIM_Cmd(TIM3, ENABLE);                                         //开TIM3
//
//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                //设置TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;      //抢占优先级2
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;             //子优先级0
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //中断通道使能
//	NVIC_Init(&NVIC_InitStructure);                                //设置中断
//}
///*-------------------------------------------------*/
///*函数名：定时器3使能2s定时                        */
///*参  数：无                                       */
///*返回值：无                                       */
///*-------------------------------------------------*/
//void TIM3_ENABLE_2S(void)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //定义一个设置定时器的变量
//	NVIC_InitTypeDef NVIC_InitStructure;                           //定义一个设置中断的变量
//
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);           //使能TIM3时钟
//	TIM_DeInit(TIM3);                                              //定时器3寄存器恢复默认值
//	TIM_TimeBaseInitStructure.TIM_Period = 20000-1; 	           //设置自动重装载值
//	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;              //设置定时器预分频数
//	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
//	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1分频
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);            //设置TIM3
//
//	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);                    //清除溢出中断标志位
//	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                     //使能TIM3溢出中断
//	TIM_Cmd(TIM3, ENABLE);                                         //开TIM3
//
//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                //设置TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;      //抢占优先级1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;             //子优先级0
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //中断通道使能
//	NVIC_Init(&NVIC_InitStructure);                                //设置中断
//}

