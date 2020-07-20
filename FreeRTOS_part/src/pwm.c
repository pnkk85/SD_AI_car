/*
 * pwm.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

/* includes -----------------------------------------------------------------*/
#include "can.h"
#include "defines.h"
#include "menu.h"
#include "pwm.h"
#include "uart_STLink.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"

/* variables ----------------------------------------------------------------*/
__IO uint32_t steer_duty_cycle = 0;	 // do xMutex_steer_duty on this
__IO uint32_t throttle_duty_cycle = 0; // do xMutex_throttle_duty on this
__IO uint32_t DutyCycle = 0;
__IO uint32_t Frequency = 0;

/* Functions definitions ----------------------------------------------------*/

/*
 * @brief 	configure TIM4 and PB7 (used for reading PWM duty cycle of steering angle signal)
 * @param 	None
 * @retval 	None
 * */
void pwm_read_PB7_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    /* enable TIM4 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* TIM4 chennel2 configuration PB7 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Connect PB7 pin to AF2 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    /* Enable the TIM4 global Interrupt */
    NVIC_SetPriority(TIM4_IRQn,6);
    NVIC_EnableIRQ(TIM4_IRQn);

    /* TIM4 time base configuration */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 9;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* ----------------------------------------------------------------
     *
     *  RC frequency = TIM4CLK / (TIM4_CCR2*(TIM_Prescaler + 1)) in Hz
     *  RC duty cycle = (TIM4_CCR1 / (TIM4_CCR2*(TIM_Prescaler + 1)) ) * 100 in %
     *
     * ---------------------------------------------------------------*/

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_PWMIConfig(TIM4, &TIM_ICInitStructure);

    /* Select the TIM4 Input Trigger: TI2FP2 */
    TIM_SelectInputTrigger(TIM4, TIM_TS_TI2FP2);

    /* Select the slave Mode: Reset Mode */
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM4,TIM_MasterSlaveMode_Enable);

    /* TIM enable counter */
    TIM_Cmd(TIM4, ENABLE);

    /* Enable the CC2 Interrupt Request */
    TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);

#if PWMREAD_UART2_DEBUG
    uart2_send("TIM4 configuration completed");
#endif
}

/*
 * @brief configure TIM3 and PB5 (used for reading PWM duty cycle of throttle position signal)
 * @param None
 * @retval None
 * */
void pwm_read_PB5_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    /* enable TIM3 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* GPIOB clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* TIM3 chennel2 configuration PB7 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Connect PB5 pin to AF2 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

    /* Enable the TIM3 global Interrupt */
    NVIC_SetPriority(TIM3_IRQn,6);
    NVIC_EnableIRQ(TIM3_IRQn);

    /* TIM4 time base configuration */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 9;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* ----------------------------------------------------------------
     *
     *  RC frequency = TIM3CLK / (TIM3_CCR2*(TIM_Prescaler + 1)) in Hz
     *  RC duty cycle = (TIM3_CCR1 / (TIM3_CCR2*(TIM_Prescaler + 1)) ) * 100 in %
     *
     * ---------------------------------------------------------------*/

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);

    /* Select the TIM3 Input Trigger: TI2FP2 */
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);

    /* Select the slave Mode: Reset Mode */
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM3,TIM_MasterSlaveMode_Enable);

    /* TIM enable counter */
    TIM_Cmd(TIM3, ENABLE);

    /* Enable the CC2 Interrupt Request */
    TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);

#if PWMREAD_UART2_DEBUG
    uart2_send("TIM3 configuration completed");
#endif
}

