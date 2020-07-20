/*
 * menu.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "defines.h"
#include "menu.h"
#include "pwm.h"
#include "uart_STLink.h"
#include "stdlib.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Variables ------------------------------------------------------------------------------------------------*/
TaskHandle_t xTask3_menu_handle;
TimerHandle_t xMenu_timer_handle;
enum car_modes act_mode = autonomous_mode;

/* Private variables ----------------------------------------------------------------------------------------*/
char user_msg[2];						/* race condition possible but not dangerous */
uint8_t sw_timer_ticks_counter = 1;		/* race condition possible but not dangerous */

/* Functions definitions ----------------------------------------------------------------------------------- */

/*
 * @brief	Configure PA5 as output for LED control
 * @param 	None
 * @retval	None
 *
 * */
void menu_led_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* enable GPIOA peripheral clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* set PA5 as output */
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
 * @brief	configures LED and PA5 output
 * @param	None
 * @retval	None
 *
 * */
void menu_button_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    // --- input configuration for PC13 button --------------------------------------
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // --- interrupt configuration for PC13 button ----------------------------------

    // enable APB2 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // System Configuration for SYSCFG_EXTI
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);

    // EXTI line configuration
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    // Configure NVIC (IRQ) for line 13 (IRQ position 40)
    NVIC_SetPriority(EXTI15_10_IRQn, 5);	// 5 chosen
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/*
 * @brief	sets car into default mode
 * @param	None
 * @retval	None
 *
 * */
void menu_default_mode(void)
{

    sw_timer_ticks_counter = 1;

    xSemaphoreTake(xMutex_act_mode, portMAX_DELAY);
    act_mode = manual_mode;
    xSemaphoreGive(xMutex_act_mode);

    CAN_Tx_enabled = FALSE;
    CAN_Rx_enabled = FALSE;

#if MENU_UART2_DEBUG
    itoa(act_mode, user_msg, 10);
    uart2_send(user_msg);
#endif
}

/*
 * @brief	changes act_mode to the next one
 * @param	None
 * @retval	None
 *
 * */
void menu_next(void)
{
    xSemaphoreTake(xMutex_act_mode, portMAX_DELAY);

    if(act_mode < (menu_size -1) )
    {
        act_mode+=1;
    }
    else
    {
        act_mode = 1;
    }

    xSemaphoreGive(xMutex_act_mode);
}

/*
 * @brief	set number of flashes to indicate act_mode (timer call back function)
 * 			1 flash = manual_mode
 * 			2 flashes = learning_mode
 * 			3 flashes = auto_mode
 * @param	xTimer: TimerHandle_t (required by FreeRTOS RM since timer call back)
 * @retval	None
 *
 * */
void menu_led_flashing(TimerHandle_t xTimer)
{

    xSemaphoreTake(xMutex_act_mode, portMAX_DELAY);

    if(sw_timer_ticks_counter <= act_mode*2)
    {

        if( sw_timer_ticks_counter % 2 )
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_5);
        }
        else
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        }

    }
    else if( sw_timer_ticks_counter >= (4 + act_mode*2) )
    {
        sw_timer_ticks_counter = 0;
    }

    sw_timer_ticks_counter += 1;

    xSemaphoreGive(xMutex_act_mode);
}
