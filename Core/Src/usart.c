/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
uint8_t RxBuffer[RXBUFFERSIZE];
__IO uint8_t RxCounter = 0;
__IO FlagStatus CommandState = RESET;

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END 0 */

/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, 2);
  NVIC_EnableIRQ(USART2_IRQn);

  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);

}

/* USER CODE BEGIN 1 */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int fputc(int ch,FILE *f)
{
	USART2->TDR = ch;
	while(!(USART2->ISR&USART_ISR_TXE))
	{;}
	return ch;
}

void Activate_USART2_RXIT(void)
{
	/* Polling USART initialisation */
  while((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2))))
  { 
  }

  /* Enable RXNE and Error interrupts */
  LL_USART_EnableIT_RXNE(USART2);
  LL_USART_EnableIT_ERROR(USART2);
}

/**
  * @brief  Send Txt information message on USART Tx line (to PC Com port).
  * @param  None
  * @retval None
  */
void PrintInfo(uint8_t *String, uint32_t Size)
{
  uint32_t index = 0;
  uint8_t *pchar = String;
  
  /* Send characters one per one, until last char to be sent */
  for (index = 0; index < Size; index++)
  {
    /* Wait for TXE flag to be raised */
    while (!LL_USART_IsActiveFlag_TXE(USART2))
    {
    }

    /* Write character in Transmit Data register.
       TXE flag is cleared by writing data in TDR register */
    LL_USART_TransmitData8(USART2, *pchar++);
  }

  /* Wait for TC flag to be raised for last char */
  while (!LL_USART_IsActiveFlag_TC(USART2))
  {
  }
}

/**
  * @brief  Function called from USART IRQ Handler when RXNE flag is set
  *         Function is in charge of reading character received on USART RX line.
  * @param  None
  * @retval None
  */
void USART_CharReception_Callback(void)
{
  /* Read Received character. RXNE flag is cleared by reading of RDR register */
  RxBuffer[RxCounter++] = LL_USART_ReceiveData8(USART2);
  /* Echo received character on TX */
  LL_USART_TransmitData8(USART2, RxBuffer[RxCounter-1]);
	/* Check if received value is corresponding to specific one : S or s */
  if (RxBuffer[RxCounter-2] == 0x0D && RxBuffer[RxCounter-1] == 0x0A)
  {
    /* Clear RxCounter : Expected character has been received */
    RxCounter = 0x00;
		CommandState = SET;
  }
}

/**
  * @brief  Function called in case of error detected in USART IT Handler
  * @param  None
  * @retval None
  */
void Error_Callback(void)
{
  __IO uint32_t isr_reg;

  /* Disable USARTx_IRQn */
  NVIC_DisableIRQ(USART2_IRQn);
  
  /* Error handling example :
    - Read USART ISR register to identify flag that leads to IT raising
    - Perform corresponding error handling treatment according to flag
  */
  isr_reg = LL_USART_ReadReg(USART2, ISR);
  if (isr_reg & LL_USART_ISR_NE)
  {
    /* case Noise Error flag is raised : ... */
			LL_USART_ClearFlag_NE(USART2);
			LL_USART_DeInit(USART2);
			MX_USART2_UART_Init();
			LL_USART_EnableIT_RXNE(USART2);
			LL_USART_EnableIT_ERROR(USART2);
			RxCounter = 0x00;
  }
	else if (isr_reg & LL_USART_ISR_FE)
  {
    /* case Noise Error flag is raised : ... */
			LL_USART_ClearFlag_FE(USART2);
			LL_USART_DeInit(USART2);
			MX_USART2_UART_Init();
			LL_USART_EnableIT_RXNE(USART2);
			LL_USART_EnableIT_ERROR(USART2);
			RxCounter = 0x00;
  }
	else if (isr_reg & LL_USART_ISR_ORE)
  {
    /* case Noise Error flag is raised : ... */
			LL_USART_ClearFlag_ORE(USART2);
			LL_USART_DeInit(USART2);
			MX_USART2_UART_Init();
			LL_USART_EnableIT_RXNE(USART2);
			LL_USART_EnableIT_ERROR(USART2);
			RxCounter = 0x00;
  }
	else if (isr_reg & LL_USART_ISR_PE)
  {
    /* case Noise Error flag is raised : ... */
			LL_USART_ClearFlag_PE(USART2);
			LL_USART_DeInit(USART2);
			MX_USART2_UART_Init();
			LL_USART_EnableIT_RXNE(USART2);
			LL_USART_EnableIT_ERROR(USART2);
			RxCounter = 0x00;
  }
  else
  {
    /* Unexpected IT source : Set LED to Blinking mode to indicate error occurs */
			LL_USART_DeInit(USART2);
			MX_USART2_UART_Init();
			LL_USART_EnableIT_RXNE(USART2);
			LL_USART_EnableIT_ERROR(USART2);
			RxCounter = 0x00;
  }
}

void MX_USART2_UART_DeInit(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* Peripheral clock disable */
  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
	
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**LPUART1 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	LL_USART_Disable(USART2);
  LL_USART_DeInit(USART2);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
