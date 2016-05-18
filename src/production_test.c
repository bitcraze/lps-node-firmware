#include <stdbool.h>
#include <stdio.h>

#include "usart.h"
#include "test_support.h"

static void initUart(UART_HandleTypeDef* huart, USART_TypeDef* instance);
static void initPins(UART_HandleTypeDef* huart);

static bool runUartLoopbackTests();
static bool runUartLoopbackTest(UART_HandleTypeDef* from, UART_HandleTypeDef* to, char* errorMessage);

bool productionTestsRun() {
  bool result = true;

  printf("==============================\r\n");
  testSupportPrintStart("Production tests start");
  printf("\r\n------------------------------\r\n");

  testSupportPrintStart("UART production test");
  testSupportReport(&result, runUartLoopbackTests());

  printf("------------------------------\r\n");
  printf("TEST\t: Production tests ");
  testSupportReport(&result, result);
  printf("==============================\r\n");

  return result;
}


static bool runUartLoopbackTests() {
  UART_HandleTypeDef uart1;
  UART_HandleTypeDef uart3;

  initUart(&uart1, USART1);
  initUart(&uart3, USART3);

  initPins(&uart1);
  initPins(&uart3);

  return runUartLoopbackTest(&uart1, &uart3, "UART1 to UART3") &&
         runUartLoopbackTest(&uart3, &uart1, "UART3 to UART1");
}

static bool runUartLoopbackTest(UART_HandleTypeDef* from, UART_HandleTypeDef* to, char* errorMessage) {
  uint8_t data = 0;
  uint8_t received = 0;
  uint32_t timeout = 1;

  for (data = 0; data < 255; data++) {
    if (HAL_UART_Transmit(from, &data, 1, timeout) != HAL_OK) {
      printf(" Failed to write data for %s ", errorMessage);
      return false;
    }

    if (HAL_UART_Receive(to, &received, 1, timeout) == HAL_OK) {
      if (data != received) {
        printf(" Got unexpected value for %s ", errorMessage);
        return false;
      }
    } else {
        printf(" Did not get any data for %s ", errorMessage);
        return false;
    }
  }

  return true;
}


static void initUart(UART_HandleTypeDef* huart, USART_TypeDef* instance) {
  huart->Instance = instance;
  huart->Init.BaudRate = 115200;
  huart->Init.WordLength = UART_WORDLENGTH_8B;
  huart->Init.StopBits = UART_STOPBITS_1;
  huart->Init.Parity = UART_PARITY_NONE;
  huart->Init.Mode = UART_MODE_TX_RX;
  huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart->Init.OverSampling = UART_OVERSAMPLING_16;
  huart->Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(huart);
}

void initPins(UART_HandleTypeDef* huart) {
  GPIO_InitTypeDef GPIO_InitStruct;

  if(huart->Instance==USART1) {
    /* Peripheral clock enable */
    __USART1_CLK_ENABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  } else if(huart->Instance==USART3) {
    /* Peripheral clock enable */
    __USART3_CLK_ENABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}
