/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for
  *                      the gpio
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define LED_GREEN_OFF()				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED_GREEN_ON()				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED_GREEN_TOG()				HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)

#define CC1101_CSN_LOW()			HAL_GPIO_WritePin(SPI_NSS1_GPIO_Port, SPI_NSS1_Pin, GPIO_PIN_RESET)
#define CC1101_CSN_HIGH()			HAL_GPIO_WritePin(SPI_NSS1_GPIO_Port, SPI_NSS1_Pin, GPIO_PIN_SET)
#define ADXL362_CSN_LOW()			HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_RESET)
#define ADXL362_CSN_HIGH()		HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET)

#define CC1101_IRQ_READ()   	HAL_GPIO_ReadPin(GDO0_GPIO_Port, GDO0_Pin)
#define CC1101_GDO2_READ()   	HAL_GPIO_ReadPin(GDO2_GPIO_Port, GDO2_Pin)
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
