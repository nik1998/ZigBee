#include "main.h"

#define SWITCH_DELAY 500000


int arr[3];
int main(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
//button interrupt
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
  //Init Leds 
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource8/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource9/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource10/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);

  GPIO_InitStructure.GPIO_Pin= GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //Init LedGPIO
  GPIO_InitTypeDef  GPIO_InitStructure1;
  GPIO_InitStructure1.GPIO_Pin= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure1.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure1.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure1);
  //light
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
  aaa.TIM_Pulse=0;
  aaa.TIM_OCPolarity=TIM_OCPolarity_High; 
  TIM_OC1Init(TIM1,&aaa);
  TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);
  TIM_OC2Init(TIM1,&aaa);
  TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);
  TIM_OC3Init(TIM1,&aaa);
  TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);

  //button
  GPIO_InitTypeDef GPIO_In;
GPIO_In.GPIO_Pin= GPIO_Pin_1|GPIO_Pin_0;
  GPIO_In.GPIO_Mode = GPIO_Mode_IN;
  GPIO_In.GPIO_OType = GPIO_OType_PP;
  GPIO_In.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_In.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_In);
 //interrupt
   EXTI_InitTypeDef eee;
   eee.EXTI_Line=EXTI_Line0;
   eee.EXTI_LineCmd=ENABLE;
   eee.EXTI_Mode=EXTI_Mode_Interrupt;
   eee.EXTI_Trigger=EXTI_Trigger_Falling;
   EXTI_Init(&eee);

  eee.EXTI_Line=EXTI_Line1;
  EXTI_Init(&eee);
   //vector
  NVIC_InitTypeDef nvec;
  nvec.NVIC_IRQChannel=EXTI0_IRQn;
  nvec.NVIC_IRQChannelPreemptionPriority=0x00;
  nvec.NVIC_IRQChannelSubPriority=0x01;
  nvec.NVIC_IRQChannelCmd=ENABLE;
  NVIC_Init(&nvec);

  nvec.NVIC_IRQChannel=EXTI1_IRQn;
  NVIC_Init(&nvec);
  arr[0]=0;
  arr[1]=0;
  arr[2]=0;
  GPIO_SetBits(GPIOD,GPIO_Pin_14); 
  while (1)
  {    
   // GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15);
  }
}  
 int ind=1;

void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
  {
    GPIO_ResetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15); 
      if(ind==1)
      {
        GPIO_SetBits(GPIOD,GPIO_Pin_12); 
        ind=2;
      }
      else
            if(ind==2)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_15); 
              ind=3;
            }
            else
            if(ind==3)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_14); 
              ind=1;
            }
      
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1)!=RESET)
  {
  //  GPIO_SetBits(GPIOA,GPIO_Pin_8);
    arr[ind-1]+=100;
    if(ind==1)
    TIM_SetCompare1(TIM1,arr[ind-1]);
    else
    if(ind==2)
    TIM_SetCompare2(TIM1,arr[ind-1]); 
    else
    TIM_SetCompare3(TIM1,arr[ind-1]); 
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
