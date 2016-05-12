#include <stdbool.h>
#include <stdio.h>

#include "usart.h"
#include "test_support.h"

static void initUart(UART_HandleTypeDef* huart, USART_TypeDef* instance);
static bool runUartLoopbackTest();

static UART_HandleTypeDef uart1;
static UART_HandleTypeDef uart3;

bool productionTestsRun() {
  initUart(&uart1, USART1);
  initUart(&uart3, USART3);

  bool result = true;

  testSupportPrintStart("UART production test");
  testSupportReport(&result, runUartLoopbackTest());

  printf("------------------------------\r\n");
  printf("Production test overall result\r\n");
  testSupportReport(&result, result);

  return result;
}


void initUart(UART_HandleTypeDef* huart, USART_TypeDef* instance) {
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

bool runUartLoopbackTest() {
  bool result = true;

  // TODO Add test

  if (!result) {
    printf("TODO report why\r\n");
  }

  return result;
}
