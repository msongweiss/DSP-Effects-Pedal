/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "cs43l22.h"
#include "overdrive.h"
#include "tremolo.h"
#include <stdio.h>
#include <math.h>
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
#define BLOCK_SIZE_FLOAT 128
#define BLOCK_SIZE_U16 512
#define OD_GAIN 100.0f
#define TR_DEPTH 0.5f // Depth 0.0 (passthrough) to 1.0 (full wub)
#define TR_RATE 5.0f // Hz

uint8_t callback_state = 0;
uint16_t adcControlData[1];
uint16_t adcControlDataLast = 69;

uint16_t rxBuf[BLOCK_SIZE_U16*2];
uint16_t txBuf[BLOCK_SIZE_U16*2];
float l_buf_in [BLOCK_SIZE_FLOAT*2];
float r_buf_in [BLOCK_SIZE_FLOAT*2];
float l_buf_out [BLOCK_SIZE_FLOAT*2];
float r_buf_out [BLOCK_SIZE_FLOAT*2];
Overdrive od;
Tremolo tr;


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// === DMA callbacks ===

// FOR DOUBLE BUFFERING
// Half complete buffer
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	callback_state = 1;
}

// Fully complete buffer
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
	callback_state = 2;
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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_I2C3_Init();
  MX_USB_HOST_Init();
  MX_I2S2_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // Initialize overdrive
  // 41.6kHz Fs
  // 800Hz HPF input stage
  // 4000Hz LPF output stage
  // See OD_GAIN defined in user defines
  Overdrive_Init(&od, 41666.0f, 800.0f, 4000.0f, OD_GAIN);

  // Initialize Tremolo
  Tremolo_Init(&tr, 41666.0f, TR_RATE, TR_DEPTH);

  // Initialize I2S DMA
  HAL_I2SEx_TransmitReceive_DMA (&hi2s2, txBuf, rxBuf, BLOCK_SIZE_U16);
  int offset_r_ptr;
  int offset_w_ptr, w_ptr;

  // Initialize pot ADC
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcControlData, 1);
  HAL_TIM_Base_Start(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (fabs(adcControlDataLast - adcControlData[0]) > 10) {
//		  Tremolo_Set_Depth(&tr, adcControlData[0] / 4095.0f);
		  Overdrive_Set_Gain(&od, 100.0f * adcControlData[0] / 4095.0f);
		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
	  }

	  adcControlDataLast = adcControlData[0];

	  if (callback_state != 0) {

		  // decide if it was half or cplt callback
		  if (callback_state == 1)   {
			  	  offset_r_ptr = 0;
			  	  offset_w_ptr = 0;
			  	  w_ptr = 0;
			  	  // Set pointer to first half of DMA
			  }

		  else if (callback_state == 2) {
			  offset_r_ptr = BLOCK_SIZE_U16;
			  offset_w_ptr = BLOCK_SIZE_FLOAT;
			  w_ptr = BLOCK_SIZE_FLOAT;
			  // Set pointer to second half of DMA
		  }


		  //restore input sample buffer to float array
		  for (int i=offset_r_ptr; i<offset_r_ptr+BLOCK_SIZE_U16; i=i+4) {

			  // Rebuild signed 24-bit sample from 16-bit rxBuf
			  int32_t sample_l = ((int32_t)(rxBuf[i] << 16) | (rxBuf[i + 1]));

			  // Convert to float in range [-1.0f, 1.0f] for easier DSP
			  float sample_f_l = sample_l / 2147483648.0f;  // 2^31

			  // Light up debug LED if this is somehow outside of range
			  if (sample_f_l > 1.0f | sample_f_l < -1.0f) {
				  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
			  }
			  l_buf_in[w_ptr] = sample_f_l;

			  // Repeat for right channel
			  int32_t sample_r = ((int32_t)(rxBuf[i + 2] << 16) | (rxBuf[i + 3]));
			  float sample_f_r = sample_r / 2147483648.0f;
			  r_buf_in[w_ptr] = sample_f_r;


			  w_ptr++;
		  }


		  for (int i=offset_w_ptr; i<offset_w_ptr+BLOCK_SIZE_FLOAT; i++) {
			  // CODE FOR PASSTHROUGH, DON'T DELETE
//			  l_buf_out[i] = l_buf_in[i];
//			  r_buf_out[i] = r_buf_in[i];

//			  // Populate output buffer with overdrive-processed input buffer data
			  l_buf_out[i] = Overdrive_Update(&od, l_buf_in[i])/32.0f; // 1/16 for appropriate amp-level volume
			  r_buf_out[i] = Overdrive_Update(&od, r_buf_in[i])/32.0f;

			  // Populate output buffer with tremolo-processed input buffer data
//			  l_buf_out[i] = Tremolo_Update(&tr, l_buf_in[i]); // 1/16 for appropriate amp-level volume
//			  r_buf_out[i] = Tremolo_Update(&tr, l_buf_in[i]);
		  }

		  //restore processed float-array to output sample-buffer
		  w_ptr = offset_w_ptr;

		  for (int i=offset_r_ptr; i<offset_r_ptr+BLOCK_SIZE_U16; i=i+4) {
			  int sample_out_l = (int)(l_buf_out[w_ptr] * 2147483648.0f);  // back to 24-bit signed
			  txBuf[i]   = (sample_out_l >> 16) & 0xFFFF;  // upper 16 bits
			  txBuf[i+1] = sample_out_l & 0xFFFF;          // lower 16 bits
			  int sample_out_r = (int)(r_buf_out[w_ptr] * 2147483648.0f);  // back to 24-bit signed
			  txBuf[i+2]   = (sample_out_r >> 16) & 0xFFFF;  // upper 16 bits
			  txBuf[i+3] = sample_out_r & 0xFFFF;          // lower 16 bits
			  w_ptr++;
		  }

		  callback_state = 0;

	  }
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
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
