/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v2.0_Cube
  * @brief          : Memory management layer.
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
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */
#include <stdio.h>
#include <stm32f1xx_hal.h>
#include <main.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  0x10000
#define STORAGE_BLK_SIZ                  0x200

/* USER CODE BEGIN PRIVATE_DEFINES */
/* block size 512 bytes (STORAGE_BLK_SIZ = 0x200).
 * and block number is 65535(STORAGE_BLK_NBR).
 * total ~32Mbytes
 */

// Redefine
#undef STORAGE_BLK_NBR
#undef STORAGE_BLK_SIZ

#define STORAGE_BLK_NBR                  0xFFFF
#define STORAGE_BLK_SIZ                  0x200

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */

  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'T', 'L', 'H', 'X', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'i', 'C', 'E', 'B', 'L', 'A', 'S', 'T', /* Product      : 16 Bytes */
  'E', 'R', ' ', 'V', 'F', 'S', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */
// FAT12 template
const uint8_t ms_fat12[62] = {
		0xEB, 0x3C, 0x90, // Jump instruction to bootstrap (x86 instruction)
		0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30,// OEM name as "MSDOS5.0"
		0x00, 0x02,// sector size -> 0x200 = 512 bytes
		0x01,// 1 Sector per cluster.
		0x01, 0x00,// 1 sector reserved (FAT12)
		0x02,// number of FATs == 2
		0x00, 0x02,// 32-byte directory entries in the root directory == 512 bytes
		0x00, 0x02, // total sector of 512 sectors
		0xF8, // Non-removable disk
		0x01, 0x00,// 1 Sector per FAT (FAT12)
		0x01, 0x00,// 1 sector per track
		0x01, 0x00,// 1 (reading?) head (irrelevant)
		0x00, 0x00, 0x00, 0x00,// No hidden physical sectors.
		0x00, 0x00, 0x00, 0x00,// Total number of sectors (For FAT32). Remains 0 since we use FAT12.
		0x80,// Drive number (0x80 == fixed disk).
		0x00,// Reserved (For WinNT).
		0x29,// Extended boot signature
		0xD5, 0x80, 0x9A, 0x1C,// Volume serial number
		'I', 'C', 'E', 'B', 'L', 'A', 'S', 'T', 'E', 'R', ' ',// Volume label "ICEBLASTER ".
		0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20// "FAT12   "
		};
// Volume label
const uint8_t ice_label[16] = {'i', 'C', 'E', 'B', 'l', 'a', 's', 't', 'e', 'r', ' ', 0x08, 0x00, 0x00};

const uint8_t dummy[19] = {0};// dummy byte to kick start bit stream loading process.

uint8_t SPIsend_fsm = 0;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
extern SPI_HandleTypeDef hspi1;
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
void usb_vfs_read(uint8_t *buffer, uint32_t block_number);
void usb_vfs_write(uint8_t *buffer, uint32_t block_number);
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */
  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */
  *block_num  = STORAGE_BLK_NBR;
  *block_size = STORAGE_BLK_SIZ;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 6 */
	usb_vfs_read(buf, blk_addr);
	return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 7 */
	usb_vfs_write(buf, blk_addr);
  return (USBD_OK);
  /* USER CODE END 7 */
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */
  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
// USB vfs for iCEBlaster, No longer use internal flash as storage medium.
// USB info : sector size 512 bytes. 65535 sectors.
// Coded by TinLethax 2022/07/05 +7

void usb_vfs_read(uint8_t *buffer, uint32_t block_number){

	switch(block_number){// Put sector number into switch case

	case 0: // Sector 0. return FAT filesystem and signature.
			memcpy(buffer, ms_fat12, 62);// write FAT12 filesystem
			// write FAT signature
			*(buffer + 510) = 0xAA;
			*(buffer + 511) = 0x55;
			break;

	case 3: // Sector 3. Beginning of Volume. Return Volume label
			memcpy(buffer, ice_label, 16);
			//memset(buffer+16, 0x00, 496);// 0s padding
			break;

	default: // Other sector just sends 0s
			memset(buffer, 0x00, 512);
			break;
	}

}

void usb_vfs_write(uint8_t *buffer, uint32_t block_number){
	uint16_t end_addr = 0;
	uint8_t temp_buf[512] = {0};
	uint32_t cnt = 0x1F0000;

	memcpy(temp_buf, buffer, 512);// make a nice copy.

	switch(SPIsend_fsm){
	case 0: // detect the preamble and send bit stream
		for(uint16_t i=0; i < 512; i++){// Detect Bitstream preamble
			if(temp_buf[i] == 0x7E){
				if(temp_buf[i+1] == 0xAA){
					if(temp_buf[i+2] == 0x99){
						if(temp_buf[i+3] == 0x7E){
							// LED status on
							HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);

							/* This part is reset sequence of iCE40 FPGA
							 * CE low
							 * CRESET low
							 * (optional)delay 2ms
							 * CRESET high
							 * delay 200ms(crucial)
							 * SPI TX
							 */

							HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
							HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_RESET);
							HAL_GPIO_WritePin(CRESET_pin_GPIO_Port, CRESET_pin_Pin, GPIO_PIN_SET);


							//HAL_Delay(200);// wait for 200ms to let CRAM cleared.

							while(cnt--);// delay ~250ms using while loop. For some reason it's working!

							HAL_SPI_Transmit(&hspi1, temp_buf + i, 512 - i, HAL_MAX_DELAY);// start SPI TX
							SPIsend_fsm = 1;
							break;
						}
					}
				}
			}
		}
		break;

	case 1:
		// make sure that there's no Bitstream ending before we write from current sector data.
		// This make sure that we can handle the different Bitstream size that was split into n numbers of 512 bytes sector.
		end_addr = 512;
		for(uint16_t i=0; i < 512; i++){// locate ending
			if(temp_buf[i] == 0x01){
	    		if(temp_buf[i+1] == 0x06){
	    			if(temp_buf[i+2] == 0x00){
	    				end_addr = i + 3;// save the buffer address (as size) that contain Bitstream ending.
	    				SPIsend_fsm = 0;// Set SPI state machine to 0
	    			}
	    		}
	    	}
		}

		HAL_SPI_Transmit(&hspi1, temp_buf, end_addr, HAL_MAX_DELAY);// SPI TX
		if(SPIsend_fsm == 0){// This part of code will execute if SPI state machine will go back to 0 (Because we found Bitstream Ending).
			HAL_SPI_Transmit(&hspi1, dummy, 19, HAL_MAX_DELAY);// Send dummy bytes to kickstart iCE40
			HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);// Release SPI CE
			HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_SET);// turn Status LED off
			reset_flag = 1;// Code in while loop monitor this flag. When flag goes 1, USB reconnect and clear the flag.
		}
		break;

	default:
		break;
	}// switch(SPIsend_fsm)

	//return (USBD_OK);
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

