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
/* uwb.c: Uwb radio implementation */

#include "uwb.h"

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "cfg.h"
#include "led.h"

#include "libdw1000.h"

#include "dwOps.h"
#include "mac.h"

static SemaphoreHandle_t irqSemaphore;


static bool isInit = false;
static int uwbErrorCode = 0;

uint8_t base_address[] = {0,0,0,0,0,0,0xcf,0xbc};

// Static system configuration
static struct uwbConfig_s config = {
  mode: modeAnchor,
  address: {0,0,0,0,0,0,0xcf,0xbc},
};

// The four packets for ranging
#define POLL 0x01   // Poll is initiated by the tag
#define ANSWER 0x02
#define FINAL 0x03
#define REPORT 0x04 // Report contains all measurement from the anchor

typedef struct {
  uint8_t pollRx[5];
  uint8_t answerTx[5];
  uint8_t finalRx[5];

  float pressure;
  float temperature;
  float asl;
  uint8_t pressure_ok;
} __attribute__((packed)) reportPayload_t;

typedef union timestamp_u {
  uint8_t raw[5];
  uint64_t full;
  struct {
    uint32_t low32;
    uint8_t high8;
  } __attribute__((packed));
  struct {
    uint8_t low8;
    uint32_t high32;
  } __attribute__((packed));
} timestamp_t;

// Timestamps for ranging
dwTime_t poll_tx;
dwTime_t poll_rx;
dwTime_t answer_tx;
dwTime_t answer_rx;
dwTime_t final_tx;
dwTime_t final_rx;

uint32_t rangingTick;

float pressure, temperature, asl;
bool pressure_ok;

const double C = 299792458.0;       // Speed of light
const double tsfreq = 499.2e6 * 128;  // Timestamp counter frequency

#define ANTENNA_OFFSET 154.6   // In meter
#define ANTENNA_DELAY  (ANTENNA_OFFSET*499.2e6*128)/299792458.0 // In radio tick

static packet_t rxPacket;
static packet_t txPacket;
static volatile uint8_t curr_seq = 0;

// Sniffer queue
#define QUEUE_LEN 16
static packet_t snifferPacketQueue[QUEUE_LEN];
static int snifferPacketLength[QUEUE_LEN];
static dwTime_t snifferPacketTime[QUEUE_LEN];
static int queue_head = 0;
static int queue_tail = 0;
static volatile uint32_t dropped = 0;


// #define printf(...)
#define debug(...) // printf(__VA_ARGS__)

void txcallback(dwDevice_t *dev)
{
  dwTime_t departure;
  dwGetTransmitTimestamp(dev, &departure);

  debug("TXCallback: ");

  switch (txPacket.payload[0]) {
    case POLL:
      rangingTick = HAL_GetTick();
      debug("POLL\r\n");
      poll_tx = departure;
      break;
    case ANSWER:
    debug("ANSWER to %02x at %04x\r\n", txPacket.destAddress[0], (unsigned int)departure.low32);
      answer_tx = departure;
      break;
    case FINAL:
      debug("FINAL\r\n");
      final_tx = departure;
      break;
    case REPORT:
      debug("REPORT\r\n");
      break;
  }
}

#define TYPE 0
#define SEQ 1

