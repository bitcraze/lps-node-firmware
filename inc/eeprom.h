#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stm32f0xx_hal.h>

#include <stdbool.h>

void eepromInit(I2C_HandleTypeDef * i2c);

bool eepromRead(int address, void* data, size_t length);

bool eepromWrite(int address, void* data, size_t length);

bool eepromTest();

#endif
