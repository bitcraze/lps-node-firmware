/**
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
 *
 * @file lps25h.c
 * Driver for the lps25h pressure sensor from ST.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <stm32f0xx_hal.h>

#include "lps25h.h"

#define DEBUG_PRINT(args...) printf(args)

// addresses of the device
#define LPS25H_I2C_ADDR 0x5D<<1 // 1011101b

#define LPS25H_LSB_PER_MBAR      4096UL
#define LPS25H_LSB_PER_CELSIUS   480UL
#define LPS25H_ADDR_AUTO_INC     (1<<7)

// register addresses

// register addresses
// Note: Some of the register names in the datasheet are inconsistent
// between Table 14 in section 6 and the register descriptions in
// section 7. Where they differ, the names from section 7 have been
// used here.

#define LPS25H_REF_P_XL       0x08
#define LPS25H_REF_P_L        0x09
#define LPS25H_REF_P_H        0x0A

#define LPS25H_WHO_AM_I       0x0F

#define LPS25H_RES_CONF       0x10

#define LPS25H_CTRL_REG1      0x20
#define LPS25H_CTRL_REG2      0x21
#define LPS25H_CTRL_REG3      0x22
#define LPS25H_CTRL_REG4      0x23
#define LPS25H_INTERRUPT_CFG  0x24
#define LPS25H_INT_SOURCE     0x25
#define LPS25H_STATUS_REG     0x27

#define LPS25H_PRESS_OUT_XL   0x28
#define LPS25H_PRESS_OUT_L    0x29
#define LPS25H_PRESS_OUT_H    0x2A

#define LPS25H_TEMP_OUT_L     0x2B
#define LPS25H_TEMP_OUT_H     0x2C

#define LPS25H_FIFO_CTRL      0x2E
#define LPS25H_FIFO_STATUS    0x2F
#define LPS25H_THS_P_L        0x30
#define LPS25H_THS_P_H        0x31
#define LPS25H_RPDS_L         0x39
#define LPS25H_RPDS_H         0x3A

/* CUSTOM VALUES FOR LPS25H SENSOR */
#define LPS25H_WAI_ID             0xBD
#define LPS25H_ODR_ADDR           0x20
#define LPS25H_ODR_MASK           0x70
#define LPS25H_ODR_AVL_1HZ_VAL    0x01
#define LPS25H_ODR_AVL_7HZ_VAL    0x02
#define LPS25H_ODR_AVL_13HZ_VAL   0x03
#define LPS25H_ODR_AVL_25HZ_VAL   0x04
#define LPS25H_PW_ADDR            0x20
#define LPS25H_PW_MASK            0x80
#define LPS25H_FS_ADDR            0x00
#define LPS25H_FS_MASK            0x00
#define LPS25H_FS_AVL_1260_VAL    0x00
#define LPS25H_FS_AVL_1260_GAIN   KPASCAL_NANO_SCALE
#define LPS25H_FS_AVL_TEMP_GAIN   CELSIUS_NANO_SCALE
#define LPS25H_BDU_ADDR           0x20
#define LPS25H_BDU_MASK           0x04
#define LPS25H_DRDY_IRQ_ADDR      0x23
#define LPS25H_DRDY_IRQ_INT1_MASK 0x01
#define LPS25H_DRDY_IRQ_INT2_MASK 0x10
#define LPS25H_MULTIREAD_BIT      true
#define LPS25H_TEMP_OFFSET        (float)(42.5f)
#define LPS25H_OUT_XL_ADDR        0x28
#define LPS25H_TEMP_OUT_L_ADDR    0x2B

// Self test parameters. Only checks that values are sane
#define LPS25H_ST_PRESS_MAX   (1100.0f) //mbar
#define LPS25H_ST_PRESS_MIN   (450.0f)  //mbar
#define LPS25H_ST_TEMP_MAX    (80.0f)   //degree celcius
#define LPS25H_ST_TEMP_MIN    (-20.0f)  //degree celcius

static uint8_t devAddr;
static bool isInit;

static I2C_HandleTypeDef * hi2c;

#define SDA GPIO_PIN_7
#define SCL GPIO_PIN_6

void i2cDelay()
{
  HAL_Delay(1);
}

void i2cSet(int pin, int value) {
  if (value) {
    HAL_GPIO_WritePin(GPIOB, pin, 1);
    while(HAL_GPIO_ReadPin(GPIOB, pin) == 0);
  } else {
    HAL_GPIO_WritePin(GPIOB, pin, 0);
  }
}

void i2cClock() {
  i2cDelay();
  i2cSet(SCL, 1);
  i2cDelay();
  i2cSet(SCL, 0);
}

void i2cInit()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  HAL_GPIO_WritePin(GPIOB, SCL, 1);
  HAL_GPIO_WritePin(GPIOB, SDA, 1);

  GPIO_InitStruct.Pin = SDA | SCL;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

bool i2cSendByte(uint8_t data)
{
  bool ack;

  i2cSet(SCL, 0);

  for (int i=0; i<8; i++) {
    i2cSet(SDA, data&0x01);
    i2cClock();

    data >>= 1;
  }


  // Read Ack
  i2cSet(SCL, 0);
  HAL_GPIO_WritePin(GPIOB, SDA, 1);
  i2cDelay();
  i2cSet(SCL, 1);
  i2cDelay();

  ack = HAL_GPIO_ReadPin(GPIOA, SDA) == 0;

  return ack;
}

bool i2cReceiveByte()
{
  return false;
}

void i2cStart()
{
  i2cSet(SCL, 1);
  i2cDelay();
  i2cSet(SDA, 0);
  i2cDelay();
  i2cSet(SCL, 0);
}

