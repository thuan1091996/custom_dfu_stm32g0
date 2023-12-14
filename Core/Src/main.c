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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_TEST_MX25     (1)
#define MX25_SPI_PORT1      (0) /* 1 -> SPI1, 0 -> SPI2 */

#if (MX25_SPI_PORT1 != 0)
#warning  "De-init SPI1 pins currently still not supported (nRF will failed to read flash)"
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#if (FLASH_TEST_MX25 == 1)
uint8_t flash_info[20] = {0};
volatile bool g_test_flash = false;
#include "MX25Series.h"

MX25Series_t flash_test = {0};
uint8_t buff_read[512] = {0};
uint8_t buff_write[10] = {1, 2, 3};
#endif /* End of (FLASH_TEST_MX25 == 1) */
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
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

// Config all external flash pins as analog input
void flash_pin_config_as_analog_input(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_DeInit(GPIOA, FLASH_RESET_PIN_Pin | FLASH_WP_PIN_Pin);
    #if (MX25_SPI_PORT1 != 0)

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
    /*Configure GPIO pin : SPI1 NSS */
    
    HAL_GPIO_DeInit(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin);
    #else /* !(MX25_SPI_PORT1 != 0) */
    /*Configure GPIO pin : SPI2 NSS */
    
    HAL_GPIO_DeInit(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin);
    /*Configure GPIO pin : SPI2 CLK */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);

    /*Configure GPIO pin : SPI2 MISO, MOSI */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2 | GPIO_PIN_3);
    #endif /* End of (MX25_SPI_PORT1 != 0) */

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

    /* MCU Configuration-------------------------------------------------------- */

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
    /* USER CODE BEGIN 2 */
#if (FLASH_TEST_MX25 != 0)
    #if (MX25_SPI_PORT1 != 0)
    if (MX25Series_status_ok == MX25Series_init(&flash_test, &MX25R6435F_Chip_Def_Low_Power, SPI1_NSS_PIN_NUMBER,
                                                FLASH_RESET_PIN_NUMBER, FLASH_WP_PIN_NUMBER, 0, &hspi1))
    #else /* !(MX25_SPI_PORT1 != 0) */
    if (MX25Series_status_ok == MX25Series_init(&flash_test, &MX25R6435F_Chip_Def_Low_Power, SPI2_NSS_PIN_NUMBER,
                                                FLASH_RESET_PIN_NUMBER, FLASH_WP_PIN_NUMBER, 0, &hspi2))
    #endif /* End of (MX25_SPI_PORT1 != 0) */
    {
        if (MX25Series_status_ok == MX25Series_read_identification(&flash_test, &flash_info[0], &flash_info[1], &flash_info[2]))
        {
            printf("MX25Series_init ok\r\n");

            MX25Series_read_stored_data(&flash_test, true, 0x8000, 512, buff_read);
            //			MX25Series_set_write_enable(&flash_test, 1);
            //			MX25Series_write_stored_data(&flash_test, 0x1000, 10, buff_write);
            //			MX25Series_read_stored_data(&flash_test, true, 0x1000, 10, buff_read);
//            continue;
        }
        else
            printf("MX25Series_read_manufacture_and_device_id fail\r\n");
    }
    else
        printf("MX25Series_init fail\r\n");
#endif
    flash_pin_config_as_analog_input();
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
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
