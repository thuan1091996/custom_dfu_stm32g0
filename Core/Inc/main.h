/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

    /* Private includes ----------------------------------------------------------*/
    /* USER CODE BEGIN Includes */

    /* USER CODE END Includes */

    /* Exported types ------------------------------------------------------------*/
    /* USER CODE BEGIN ET */

    /* USER CODE END ET */

    /* Exported constants --------------------------------------------------------*/
    /* USER CODE BEGIN EC */

    /* USER CODE END EC */

    /* Exported macro ------------------------------------------------------------*/
    /* USER CODE BEGIN EM */

    /* USER CODE END EM */

    /* Exported functions prototypes ---------------------------------------------*/
    void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOF
#define SPI1_NSS_Pin GPIO_PIN_10
#define SPI1_NSS_GPIO_Port GPIOA
#define FLASH_RESET_PIN_Pin GPIO_PIN_11
#define FLASH_RESET_PIN_GPIO_Port GPIOA
#define FLASH_WP_PIN_Pin GPIO_PIN_12
#define FLASH_WP_PIN_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SPI2_NSS_Pin GPIO_PIN_15
#define SPI2_NSS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define FLASH_RESET_PORT GPIOA
#define FLASH_RESET_PIN GPIO_PIN_11
#define FLASH_WP_PORT GPIOA
#define FLASH_WP_PIN GPIO_PIN_12
#define SPI1_NSS_PIN_NUMBER 15
#define FLASH_RESET_PIN_NUMBER 11
#define FLASH_WP_PIN_NUMBER 12
    /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
