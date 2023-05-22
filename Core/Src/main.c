/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "ipcc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "pll_maxim.h"
#include "command_processor.h"

float read_voltage();
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TIMEOUT 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define RX_BFR_SIZE 64

char RxBuffer[RX_BFR_SIZE];
command_t global_command = AMS_NONE;
uint32_t global_args[MAX_ARG_LEN];


int _write(int file, char *ptr, int len) {
  int DataIdx;
  for (DataIdx = 0; DataIdx < len; DataIdx++) {
    ITM_SendChar(*ptr++);
  }
  return len;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if (global_command==AMS_NONE || global_command==AMS_STOP){
			HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
			DisableRFOutput();//safeguard in case something bad happened in the code
		}else{
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
		}
	}
}


void HAL_Uart_RxCpltCallback(UART_HandleTypeDef *huart);



char strbuf[100];
void do_commands(){
	// check if there is command and if is, then process it
	if (global_command == AMS_SWEEP){
		uint32_t from = global_args[0];
		uint32_t to = global_args[1];
		uint32_t step = global_args[2];
		uint32_t pwr = global_args[3];
		EnableRFOutput();
		setRFA_PWR(pwr-1); // -1 because pwr is defined as 1,2,3,4 but MAX2870 expects 0,1,2,3
		uint32_t i;
		for (i=from; i<=to; i+=step){
			  //printf("setting %d MHz\n", i);
			  if (global_command==AMS_STOP)
				  return;
			  set_requested_frequency(i);
			  HAL_Delay(1);
			  uint16_t raw = read_raw();
			  sprintf(strbuf, "{%lu, %u}", i, raw);
			  HAL_UART_Transmit(&huart1, (uint8_t*)&strbuf, strlen(strbuf), UART_TIMEOUT);
		 }
		// send last frequency
		if (i!=to){
			set_requested_frequency(to);
			//HAL_Delay(1);
			uint16_t raw = read_raw();
			sprintf(strbuf, "{%lu, %u}", to, raw);
			HAL_UART_Transmit(&huart1, (uint8_t*)&strbuf, strlen(strbuf), UART_TIMEOUT);
		}
		HAL_UART_Transmit(&huart1, (uint8_t*)";", 2, UART_TIMEOUT);
		printf("Done\n");
		global_command = AMS_NONE;
		DisableRFOutput();

	}else if (global_command == AMS_SWEEP_CONT){
		uint32_t from = global_args[0];
		uint32_t to = global_args[1];
		uint32_t step = global_args[2];
		uint32_t pwr = global_args[3];
		EnableRFOutput();
		setRFA_PWR(pwr-1); // -1 because pwr is defined as 1,2,3,4 but MAX2870 expects 0,1,2,3
		uint32_t i;
		while (global_command!=AMS_STOP){
			for (i=from; i<=to; i+=step){
				  //printf("setting %d MHz\n", i);
				  if (global_command==AMS_STOP)
					  return;
				  set_requested_frequency(i);
				  HAL_Delay(1);
				  uint16_t raw = read_raw();
				  sprintf(strbuf, "{%lu, %u}", i, raw);
				  HAL_UART_Transmit(&huart1, (uint8_t*)&strbuf, strlen(strbuf), UART_TIMEOUT);
			 }
			// send last frequency
			if (i!=to){
				set_requested_frequency(to);
				//HAL_Delay(1);
				uint16_t raw = read_raw();
				sprintf(strbuf, "{%lu, %u}", to, raw);
				HAL_UART_Transmit(&huart1, (uint8_t*)&strbuf, strlen(strbuf), UART_TIMEOUT);
			}
		}
		HAL_UART_Transmit(&huart1, (uint8_t*)";", 2, UART_TIMEOUT);
		DisableRFOutput();
		global_command = AMS_NONE;
	}else if (global_command == AMS_SINGLE){
		EnableRFOutput();
		uint32_t f = global_args[0];
		uint32_t pwr = global_args[1];
		setRFA_PWR(pwr-1);
		set_requested_frequency(f);
		while(global_command==AMS_SINGLE){
			;
		}
		DisableRFOutput();
		global_command = AMS_NONE;
	}else if (global_command == AMS_REGISTER){
		// all registers should be 32bit numbers
		uint32_t reg_value_1 = global_args[0];
		uint32_t reg_value_2 = global_args[1];
		uint32_t reg_value_3 = global_args[2];
		uint32_t reg_value_4 = global_args[3];
		uint32_t reg_value_5 = global_args[4];
		write_reg(reg_value_1);
		write_reg(reg_value_2);
		write_reg(reg_value_3);
		write_reg(reg_value_4);
		write_reg(reg_value_5);
		EnableRFOutput();
		while(global_command==AMS_SINGLE){
					;
		}
		global_command = AMS_NONE;
	}else if (global_command == AMS_VERSION){
		strcpy(strbuf, "AMS_MSG(Antenna Measurement System Version 0.1);");
		HAL_UART_Transmit_IT(&huart1, (uint8_t*)&strbuf, strlen(strbuf));
		global_command = AMS_NONE;

	}else if (global_command == AMS_HOWAREYOU){
		strcpy(strbuf, "AMS_MSG(I am fine and working. So far so good.\nI feel bit exhausted, but ya know, that's life.);");
		HAL_UART_Transmit_IT(&huart1, (uint8_t*)&strbuf, strlen(strbuf));
		global_command = AMS_NONE;

	}else if (global_command == AMS_CHECK){
		strcpy(strbuf, "AMS_OK;");
		HAL_UART_Transmit_IT(&huart1, (uint8_t*)&strbuf, strlen(strbuf));
		global_command = AMS_NONE;
	}
}
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

  /* IPCC initialisation */
  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *) RxBuffer, RX_BFR_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Starting STM32\n");
  printf("Starting MAX\n");
  EnableChip();
  // just to be sure PLL RF output is turned off
  DisableRFOutput();
  // PLL Must be initialized on PowerUp
  init_PLL();

  // PLL must be set to default configuration specified in write_regs_SOFT
  write_regs_SOFT();
  setIntegerMode();

  //calibrate();
  for (int i=0; i<4; i++){
	  printf("RFA PWR: %d\n", i);
	  setRFA_PWR(i);
	  program_PLL();
	  //HAL_Delay(1);
  }
  //print_registers();
  printf("Setting test frequency to 65MHz\n");
  set_requested_frequency(65);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  do_commands();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

		/* start the DMA again */
		printf("Size %d\n", Size);
		printf(RxBuffer);
		printf("Received\n");
		process_command_string(RxBuffer, &global_command, global_args);
		HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *) RxBuffer, RX_BFR_SIZE);
		//__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
