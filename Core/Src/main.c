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
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "n25q128a.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_TEST_MX25 (0)
#define FLASH_TEST_N25Q (1)


#if (FLASH_TEST_MX25 != 0)
#define MX25_SPI_PORT1 (0) /* 1 -> SPI1, 0 -> SPI2 */
#define MX25_DEFAULT_IMG_ADDR (0x7000)

    #if (MX25_SPI_PORT1 != 0)
    #warning "De-init SPI1 pins currently still not supported (nRF will failed to read flash)"
    #endif

#include "MX25Series.h"
MX25Series_t flash_test = {0};


#endif /* End of (FLASH_TEST_MX25 != 0) */


#if (FLASH_TEST_N25Q != 0)
#define FLASH_N25_MANUFACTURE_ID                (0x20)
#define FLASH_N25_MEM_TYPE_ID                   (0xBA)
#define FLASH_N25_MEM_CAPACITY_ID               (0x19)   // 256Mbit
#define FLASH_N25_READ_ID_MSG_LEN (20)


// ====================== FW ====================== //
#define FLASH_N25_FW_START_ADDR            (0x000000)

#endif /* End of (FLASH_TEST_N25Q != 0)) */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Symbols relate to FW binary data (defined in linker script) */
extern int _start_fw_data, _end_fw_data;
uint8_t* fw_binary_data_start = (uint8_t*) &_start_fw_data;
uint8_t* fw_binary_data_end = (uint8_t*) &_end_fw_data;
uint32_t fw_binary_data_len = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart2, (uint8_t *) &ch, 1, 0xFFFF);

    return ch;
}

#if (FLASH_TEST_MX25 != 0)

int flash_mx25_init(void)
{

#if (MX25_SPI_PORT1 != 0)

    if (MX25Series_status_ok == MX25Series_init(&flash_test, &MX25R6435F_Chip_Def_Low_Power, SPI1_NSS_PIN_NUMBER,
                                                FLASH_RESET_PIN_NUMBER, FLASH_WP_PIN_NUMBER, 0, &hspi1))
#else  /* !(MX25_SPI_PORT1 != 0) */
    if (MX25Series_status_ok == MX25Series_init(&flash_test, &MX25R6435F_Chip_Def_Low_Power, SPI2_NSS_PIN_NUMBER,
                                                FLASH_RESET_PIN_NUMBER, FLASH_WP_PIN_NUMBER, 0, &hspi2))
#endif /* End of (MX25_SPI_PORT1 != 0) */
    {
        if (MX25Series_status_ok ==
            MX25Series_read_identification(&flash_test, &flash_info[0], &flash_info[1], &flash_info[2]))
        {
            printf("MX25Series_init ok\r\n");
            // Check valid image
            if (!is_image_valid(MX25_DEFAULT_IMG_ADDR))
            {
                printf("Found valid image \r\n");
            }
            else
            {
                printf("Image is invalid\r\n");
            }
        }
        else
            printf("MX25Series_read_manufacture_and_device_id fail\r\n");
    }
    else
        printf("MX25Series_init fail\r\n");
    return 0;
}

// Config all external flash pins as analog input
void flash_mx25_deinit(void)
{
    HAL_GPIO_DeInit(GPIOA, FLASH_RESET_PIN_Pin | FLASH_WP_PIN_Pin);
#if (MX25_SPI_PORT1 != 0)

    /*Configure GPIO pin : SPI1 NSS */
    HAL_GPIO_DeInit(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin);
    HAL_SPI_MspDeInit(&hspi1);

#else  /* !(MX25_SPI_PORT1 != 0) */

    /*Configure GPIO pin : SPI2 NSS */
    HAL_GPIO_DeInit(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin);
    HAL_SPI_MspDeInit(&hspi2);
#endif /* End of (MX25_SPI_PORT1 != 0) */
}
#endif /* (FLASH_TEST_MX25 != 0) */

int flash_n25q_init(void)
{
    uint8_t flash_id[FLASH_N25_READ_ID_MSG_LEN] = {0};
    // DQ2, DQ3 must be high for SPI operation
    HAL_GPIO_WritePin(FLASH_RESET_PORT, FLASH_RESET_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FLASH_WP_PORT, FLASH_WP_PIN, GPIO_PIN_SET);
    // Check flash ID
    N25Q_ReadID(flash_id, sizeof(flash_id));
    if ((flash_id[0] != FLASH_N25_MANUFACTURE_ID) || (flash_id[1] != FLASH_N25_MEM_TYPE_ID))
    {
        printf("[ERR] Flash ID is not correct \r\n");
        return -1;
    }
    if (flash_id[2] != FLASH_N25_MEM_CAPACITY_ID)
    {
        printf("[ERR] Flash capacity is not correct \r\n");
    }

    fw_binary_data_len = _end_fw_data - _start_fw_data;
    assert(fw_binary_data_len > 0);
    
    return 0;
}

int flash_n25q_fw_validate()
{
    
}

int flash_n25q_fw_update(void)
{
    int ext_flash_read_buf[fw_binary_data_len];
    memset(ext_flash_read_buf, 0, sizeof(ext_flash_read_buf));
    
    // Read FW from external memory
    N25Q_ReadDataFromAddress(ext_flash_read_buf, (uint32_t) FLASH_N25_FW_START_ADDR, fw_binary_data_len);
    // Compare FW content
    if (memcmp(ext_flash_read_buf, fw_binary_data_start, fw_binary_data_len) == 0)
    {
        printf("[INFO] Found valid Firmware, size = %d bytes \r\n", (int)fw_binary_data_len);
        return 0;
    }
    printf("[ERR] No valid Firmware found, re-writing Firmware to external memory ... \r\n");
#if !1 // TODO - TMT: CHECKME
    N25Q_BulkErase();
#else
    // Calculate number of sectors need to erase
    uint32_t num_sector = ceil((float) fw_binary_data_len / N25Q128A_SECTOR_SIZE);
    // Erase sectors
    for (uint32_t i = 0; i < num_sector; i++)
    {
        N25Q_SectorErase(FLASH_N25_FW_START_ADDR + i * N25Q128A_SECTOR_SIZE);
    }

#endif /* End of 0 */
    N25Q_ReadDataFromAddress(ext_flash_read_buf, (uint32_t) FLASH_N25_FW_START_ADDR, fw_binary_data_len);

    // Write Firmware to external memory
    N25Q_ProgramFromAddress(fw_binary_data_start, (uint32_t) FLASH_N25_FW_START_ADDR, fw_binary_data_len);
    // Read back Firmware from external memory
    N25Q_ReadDataFromAddress(ext_flash_read_buf, (uint32_t) FLASH_N25_FW_START_ADDR, fw_binary_data_len);
    // Compare Firmware
    if (memcmp(ext_flash_read_buf, fw_binary_data_start, fw_binary_data_len) != 0)
    {
        printf("[ERR] Failed to write Firmware to external memory \r\n");
        return -1;
    }
    return 0;
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
    MX_ADC1_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */
    if (flash_n25q_init() != 0)
    {
        printf("[ERR] flash_n25q_init() failed \r\n");
    }

    if (flash_n25q_fw_update() != 0)
    {
        printf("[ERR] flash_n25q_fw_update() failed \r\n");
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
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
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 16;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
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
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    printf("Wrong parameters value: file %s on line %d\r\n", file, (int) line);

    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
