/*
 * it.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#include "can.h"
#include "defines.h"
#include "menu.h"
#include "pwm.h"
#include "tasks.h"
#include "stm32f4xx.h"

/* Variables -------------------------------------------------------------------------- */
CanRxMsg RxMessage;
BaseType_t xHigherPriorityTaskWoken_CAN1 = pdFALSE;
BaseType_t xHigherPriorityTaskWoken_TIM4 = pdFALSE;
BaseType_t xHigherPriorityTaskWoken_TIM3 = pdFALSE;
BaseType_t xHigherPriorityTaskWoken_EXTI15_10 = pdFALSE;

/* Interrupt handlers ------------------------------------------------------------------*/

/*
 * @brief 	notifies proper task if CAN message received and car in autonomous mode
 *
 * */
void CAN1_RX0_IRQHandler(void)
{
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    CAN_FIFORelease(CAN1, CAN_FIFO0);

    if(CAN_Rx_enabled ==  TRUE)
    {
        xTaskNotifyFromISR( xTask4_can_ctrl_to_car_handle, 0, eNoAction, &xHigherPriorityTaskWoken_CAN1 );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken_CAN1 );
    }
}

/*
 * @brief	reads duty cycle of steer signal (given in clk ticks) and set global variable steer_duty_cycle
 * 			(constant period of 51Hz)
 *
 * */
void TIM4_IRQHandler(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        /* Clear TIM4 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);

        /* Get the Input Capture value */
        steer_duty_cycle = TIM_GetCapture1(TIM4);
    }

    /* Notify proper task */
    xTaskNotifyFromISR( xTask1_remote_ctrl_steer_to_car_handle, 0, eNoAction, &xHigherPriorityTaskWoken_TIM4 );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken_TIM4 );
}

/*
 * @brief	reads duty cycle of throttle signal (given in clk ticks) and set global variable throttle_duty_cycle
 * 			(constant period of 51Hz)
 *
 * */
void TIM3_IRQHandler(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        /* Clear TIM3 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

        /* Get the throttle pulse width as input capture value */
        throttle_duty_cycle = TIM_GetCapture1(TIM3);
    }

    /* Notify proper task */
    xTaskNotifyFromISR(xTask2_remote_ctrl_throttle_to_car_handle, 0, eNoAction, &xHigherPriorityTaskWoken_TIM3);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken_TIM3 );
}

/*
 * @brief	user button handling
 *
 *
 * */
void EXTI15_10_IRQHandler(void)
{
    /* clear the interrupt pending bit of the EXTI line (13) */
    EXTI_ClearITPendingBit(EXTI_Line13);

    /* Notify proper task */
    xTaskNotifyFromISR( xTask3_menu_handle, 0, eNoAction, &xHigherPriorityTaskWoken_EXTI15_10 );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken_EXTI15_10 );
}
