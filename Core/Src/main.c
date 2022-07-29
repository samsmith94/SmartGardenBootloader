/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "crc.h"
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"

#include "bootloader.h"
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
FATFS fs;
FIL fil;
FRESULT fresult;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

char SDPath[4]; /* SD logical drive path */
FATFS SDFatFs; /* File system object for SD logical drive */
FIL SDFile; /* File object for SD */

int error = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int __io_putchar(int ch);

void SD_Eject(void);
void SD_DeInit(void);
void Enter_Bootloader(void);

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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

#if 0
	HAL_Delay(500);
	printf("Welcome to SmartGarden bootloader!\r\n");
	fresult = f_mount(&fs, "", 0);
	if (fresult != FR_OK) {
		printf("Error when mounting SD card.\r\n");
	} else {
		printf("SD card mounted successfully.\r\n");
	}
	HAL_Delay(100);

	/* Create second file with read write access and open it */
	fresult = f_open(&fil, "File1.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) {
		printf("File1.txt created succesfully.\r\n");
	} else {
		printf("Error while creating File1.txt!\r\n");
	}

	printf("Writing Hello to File1.txt\r\n");
	/* Writing text */
	fresult = f_write(&fil, "Hello", strlen("Hello"), &bw);

	/* Close file */
	f_close(&fil);
#endif

	printf("\r\n************** BOOTLOADER **************\r\n");
	printf("****************************************\r\n");
	fresult = f_mount(&fs, "", 0);
	if (fresult != FR_OK) {
		printf("Error when mounting SD card.\r\n");
		error = -1;
	} else {
		printf("SD card mounted successfully.\r\n");
	}
	HAL_Delay(10);


	printf("****************************************\r\n");
	printf("Entering Bootloader (flashing GREEN LED)...\r\n");
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	HAL_Delay(250);
	Enter_Bootloader();

	// CHECK FOR APPLICATION
	if (Bootloader_CheckForApplication() == 0) {
		printf("Application found in FLASH.\r\n");
		// VERIFY CHECKSUM:
		if (Bootloader_VerifyChecksum() != 0) {
			printf("Checksum error.\r\n");
			error = -3;
		} else {
			printf("Checksum OK.\r\n");
		}
	}

	//-- reset peripherals to guarantee flawless start of user application
	/*
	HAL_GPIO_DeInit(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_DeInit(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_DeInit(GPIOD, &GPIO_InitStruct);
*/


	HAL_SPI_DeInit(&hspi1);
	//	MX_FATFS_Init(); deinitje?
	HAL_CRC_DeInit(&hcrc);

	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();

	if (error == 0) {
		printf("Launching application in 3 seconds...\r\n");
		HAL_Delay(3000);
		Bootloader_JumpToApplication();
	}
	printf("\r\nWe should never get to here.\r\n");
	printf("Some error must have happened.\r\n");
	HAL_UART_DeInit(&huart2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}


/* USER CODE BEGIN 4 */
void SD_Eject(void)
{
    f_mount(NULL, (TCHAR const *)SDPath, 0);
}

void SD_DeInit(void)
{
    // BSP_SD_DeInit();
    // FATFS_DeInit();
    // SDCARD_OFF();
}

/*** Bootloader ***************************************************************/
void Enter_Bootloader(void)
{
    FRESULT fr;
    UINT num;
    uint8_t status;
    uint64_t data;
    uint32_t cntr;
    uint32_t addr;
    char msg[40] = {0x00};

    /* Check for flash write protection */
    if (FLASH_If_GetWriteProtectionStatus() & FLASHIF_PROTECTION_WRPENABLED)
    {
        printf("Application space in flash is write protected. Disabling write protection and generating system reset...\r\n");
        FLASH_If_WriteProtectionConfig(FLASHIF_WRP_DISABLE);
    }

    // Mount SD card
    fr = f_mount(&SDFatFs, (TCHAR const *)SDPath, 1);
    if (fr != FR_OK)
    {
        // f_mount failed
        printf("SD card cannot be mounted. FatFs error code: %u\r\n", fr);
        return;
    }
    printf("SD mounted.\r\n");

    // Open file for programming
    fr = f_open(&SDFile, "application.bin", FA_READ);

    if (fr != FR_OK)
    {
        // f_open failed
        //printf("File cannot be opened. FatFs error code: %u\r\n", fr);

		printf("Error when opening file.\r\n");

		printf("Maybe there is no file on SD card,\r\nas production.bin is flashed.\r\n");
		error = -2;


        SD_Eject();
        printf("SD ejected.\r\n");


        Bootloader_JumpToApplication();

        return;
    }
    printf("Software found on SD.\r\n");

    // Check size of application found on SD card
    /*
    if (Bootloader_CheckSize(f_size(&SDFile)) != BL_OK)
    {
        printf("Error: app on SD card is too large.\r\n");

        f_close(&SDFile);
        SD_Eject();
        printf("SD ejected.\r\n");
        return;
    }
    printf("App size OK.\r\n");
    */

    // Step 1: Init Bootloader and Flash
    FLASH_If_Init();

    // Step 2: Erase Flash
    printf("Erasing flash...\r\n");
    FLASH_If_Erase(USER_START_ADDRESS);
    printf("Flash erase finished.\r\n");

    // Step 3: Programming
    printf("Starting programming...\r\n");
    cntr = 0;

    FLASH_If_FlashBegin();
    do
    {
        data = 0xFFFFFFFFFFFFFFFF;
        fr = f_read(&SDFile, &data, 4, &num);
        if (num)
        {
            status = FLASH_If_Write(data);
            if (status == FLASHIF_OK)
            {
                cntr++;
            }
            else
            {
                printf(msg, "Programming error at: %lu byte\r\n", (cntr * 8));
                f_close(&SDFile);
                SD_Eject();
                printf("SD ejected.\r\n");
                return;
            }
        }
    } while ((fr == FR_OK) && (num > 0));

    // Step 4: Finalize Programming
    FLASH_If_FlashEnd();

    f_close(&SDFile);
    printf("Programming finished. Flashed: %lu bytes.\r\n", (cntr * 4));

    // Open file for verification
    fr = f_open(&SDFile, "application.bin", FA_READ);
    if (fr != FR_OK)
    {
        // f_open failed
        printf("File cannot be opened. FatFs error code: %u\r\n", fr);

        SD_Eject();
        printf("SD ejected.\r\n");
        return;
    }

    // Step 5: Verify Flash Content
    addr = USER_START_ADDRESS;
    cntr = 0;
    do
    {
        data = 0xFFFFFFFFFFFFFFFF;
        fr = f_read(&SDFile, &data, 4, &num);
        if (num)
        {
            if (*(uint32_t *)addr == (uint32_t)data)
            {
                addr += 4;
                cntr++;
            }
            else
            {
                printf("Verification error at: %lu byte.\r\n", (cntr * 4));
                f_close(&SDFile);
                SD_Eject();
                printf("SD ejected.\r\n");
                return;
            }
        }
    } while ((fr == FR_OK) && (num > 0));
    printf("Verification passed.\r\n");

    // Eject SD card
    SD_Eject();
    printf("SD ejected.\r\n");

    // Enable flash write protection
    /*
    printf("Enablig flash write protection and generating system reset...\r\n");
    if (FLASH_If_WriteProtectionConfig(FLASHIF_WRP_ENABLE) != FLASHIF_OK)
    {
        printf("Failed to enable write protection. Exiting Bootloader.\r\n");
    }
    */
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
	while (1) {
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
