/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
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
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"
#include "app_conf.h"
#include "app_entry.h"
#include "app_common.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "custom_app.h"
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
void MX_USART1_UART_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SCL_Pin GPIO_PIN_3
#define SCL_GPIO_Port GPIOA
#define LDAC_Pin GPIO_PIN_4
#define LDAC_GPIO_Port GPIOA
#define SDATA_Pin GPIO_PIN_5
#define SDATA_GPIO_Port GPIOA
#define SCLK_Pin GPIO_PIN_6
#define SCLK_GPIO_Port GPIOA
#define FSYNC_Pin GPIO_PIN_7
#define FSYNC_GPIO_Port GPIOA
#define SW1_User_Pin GPIO_PIN_4
#define SW1_User_GPIO_Port GPIOC
#define SW1_User_EXTI_IRQn EXTI4_IRQn
#define M3_Pin GPIO_PIN_2
#define M3_GPIO_Port GPIOB
#define M1_Pin GPIO_PIN_0
#define M1_GPIO_Port GPIOB
#define M2_Pin GPIO_PIN_1
#define M2_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_12
#define SDA_GPIO_Port GPIOC
#define SW2_User_Pin GPIO_PIN_0
#define SW2_User_GPIO_Port GPIOD
#define SW2_User_EXTI_IRQn EXTI0_IRQn
#define SW3_User_Pin GPIO_PIN_1
#define SW3_User_GPIO_Port GPIOD
#define SW3_User_EXTI_IRQn EXTI1_IRQn
#define M4_Pin GPIO_PIN_3
#define M4_GPIO_Port GPIOB
#define ME_Pin GPIO_PIN_4
#define ME_GPIO_Port GPIOB
#define Blue_Led_Pin GPIO_PIN_5
#define Blue_Led_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
