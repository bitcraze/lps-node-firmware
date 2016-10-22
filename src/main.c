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
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "system.h"
#include "spi.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "led.h"

#include "cfg.h"
#include "eeprom.h"

#include "usb_device.h"
#include "usbcomm.h"

#include "lps25h.h"
#include "test_support.h"
#include "production_test.h"

#include "uwb.h"

const uint8_t *uid = (uint8_t*)MCU_ID_ADDRESS;

static void restConfig();
static void changeAddress(uint8_t addr);
static void handleInput(char ch);
static void changeMode(unsigned int newMode);
static void printModeList();
static void printMode();
static void help();

static void main_task(void *pvParameters) {
  int i;
  char ch;
  bool selftestPasses = true;

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();

  // Light up all LEDs to test
  ledOn(ledRanging);
  ledOn(ledSync);
  ledOn(ledMode);

  printf("\r\n\r\n====================\r\n");

  printf("SYSTEM\t: CPU-ID: ");
  for (i=0; i<12; i++) {
    printf("%02x", uid[i]);
  }
  printf("\r\n");

  // Initializing pressure sensor (if present ...)
  lps25hInit(&hi2c1);
  testSupportPrintStart("Initializing pressure sensor");
  if (lps25hTestConnection()) {
    printf("[OK]\r\n");
    lps25hSetEnabled(true);
  } else {
    printf("[FAIL] (%u)\r\n", (unsigned int)hi2c1.ErrorCode);
    selftestPasses = false;
  }

  testSupportPrintStart("Pressure sensor self-test");
  testSupportReport(&selftestPasses, lps25hSelfTest());

  // Initializing i2c eeprom
  eepromInit(&hi2c1);
  testSupportPrintStart("EEPROM self-test");
  testSupportReport(&selftestPasses, eepromTest());

  cfgInit();

  // Initialising radio
  testSupportPrintStart("Initialize UWB ");
  uwbInit();
  if (uwbTest()) {
    printf("[OK]\r\n");
  } else {
    printf("[ERROR]: %s\r\n", uwbStrError());
    selftestPasses = false;
  }

  if (!selftestPasses) {
    printf("TEST\t: One or more self-tests failed, blocking startup!\r\n");
    usbcommSetSystemStarted(true);
  }

  // Printing UWB configuration
  struct uwbConfig_s * uwbConfig = uwbGetConfig();
  printf("CONFIG\t: Address is 0x%X\r\n", uwbConfig->address[0]);
  printf("CONFIG\t: Mode is %s\r\n", uwbAlgorithmName(uwbConfig->mode));
  printf("CONFIG\t: Tag mode anchor list (%i): ", uwbConfig->anchorListSize);
  for (i = 0; i < uwbConfig->anchorListSize; i++) {
    printf("0x%02X ", uwbConfig->anchors[i]);
  }
  printf("\r\n");

  HAL_Delay(500);

  ledOff(ledRanging);
  ledOff(ledSync);
  ledOff(ledMode);

  printf("SYSTEM\t: Node started ...\r\n");
  printf("SYSTEM\t: Press 'h' for help.\r\n");

  usbcommSetSystemStarted(true);

  // Starts UWB protocol
  uwbStart();

  // Main loop ...
  while(1) {
    usbcommPrintWelcomeMessage();

    ledTick();
    // // Measure pressure
    // if (uwbConfig.mode != modeSniffer) {
    //   if(lps25hGetData(&pressure, &temperature, &asl)) {
    //     pressure_ok = true;
    //   } else {
    //     printf("Fail reading pressure\r\n");
    //     printf("pressure not ok\r\n");
    //   }
    // }

    // Accepts serial commands
#ifdef USE_FTDI_UART
    if (HAL_UART_Receive(&huart1, (uint8_t*)&ch, 1, 0) == HAL_OK) {
#else
    if(usbcommRead(&ch, 1)) {
#endif
      handleInput(ch);
    }
  }
}

/* Function required to use "printf" to print on serial console */
int _write (int fd, const void *buf, size_t count)
{
  // stdout
  if (fd == 1) {
    #ifdef USE_FTDI_UART
      HAL_UART_Transmit(&huart1, (uint8_t *)buf, count, HAL_MAX_DELAY);
    #else
      usbcommWrite(buf, count);
    #endif
  }

  // stderr
  if (fd == 2) {
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, count, HAL_MAX_DELAY);
  }

  return count;
}

