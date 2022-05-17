/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32f1xx_hal.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PAGEError = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* iCEBlaster. STM32 based iCE40 Bitstream programmer tool with USB MSC Drag and Drop feature.
 * Coded by TinLethax
 * https://github.com/TiNredmc/iCEBlaster_maplemini
 */

/* directLoad is used when sending bit stream directly to iCE40 FPGA.
 * treat it as a slave device. By make sure that the CE pin is held low before release CRESET.
 * after iCE40 boot up, it will start in SPI slave mode and then we can send the bit stream directly.
 */
uint8_t directLoad(uint8_t *data, uint32_t size){
	uint8_t dummy[19] = {0};// dummy byte to kick start bit stream loading process.

	// send series of bitstream to FPGA via SPI.
	HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);

	// Since we don't monitor the state of CDONE pin (Do it blindly).
	// And to be sure that iCE40 is ready to boot
	// We need to wait at least 100 SPI clock cycles.
	// Send at least 100 clock cycles or 12 dummy bytes (96 cycles)
	HAL_SPI_Transmit(&hspi1, dummy, 12, HAL_MAX_DELAY);

	// After 49 clock cycles. Pins that are used for configuration can be reconfigured and use as normal PIO.
	// Send at least 49 clock cycles or 7 dummy bytes (56 cycles)
	HAL_SPI_Transmit(&hspi1, dummy, 7, HAL_MAX_DELAY);

	// CE High
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

	return 0;
}


// Drive mkfs
void ice_mkfs(){
	const uint8_t ms_fat12[62] = {
			0xEB, 0x3C, 0x90, // Jump instruction to bootstrap (x86 instruction)
			0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30,// OEM name as "MSDOS5.0"
			0x00, 0x04,// sector size -> 0x400 = 1024 bytes
			0x01,// 1 Cluster = 1 sector = 1024 bytes.
			0x02, 0x00,// 2 sector reserved (FAT12)
			0x02,// number of FATs == 2
			0x00, 0x02,// 32-byte directory entries in the root directory == 512 bytes
			0x40, 0x00, // total sector of 64 sectors
			0xF8, // Non-removable disk
			0x01, 0x00,// FAT occupied 1 sector (FAT12)
			0x01, 0x00,// 1 sector per track
			0x01, 0x00,// 1 (reading?) head (irrelevant)
			0x00, 0x00, 0x00, 0x00,// No hidden physical sectors.
			0x00, 0x00, 0x00, 0x00,// Total number of sectors (For FAT32). Remains 0 since we use FAT12.
			0x80,// Drive number (0x80 == fixed disk).
			0x00,// Reserved (For WinNT).
			0x29,// Extended boot signature
			0xD5, 0x80, 0x9A, 0x1C,// Volume serial number
			0x49, 0x43, 0x45, 0x42, 0x4C, 0x41, 0x53, 0x54, 0x45, 0x52, 0x20,// Volume label "ICEBLASTER ".
			0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20// "FAT12   "
			};

		// Format the drive


		HAL_FLASH_Unlock();
		// Page erase
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; // erase 1024 KBytes (which is the size of 1 page).
		EraseInitStruct.PageAddress = FLASH_MEM_BASE_ADDR; // We start erase from the beginning of sector.
		EraseInitStruct.NbPages = 64; // this tells eraser for how many page we want to erase.

		HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

		for(uint8_t i=0; i < 62; i+= 2){// Write Boot sector (BIOS Parameter Block) to the internal Flash.
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
					FLASH_MEM_BASE_ADDR + i, (ms_fat12[i] | ms_fat12[i+1] << 8)); // flash modified data onto Flash memory.
		}

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEM_BASE_ADDR + 510, 0xAA55);// Write the signature of FAT file system at the end of sector 0.

		for(uint32_t n = FLASH_MEM_BASE_ADDR + 512; n < (FLASH_MEM_BASE_ADDR + 0x1000); n += 4)
									HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, n, 0x00000000);

		// Set Label name at the beginning of sector 3.
		const uint8_t ice_label[16] = {'i', 'C', 'E', 'B', 'l', 'a', 's', 't', 'e', 'r', ' ', 0x08, 0x00, 0x00};
		for(uint8_t i =0; i < 16; i+= 2)
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEM_BASE_ADDR + 0x1000 +i, (ice_label[i] | ice_label[i+1] << 8));

		for(uint32_t n = FLASH_MEM_BASE_ADDR + 0x1010; n < 0x08020000; n += 4)
							HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, n, 0x00000000);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t bitstream_size = 0;

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
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Important source files worth taking a look at :
  // USB_DEVICE/App/usbd_storage_if.c

  /* On Maple Mini board has USB bus gating controlled by PB9
     * If USB function of your STM32F103 Maple Mini doesn't work
     * Or your PC doesn't detect your board
     * Set PB9 to Logic low */

     printf("%c",0x0c);// clear UART terminal screen

     HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_SET);// Release reset pin, lets FPGA run.
     ice_mkfs();

     HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_RESET);// Allow Electrical USB connection.

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	uint8_t *flash_scan = (uint8_t *)(FLASH_MEM_BASE_ADDR);
	//Locate the Bitstream byte on internal flash by using "pattern match" algorithm.
	while(1){
	  //flash_scan++;
		  if(flash_scan == (uint8_t *)FLASH_MEM_END_ADDR)
					flash_scan = (uint8_t *)(FLASH_MEM_BASE_ADDR);

	  if(*(flash_scan++) == 0x7E){//MSB byte of preamble should be at 0x8015805.
		  if(*(flash_scan++) == 0xAA){
			  if(*(flash_scan++) == 0x99){
				  if(*flash_scan == 0x7E){
					  // found!
					  HAL_Delay(1000);// slow down a bit. Let's the Bitstream copied.
					  break;
				  }
			  }
		  }
	  }
	}

	flash_scan -= 3;// After detected the preamble, move address back 3 byte to the beginning of the Bitstream.

	printf("INFO:Bitstream found at : %p\n", flash_scan);

	// some temporary buffer to hold bit stream address on flash.
	uint8_t *fdata;
    fdata = (uint8_t *)(flash_scan);

    while(1){// detect wake up command (end of Bitstream).
    	if(*(flash_scan++) == 0x01){
    		if(*(flash_scan++) == 0x06){
    			if(*flash_scan == 0x00){
    				break;
    			}
    		}
    	}
    }

    // Calculate Bitstream size.
    bitstream_size = ((uint8_t *)flash_scan - fdata) + 1;
    printf("INFO:Bitstream size : %ld\n", bitstream_size);

	// release the reset and quickly blast Bitstream to FPGA.
	// procedure according to TN-02001 app note.

	/* FPGA Reset sequence*/
	// CE Low
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

	// CRESET low
	HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_RESET);

	HAL_Delay(2);// wait for 2ms.

	// CRESET High
	HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_SET);

	HAL_Delay(2);// wait for 2ms.

	__disable_irq();

	// FPGA bit stream loading
	directLoad(fdata, bitstream_size);

	__enable_irq();

	// release reset to lets FPGA run.
	HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_SET);
	printf("DONE:Bit stream is flashed into iCE40");

	// Reformat the drive space.
	ice_mkfs();

	// Disconnect and reconnect USB.
	HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_SET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_RESET);

  }// while
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CRESET_pin_Pin */
  GPIO_InitStruct.Pin = CRESET_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CRESET_pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_EN_Pin */
  GPIO_InitStruct.Pin = USB_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_EN_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
