#include "ir_nec.h"

uint8_t IRSTAT;//Переменная для хранения флагов состояние при приёме данных с пду.
uint8_t ir_data_address_low;//Переменная для хранения адреса уст-ва работающего по протоколу NEC
uint8_t ir_data_address_hight;
uint8_t ir_data_cmd;//Переменная для хранения команды уст-ва работающего по протоколу NEC
uint8_t ir_data_cmd_invert;
uint8_t bit = 0;//Счётчик бит.
uint16_t count_time = 0;

void IR_NEC_Init(void)
{
	    IRSTAT |=(1<<receiving_flag);//Установка флага первого импульса.
		/*Настройка таймера.*/
		RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	    TIM6->PSC = 3072-1;//получим частоту в 15625 Гц ( это выбранно для того чтобы интервалы можно было портировать на 8 бит таймеры другого М.К.)
	    TIM6->CR1 |= TIM_CR1_URS;//Прерывание только по переполнению таймера.Прерывание означает отсуствиие сигнала ПДУ.
	    TIM6->CR1 |= TIM_CR1_CEN;//вкл. таймер.
	    TIM6->DIER|= TIM_DIER_UIE;//Вкл. прерывание.
	    TIM6->ARR = 1718;//А больше на и не надо, так максимальный интервал это 110 мс = 1718 тиков.
	    NVIC_SetPriority(TIM6_DAC_IRQn,10);
	    NVIC_EnableIRQ(TIM6_DAC_IRQn);

		/*Настройка внешнего прерывания*/
		RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
		GPIOA->MODER &=~ GPIO_MODER_MODER0;//Вход
		GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;//подтяжка к земле
		EXTI->IMR |= EXTI_IMR_MR0;//line 0 masked
		EXTI->FTSR|= EXTI_FTSR_TR0;//Падение сигнала.

		NVIC_SetPriority(EXTI0_1_IRQn,9);
		NVIC_EnableIRQ(EXTI0_1_IRQn);
		__enable_irq();
}

uint8_t IR_NEC_cmd(void)
{
    uint8_t temp = 0;
    uint8_t temp_invert = 0;
    if(IRSTAT & (1<<data_read))//Если чтение данных разрешенно.
    {
    	IRSTAT &=~(1<<data_read);//Запрещаем чтение данных
    	temp_invert =~ ir_data_cmd_invert;//инвертируем содержимого инвертированного поля команды протокола NEC и сохраняем это значение.
        temp |= ir_data_cmd;//Сохраняем значение команды
        ir_data_cmd = 0;
        ir_data_cmd_invert = 0;
        if(temp == temp_invert) //Сравниваем 2 значение, они должны совпасть.
        {
            ir_data_cmd = 0;//Обнуляем значение команды
            IRSTAT &=~(1<<data_read);//Запрещаем чтение данных
        }
        else
        {
            temp = 0;
        }

    }
    else
    {
        temp = 0;
    }
    return temp; //Возвращаем команду.
}

void IRreception(void)
{
	count_time = TIM6->CNT;

	if(IRSTAT & (1<<receiving_flag))//Первый импульс.
	{
		count_time = 0;
		IRSTAT &=~(1<<receiving_flag);// очищаем флаг первого импульса.
		TIM6->CNT = 0x00000;//Обнуляем таймер. Начинаем отсчёт стартового импульса.
	}

	if (bit>31)//Если принято все 32 бита
	{
		bit=0;
	    IRSTAT &=~(1<<data_flag);//Запретить приём данныйх.
		IRSTAT |= (1<<data_read);//Разрешаем чтение данных.
	}
	if (count_time>start_pulse_interval_down && count_time<start_pulse_interval_up)
	{
		//Значит пришёл интервал от стартового импульса. 9мс+4.5мс
		IRSTAT |=(1<<data_flag);//Устанавливаем флаг готовности приёма данных после стартового импульса.
		TIM6->CNT=0; //Обнуляем таймер.
	}

	if (IRSTAT & (1<<data_flag) && (!(IRSTAT & (1<<data_read))))//Если флаг данных установлен и если предидущие данные не прочитаны.
	{
		if((count_time>log_level_1_down) && (count_time<log_level_1_up))//Проверяем интервал
		{
			//"1"
			if(bit>=0 && bit<=7)//если счётчик количества бит меньше 8,значит принимаем адрес первых 8 бит.
			{
				ir_data_address_low |=(1<<bit);//Записываем адрес.
			}
			if(bit>=8 && bit<=15)//принимаем следущий 8 бит адреса..
            {
                ir_data_address_hight |=(1<<(bit-8));
            }

			if(bit>=16 && bit<=23)//Если счётчик количества бит больше 16(адрес+его инверсия) и меньше 24(инверсии команды).
			{
				ir_data_cmd |=(1<<(bit-16));//Записываем команду.
			}
            if (bit>=24 && bit<=31)//Принимаем инверсию команды.
            {
                ir_data_cmd_invert |= (1<<(bit-24));
            }
			bit++;
		}
		if((count_time>log_level_0_down) && (count_time<log_level_0_up))//Проверяем интервал
		{

			//"0"
			if(bit>= 0 && bit<=7)//если счётчик количества бит меньше 8,значит принимаем адрес, первые 8 бит.
			{
				ir_data_address_low &=~(1<<bit);//Записываем адрес.
			}
			if(bit>=8 && bit<=15)//принимаем следущий 8 бит адреса.
            {
                ir_data_address_hight &=~(1<<(bit-8));
            }

			if((bit>=16) && (bit<=23))//Если счётчик количества бит больше 16(адрес+его инверсия) и меньше 24(инверсии команды).
			{
				ir_data_cmd &=~(1<<(bit-16));//Записываем команду.
			}
             if (bit>=24 && bit<=31)//Принимаем инверсию команды.
            {
                ir_data_cmd_invert&=~ (1<<(bit-24));
            }
			bit++;
		}

	}
	TIM6->CNT=0;
}

void lost_control (void)
{
    IRSTAT |=(1<<receiving_flag);//Устанавливаем флаг первого импульса.
	IRSTAT &=~(1<<data_flag);//Сбрасываем флаг приёма данных
	if (bit>=32)//Если счетчик битов дотикал до 32, тоесть вся посылка.
    {
        //А мы находимся в прерывание по переполнению таймера
        IRSTAT |= (1<<data_read);//Разрешаем чтение данных.
    }
	bit = 0;//Очищаем счётчик приёма бит.
}
