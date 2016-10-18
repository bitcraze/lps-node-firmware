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

static const led_t leds_revd[] = {
    [ledRanging] = {.pin = GPIO_PIN_1, .port = GPIOF},
    [ledSync] = {.pin = GPIO_PIN_1, .port = GPIOA},
    [ledMode] = {.pin = GPIO_PIN_2, .port = GPIOA}
};

static const led_t leds_revc[] = {
    [ledRanging] = {.pin = GPIO_PIN_13, .port = GPIOC},
    [ledSync] = {.pin = GPIO_PIN_14, .port = GPIOC},
    [ledMode] = {.pin = GPIO_PIN_15, .port = GPIOC}
};

static bool isBlinking[N_LEDS];
static uint32_t disableTime[N_LEDS];

void ledInit(void) {
  /* Do nothing */
}

static inline void setLed(led_e led, bool value)
{
  HAL_GPIO_WritePin(leds_revd[led].port, leds_revd[led].pin, value?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(leds_revc[led].port, leds_revc[led].pin, value?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void ledOn(led_e led) {
  isBlinking[led] = false;
  setLed(led, true);
}

void ledOff(led_e led) {
  isBlinking[led] = false;
  setLed(led, false);
}

void ledBlink(led_e led, bool oneshot)
{
  isBlinking[led] = true;
  if (oneshot) {
    disableTime[led] = HAL_GetTick() + 50;
    setLed(led, true);
  }
}

void ledTick()
{
  static uint32_t last_tick;
  static bool blinkStatus;

  for (int led=0; led<N_LEDS; led++) {
    if (isBlinking[led] && disableTime[led] && disableTime[led]<HAL_GetTick()) {
      setLed(led, false);
      disableTime[led] = 0;
      isBlinking[led] = false;
    }
  }

  if (HAL_GetTick()>(last_tick+250)) {
    blinkStatus = !blinkStatus;
    last_tick = HAL_GetTick();
    for (int led=0; led<N_LEDS; led++) {
      if (isBlinking[led] && !disableTime[led]) {
        setLed(led, blinkStatus);
      }
    }
  }
}
