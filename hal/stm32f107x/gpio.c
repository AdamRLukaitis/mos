/**
 * @file gpio.c
 *
 * @brief impliment the gpio module for the stm32107
 *
 * @author OT
 *
 * @date Jan 2013
 *
 */


#include <stm32f10x_conf.h>
#include "hal.h"
#include "gpio_hw.h"


// look up which APB2 perph clk is associated with this pin
static uint32_t pin_to_rcc_periph(gpio_pin_t *pin)
{
	uint32_t ret;

	// map pin to bits to turn on port clock
	switch ((uint32_t)pin->port)
	{
		case (uint32_t)GPIOA:
			ret = RCC_APB2Periph_GPIOA;
			break;
		case (uint32_t)GPIOB:
			ret = RCC_APB2Periph_GPIOB;
			break;
		case (uint32_t)GPIOC:
			ret = RCC_APB2Periph_GPIOC;
			break;
		case (uint32_t)GPIOD:
			ret = RCC_APB2Periph_GPIOD;
			break;
		case (uint32_t)GPIOE:
			ret = RCC_APB2Periph_GPIOE;
			break;
		case (uint32_t)GPIOF:
			ret = RCC_APB2Periph_GPIOF;
			break;
		case (uint32_t)GPIOG:
			ret = RCC_APB2Periph_GPIOG;
			break;
		default:
			ret = RCC_APB2Periph_GPIOA;
			break;
	}

	// enable alt function clock for all pins for now
	ret |= RCC_APB2Periph_AFIO;

	return ret;
}


// convert gpio pin (st GPIO_TypeDef def) to st exti GPIO_PortSourceGPIOx
static uint8_t gpio_pin_to_port_source(gpio_pin_t *pin)
{
	switch ((uint32_t)pin->port)
	{
		case (uint32_t)GPIOA:
			return GPIO_PortSourceGPIOA;
		case (uint32_t)GPIOB:
			return GPIO_PortSourceGPIOB;
		case (uint32_t)GPIOC:
			return GPIO_PortSourceGPIOC;
		case (uint32_t)GPIOD:
			return GPIO_PortSourceGPIOD;
		case (uint32_t)GPIOE:
			return GPIO_PortSourceGPIOE;
		case (uint32_t)GPIOF:
			return GPIO_PortSourceGPIOF;
		case (uint32_t)GPIOG:
			return GPIO_PortSourceGPIOG;
		default:
			///@todo error
			return GPIO_PortSourceGPIOA;
	}
}


// convert gpio pin (st GPIO pin def) to st exti GPIO_PinSourceGPIOx
static uint8_t gpio_pin_to_pin_source(gpio_pin_t *pin)
{
	switch ((uint32_t)pin->cfg.GPIO_Pin)
	{
		case (uint32_t)GPIO_Pin_0:
			return GPIO_PinSource0;
		case (uint32_t)GPIO_Pin_1:
			return GPIO_PinSource1;
		case (uint32_t)GPIO_Pin_2:
			return GPIO_PinSource2;
		case (uint32_t)GPIO_Pin_3:
			return GPIO_PinSource3;
		case (uint32_t)GPIO_Pin_4:
			return GPIO_PinSource4;
		case (uint32_t)GPIO_Pin_5:
			return GPIO_PinSource5;
		case (uint32_t)GPIO_Pin_6:
			return GPIO_PinSource6;
		case (uint32_t)GPIO_Pin_7:
			return GPIO_PinSource7;
		case (uint32_t)GPIO_Pin_8:
			return GPIO_PinSource8;
		case (uint32_t)GPIO_Pin_9:
			return GPIO_PinSource9;
		case (uint32_t)GPIO_Pin_10:
			return GPIO_PinSource10;
		case (uint32_t)GPIO_Pin_11:
			return GPIO_PinSource11;
		case (uint32_t)GPIO_Pin_12:
			return GPIO_PinSource12;
		case (uint32_t)GPIO_Pin_13:
			return GPIO_PinSource13;
		case (uint32_t)GPIO_Pin_14:
			return GPIO_PinSource14;
		case (uint32_t)GPIO_Pin_15:
			return GPIO_PinSource15;
		default:
			///@todo error
			return GPIO_PinSource0;
	}
}


