/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Copyright (c) 2014, Bitcraze AB, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */
#include <stdbool.h>

#include "gpio.h"
#include "button.h"

static ButtonEvent state;

static bool buttonRead(void)
{
  return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);
}

void buttonInit(ButtonEvent initialEvent)
{
  state = initialEvent;
}

void buttonProcess()
{
  static unsigned int lastTick;
  static unsigned int pressedTick;
  static bool pressed;

  if (lastTick != HAL_GetTick())
  {
    lastTick = HAL_GetTick();

    if (pressed == false && buttonRead() == BUTTON_PRESSED)
    {
      pressed = true;
      pressedTick = HAL_GetTick();
    }
    else if (pressed == true && buttonRead() == BUTTON_RELEASED)
    {
      pressed = false;
      if ((HAL_GetTick() - pressedTick) < BUTTON_LONGPRESS_TICK)
      {
        state = buttonShortPress;
      }
      else
      {
        state = buttonLongPress;
      }
    }
  }
}

ButtonEvent buttonGetState()
{
  ButtonEvent currentState = state;

  state = buttonIdle;

  return currentState;
}


