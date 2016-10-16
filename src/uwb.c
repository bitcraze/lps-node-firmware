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
/* uwb.c: Uwb radio implementation, low level handling */

#include <stm32f0xx_hal.h>

#include "uwb.h"

#include "libdw1000.h"
#include "dwOps.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

// Static algorithm
extern uwbAlgorithm_t uwbTwrAnchorAlgorithm;
extern uwbAlgorithm_t uwbTwrTagAlgorithm;
extern uwbAlgorithm_t uwbSnifferAlgorithm;
static uwbAlgorithm_t *algorithm = &uwbTwrAnchorAlgorithm;

// Low level radio handling context (to be separated)
static bool isInit = false;
static int uwbErrorCode = 0;
static SemaphoreHandle_t irqSemaphore;
static dwDevice_t dwm_device;
static dwDevice_t *dwm = &dwm_device;

// Static system configuration
static struct uwbConfig_s config = {
  mode: modeAnchor,
  address: {0,0,0,0,0,0,0xcf,0xbc},
};

static uint32_t timeout;

static void txcallback(dwDevice_t *dev)
{
  timeout = algorithm->onEvent(dev, eventPacketSent);
}

static void rxcallback(dwDevice_t *dev)
{
  timeout = algorithm->onEvent(dev, eventPacketReceived);
}

static void rxTimeoutCallback(dwDevice_t * dev) {
  timeout = algorithm->onEvent(dev, eventReceiveTimeout);
}

static void rxfailedcallback(dwDevice_t *dev) {
  timeout = algorithm->onEvent(dev, eventReceiveFailed);
}


void uwbInit()
{
  // Initializing the low level radio handling
  static StaticSemaphore_t irqSemaphoreBuffer;
  irqSemaphore = xSemaphoreCreateBinaryStatic(&irqSemaphoreBuffer);

  dwInit(dwm, &dwOps);       // Init libdw
  dwOpsInit(dwm);
  uwbErrorCode = dwConfigure(dwm); // Configure the dw1000 chip
  if (uwbErrorCode == 0) {
    dwEnableAllLeds(dwm);
  } else {
    return;
  }
  dwTime_t delay = {.full = 0};
  dwSetAntenaDelay(dwm, delay);

  // Reading and setting node configuration
  cfgReadU8(cfgAddress, &config.address[0]);
  cfgReadU8(cfgMode, &config.mode);
  cfgFieldSize(cfgAnchorlist, &config.anchorListSize);
  if (config.anchorListSize <= MAX_ANCHORS) {
    cfgReadU8list(cfgAnchorlist, config.anchors, config.anchorListSize);
  }

  switch(config.mode) {
    case modeAnchor:
      algorithm = &uwbTwrAnchorAlgorithm;
      break;
    case modeTag:
      algorithm = &uwbTwrTagAlgorithm;
      break;
    case modeSniffer:
      algorithm = &uwbSnifferAlgorithm;
      break;
  }

  dwAttachSentHandler(dwm, txcallback);
  dwAttachReceivedHandler(dwm, rxcallback);
  dwAttachReceiveTimeoutHandler(dwm, rxTimeoutCallback);
  dwAttachReceiveFailedHandler(dwm, rxfailedcallback);

  dwNewConfiguration(dwm);
  dwSetDefaults(dwm);
  dwEnableMode(dwm, MODE_SHORTDATA_FAST_ACCURACY);
  dwSetChannel(dwm, CHANNEL_2);
  dwSetPreambleCode(dwm, PREAMBLE_CODE_64MHZ_9);

  dwCommitConfiguration(dwm);

  isInit = true;
}

bool uwbTest()
{
  return isInit;
}

static int checkIrq()
{
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}

static void uwbTask(void* parameters)
{
  configASSERT(isInit);

  algorithm->init(&config, dwm);

  while(1) {
    if (xSemaphoreTake(irqSemaphore, timeout/portTICK_PERIOD_MS)) {
      do{
          dwHandleInterrupt(dwm);
      } while(checkIrq() != 0); //while IRS line active (ARM can only do edge sensitive interrupts)
    } else {
      timeout = algorithm->onEvent(dwm, eventTimeout);
    }
  }
}

void uwbStart()
{
  static StaticTask_t uwbStaticTask;
  static StackType_t uwbStaticStack[2*configMINIMAL_STACK_SIZE];

  if (isInit) {
    xTaskCreateStatic(uwbTask, "uwb", 2*configMINIMAL_STACK_SIZE, NULL,
                      configMAX_PRIORITIES - 1, uwbStaticStack, &uwbStaticTask);
  }
}

char * uwbStrError()
{
  return dwStrError(uwbErrorCode);
}

struct uwbConfig_s * uwbGetConfig()
{
  return &config;
}

/**** DWM1000 interrupt handling *****/
#define DWM_IRQn EXTI0_1_IRQn
#define DWM_IRQ_PIN GPIO_PIN_0

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  switch (GPIO_Pin) {
    case DWM_IRQ_PIN:
      xSemaphoreGiveFromISR(irqSemaphore, &higherPriorityTaskWoken);

      HAL_NVIC_ClearPendingIRQ(DWM_IRQn);
      break;
    default:
      break;
  }
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
