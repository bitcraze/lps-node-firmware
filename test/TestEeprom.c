#include "unity.h"
#include "eeprom.h"

#include "mock_stm32f0xx_hal_i2c.h"

static int devAddr = 0xA0;

static I2C_HandleTypeDef handle;
static uint8_t data[1];
static int address = 0x4711;
static size_t length = 4711;

void setUp(void) {
  eepromInit(&handle);
}

void tearDown(void) {
}


void test_eepromReadShouldReturnTrueOnSuccess() {
  // Fixture
  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Read_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  bool actual = eepromRead(address, data, length);

  // Assert
  TEST_ASSERT_TRUE(actual);
}


void test_eepromReadShouldReturnFalseOnFailure() {
  // Fixture
  HAL_StatusTypeDef returnValue = HAL_ERROR;
  HAL_I2C_Mem_Read_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  bool actual = eepromRead(address, data, length);

  // Assert
  TEST_ASSERT_FALSE(actual);
}

void test_eepromWriteShouldReturnTrueOnSuccess() {
  // Fixture
  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  bool actual = eepromWrite(address, data, length);

  // Assert
  TEST_ASSERT_TRUE(actual);
}


void test_eepromWriteShouldReturnFalseOnFailure() {
  // Fixture
  HAL_StatusTypeDef returnValue = HAL_ERROR;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  bool actual = eepromWrite(address, data, length);

  // Assert
  TEST_ASSERT_FALSE(actual);
}
