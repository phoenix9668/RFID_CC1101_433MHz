/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "iwdg.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cc1101.h"
#include "adxl362.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t Device_Serial0;
uint32_t Device_Serial1;
uint32_t Device_Serial2;
uint8_t addr_eeprom;
uint16_t sync_eeprom;
uint32_t rfid_eeprom;
uint8_t rx_index = 0x0;
uint8_t	Chip_Addr = 0;
uint8_t	RSSI = 0;

uint8_t step_stage;
uint16_t timedelay;
uint8_t battery_low;
uint16_t step[STEP_NUM];

uint8_t SendBuffer[SEND_LLENGTH] = {0};
uint8_t RecvBuffer[RECV_LENGTH] = {0};

extern uint8_t RxBuffer[RXBUFFERSIZE];
extern __IO FlagStatus CommandState;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  System_Initial();
  Show_Message();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/* Refresh IWDG down-counter to default value */
		LL_IWDG_ReloadCounter(IWDG);
		rx_index = RF_RecvHandler();
		if(rx_index != 0x0)
		{
			RF_SendPacket(rx_index);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(CommandState == SET)
		{
			Set_DeviceInfo();
			CommandState = RESET;
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief System Initial
  * @retval None
*/
void System_Initial(void)
{
	uint8_t i;
	/*##-1- initial all peripheral ##*/
	#ifdef DEBUG
		Activate_USART2_RXIT();
	#endif
	#ifndef DEBUG
		MX_USART2_UART_DeInit();
	#endif
	Activate_SPI();
	Get_SerialNum();
	ADXL362_Init();
	/*##-2- initial CC1101 peripheral,configure it's address and sync code ##*/
	addr_eeprom = (uint8_t)(0xff & DATAEEPROM_Read(EEPROM_START_ADDR)>>16);
	sync_eeprom = (uint16_t)(0xffff & DATAEEPROM_Read(EEPROM_START_ADDR));
	rfid_eeprom	= DATAEEPROM_Read(EEPROM_START_ADDR+4);
	#ifdef DEBUG
		printf("addr_eeprom = %x\n",addr_eeprom);
		printf("sync_eeprom = %x\n",sync_eeprom);
		printf("rfid_eeprom = %x\n",(unsigned int)rfid_eeprom);
	#endif
	RFIDInitial(addr_eeprom, sync_eeprom, IDLE_MODE);
	
	for(i = 0;i < STEP_NUM; i++)
	{
		step[i] = (uint16_t)(0x0000FFFF & DATAEEPROM_Read(EEPROM_START_ADDR+4*i+8));
		DATAEEPROM_Program(EEPROM_START_ADDR+4*i+8, 0x0);
	}

	step_stage = (uint8_t)(0x000000FF & DATAEEPROM_Read(EEPROM_START_ADDR+152));
	DATAEEPROM_Program(EEPROM_START_ADDR+152, 0x0);
	battery_low = (uint8_t)(0x000000FF & DATAEEPROM_Read(EEPROM_START_ADDR+160));
	DATAEEPROM_Program(EEPROM_START_ADDR+160, 0x0);
	timedelay = (uint16_t)(0x0000FFFF & DATAEEPROM_Read(EEPROM_START_ADDR+168));
	DATAEEPROM_Program(EEPROM_START_ADDR+168, Time_Delay);
}

/**
  * @brief Get Device Unique Code
  * @retval None
*/
void Get_SerialNum(void)
{
  Device_Serial0 = *(uint32_t*)(0x1FF80050);
  Device_Serial1 = *(uint32_t*)(0x1FF80054);
  Device_Serial2 = *(uint32_t*)(0x1FF80064);
}

/**
  * @brief Show Message
  * @retval None
*/
void Show_Message(void)
{
	unsigned int  ReadValueTemp;
	#ifdef DEBUG
		printf("\r\n CC1101 chip transfer program \n");
		printf(" using USART2,configuration:%d 8-N-1 \n",9600);
		printf(" when in transfer mode,the data must not exceed 60 bytes!!\r\n");
	#endif
	#ifdef DEBUG
		printf("Device_Serial0 : %x\n",(unsigned int)Device_Serial0);
		printf("Device_Serial1 : %x\n",(unsigned int)Device_Serial1);
		printf("Device_Serial2 : %x\n",(unsigned int)Device_Serial2);
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_DEVID_AD);     	//Analog Devices device ID, 0xAD
	if(ReadValueTemp == 0xAD)
	{
		LED_GREEN_OFF();
		HAL_Delay(500);
	}
	#ifdef DEBUG
		printf("Analog Devices device ID: %x\n",ReadValueTemp);	 	//send via UART
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_DEVID_MST);    	//Analog Devices MEMS device ID, 0x1D
	if(ReadValueTemp == 0x1D)
	{
		LED_GREEN_ON();
		HAL_Delay(500);
	}
	#ifdef DEBUG
		printf("Analog Devices MEMS device ID: %x\n",ReadValueTemp);	//send via UART
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_PARTID);       	//part ID, 0xF2
	if(ReadValueTemp == 0xF2)
	{
		LED_GREEN_OFF();
		HAL_Delay(500);
	}
	#ifdef DEBUG
		printf("Part ID: %x\n",ReadValueTemp);										//send via UART
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_REVID);       	//version ID, 0x02
	if(ReadValueTemp == 0x02)
	{
		LED_GREEN_ON();
	}
	#ifdef DEBUG
		printf("Version ID: %x\n",ReadValueTemp);									//send via UART
	#endif
}

