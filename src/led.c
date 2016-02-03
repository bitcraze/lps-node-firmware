#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>

#include "led.h"

#define LED_PORT GPIOC

void ledInit(void) {
  /* Do nothing */
}

void ledOn(Led led) {
  HAL_GPIO_WritePin(LED_PORT, led, GPIO_PIN_SET);
}

void ledOff(Led led) {
  HAL_GPIO_WritePin(LED_PORT, led, GPIO_PIN_RESET);
}
