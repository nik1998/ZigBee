// zb_cortexm4_exti.c
#include "zb_common.h"
#include "zb_cortexm4_exti.h"
#include "zb_g_context.h"
#include "zb_mrf24j40.h"

#if defined cortexm4


void LED_BUTTON_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_Led; 
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // LED
          	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // LED
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // Button
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		
 	GPIO_Led.GPIO_Pin = GPIO_Pin_13;
 	GPIO_Led.GPIO_Mode = GPIO_Mode_OUT;
 	GPIO_Led.GPIO_OType = GPIO_OType_PP;
 	GPIO_Led.GPIO_PuPd = GPIO_PuPd_NOPULL;
 	GPIO_Led.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(GPIOC, &GPIO_Led); // Init LED
	
	GPIO_Led.GPIO_Pin = GPIO_Pin_0;
 	GPIO_Led.GPIO_Mode = GPIO_Mode_IN;
 	GPIO_Led.GPIO_OType = GPIO_OType_PP;
 	GPIO_Led.GPIO_PuPd = GPIO_PuPd_DOWN;
 	GPIO_Led.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_Led); // Init Button

 	GPIO_Led.GPIO_Pin = GPIO_Pin_15;
 	GPIO_Led.GPIO_Mode = GPIO_Mode_OUT;
 	GPIO_Led.GPIO_OType = GPIO_OType_PP;
 	GPIO_Led.GPIO_PuPd = GPIO_PuPd_NOPULL;
 	GPIO_Led.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(GPIOD, &GPIO_Led); // Init LED

 	GPIO_Led.GPIO_Pin = GPIO_Pin_14;
 	GPIO_Led.GPIO_Mode = GPIO_Mode_OUT;
 	GPIO_Led.GPIO_OType = GPIO_OType_PP;
 	GPIO_Led.GPIO_PuPd = GPIO_PuPd_NOPULL;
 	GPIO_Led.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(GPIOD, &GPIO_Led); // Init LED        
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	EXTI_InitTypeDef exti_struct;
	exti_struct.EXTI_Line = EXTI_Line0;
	exti_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_struct);
	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	  /* Enable the Button External Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0x01;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0x01;
 	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}


// User button interrupt handler. Do nothing except the led.
void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0))
	{
		EXTI->PR = EXTI_Line0;
		//USART_SendData(ZB_SERIAL_DEV,'i');
		//GPIO_ToggleBits(GPIOC,GPIO_Pin_13);
		//ZB_SET_TRANS_INT();
                //TRANS_CTX().interrupt_flag = 1;
		EXTI->PR = EXTI_Line0;
	}
}



// Radio interrupt pin handler. Toggle Led.
void EXTI9_5_IRQHandler(void)
{	
	NVIC_DisableIRQ(EXTI9_5_IRQn);
	if (EXTI->PR & EXTI_Line8)	// If we have pending bit on Line 9
	{
		EXTI->PR = EXTI_Line8;	// Clear, writing it to 1

/*                alter++;
                if (alter == 1)
                {
                  ZB_SCHEDULE_ALARM(abcd, 0, ZB_TIME_ONE_SECOND);
                }
*/
                #if 0
                alter++;
                if (alter % 2 == 0)
                  
                  GPIO_ToggleBits(GPIOD,GPIO_Pin_15);
                else
                                    GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
                #endif
		ZB_SET_TRANS_INT();
                TRANS_CTX().interrupt_flag = 1;
	}
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void RF_INT_Configuration(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // LED
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	EXTI_InitTypeDef exti_struct;
	GPIO_InitTypeDef  GPIO_Led; 

	GPIO_Led.GPIO_Pin = GPIO_Pin_8;
 	GPIO_Led.GPIO_Mode  = GPIO_Mode_IN;
 	GPIO_Led.GPIO_OType = GPIO_OType_PP;
 	GPIO_Led.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;
 	GPIO_Led.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOD, &GPIO_Led);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource8);
	
	exti_struct.EXTI_Line = EXTI_Line8;
	exti_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_struct);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI9_5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0x0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0x0;
 	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

#endif
