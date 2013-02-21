/*
*Функции реализующие задержку в микросекундах и милисекундах.
* Значения SystemCoreClock берётся из файла system_stm32f0xx.c*/
 
#ifndef __delay_tim6_h
#define __delay_tim6t_h

#include "stm32f0xx.h"

void InitDelayTIM6(void);
void TIM6delay_ms(uint16_t value);
void TIM6delay_us(uint16_t value);

#endif
