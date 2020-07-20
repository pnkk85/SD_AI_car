/*
 * pwm.h
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#ifndef PWM_H_
#define PWM_H_

#include "stm32f4xx.h"

/* Defines -------------------------------------------------------------------------------------*/
#define STEER_DUTY_MAX		2809
#define STEER_DUTY_MIN		1780
#define THROTTLE_DUTY_MAX	3435
#define THROTTLE_DUTY_MIN	1510

/* Global variables ----------------------------------------------------------------------------*/
__IO uint32_t steer_duty_cycle;	 // do xMutex_steer_duty on this
__IO uint32_t throttle_duty_cycle; // do xMutex_throttle_duty on this
__IO uint32_t DutyCycle;
__IO uint32_t Frequency;

/* Function declarations -----------------------------------------------------------------------*/
void pwm_read_PB7_config(void);
void pwm_read_PB5_config(void);
void pwm_output_PA0_config(void);
void pwm_output_PB8_config(void);
void car_standstill(void);
void set_pwm_duty_of_PA0_output(__IO uint32_t ccr_value);
void set_pwm_duty_of_PA5_output(__IO uint32_t ccr_value);

#endif /* PWM_H_ */
