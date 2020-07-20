/*
 * can.h
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#ifndef CAN_H_
#define CAN_H_

#include "stm32f4xx.h"

/* Functions declarations --------------------------------------------------------------------------------------*/
void can1_networking_config(void);
void can1_transmit(uint32_t id, __IO uint32_t value1, __IO uint32_t value2);
void can1_receive(void);
void can1_RxMessage_init(CanRxMsg *RxMessage);
void NVIC_config(void);

#endif /* CAN_H_ */