// map gpio pin (st GPIO pin def) to exti line
static uint32_t gpio_pin_to_exti_line(gpio_pin_t *pin)
{
	switch ((uint32_t)pin->cfg.GPIO_Pin)
	{
		case (uint32_t)GPIO_Pin_0:
			return EXTI_Line0;
		case (uint32_t)GPIO_Pin_1:
			return EXTI_Line1;
		case (uint32_t)GPIO_Pin_2:
			return EXTI_Line2;
		case (uint32_t)GPIO_Pin_3:
			return EXTI_Line3;
		case (uint32_t)GPIO_Pin_4:
			return EXTI_Line4;
		case (uint32_t)GPIO_Pin_5:
			return EXTI_Line5;
		case (uint32_t)GPIO_Pin_6:
			return EXTI_Line6;
		case (uint32_t)GPIO_Pin_7:
			return EXTI_Line7;
		case (uint32_t)GPIO_Pin_8:
			return EXTI_Line8;
		case (uint32_t)GPIO_Pin_9:
			return EXTI_Line9;
		case (uint32_t)GPIO_Pin_10:
			return EXTI_Line10;
		case (uint32_t)GPIO_Pin_11:
			return EXTI_Line11;
		case (uint32_t)GPIO_Pin_12:
			return EXTI_Line12;
		case (uint32_t)GPIO_Pin_13:
			return EXTI_Line13;
		case (uint32_t)GPIO_Pin_14:
			return EXTI_Line14;
		case (uint32_t)GPIO_Pin_15:
			return EXTI_Line15;
		default:
			///@todo error
			return EXTI_Line0;
	}
}


// map gpio pin (st GPIO pin def) to exti irq
static uint32_t gpio_pin_to_exti_irq(gpio_pin_t *pin)
{
	switch ((uint32_t)pin->cfg.GPIO_Pin)
	{
		case (uint32_t)GPIO_Pin_0:
			return EXTI0_IRQn;
		case (uint32_t)GPIO_Pin_1:
			return EXTI1_IRQn;
		case (uint32_t)GPIO_Pin_2:
			return EXTI2_IRQn;
		case (uint32_t)GPIO_Pin_3:
			return EXTI3_IRQn;
		case (uint32_t)GPIO_Pin_4:
			return EXTI4_IRQn;
		case (uint32_t)GPIO_Pin_5:
		case (uint32_t)GPIO_Pin_6:
		case (uint32_t)GPIO_Pin_7:
		case (uint32_t)GPIO_Pin_8:
		case (uint32_t)GPIO_Pin_9:
			return EXTI9_5_IRQn;
		case (uint32_t)GPIO_Pin_10:
		case (uint32_t)GPIO_Pin_11:
		case (uint32_t)GPIO_Pin_12:
		case (uint32_t)GPIO_Pin_13:
		case (uint32_t)GPIO_Pin_14:
		case (uint32_t)GPIO_Pin_15:
			return EXTI15_10_IRQn;
		default:
			///@todo error
			return EXTI0_IRQn;
	}
}


// save the pin into a list so it can be recalled in the isr's
static gpio_pin_t *gpio_pin_irq_list[16] = {0,};
static void save_pin_for_irq(gpio_pin_t *pin)
{
	uint8_t index = 0;

	// save the pin registered for this irq so it can be retrieved when it occurs
	switch ((uint32_t)pin->cfg.GPIO_Pin)
	{
		case (uint32_t)GPIO_Pin_0:
			index = 0;
			break;
		case (uint32_t)GPIO_Pin_1:
			index = 1;
			break;
		case (uint32_t)GPIO_Pin_2:
			index = 2;
			break;
		case (uint32_t)GPIO_Pin_3:
			index = 3;
			break;
		case (uint32_t)GPIO_Pin_4:
			index = 4;
			break;
		case (uint32_t)GPIO_Pin_5:
			index = 5;
			break;
		case (uint32_t)GPIO_Pin_6:
			index = 6;
			break;
		case (uint32_t)GPIO_Pin_7:
			index = 7;
			break;
		case (uint32_t)GPIO_Pin_8:
			index = 8;
			break;
		case (uint32_t)GPIO_Pin_9:
			index = 9;
			break;
		case (uint32_t)GPIO_Pin_10:
			index = 10;
			break;
		case (uint32_t)GPIO_Pin_11:
			index = 11;
			break;
		case (uint32_t)GPIO_Pin_12:
			index = 12;
			break;
		case (uint32_t)GPIO_Pin_13:
			index = 13;
			break;
		case (uint32_t)GPIO_Pin_14:
			index = 14;
			break;
		case (uint32_t)GPIO_Pin_15:
			index = 15;
			break;
		default:
			///@todo error
			break;
	}
	gpio_pin_irq_list[index] = pin;
}


