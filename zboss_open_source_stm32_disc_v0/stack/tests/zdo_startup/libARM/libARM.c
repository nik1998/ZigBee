
#include "libARM.h"

#define SWITCH_DELAY 500000
static int pulse=1600;

 // GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable peripheral clock for GPIOD port */
 // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  // SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource0);

  //Init Leds
  void InitLeds(int Pins,int PinSource)
  {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA,PinSource,GPIO_AF_TIM1);
  GPIO_InitStructure.GPIO_Pin= Pins;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  }
   
  //Int Timer
  void TimTim()
  {
    TIM_TimeBaseInitTypeDef ttt;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
    ttt.TIM_Period=100000/60-1;
    ttt.TIM_Prescaler=1680;
    ttt.TIM_ClockDivision=0;
    ttt.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1,&ttt);
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
    TIM_Cmd(TIM1,ENABLE);
    TIM_OCInitTypeDef aaa;
    aaa.TIM_OCMode=TIM_OCMode_PWM1;
    aaa.TIM_Pulse=1600;
    aaa.TIM_OCPolarity=TIM_OCPolarity_High; 
    TIM_OC1Init(TIM1,&aaa);
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC2Init(TIM1,&aaa);
    TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC3Init(TIM1,&aaa);
    TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);
  }

 /* void Timm()
  {
    TIM_TimeBaseInitTypeDef ttt;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
    ttt.TIM_Period=100000/60-1;
    ttt.TIM_Prescaler=1680;
    ttt.TIM_ClockDivision=0;
    ttt.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1,&ttt);
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
    TIM_Cmd(TIM1,ENABLE);
    TIM_OCInitTypeDef aaa;
    aaa.TIM_OCMode=TIM_OCMode_PWM1;
    aaa.TIM_Pulse=1600;
    aaa.TIM_OCPolarity=TIM_OCPolarity_High; 
    TIM_OC1Init(TIM1,&aaa);
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);
  } */

  void  ChangePulse(int delta)      
  {    
     if(pulse+delta>0 && pulse+delta<1600)
     {
        pulse=pulse+delta;
     }
     TIM_SetCompare1(TIM1,pulse);
  }


 