/**
  * @brief Receive RF Single
  * @retval index
*/
uint8_t RF_RecvHandler(void)
{
	uint8_t length=0;
	int16_t rssi_dBm;
	uint32_t timeout;
	uint8_t i;

	if(CC1101_IRQ_READ() == 0)
		{
			#ifdef DEBUG
				printf("interrupt occur\n");
			#endif
			timeout = Delay_TimeOut;
			while (CC1101_IRQ_READ() == 0 && timeout != 0)
			{
				timeout--;
			}
			for (i=0; i<RECV_LENGTH; i++)   { RecvBuffer[i] = 0; } // clear array
			length = CC1101RecPacket(RecvBuffer, &Chip_Addr, &RSSI);

			#ifdef DEBUG
				rssi_dBm = CC1101CalcRSSI_dBm(RSSI);
				printf("RSSI = %ddBm, length = %d, address = %d\n",rssi_dBm,length,Chip_Addr);
				for(i=0; i<RECV_LENGTH; i++)
				{
					printf("%x ",RecvBuffer[i]);
				}
			#endif
			if(length == 0)
				{
					#ifdef DEBUG
						printf("receive error or Address Filtering fail\n");
					#endif
					return 0x01;
				}
			else
				{
					if(RecvBuffer[0] == 0xAB && RecvBuffer[1] == 0xCD)
					{
						if(RecvBuffer[4] == 0xC0 && RecvBuffer[5] == 0xC0)
						{return 0x04;}
						else if(RecvBuffer[4] == 0xC2 && RecvBuffer[5] == 0xC2)
						{return 0x05;}
						else if(RecvBuffer[4] == 0xC3 && RecvBuffer[5] == 0xC3)
						{return 0x06;}
						else if(RecvBuffer[4] == 0xC4 && RecvBuffer[5] == 0xC4)
						{return 0x07;}
						else if(RecvBuffer[4] == 0xC5 && RecvBuffer[5] == 0xC5)
						{return 0x08;}
						else if(RecvBuffer[4] == 0xC6 && RecvBuffer[5] == 0xC6)
						{return 0x09;}
						else
						{
							#ifdef DEBUG
								printf("receive function order error\r\n");
							#endif
							return 0x03;}
					}
					else
					{
						#ifdef DEBUG
							printf("receive package beginning error\r\n");
						#endif
						return 0x02;}
				}
		}
	else	{return 0x00;}
}
/**
  * @brief Send RF Single
  * @retval None
*/
void RF_SendPacket(uint8_t index)
{
	uint32_t data;
	uint32_t dataeeprom;// 从eeprom中读出的数
	uint8_t i;
	
//	LED_GREEN_OFF();
	
	switch(index)
	{
		case 0x01://receive error or Address Filtering fail
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0x01;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			SendBuffer[10] = 0xE0;
			SendBuffer[18] = RSSI;
		
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
		case 0x02://receive package beginning error
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0x02;
			SendBuffer[5] = 0x02;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			SendBuffer[10] = 0xE1;
			SendBuffer[18] = RSSI;
		
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
		case 0x03://receive function order error
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0x03;
			SendBuffer[5] = 0x03;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			SendBuffer[10] = 0xE2;
			SendBuffer[18] = RSSI;
		
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
			
		case 0x04:
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0xD0;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = (uint8_t)(0x000000FF & Device_Serial2>>24);
			SendBuffer[7] = (uint8_t)(0x000000FF & Device_Serial2>>16);
			SendBuffer[8] = (uint8_t)(0x000000FF & Device_Serial2>>8);
			SendBuffer[9] = (uint8_t)(0x000000FF & Device_Serial2);
			SendBuffer[10] = (uint8_t)(0x000000FF & Device_Serial1>>24);
			SendBuffer[11] = (uint8_t)(0x000000FF & Device_Serial1>>16);
			SendBuffer[12] = (uint8_t)(0x000000FF & Device_Serial1>>8);
			SendBuffer[13] = (uint8_t)(0x000000FF & Device_Serial1);
			SendBuffer[14] = (uint8_t)(0x000000FF & Device_Serial0>>24);
			SendBuffer[15] = (uint8_t)(0x000000FF & Device_Serial0>>16);
			SendBuffer[16] = (uint8_t)(0x000000FF & Device_Serial0>>8);
			SendBuffer[17] = (uint8_t)(0x000000FF & Device_Serial0);
		
			for(i = 0;i < STEP_NUM; i++)
			{
				SendBuffer[18+i*2] = (uint8_t)(0x00FF & step[i]>>8);
				SendBuffer[19+i*2] = (uint8_t)(0x00FF & step[i]);
			}
			SendBuffer[90] = step_stage;
			SendBuffer[91] = battery_low;
			SendBuffer[92] = RSSI;

//			for(i=0; i<SEND_LLENGTH; i++)
//			{
//				printf("%x ",SendBuffer[i]);
//			}
//			printf("\r\n");
			
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_LLENGTH, ADDRESS_CHECK);
			break;
			
		case 0x05:
			battery_low = 0x00;
			DATAEEPROM_Program(EEPROM_START_ADDR+160, battery_low);
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];		
			SendBuffer[4] = 0xD2;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			battery_low = (uint8_t)(0x000000FF & DATAEEPROM_Read(EEPROM_START_ADDR+160));
			SendBuffer[10] = battery_low;
			SendBuffer[18] = RSSI;
		
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
			
		case 0x06:
			ADXL362_ReInit(RecvBuffer[10], RecvBuffer[11], RecvBuffer[13], RecvBuffer[14], RecvBuffer[15], RecvBuffer[16], RecvBuffer[17]);
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0xD3;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			SendBuffer[10] = ADXL362RegisterRead(XL362_THRESH_ACT_H);
			SendBuffer[11] = ADXL362RegisterRead(XL362_THRESH_ACT_L);
			SendBuffer[12] = ADXL362RegisterRead(XL362_TIME_ACT);
			SendBuffer[13] = ADXL362RegisterRead(XL362_THRESH_INACT_H);
			SendBuffer[14] = ADXL362RegisterRead(XL362_THRESH_INACT_L);
			SendBuffer[15] = ADXL362RegisterRead(XL362_TIME_INACT_H);
			SendBuffer[16] = ADXL362RegisterRead(XL362_TIME_INACT_L);
			SendBuffer[17] = ADXL362RegisterRead(XL362_FILTER_CTL);
			SendBuffer[18] = RSSI;
		
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;

		case 0x07:
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0xD4;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = (uint8_t)(0x000000FF & Device_Serial2>>24);
			SendBuffer[7] = (uint8_t)(0x000000FF & Device_Serial2>>16);
			SendBuffer[8] = (uint8_t)(0x000000FF & Device_Serial2>>8);
			SendBuffer[9] = (uint8_t)(0x000000FF & Device_Serial2);
			SendBuffer[10] = (uint8_t)(0x000000FF & Device_Serial1>>24);
			SendBuffer[11] = (uint8_t)(0x000000FF & Device_Serial1>>16);
			SendBuffer[12] = (uint8_t)(0x000000FF & Device_Serial1>>8);
			SendBuffer[13] = (uint8_t)(0x000000FF & Device_Serial1);
			SendBuffer[14] = (uint8_t)(0x000000FF & Device_Serial0>>24);
			SendBuffer[15] = (uint8_t)(0x000000FF & Device_Serial0>>16);
			SendBuffer[16] = (uint8_t)(0x000000FF & Device_Serial0>>8);
			SendBuffer[17] = (uint8_t)(0x000000FF & Device_Serial0);

			for(i = 0;i < STEP_NUM; i++)
			{
				SendBuffer[18+i*2] = (uint8_t)(0x00FF & step[i]>>8);
				SendBuffer[19+i*2] = (uint8_t)(0x00FF & step[i]);
			}
			SendBuffer[90] = step_stage;
			SendBuffer[91] = battery_low;
			SendBuffer[92] = RSSI;
			
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_LLENGTH, ADDRESS_CHECK);
			break;
			
		case 0x08:
			data = ((uint32_t)(0xFF000000 & RecvBuffer[10]<<24)+(uint32_t)(0x00FF0000 & RecvBuffer[11]<<16)+(uint32_t)(0x0000FF00 & RecvBuffer[12]<<8)+(uint32_t)(0x000000FF & RecvBuffer[13]));
			DATAEEPROM_Program(EEPROM_START_ADDR, data);
			data = ((uint32_t)(0xFF000000 & RecvBuffer[14]<<24)+(uint32_t)(0x00FF0000 & RecvBuffer[15]<<16)+(uint32_t)(0x0000FF00 & RecvBuffer[16]<<8)+(uint32_t)(0x000000FF & RecvBuffer[17]));
			DATAEEPROM_Program(EEPROM_START_ADDR+4, data);
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0xD5;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			dataeeprom = DATAEEPROM_Read(EEPROM_START_ADDR);
			SendBuffer[10] = (uint8_t)(0x000000FF & dataeeprom>>24);
			SendBuffer[11] = (uint8_t)(0x000000FF & dataeeprom>>16);
			SendBuffer[12] = (uint8_t)(0x000000FF & dataeeprom>>8);
			SendBuffer[13] = (uint8_t)(0x000000FF & dataeeprom);
			dataeeprom = DATAEEPROM_Read(EEPROM_START_ADDR+4);
			SendBuffer[14] = (uint8_t)(0x000000FF & dataeeprom>>24);
			SendBuffer[15] = (uint8_t)(0x000000FF & dataeeprom>>16);
			SendBuffer[16] = (uint8_t)(0x000000FF & dataeeprom>>8);
			SendBuffer[17] = (uint8_t)(0x000000FF & dataeeprom);
			SendBuffer[18] = RSSI;
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
			
		case 0x09:
			for(i = 0;i < STEP_NUM; i++)
			{
				step[i] = 0;
				DATAEEPROM_Program(EEPROM_START_ADDR+4*i+8, 0x0);
			}
			step_stage = 0;
			timedelay = Time_Delay;
			DATAEEPROM_Program(EEPROM_START_ADDR+152, 0x0);
			DATAEEPROM_Program(EEPROM_START_ADDR+168, Time_Delay);
			SendBuffer[0] = 0xAB;
			SendBuffer[1] = 0xCD;
			SendBuffer[2] = RecvBuffer[2];
			SendBuffer[3] = RecvBuffer[3];
			SendBuffer[4] = 0xD6;
			SendBuffer[5] = 0x01;
			SendBuffer[6] = RecvBuffer[6];
			SendBuffer[7] = RecvBuffer[7];
			SendBuffer[8] = RecvBuffer[8];
			SendBuffer[9] = RecvBuffer[9];
			SendBuffer[18] = RSSI;
			HAL_Delay(Time_Delay);
			CC1101SendPacket(SendBuffer, SEND_SLENGTH, ADDRESS_CHECK);
			break;
		default : break;
	}

	for(i=0; i<SEND_LLENGTH; i++) // clear array
	{SendBuffer[i] = 0;}

	CC1101SetIdle();
	CC1101WORInit();
	CC1101SetWORMode();