void i2cStop()
{
  i2cSet(SCL, 0);
  i2cDelay();
  i2cSet(SDA, 0);
  i2cDelay();
  i2cSet(SCL, 1);
  i2cDelay();
  i2cSet(SDA, 1);
  i2cDelay();
}

bool lps25hInit(I2C_HandleTypeDef *i2c)
{
  if (isInit)
    return true;

  hi2c = i2c;
  devAddr = LPS25H_I2C_ADDR;

  //i2cInit();

  //i2cStart();
  //isInit = i2cSendByte(devAddr<<1);
  //i2cStop();

  isInit = true;

  return isInit;
}

bool lps25hTestConnection(void)
{
  uint8_t id;
  bool status;

  if (!isInit)
	  return false;

  status = HAL_I2C_Mem_Read(hi2c, devAddr, LPS25H_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &id, 1, 1000);

  if (status == HAL_OK && id == LPS25H_WAI_ID)
	  return true;

  return false;
}

bool lps25hSelfTest(void)
{
  bool testStatus = false;
  float pressure;
  float temperature;
  float asl;

  if (!isInit)
    return false;

  lps25hGetData(&pressure, &temperature, &asl);

  if (lps25hEvaluateSelfTest(LPS25H_ST_PRESS_MIN, LPS25H_ST_PRESS_MAX, pressure, "pressure") &&
      lps25hEvaluateSelfTest(LPS25H_ST_TEMP_MIN, LPS25H_ST_TEMP_MAX, temperature, "temperature"))
  {
    testStatus = true;
  }
  else
  {
   testStatus = false;
  }

  return testStatus;
}

bool lps25hEvaluateSelfTest(float min, float max, float value, char* string)
{
  if (value < min || value > max)
  {
    DEBUG_PRINT("\r\nPRESS\t: Self test %s [FAIL]. low: %d, high: %d, measured: %d\r\n",
                string, (int)min, (int)max, (int)value);
    return false;
  }
  return true;
}

bool lps25hSetEnabled(bool enable)
{
  uint8_t enable_mask;
  bool status;

	if (!isInit)
	  return false;

	if (enable)
	{
	  enable_mask = 0b11000110; // Power on, 25Hz, BDU, reset zero
    status = HAL_I2C_Mem_Write(hi2c, devAddr, LPS25H_CTRL_REG1, I2C_MEMADD_SIZE_8BIT, &enable_mask, 1, 1000);
	  enable_mask = 0b00001111; // AVG-P 512, AVG-T 64
    status = HAL_I2C_Mem_Write(hi2c, devAddr, LPS25H_RES_CONF, I2C_MEMADD_SIZE_8BIT, &enable_mask, 1, 10000);
// TODO: Investigate why temp values becomes wrong when FIFO averaging is enabled.
//	  enable_mask = 0b11000011; // FIFO Mean mode, 4 moving average
//	  status = i2cdevWrite(I2Cx, devAddr, LPS25H_FIFO_CTRL, 1, &enable_mask);
//	  enable_mask = 0b01000000; // FIFO Enable
//	  status = i2cdevWrite(I2Cx, devAddr, LPS25H_CTRL_REG2, 1, &enable_mask);
	}
	else
	{
	  enable_mask = 0x00; // Power off and default values
    status = HAL_I2C_Mem_Write(hi2c, devAddr, LPS25H_CTRL_REG1, I2C_MEMADD_SIZE_8BIT, &enable_mask, 1, 1000);
	}

	return status;
}

bool lps25hGetData(float* pressure, float* temperature, float* asl)
{
  uint8_t data[5];
  uint32_t rawPressure;
  int16_t rawTemp;
  int status;

  status = HAL_I2C_Mem_Read(hi2c, devAddr,
                            LPS25H_PRESS_OUT_XL | LPS25H_ADDR_AUTO_INC, I2C_MEMADD_SIZE_8BIT,
                            data, 5, 1000);

  rawPressure = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | data[0];
  *pressure = (float)rawPressure / LPS25H_LSB_PER_MBAR;

  rawTemp = ((int16_t)data[4] << 8) | data[3];
  *temperature = LPS25H_TEMP_OFFSET + ((float)rawTemp / LPS25H_LSB_PER_CELSIUS);

  *asl = lps25hPressureToAltitude(pressure);

  return status==0;
}

#include "math.h"
// Constants used to determine altitude from pressure
#define CONST_SEA_PRESSURE 102610.f //1026.1f //http://www.meteo.physik.uni-muenchen.de/dokuwiki/doku.php?id=wetter:stadt:messung
#define CONST_PF 0.1902630958f //(1/5.25588f) Pressure factor
#define CONST_PF2 44330.0f
#define FIX_TEMP 25         // Fixed Temperature. ASL is a function of pressure and temperature, but as the temperature changes so much (blow a little towards the flie and watch it drop 5 degrees) it corrupts the ASL estimates.
                            // TLDR: Adjusting for temp changes does more harm than good.

//TODO: pretty expensive function. Rather smooth the pressure estimates and only call this when needed
/**
 * Converts pressure to altitude above sea level (ASL) in meters
 */
float lps25hPressureToAltitude(float* pressure/*, float* ground_pressure, float* ground_temp*/)
{
    if (*pressure > 0)
    {
        //return (1.f - pow(*pressure / CONST_SEA_PRESSURE, CONST_PF)) * CONST_PF2;
        //return ((pow((1015.7 / *pressure), CONST_PF) - 1.0) * (25. + 273.15)) / 0.0065;
        return ((powf((1015.7f / *pressure), CONST_PF) - 1.0f) * (FIX_TEMP + 273.15f)) / 0.0065f;
    }
    else
    {
        return 0;
    }
}
