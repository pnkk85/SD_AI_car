/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------ */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "uart_STLink.h"
#include "pwm.h"
#include "menu.h"
#include "can.h"
#include "tasks.h"
#include "defines.h"

/* Function prototypes --------------------------------------------------------------------- */
void hardware_config(void);

/* main function body ----------------------------------------------------------------------- */
int main(void)
{
    /* configure hardware */
    hardware_config();

    /* create mutexes */
    xMutex_act_mode = xSemaphoreCreateMutex();

    /* check if mutexes created successfully */
    if( xMutex_act_mode != NULL)
    {
        xSemaphoreGive(xMutex_act_mode);

        /* start default car mode */
        menu_default_mode();

        /* create RTOS tasks */
        xTaskCreate( vTask1_remote_ctrl_steer_to_car, "steerToCar", 500, NULL, 2, &xTask1_remote_ctrl_steer_to_car_handle );
        xTaskCreate( vTask2_remote_ctrl_throttle_to_car_and_CAN, "throToCar", 500, NULL, 1, &xTask2_remote_ctrl_throttle_to_car_handle );
        xTaskCreate( vTask3_menu, "menu", 500, NULL, 3, &xTask3_menu_handle);
        xTaskCreate( vTask4_can_ctrl_to_car, "CANtoCar", 500, NULL, 2, &xTask4_can_ctrl_to_car_handle);

            /* create RTOS timers */
            xMenu_timer_handle = xTimerCreate( "LedTImer", pdMS_TO_TICKS(250), pdTRUE, ( void* ) 0, menu_led_flashing );
            if( xMenu_timer_handle == NULL)
            {
                uart2_send("software timer not created");
            }
            else
            {
                /* start the software timer */
                xTimerStart(xMenu_timer_handle, portMAX_DELAY);
            }

            /* start RTOS scheduler */
            vTaskStartScheduler();
    }
    else
    {
        uart2_send("mutexes creation failed!");
    }

    for(;;);
}

/* Function definitions -----------------------------------------------------------------------*/

/*
 * @brief	peripheral configuration
 * @param	None
 * @retval	None
 *
 * */
void hardware_config(void)
{
    /* default clock configuration */
    RCC_DeInit();

    /* Update SystemCoreClock variable according to Clock Register Values */
    SystemCoreClockUpdate();

    /* uart configuration */
    uart2_config();

    /* PWM read - TIM4 and PB7 configuration */
    pwm_read_PB7_config();

    /* PWM read - TIM3 and PB5 */
    pwm_read_PB5_config();

    /* PWM output - TIM5 on PA0 */
    pwm_output_PA0_config();

    /* PWM output - TIM2 on PB8 */
    pwm_output_PB8_config();

    /* set car into standstill */
    car_standstill();

    /* configure LED for menu indication */
    menu_led_config();

    /* configure button for menu */
    menu_button_config();

    /* configure CAN */
    can1_networking_config();
    NVIC_config();
}