void rxcallback(dwDevice_t *dev) {
  dwTime_t arival = { .full=0 };
  int dataLength = dwGetDataLength(dev);

  if (dataLength == 0) return;

  bzero(&rxPacket, MAC802154_HEADER_LENGTH);

  debug("RXCallback(%d): ", dataLength);

  if (config.mode == modeSniffer) {
    if ((queue_head+1)%QUEUE_LEN == queue_tail) {
      dropped++;
      dwNewReceive(dev);
      dwSetDefaults(dev);
      dwStartReceive(dev);
      return;
    }

    // Queue the received packet, the main loop will print it on the console
    dwGetReceiveTimestamp(dev, &arival);
    dwGetData(dev, (uint8_t*)&snifferPacketQueue[queue_head], dataLength);
    snifferPacketLength[queue_head] = dataLength;
    snifferPacketTime[queue_head].full = arival.full;
    queue_head = (queue_head+1)%QUEUE_LEN;
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
    return;
  }

  dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

  if (memcmp(rxPacket.destAddress, config.address, 8)) {
    debug("Not for me! for %02x with %02x\r\n", rxPacket.destAddress[0], rxPacket.payload[0]);
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
    return;
  }



  memcpy(txPacket.destAddress, rxPacket.sourceAddress, 8);
  memcpy(txPacket.sourceAddress, rxPacket.destAddress, 8);

  switch(rxPacket.payload[TYPE]) {
    // Anchor received messages
    case POLL:
      debug("POLL from %02x at %04x\r\n", rxPacket.sourceAddress[0], (unsigned int)arival.low32);
      rangingTick = HAL_GetTick();

      txPacket.payload[TYPE] = ANSWER;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];

      dwNewTransmit(dev);
      dwSetDefaults(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);

      dwGetReceiveTimestamp(dev, &arival);
      poll_rx = arival;
      break;
    case FINAL:
    {
      reportPayload_t *report = (reportPayload_t *)(txPacket.payload+2);

      debug("FINAL\r\n");

      dwGetReceiveTimestamp(dev, &arival);
      final_rx = arival;

      txPacket.payload[TYPE] = REPORT;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];
      memcpy(&report->pollRx, &poll_rx, 5);
      memcpy(&report->answerTx, &answer_tx, 5);
      memcpy(&report->finalRx, &final_rx, 5);
      report->pressure = pressure;
      report->temperature = temperature;
      report->asl = asl;
      report->pressure_ok = pressure_ok;

      dwNewTransmit(dev);
      dwSetDefaults(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2+sizeof(reportPayload_t));

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);

      break;
    }
    // Tag received messages
    case ANSWER:
      debug("ANSWER\r\n");

      if (rxPacket.payload[SEQ] != curr_seq) {
        debug("Wrong sequence number!\r\n");
        return;
      }

      txPacket.payload[0] = FINAL;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];

      dwNewTransmit(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);

      dwGetReceiveTimestamp(dev, &arival);
      answer_rx = arival;
      break;
    case REPORT:
    {
      reportPayload_t *report = (reportPayload_t *)(rxPacket.payload+2);
      double tround1, treply1, treply2, tround2, tprop_ctn, tprop, distance;

      debug("REPORT\r\n");

      if (rxPacket.payload[SEQ] != curr_seq) {
        printf("Wrong sequence number!\r\n");
        return;
      }

      memcpy(&poll_rx, &report->pollRx, 5);
      memcpy(&answer_tx, &report->answerTx, 5);
      memcpy(&final_rx, &report->finalRx, 5);

      printf("%02x%08x ", (unsigned int)poll_tx.high8, (unsigned int)poll_tx.low32);
      printf("%02x%08x\r\n", (unsigned int)poll_rx.high8, (unsigned int)poll_rx.low32);
      printf("%02x%08x ", (unsigned int)answer_tx.high8, (unsigned int)answer_tx.low32);
      printf("%02x%08x\r\n", (unsigned int)answer_rx.high8, (unsigned int)answer_rx.low32);
      printf("%02x%08x ", (unsigned int)final_tx.high8, (unsigned int)final_tx.low32);
      printf("%02x%08x\r\n", (unsigned int)final_rx.high8, (unsigned int)final_rx.low32);

      tround1 = answer_rx.low32 - poll_tx.low32;
      treply1 = answer_tx.low32 - poll_rx.low32;
      tround2 = final_rx.low32 - answer_tx.low32;
      treply2 = final_tx.low32 - answer_rx.low32;

      printf("%08x %08x\r\n", (unsigned int)tround1, (unsigned int)treply2);
      printf("\\    /   /     \\\r\n");
      printf("%08x %08x\r\n", (unsigned int)treply1, (unsigned int)tround2);

      tprop_ctn = ((tround1*tround2) - (treply1*treply2)) / (tround1 + tround2 + treply1 + treply2);

      printf("TProp (ctn): %d\r\n", (unsigned int)tprop_ctn);

      tprop = tprop_ctn/tsfreq;
      distance = C * tprop;

      printf("distance %d: %5dmm\r\n", rxPacket.sourceAddress[0], (unsigned int)(distance*1000));

      dwGetReceiveTimestamp(dev, &arival);
      printf("Total in-air time (ctn): 0x%08x\r\n", (unsigned int)(arival.low32-poll_tx.low32));

      break;
    }
  }
}

bool contains(int* list, int length, int value)
{
  int i;

  for (i=0; i<length; i++) {
    if (list[i] == value) {
      return true;
    }
  }

  return false;
}

