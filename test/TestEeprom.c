#include "unity.h"
#include "eeprom.h"

#include "mock_stm32f0xx_hal_i2c.h"

static int devAddr = 0xA0;

static I2C_HandleTypeDef handle;
static uint8_t data[96] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi ut eleifend purus, at massa nunc.";
static int address = 0x4711;
static size_t length = 4711;

static bool ernableDelayTest = false;
static int vTaskDelayCallCount = 0;
static int vTaskDelayRequiredDelay = 0;

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


void test_eepromWriteShouldDelay10msForEachWrittenPage() {
  // Fixture
  int address = 0;
  int length = 48;

  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Write_IgnoreAndReturn(returnValue);

  ernableDelayTest = true;
  vTaskDelayCallCount = 0;
  vTaskDelayRequiredDelay = 10;

  // Test
  eepromWrite(address, data, length);

  // Assert
  TEST_ASSERT_EQUAL(vTaskDelayCallCount, 2);
}


void test_eepromWriteShouldReturnTrueOnSuccess() {
  // Fixture
  int address = 0;
  int length = 1;

  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Write_IgnoreAndReturn(returnValue);

  // Test
  bool actual = eepromWrite(address, data, length);

  // Assert
  TEST_ASSERT_TRUE(actual);
}


void test_eepromWriteSinglePageShouldMakeOneI2cTransaction() {
  // Fixture
  int address = 0;
  int length = 7;

  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  eepromWrite(address, data, length);
}


void test_eepromWriteFullSinglePageShouldMakeOneI2cTransaction() {
  // Fixture
  int address = 0;
  int length = 32;

  HAL_StatusTypeDef returnValue = HAL_OK;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address, I2C_MEMADD_SIZE_16BIT, data, length, 100, returnValue);

  // Test
  eepromWrite(address, data, length);
}


void test_eepromWriteBetweenTwoPagesShouldMakeTwoI2cTransactions() {
  // Fixture
  int startAddress = 16;
  int totalLength = 40;
  HAL_StatusTypeDef returnValue = HAL_OK;

  int address1 = 16, length1 = 16;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address1, I2C_MEMADD_SIZE_16BIT, data, length1, 100, returnValue);
  int address2 = 32, length2 = 24;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address2, I2C_MEMADD_SIZE_16BIT, &data[length1], length2, 100, returnValue);

  // Test
  eepromWrite(startAddress, data, totalLength);
}


void test_eepromWriteThreePagesShouldMakeThreeI2cTransaction() {
  // Fixture
  int startAddress = 0;
  int totalLength = 96;
  HAL_StatusTypeDef returnValue = HAL_OK;

  int address1 = 0, length1 = 32;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address1, I2C_MEMADD_SIZE_16BIT, data, length1, 100, returnValue);
  int address2 = 32, length2 = 32;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address2, I2C_MEMADD_SIZE_16BIT, &data[length1], length2, 100, returnValue);
  int address3 = 64, length3 = 32;
  HAL_I2C_Mem_Write_ExpectAndReturn(&handle, devAddr, address3, I2C_MEMADD_SIZE_16BIT, &data[length1+length2], length3, 100, returnValue);

  // Test
  eepromWrite(startAddress, data, totalLength);
}


void test_eepromWriteShouldReturnFalseOnFailure() {
  // Fixture
  HAL_StatusTypeDef returnValue = HAL_ERROR;
  HAL_I2C_Mem_Write_IgnoreAndReturn(returnValue);

  // Test
  eepromWrite(address, data, length);
}

////////////////////////////////////////////////////////////////////////////////

void vTaskDelay(uint32_t delay)
{
  if (ernableDelayTest) {
    TEST_ASSERT_EQUAL(delay, vTaskDelayRequiredDelay);
    vTaskDelayCallCount++;
  }
}
