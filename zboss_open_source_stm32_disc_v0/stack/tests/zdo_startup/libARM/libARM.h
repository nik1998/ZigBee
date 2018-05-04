#ifndef __LEDS_AND_BUTTON_H_2
#define __LEDS_AND_BUTTON_H__
#define HSE_VALUE 8000000
#include "stm32f4xx.h"
#define HSE_VALUE  8000000
#define PLL_M 8
void ChangePulse(int delta);
void Timm();
void InitLeds(int Pins, int PinSource);
#endif

//Library has 3 functions:-
//1) void InitLeds(int Pins , int PinSource) -initialize LEDs
//-the first argument  Pins - pins of the LEDS
//-the second argument PinSource pin configuration PWM
//2)void TimTim() initialize first Timer (TIM1)
//3) void ChangePulse (int delta)
//alters the pulse (pulse width) on delta
//If the ending value is greater than 1600 or less than 0,
//the value does not change.
