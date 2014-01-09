#include "stm32f0xx_it.h"

void EXTI0_1_IRQHandler(void)
{
	__disable_irq();
	if (EXTI->PR & EXTI_PR_PR0)
	{
		GPIOC->ODR ^= GPIO_ODR_8;
		IRreception();
		EXTI->PR |= EXTI_PR_PR0;
	}
	__enable_irq();

}

void TIM6_DAC_IRQHandler(void)
{
  __disable_irq();
  if (TIM6->SR & TIM_SR_UIF)
  {
	  GPIOC->ODR ^= GPIO_ODR_8;
	  lost_control();
	TIM6->SR &=~ TIM_SR_UIF;
  }
  __enable_irq();
}