// handle all the edge events in this one place
void exti_isr(uint8_t line_start, uint8_t line_end)
{
	uint8_t l;
	const uint32_t lines[] = {EXTI_Line0, EXTI_Line1, EXTI_Line2, EXTI_Line3, EXTI_Line4, EXTI_Line5, EXTI_Line6, EXTI_Line7, EXTI_Line8, EXTI_Line9, EXTI_Line10, EXTI_Line11, EXTI_Line12, EXTI_Line13, EXTI_Line14, EXTI_Line15};

	// test all the lines that could have generated this irq if one is set, run the appropriate edge callback
	for (l = line_start; l <= line_end; l++)
	{
		if (EXTI_GetITStatus(lines[l]) != RESET)
		{
			gpio_pin_t *pin = gpio_pin_irq_list[l];

			// clear the irq for this line
			EXTI_ClearITPendingBit(lines[l]);

			// if pin is valid, what type of edge was generated
			if (pin != NULL)
			{
				if ((pin->port->IDR & pin->cfg.GPIO_Pin) == 0)
				{
					// pin is low so it must be a falling edge
					if (pin->falling_cb != NULL)
						pin->falling_cb(pin, pin->falling_cb_param);
				}
				else
				{
					// pin is hi so it must be a rising edge
					if (pin->rising_cb!= NULL)
						pin->rising_cb(pin, pin->rising_cb_param);
				}
			}
			else
			{
				///@todo error
			}
		}
	}
}


void EXTI0_IRQHandler(void)
{
	exti_isr(0, 0);
}


void EXTI1_IRQHandler(void)
{
	exti_isr(1, 1);
}


void EXTI2_IRQHandler(void)
{
	exti_isr(2, 2);
}


void EXTI3_IRQHandler(void)
{
	exti_isr(3, 3);
}


void EXTI4_IRQHandler(void)
{
	exti_isr(4, 4);
}


void EXTI9_5_IRQHandler(void)
{
	exti_isr(5, 9);
}


void EXTI15_10_IRQHandler(void)
{
	exti_isr(10, 15);
}


// map the pin to exti mode and setup the irq etc
static void gpio_set_edge_event(gpio_pin_t *pin, EXTITrigger_TypeDef trig, gpio_edge_event cb, void *param)
{
	///@todo only run the parts here that are needed (ie if already inited)
	///@todo if NULL cb deinit

	EXTI_InitTypeDef exti_init;
	NVIC_InitTypeDef nvic_init;
	uint32_t exti_line = gpio_pin_to_exti_line(pin);

	// set pin to alternate function EXTI mode
	GPIO_EXTILineConfig(gpio_pin_to_port_source(pin), gpio_pin_to_pin_source(pin));

	// Configure EXTIx line to trigger irq
	if ((trig == EXTI_Trigger_Falling) && (EXTI->RTSR & exti_line))
		trig = EXTI_Trigger_Rising_Falling;  // was already configured as rising so set to both
	else if ((trig == EXTI_Trigger_Rising) && (EXTI->FTSR & exti_line))
		trig = EXTI_Trigger_Rising_Falling;  // was already configured as falling so set to both
	exti_init.EXTI_Line = exti_line;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = trig;
	exti_init.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init);

	// Enable and set EXTI9_5 Interrupt to the lowest priority
	save_pin_for_irq(pin);  ///@todo this may need to warn if another pin is registered with this irq in case the caller forgets the pins share irq's
	nvic_init.NVIC_IRQChannel = gpio_pin_to_exti_irq(pin);
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0x0F;
	nvic_init.NVIC_IRQChannelSubPriority = 0x0F;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);
}




