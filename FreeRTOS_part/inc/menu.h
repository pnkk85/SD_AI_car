/*
 * menu.h
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#ifndef MENU_H_
#define MENU_H_

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Variables -------------------------------------------------------------------------------------------------- */
enum car_modes{manual_mode = 1, data_collecting_mode, autonomous_mode, menu_size};
enum car_modes act_mode;			// MUTEX tbd!!!
SemaphoreHandle_t xMutex_act_mode;
uint32_t CAN_Tx_enabled;
uint32_t CAN_Rx_enabled;
TimerHandle_t xMenu_timer_handle;

/* Functions prototypes ----------------------------------------------------------------------------------------*/
void menu_led_config(void);
void menu_button_config(void);
void menu_button_pressed(void);
void menu_led_flashing(TimerHandle_t xTimer);
void menu_default_mode(void);
void menu_next(void);

#endif /* MENU_H_ */