static void handleInput(char ch) {
  bool configChanged = true;
  static enum menu_e {mainMenu, modeMenu} currentMenu = mainMenu;

  switch (currentMenu) {
    case mainMenu:
      switch (ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          changeAddress(ch - '0');
          break;
        case 'a': changeMode(MODE_ANCHOR); break;
        case 't': changeMode(MODE_TAG); break;
        case 's': changeMode(MODE_SNIFFER); break;
        case 'm':
          printModeList();
          printf("Type 0-9 to choose new mode...\r\n");
          currentMenu = modeMenu;
          configChanged = false;
          break;
        case 'd': restConfig(); break;
        case 'h':
          help();
          configChanged = false;
          break;
        case '#':
          productionTestsRun();
          printf("System halted, reset to continue\r\n");
          while(true){}
          break;
        default:
          configChanged = false;
          break;
      }
      break;
    case modeMenu:
      switch(ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          changeMode(ch - '0');
          currentMenu = mainMenu;
          break;
        default:
          printf("Incorrect mode '%c'\r\n", ch);
          currentMenu = mainMenu;
          configChanged = false;
          break;
      }
      break;
  }

  if (configChanged) {
    printf("EEPROM configuration changed, restart for it to take effect!\r\n");
  }
}

static void restConfig() {
  printf("Resetting EEPROM configuration...");
  if (cfgReset()) {
    printf("OK\r\n");
  } else {
    printf("ERROR\r\n");
  }
}

static void changeAddress(uint8_t addr) {
  printf("Updating address to 0x%02X\r\n", addr);
  cfgWriteU8(cfgAddress, addr);
  if (cfgReadU8(cfgAddress, &addr)) {
    printf("Device address: 0x%X\r\n", addr);
  } else {
    printf("Device address: Not found!\r\n");
  }
}

static void changeMode(unsigned int newMode) {
    printf("Previous device mode: ");
    printMode();

    cfgWriteU8(cfgMode, newMode);

    printf("New device mode: ");
    printMode();
}

static void printModeList()
{
  unsigned int count = uwbAlgorithmCount();
  int current_mode = -1;
  uint8_t mode;

  if (cfgReadU8(cfgMode, &mode)) {
    current_mode = mode;
  }

  printf("-------------------\r\n");
  printf("Available UWB modes:\r\n");
  for (int i=0; i<count; i++) {
    printf(" %d - %s%s\r\n", i, uwbAlgorithmName(i),
                             (i == current_mode)?" (Current mode)":"");
  }
}

static void printMode() {
  uint8_t mode;

  if (cfgReadU8(cfgMode, &mode)) {
    printf(uwbAlgorithmName(mode));
  } else {
    printf("Not found!");
  }

  printf("\r\n");
}

static void help() {
  printf("Help\r\n");
  printf("-------------------\r\n");
  printf("0-9 - set address\r\n");
  printf("a   - anchor mode\r\n");
  printf("t   - tag mode\r\n");
  printf("s   - sniffer mode\r\n");
  printf("m   - List and change mode\r\n");
  printf("d   - reset configuration\r\n");
  printf("h   - This help\r\n");
}

static StaticTask_t xMainTask;
static StackType_t ucMainStack[configMINIMAL_STACK_SIZE];

int main() {
  // Reset of all peripherals, Initializes the Flash interface and the Systick.
  HAL_Init();

  // Configure the system clock
  SystemClock_Config();

  // Setup main task
  xTaskCreateStatic( main_task, "main", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, ucMainStack, &xMainTask );

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();

  // Should never reach there
  while(1);

  return 0;
}

// Freertos required callbacks
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
  printf("Assert failed at %s:%lu", pcFileName, ulLine);
  while(1);
}
