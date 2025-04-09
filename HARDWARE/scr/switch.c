#include "switch.h"
#include "stm32f10x.h"

//4个继电器 2个限位开关
void Switch_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTC

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

  //PC4 使能温控器485 高电平可发送 低电平可接收 初始化为低电平
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOC,GPIO_Pin_4);

  //PC5 使能开关电源485 高电平可发送 低电平可接收 初始化为低电平
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOC,GPIO_Pin_5);

  //PA8 负载通断 低电平有效 初始化为高电平
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA,GPIO_Pin_8);

  //数字输入限位开关的初始化
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PB0 //上限位
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
  GPIO_ResetBits(GPIOB,GPIO_Pin_0);

  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;//PB1 下限位
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
  GPIO_ResetBits(GPIOB,GPIO_Pin_1);

  //电机
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;//PB12 电机下 低电平有效 初始化为高电平
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_12);

  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;//PB13  电机上 低电平有效 初始化为高电平
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_13);

  //风扇
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;//PB14 下风扇 低电平有效 初始化为高电平
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_14);

  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;//PB15  上风扇 低电平有效 初始化为高电平
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_15);

  //PC13 蜂鸣器
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;    //PC13 蜂鸣器 高电平有效 初始化为低电平
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOC,GPIO_Pin_13);

}