/**
* @brief	Configure PWM mode using TIM5 and PA0 for steer control
* @param	None
* @retval	None
*/
void pwm_output_PA0_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* TIM5 IO configuration ----------------------------------------------- */
    /* Enable GPIOA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* Connect TIM5 (PA0) to AF2 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);

    /* Configure TIM5 CH1 pin (PA0) as alternate function */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* TIM5 configuration ------------------------------------------------------
     *
     *  THIS IS AN EXAMPLE!
     *  VALUES USED IN CODE CAN DIFFER FROM THE EXAMPLE!
     *
     * TIM5 configured to generate PWM signal on CH1 with
     * a frequency of 51Hz and 7,752% duty cycle
     *
     * TIM5 input clock (TIM5CLK) is equal to PCLK1 if PCLK1 prescaler is 1
     *
     * Assumed is 	HSI = 16Mhz
     *				PCLK1 = 16MHz
     *				TIM1CLK = 16MHz
     *
     * TIM5 signal frequency = TIM5CLK/((TIM_Prescaler + 1) * TIM_Period) = 51Hz
     *
     * 				TIM_Period = TIM5CLK/(Frequency * (TIM_Prescaler + 1))
     *
     * Setting the TIM_Prescaller to 9,
     * 						-> TIM_Period = TIM5CLK/51Hz*10 = 31372
     *
     * TIM5 signal duty cycle	= (CCR / ARR) = 7,752%
     *							= (TIM_Pulse / TIM_Period) = 7,752%
     *					-> TIM_Pulse = 0,07752 * TIM_Period = 2431
     *
     *
     * ----------------------------------------------------------------------  */

    /* Enable TIM5 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 31372;
    TIM_TimeBaseStructure.TIM_Prescaler = 9;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    /* Configure CH1 PWM1 Mode */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 3000;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM5, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM5, ENABLE);

    /* Enable TIM5 counter */
    TIM_Cmd(TIM5, ENABLE);
}

/**
* @brief	Configure PWM mode using TIM2 and PB8 for throttle control
* @param	None
* @retval	None
*/
void pwm_output_PB8_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* TIM2 IO configuration -------------------------------------- */
    /* Enable GPIOB clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* Connect TIM2 (PB8) to AF1 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM2);

    /* Configure TIM2 CH1 pin (PB8) as alternate function */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* TIM5 configuration ------------------------------------------------------
     *
     *  THIS IS AN EXAMPLE!
     *  VALUES USED IN CODE CAN DIFFER FROM THE EXAMPLE!
     *
     * TIM5 configured to generate PWM signal on CH1 with
     * a frequency of 51Hz and 7,752% duty cycle
     *
     * TIM5 input clock (TIM5CLK) is equal to PCLK1 if PCLK1 prescaler is 1
     *
     * Assumed is 	HSI = 16Mhz
     *				PCLK1 = 16MHz
     *				TIM1CLK = 16MHz
     *
     * TIM5 signal frequency = TIM5CLK/((TIM_Prescaler + 1) * TIM_Period) = 51Hz
     *
     * 				TIM_Period = TIM5CLK/(Frequency * (TIM_Prescaler + 1))
     *
     * Setting the TIM_Prescaller to 9,
     * 						-> TIM_Period = TIM5CLK/51Hz*10 = 31372
     *
     * TIM5 signal duty cycle	= (CCR / ARR) = 7,752%
     *							= (TIM_Pulse / TIM_Period) = 7,752%
     *					-> TIM_Pulse = 0,07752 * TIM_Period = 2431
     *
     *
     * ----------------------------------------------------------------------  */

    /* Enable TIM2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 31372;
    TIM_TimeBaseStructure.TIM_Prescaler = 9;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* Configure CH1 PWM1 Mode */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 3000;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* Enable TIM2 counter */
    TIM_Cmd(TIM2, ENABLE);
}

/*
 * @brief 	set car into idle (used after controller restart)
 * @param 	none
 * @retval 	none
 * */
void car_standstill(void)
{
    TIM2->CCR1 = 2400; /* throttle position */
    TIM5->CCR1 = 2400; /* steering angle */
}

/*
 * @brief	sets CCR value of TIM5 CCR1 register
 * @param	ccr_value: CCR1 register value (PA0 output for steer)
 * @retval	None
 *
 * */
void set_pwm_duty_of_PA0_output(__IO uint32_t ccr_value)
{
    /* set max and min values */
    if ( ccr_value <= STEER_DUTY_MIN )
    {
        ccr_value = STEER_DUTY_MIN;

    }
    else if ( ccr_value >= STEER_DUTY_MAX )
    {
        ccr_value = STEER_DUTY_MAX;
    }

    TIM5->CCR1 = ccr_value;
}

/*
 * @brief	sets CCR value of TIM2 CCR1 register
 * @param	ccr_value: CCR1 register value (PA5 output for throttle)
 * @retval	None
 *
 * */
void set_pwm_duty_of_PA5_output(__IO uint32_t ccr_value)
{
    /* set max and min values */
    if( ccr_value <= THROTTLE_DUTY_MIN )
    {
        ccr_value = THROTTLE_DUTY_MIN;

    }
    else if( ccr_value >= THROTTLE_DUTY_MAX )
    {

        ccr_value = THROTTLE_DUTY_MAX;
    }

    TIM2->CCR1 = ccr_value;
}
