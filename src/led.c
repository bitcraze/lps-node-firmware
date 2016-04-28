/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2016, Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>

#include "led.h"

typedef struct {
  uint32_t pin;
  GPIO_TypeDef * port;
} led_t;

static led_t leds_revd[] = {
    [ledRanging] = {.pin = GPIO_PIN_1, .port = GPIOF},
    [ledSync] = {.pin = GPIO_PIN_1, .port = GPIOA},
    [ledMode] = {.pin = GPIO_PIN_2, .port = GPIOA}
};

static led_t leds_revc[] = {
    [ledRanging] = {.pin = GPIO_PIN_13, .port = GPIOC},
    [ledSync] = {.pin = GPIO_PIN_14, .port = GPIOC},
    [ledMode] = {.pin = GPIO_PIN_15, .port = GPIOC}
};


void ledInit(void) {
  /* Do nothing */
}

void ledOn(led_e led) {
  HAL_GPIO_WritePin(leds_revd[led].port, leds_revd[led].pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(leds_revc[led].port, leds_revc[led].pin, GPIO_PIN_SET);
}

void ledOff(led_e led) {
  HAL_GPIO_WritePin(leds_revd[led].port, leds_revd[led].pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(leds_revc[led].port, leds_revc[led].pin, GPIO_PIN_RESET);
}
