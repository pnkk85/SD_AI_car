/*
 * uart_STLink.c
 *
 *  Created on: 14.06.2020
 *      Author: piotr
 */

#include "string.h"
#include "uart_STLink.h"
#include "stm32f4xx.h"

/* InitTypeDef declaration ------------------------------------------*/
USART_InitTypeDef USART2_InitStructure;
GPIO_InitTypeDef GPIOA_InitStructure;

/* 	@brief	configure uart2 over ST Link (PA2 and PA3, for STM32F446RE Nucleo)
 *	@param	None
 *	@retval None
 */
void uart2_config(void)
{
    /* enable APB1 peripheral clock for UART2 and GPIOA */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* Configure AF of PA2 to UART2 Tx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

    /* Configure AF of PA3 to UART2 Rx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    /* GPIOA Configuration */
    GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIOA_InitStructure);

    /* UART2 configuration */
    USART2_InitStructure.USART_BaudRate = 115200;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART2_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;

    USART_Init(USART2, &USART2_InitStructure);

    /* Enable UART2 */
    USART_Cmd(USART2, ENABLE);

    uart2_send(" \n ******************************************** \n ");
    uart2_send("UART2 configuration completed");
}

/* send message over UART2 */
void uart2_send(char* msg)
{
    for(uint32_t i = 0; i < strlen(msg); i++)
    {
        USART_SendData(USART2, msg[i]); //send one byte

        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    }

    /* go to the next line */
    char* next_line = "\r\n";
    for(uint32_t i = 0; i < 2; i++)
    {
        USART_SendData(USART2, next_line[i]); /* send one byte */

        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
    }
}