//	LED_GREEN_ON();
}

/**
  * @brief Set_DeviceInfo
  * @retval None
*/
void Set_DeviceInfo(void)
{
  /* Set transmission flag: trasfer complete*/
	uint32_t data;
	uint8_t uart_addr_eeprom;// 从eeprom中读出的数
	uint16_t uart_sync_eeprom;
	uint32_t uart_rfid_eeprom;

	/*##-1- Check UART receive data whether is ‘ABCD’ begin or not ###########################*/
	if(RxBuffer[0] == 0x41 && RxBuffer[1] == 0x42 && RxBuffer[2] == 0x43 && RxBuffer[3] == 0x44)//输入‘ABCD’
	{
		data = ((uint32_t)(0xFF000000 & RxBuffer[4]<<24)+(uint32_t)(0x00FF0000 & RxBuffer[5]<<16)+(uint32_t)(0x0000FF00 & RxBuffer[6]<<8)+(uint32_t)(0x000000FF & RxBuffer[7]));
		DATAEEPROM_Program(EEPROM_START_ADDR, data);
		data = ((uint32_t)(0xFF000000 & RxBuffer[8]<<24)+(uint32_t)(0x00FF0000 & RxBuffer[9]<<16)+(uint32_t)(0x0000FF00 & RxBuffer[10]<<8)+(uint32_t)(0x000000FF & RxBuffer[11]));
		DATAEEPROM_Program(EEPROM_START_ADDR+4, data);
		uart_addr_eeprom = (uint8_t)(0xff & DATAEEPROM_Read(EEPROM_START_ADDR)>>16);
		uart_sync_eeprom = (uint16_t)(0xffff & DATAEEPROM_Read(EEPROM_START_ADDR));
		uart_rfid_eeprom	= DATAEEPROM_Read(EEPROM_START_ADDR+4);
		#ifdef DEBUG
			printf("eeprom program end\n");
			printf("addr_eeprom = %x\n",uart_addr_eeprom);
			printf("sync_eeprom = %x\n",uart_sync_eeprom);
			printf("rfid_eeprom = %x\n",uart_rfid_eeprom);
		#endif
	}
}
			
