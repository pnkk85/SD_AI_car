/*
 * tasks.h
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#ifndef TASKS_H_
#define TASKS_H_

/* freeRTOS related declarations ---------------------------------------------------------------*/
void vTask1_remote_ctrl_steer_to_car(void* pvParameters);
void vTask2_remote_ctrl_throttle_to_car_and_CAN(void* pvParameters);
void vTask3_menu(void* pvParameters);
void vTask4_can_ctrl_to_car(void* pvParameters);
TaskHandle_t xTask1_remote_ctrl_steer_to_car_handle;
TaskHandle_t xTask2_remote_ctrl_throttle_to_car_handle;
TaskHandle_t xTask3_menu_handle;
TaskHandle_t xTask4_can_ctrl_to_car_handle;

#endif /* TASKS_H_ */
