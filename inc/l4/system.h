/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

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
#define EXT_IO_2_Pin GPIO_PIN_13
#define EXT_IO_2_GPIO_Port GPIOC
#define EXT_IO_3_Pin GPIO_PIN_14
#define EXT_IO_3_GPIO_Port GPIOC
#define LPS25H_INT_Pin GPIO_PIN_0
#define LPS25H_INT_GPIO_Port GPIOH
#define LED_MODE_Pin GPIO_PIN_1
#define LED_MODE_GPIO_Port GPIOH
#define BTN_Pin GPIO_PIN_0
#define BTN_GPIO_Port GPIOA
#define LED_RANGING_Pin GPIO_PIN_1
#define LED_RANGING_GPIO_Port GPIOA
#define LED_SYNC_Pin GPIO_PIN_2
#define LED_SYNC_GPIO_Port GPIOA
#define DW_IRQ_Pin GPIO_PIN_0
#define DW_IRQ_GPIO_Port GPIOB
#define EXT_IO_1_Pin GPIO_PIN_1
#define EXT_IO_1_GPIO_Port GPIOB
#define EXT_IO_0_Pin GPIO_PIN_2
#define EXT_IO_0_GPIO_Port GPIOB
#define DW_RST_Pin GPIO_PIN_12
#define DW_RST_GPIO_Port GPIOB
#define DW_WAKEUP_Pin GPIO_PIN_13
#define DW_WAKEUP_GPIO_Port GPIOB
#define DW_EXTON_Pin GPIO_PIN_14
#define DW_EXTON_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define MCU_ID_ADDRESS 0x1FFF7590
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