/**
  * @brief DATAEEPROM WRITE
  * @retval None
*/
void DATAEEPROM_Program(uint32_t Address, uint32_t Data)
{
	/* Unlocks the data memory and FLASH_PECR register access *************/
	if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK)
	{
    Error_Handler();
	}
	/* Clear FLASH error pending bits */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_SIZERR |
							FLASH_FLAG_OPTVERR | FLASH_FLAG_RDERR |
								FLASH_FLAG_FWWERR | FLASH_FLAG_NOTZEROERR);
	/*Erase a word in data memory *************/
	if (HAL_FLASHEx_DATAEEPROM_Erase(Address) != HAL_OK)
	{
		Error_Handler();
	}
	/*Enable DATA EEPROM fixed Time programming (2*Tprog) *************/
	HAL_FLASHEx_DATAEEPROM_EnableFixedTimeProgram();
	/* Program word at a specified address *************/
	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, Address, Data) != HAL_OK)
	{
		Error_Handler();
	}
	/*Disables DATA EEPROM fixed Time programming (2*Tprog) *************/
	HAL_FLASHEx_DATAEEPROM_DisableFixedTimeProgram();

	/* Locks the Data memory and FLASH_PECR register access. (recommended
     to protect the DATA_EEPROM against possible unwanted operation) *********/
	HAL_FLASHEx_DATAEEPROM_Lock();

}

/**
  * @brief DATAEEPROM READ
  * @retval DATA
*/
uint32_t DATAEEPROM_Read(uint32_t Address)
{
	return *(__IO uint32_t*)Address;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
