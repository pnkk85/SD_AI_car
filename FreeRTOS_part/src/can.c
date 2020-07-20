/*
 * can.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

/* Includes ---------------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "can.h"
#include "defines.h"
#include "menu.h"
#include "pwm.h"
#include "uart_STLink.h"
#include "stdio.h"
#include "stdlib.h"

/* Variables --------------------------------------------------------------------------*/
CAN_InitTypeDef CAN1_InitStructure;
GPIO_InitTypeDef GPIOA_InitStructure;
CAN_FilterInitTypeDef CAN1_FilterInitStructure;
char user_msg3[20] = {0};
char buff_value[5];

/* Functions definitions ------------------------------------------------------------- */

/*
 *  @brief	Configures CAN1
 *  @param	None
 *  @retval	None
 * */
void can1_networking_config(void)
{
    /* CAN GPIOs configuration -----------------------------------*/

    /* enable GPIOA clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* connect to AF - PA11 to Rx and PA12 to Tx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);

    /* configure GPIO for PA11 and PA12 */
    GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOA_InitStructure);

    /* CAN configuration -----------------------------------------*/

    /* enable CAN1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    CAN_DeInit(CAN1);

    /* CAN1 configuration
     * from http://www.bittiming.can-wiki.info/:
     * 	bit rate: 250 (when prescaler = 4)
     * 	prescaler: 4 (prescaler value can be different in the code!)
     * 	no of tq: 16
     * 	Seg 1 (prop seg + phase seg1): 13
     * 	Seg 2: 2
     * 	sample point 87.5
     * 	Register CAN_BTR: 0x001c0000
     * */
    CAN1_InitStructure.CAN_ABOM = DISABLE;
    CAN1_InitStructure.CAN_AWUM = DISABLE;
    CAN1_InitStructure.CAN_BS1 = CAN_BS1_13tq;
    CAN1_InitStructure.CAN_BS2 = CAN_BS2_2tq;
    CAN1_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN1_InitStructure.CAN_NART = DISABLE;
    CAN1_InitStructure.CAN_Prescaler = 1;
    CAN1_InitStructure.CAN_RFLM = DISABLE;
    CAN1_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN1_InitStructure.CAN_TTCM = DISABLE;
    CAN1_InitStructure.CAN_TXFP = DISABLE;
    CAN_Init(CAN1, &CAN1_InitStructure);

    /* for debugging over UART only */
#if CAN1_UART2_DEBUG
    uart2_send("CAN1 InitStruct configuration completed");
#endif

    /* CAN filter configuration -------------------------------------*/
    /* CAN1 filter configuration */
    CAN1_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN1_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN1_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN1_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN1_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN1_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN1_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN1_FilterInitStructure.CAN_FilterNumber = 0;
    CAN1_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInit(&CAN1_FilterInitStructure);

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    /* for debugging over UART only */
#if CAN1_UART2_DEBUG
    uart2_send("CAN1 filter configuration completed");
#endif
}

/*
 * @brief	Initialize RxMessage
 * @param	RxMessage: pointer to CanRxMsg
 * @retval	None
 * */
void can1_RxMessage_init(CanRxMsg *RxMessage)
{
    uint8_t i = 0;

    RxMessage->StdId = 0x00;
    RxMessage->ExtId = 0x00;
    RxMessage->IDE = CAN_ID_STD;
    RxMessage->DLC = 0;
    RxMessage->FMI = 0;
    for (i = 0;i < 8;i++)
    {
        RxMessage->Data[i] = 0x00;
    }
}

/*
* @brief 	transmit value1 and value2 in one CAN message
* @param 	id: message id
* value1: 	1st part of the CAN message
* value2: 	2nd part of the CAN message
* @retval 	none
*
*/
void can1_transmit(uint32_t id, __IO uint32_t value1, __IO uint32_t value2)
{
    CanTxMsg TxMessage;

    TxMessage.DLC = 4;
    TxMessage.StdId = id;
    TxMessage.IDE = CAN_Id_Standard;
    TxMessage.RTR = CAN_RTR_Data;

    /* message to be sent */
    uint8_t msg[4];
    msg[0] =  (value1 >> 8) & 0xFF;
    msg[1] =  value1 & 0xFF;
    msg[2] =  (value2 >> 8) & 0xFF;
    msg[3] =  value2 & 0xFF;

    for(uint8_t i = 0; i < 4; i++)
    {
        TxMessage.Data[i] = msg[i];
    }

    /* TransmitMailbox not used, so can be deleted */
    CAN_Transmit(CAN1, &TxMessage);

    while(CAN_MessagePending(CAN1, CAN_FIFO0));

#if CAN1_UART2_DEBUG
    uart2_send("CAN1 message sent");
#endif
}

/*
* @brief 	receives message over CAN1.
* 			Sets global variables steer_duty_cycle and throttle_duty_cycle id id=0x03
* @param 	none
* @retval 	none
*
*/
void can1_receive(void)
{
#if CAN1_UART2_DEBUG
    uart2_send("CAN1_receive entered");
#endif

    CanRxMsg RxMessage;

    RxMessage.DLC = 0;
    RxMessage.Data[0] = 0x00;
    RxMessage.Data[1] = 0x00;
    RxMessage.Data[2] = 0x00;
    RxMessage.Data[3] = 0x00;

    RxMessage.IDE = CAN_Id_Standard;
    RxMessage.StdId = 0x00;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

    /* set global variables if proper id */
    if(RxMessage.StdId == 0x3)
    {
        steer_duty_cycle = ( RxMessage.Data[0] << 8 ) | ( RxMessage.Data[1] );
        throttle_duty_cycle = ( RxMessage.Data[2] << 8 ) | ( RxMessage.Data[3] );
    }

/* for debugging over UART only */
#if CAN1_RECEIVE_DEBUG
    itoa(steer_duty_cycle, buff_value, 10);
    sprintf(user_msg3, "CAN s: %s",  buff_value);
    uart2_send(user_msg3);
    itoa(throttle_duty_cycle, buff_value, 10);
    sprintf(user_msg3, "CAN t: %s",  buff_value);
    uart2_send(user_msg3);
#endif
}

/*
 * @brief	NVIC configuration for CAN1
 * @param	None
 * @retval	None
 * */
void NVIC_config(void)
{
    NVIC_SetPriority(CAN1_RX0_IRQn,7);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    /* for debugging over UART only */
#if CAN1_UART2_DEBUG
    uart2_send("CAN1 NVIC configuration completed");
#endif
}