static int checkIrq()
{
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}

dwDevice_t dwm_device;
dwDevice_t *dwm = &dwm_device;

static void uwbTask(void* parameters)
{
  configASSERT(isInit);
  // Initialize the packet in the TX buffer
  MAC80215_PACKET_INIT(txPacket, MAC802154_TYPE_DATA);
  txPacket.pan = 0xbccf;

  if (config.mode == modeAnchor || config.mode == modeSniffer) {
    dwNewReceive(dwm);
    dwSetDefaults(dwm);
    dwStartReceive(dwm);
  }

  bool ledState = false;
  uint32_t ledTick = 0;

  while(1) {

    if (xSemaphoreTake(irqSemaphore, 10/portTICK_PERIOD_MS)) {
      do{
          dwHandleInterrupt(dwm);
      } while(checkIrq() != 0); //while IRS line active (ARM can only do edge sensitive interrupts)
    }

    if (config.mode == modeSniffer) {
      static uint32_t prevDropped = 0;

      if (dropped != prevDropped) {
        printf("Dropped!\r\n");
        prevDropped = dropped;
      }

      if (queue_tail != queue_head) {
        printf("From %02x to %02x @%02x%08x: ", snifferPacketQueue[queue_tail].sourceAddress[0],
                                          snifferPacketQueue[queue_tail].destAddress[0],
                                          (unsigned int) snifferPacketTime[queue_tail].high8,
                                          (unsigned int) snifferPacketTime[queue_tail].low32);
        for (int i=0; i<(snifferPacketLength[queue_tail] - MAC802154_HEADER_LENGTH); i++) {
          printf("%02x", snifferPacketQueue[queue_tail].payload[i]);
        }
        queue_tail = (queue_tail+1)%QUEUE_LEN;
        printf("\r\n");
      }
    }

    if (config.mode == modeTag) {
       for (int i=0; i<config.anchorListSize; i++) {
         printf ("Interrogating anchor %d\r\n", config.anchors[i]);
         base_address[0] = config.anchors[i];
         dwIdle(dwm);

         txPacket.payload[TYPE] = POLL;
         txPacket.payload[SEQ] = ++curr_seq;

         memcpy(txPacket.sourceAddress, config.address, 8);
         memcpy(txPacket.destAddress, base_address, 8);

         dwNewTransmit(dwm);
         dwSetDefaults(dwm);
         dwSetData(dwm, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

         dwWaitForResponse(dwm, true);
         dwStartTransmit(dwm);

         HAL_Delay(30);
       }
     }

    // Handling of the LEDs
    if (HAL_GetTick() > (ledTick+250)) {
      ledTick = HAL_GetTick();
      ledState = !ledState;
    }

     if ((HAL_GetTick() < (rangingTick+50)) && ledState) {
       ledOn(ledRanging);
     } else {
       ledOff(ledRanging);
     }

     ledOff(ledSync);

     switch (config.mode) {
       case modeTag:
         ledOff(ledMode);
         break;
       case modeAnchor:
         ledOn(ledMode);
         break;
       case modeSniffer:
         if (ledState) {
           ledOn(ledMode);
         } else {
           ledOff(ledMode);
         }
         break;
       default:
         ledOn(ledMode);
         ledOn(ledSync);
         ledOn(ledRanging);
         break;
     }
  }
}

void uwbInit()
{
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

  // Reading and setting node configuration
  cfgReadU8(cfgAddress, &config.address[0]);
  cfgReadU8(cfgMode, &config.mode);
  cfgFieldSize(cfgAnchorlist, &config.anchorListSize);
  if (config.anchorListSize <= MAX_ANCHORS) {
    cfgReadU8list(cfgAnchorlist, config.anchors, config.anchorListSize);
  }

  dwTime_t delay = {.full = ANTENNA_DELAY/2};
  dwSetAntenaDelay(dwm, delay);

  dwAttachSentHandler(dwm, txcallback);
  dwAttachReceivedHandler(dwm, rxcallback);

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

void uwbStart()
{
  static StaticTask_t uwbStaticTask;
  static StackType_t uwbStaticStack[configMINIMAL_STACK_SIZE];

  if (isInit) {
    xTaskCreateStatic(uwbTask, "uwb", configMINIMAL_STACK_SIZE, NULL,
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
#include <stm32f0xx_hal.h>
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
