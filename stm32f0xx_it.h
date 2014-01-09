#ifndef __STM32F0XX_IT_H
#define __STM32F0XX_IT_H

#include "stm32f0xx.h"
#include "ir_nec.h"
/*В прерывание обрабатываются спадающие импульсы с TSOP*/
void EXTI0_1_IRQHandler(void);

/*В прерывание обрабатывается потеря импульсов c TSOP.*/
void TIM6_DAC_IRQHandler(void);

#endif
