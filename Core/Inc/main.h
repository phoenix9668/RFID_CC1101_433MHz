/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_iwdg.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define DEBUG
//#define UART_PROG
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define EEPROM_START_ADDR   0x08080000   /* Start @ of user eeprom area */
#define RXBUFFERSIZE  		14			// Size of Reception buffer
#define SEND_SLENGTH     	19
#define SEND_LLENGTH     	93
#define RECV_LENGTH   		18
#define STEP_NUM					36			// 20min per step,have 36 steps,equal 12 hours

#define	Time_Delay				1000		// cc1101 tx wait time
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI1_CS2_Pin LL_GPIO_PIN_5
#define SPI1_CS2_GPIO_Port GPIOA
#define ADXL_INT1_Pin LL_GPIO_PIN_7
#define ADXL_INT1_GPIO_Port GPIOA
#define ADXL_INT1_EXTI_IRQn EXTI4_15_IRQn
#define GDO2_Pin LL_GPIO_PIN_12
#define GDO2_GPIO_Port GPIOA
#define GDO2_EXTI_IRQn EXTI4_15_IRQn
#define SPI_NSS1_Pin LL_GPIO_PIN_15
#define SPI_NSS1_GPIO_Port GPIOA
#define GDO0_Pin LL_GPIO_PIN_6
#define GDO0_GPIO_Port GPIOB
#define LED1_Pin LL_GPIO_PIN_7
#define LED1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void System_Initial(void);
void Get_SerialNum(void);
void Show_Message(void);
uint8_t RF_RecvHandler(void);
void RF_SendPacket(uint8_t index);
void Set_DeviceInfo(void);
void DATAEEPROM_Program(uint32_t Address, uint32_t Data);
uint32_t DATAEEPROM_Read(uint32_t Address);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