void gpio_init_pin(gpio_pin_t *pin)
{
	// module should ignore null pins
	if (pin == NULL)
		return;

	// all gpio pins on the stm32f107x are connected to APB2 so this will be ok
	///@todo should we check if it is enabled before re-enabling ?
	RCC_APB2PeriphClockCmd(pin_to_rcc_periph(pin), ENABLE);
	RCC_APB2PeriphResetCmd(pin_to_rcc_periph(pin), DISABLE);

	// setup the pin configurations
	GPIO_Init(pin->port, &pin->cfg);
}


///@note this function makes no attempt deal with the fact that pins 5-9 and 10-15 share irq's it is up to the caller to no overwrite these by mistake when designing the firmware
void gpio_set_rising_edge_event(gpio_pin_t *pin, gpio_edge_event cb, void *param)
{
	pin->rising_cb = cb;
	pin->rising_cb_param = param;
	gpio_set_edge_event(pin, EXTI_Trigger_Rising, cb, param);
}


///@note this function makes no attempt deal with the fact that pins 5-9 and 10-15 share irq's it is up to the caller to no overwrite these by mistake when designing the firmware
void gpio_set_falling_edge_event(gpio_pin_t *pin, gpio_edge_event cb, void *param)
{
	pin->falling_cb = cb;
	pin->falling_cb_param = param;
	gpio_set_edge_event(pin, EXTI_Trigger_Falling, cb, param);
}


void gpio_set_pin(gpio_pin_t *pin, bool state)
{
	// module should ignore null pins
	if (pin == NULL)
		return;

	// check we are in a valid mode to set the pin output
	switch (pin->cfg.GPIO_Mode)
	{
		case GPIO_Mode_AIN:
		case GPIO_Mode_IN_FLOATING:
		case GPIO_Mode_IPD:
		case GPIO_Mode_IPU:
		case GPIO_Mode_AF_OD:
		case GPIO_Mode_AF_PP:
			///@todo set a system error
			return;

		case GPIO_Mode_Out_OD:
		case GPIO_Mode_Out_PP:
			// this are the only valid modes in which we can set the pin output
			break;
	}

	// change the pin output
	if (state)
		// set hi
		pin->port->BSRR = pin->cfg.GPIO_Pin;
	else
		// set lo
		pin->port->BRR = pin->cfg.GPIO_Pin;
}

// this toggles at about 58KHz max, see gpio utest
void gpio_toggle_pin(gpio_pin_t *pin)
{
	// module should ignore null pins
	if (pin == NULL)
		return;

	// check we are in a valid mode to set the pin output
	switch (pin->cfg.GPIO_Mode)
	{
		case GPIO_Mode_AIN:
		case GPIO_Mode_IN_FLOATING:
		case GPIO_Mode_IPD:
		case GPIO_Mode_IPU:
		case GPIO_Mode_AF_OD:
		case GPIO_Mode_AF_PP:
			///@todo set a system error
			return;

		case GPIO_Mode_Out_OD:
		case GPIO_Mode_Out_PP:
			// this are the only valid modes in which we can set the pin output
			break;
	}

	// toggle the pin
	pin->port->ODR ^= pin->cfg.GPIO_Pin;
}


bool gpio_get_pin(gpio_pin_t *pin)
{
	// module should ignore null pins
	if (pin == NULL)
		return false;

	// check we are in a valid mode to set the pin output
	switch (pin->cfg.GPIO_Mode)
	{
		case GPIO_Mode_AIN:
		case GPIO_Mode_AF_OD:
		case GPIO_Mode_AF_PP:
			///@todo set a system error
			return false;

		case GPIO_Mode_Out_OD:
		case GPIO_Mode_Out_PP:
		case GPIO_Mode_IN_FLOATING:
		case GPIO_Mode_IPD:
		case GPIO_Mode_IPU:
			// this are the only valid modes in which we can read the pin value
			break;
	}

	// return the state of the pin
	return ((pin->port->IDR & pin->cfg.GPIO_Pin) != 0);
}

