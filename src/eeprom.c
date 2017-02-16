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
#include <stdio.h>

#include <stdbool.h>
#include <string.h>

#include "eeprom.h"

#include "FreeRTOS.h"
#include "task.h"

static I2C_HandleTypeDef * hi2c;
static int devAddr = 0xA0;

void eepromInit(I2C_HandleTypeDef * i2c)
{
  hi2c = i2c;
}

bool eepromTest()
{
  uint8_t test;

  return eepromRead(0, &test, 1);
}

bool eepromRead(int address, void* data, size_t length)
{
  int status;

  status = HAL_I2C_Mem_Read(hi2c, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100);

  if (status == HAL_OK)
    return true;

  return false;
}

bool eepromWrite(int address, void* data, size_t length)
{
  int status = HAL_OK;
  size_t start_address = address;
  size_t start_i = 0;

  for (int i=0; i<length; i++) {
    if ((address+i+1)%32 == 0) {
      status = HAL_I2C_Mem_Write(hi2c, devAddr, start_address, I2C_MEMADD_SIZE_16BIT, ((uint8_t*)data)+start_i, (i+1) - start_i, 100);
      start_address = address + i + 1;
      start_i = i+1;
      vTaskDelay(10);
    }
  }
  if (start_i != length) {
    status = HAL_I2C_Mem_Write(hi2c, devAddr, start_address, I2C_MEMADD_SIZE_16BIT, ((uint8_t*)data)+start_i, length - start_i, 100);
    vTaskDelay(10);
  }

  if (status == HAL_OK)
    return true;

  return false;
}
