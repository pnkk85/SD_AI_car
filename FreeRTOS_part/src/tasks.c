/*
 * tasks.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

/* Includes ---------------------------------------------------------------- */
#include "can.h"
#include "defines.h"
#include "menu.h"
#include "pwm.h"
#include "uart_STLink.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
#include "stdio.h"

/* private variables for debugging --------------------------------------------------------*/
char msg_steer_duty[20] = {0};
char msg_throttle_duty[20] = {0};
char buff_steer_duty[5];
char buff_throttle_duty[5];

/* Tasks definitions ------------------------------------------------------- */

/*
 * @brief 	set PWM duty cycle for steering angle position
 *      	(PWM period is constant 51Hz)
 *
 * */
void vTask1_remote_ctrl_steer_to_car(void* pvParameters)
{
    for(;;)
    {
        /* wait for steering duty cycle (from ISR, pwm reading) */
        xTaskNotifyWait( 0, 0, NULL, portMAX_DELAY );

        /* set steering duty cycle */
        set_pwm_duty_of_PA0_output(steer_duty_cycle);

        /* for debugging over UART only */
#if TASKS_UART2_DEBUG
        itoa(steer_duty_cycle, buff_steer_duty, 10);
        sprintf(msg_steer_duty, "s %s",  buff_steer_duty);
        uart2_send(msg_steer_duty);
#endif
    }
}

/*
 * @brief 	set PWM duty cycle for throttle position
 *     		(PWM period is constant 51Hz)
 *
 * */
void vTask2_remote_ctrl_throttle_to_car_and_CAN(void* pvParameters)
{
    for(;;)
    {
        /* wait for throttle duty cycle (from ISR, pwm reading) */
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

        /* set throttle duty cycle */
        set_pwm_duty_of_PA5_output(throttle_duty_cycle);

        /* send steering and throttle duty over CAN if car in learning_mode */
        if( CAN_Tx_enabled ==  TRUE )
        {
            can1_transmit(0x01, steer_duty_cycle, throttle_duty_cycle);
        }

        /* for debugging over UART only */
#if TASKS_UART2_DEBUG
        itoa(throttle_duty_cycle, buff_throttle_duty, 10);
        sprintf(msg_throttle_duty, "t %s", buff_throttle_duty);
        uart2_send(msg_throttle_duty);
#endif
    }
}

/*
 * @brief sets the car in three different modes using user button
 *
 * */
void vTask3_menu(void* pvParameters)
{
    for(;;)
    {
        /* wait for throttle duty cycle (from ISR, EXTI) */
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        menu_next();
        vTaskDelay(pdMS_TO_TICKS(100));

        /* select proper car mode  */
        switch(act_mode)
        {
        case manual_mode:

            /* enable PWM read */
            TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);
            TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);

            /* disable CAN Rx */
            CAN_Rx_enabled = FALSE;

            /* disable CAN Tx */
            CAN_Tx_enabled = FALSE;
            break;

        case data_collecting_mode:

            /* enable PWM read */
            TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);
            TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);

            /* disable CAN read */
            CAN_Rx_enabled = FALSE;

            /* enable can send */
            CAN_Tx_enabled = TRUE;
            break;

        case autonomous_mode:

            /* disable PWM read */
            TIM_ITConfig(TIM4, TIM_IT_CC2, DISABLE);
            TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);
            car_standstill();

            /* enable CAN read - see CAN1_RX0_IRQHandler */
            CAN_Rx_enabled = TRUE;

            /* disable CAN Tx */
            CAN_Tx_enabled = FALSE;
            break;

        default:
            menu_default_mode();
        }
    }
}

/*
 * @brief	sets steering andle and throttle position
 *
 *
 * */
void vTask4_can_ctrl_to_car(void* pvParameters)
{
    for(;;)
    {
        /* wait for steering and throttle duty cycles (received over CAN) */
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

        /* set global variable steer_duty_cycle and throttle_duty_cycle */
        can1_receive();

        /* set steering angle and throttle position using global variables */
        set_pwm_duty_of_PA0_output(steer_duty_cycle);
        set_pwm_duty_of_PA5_output(throttle_duty_cycle);
    }
}
