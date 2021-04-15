
#include "usart.h"

static int isL4 = L4_ENABLED;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

void MX_USART1_UART_Init(void)
{
  if (isL4) {
    MX_USART1_UART_Init_l4();
  } else {
    MX_USART1_UART_Init_f0();
  }
}

void MX_USART3_UART_Init(void)
{
  if (isL4) {
    MX_USART3_UART_Init_l4();
  } else {
    MX_USART3_UART_Init_f0();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  if (isL4) {
    HAL_UART_MspInit_l4();
  } else {
    HAL_UART_MspInit_f0();
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if (isL4) {
    HAL_UART_MspDeInit_l4();
  } else {
    HAL_UART_MspDeInit_f0();
  }
}
