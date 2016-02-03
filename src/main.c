#include <stm32f0xx_hal.h>
#include "system.h"
#include "spi.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "led.h"

#include "cfg.h"
#include "eeprom.h"

#include "usb_device.h"

#include <string.h>
#include <stdio.h>

#include <libdw.h>
#include "dwOps.h"

#include "mac.h"

#include "lps25h.h"

static CfgMode mode = modeAnchor;

const uint32_t *cpuid = (uint32_t*)0x1FFFF7E8;

int initDwm1000();

#define N_NODES 9

uint8_t address[] = {0,0,0,0,0,0,0xcf,0xbc};
uint8_t base_address[] = {0,0,0,0,0,0,0xcf,0xbc};

// Static system configuration
#define MAX_ANCHORS 6
uint8_t anchors[MAX_ANCHORS];

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

float pressure, temperature, asl;
bool has_pressure = false;
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
static int queue_head = 0;
static int queue_tail = 0;
static volatile uint32_t dropped = 0;

// #define printf(...)
#define debug(...)

void txcallback(dwDevice_t *dev)
{
  dwTime_t departure;
  dwGetTransmitTimestamp(dev, &departure);

  debug("TXCallback: ");

  switch (txPacket.payload[0]) {
    case POLL:
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
  dwTime_t arival;
  int dataLength = dwGetDataLength(dev);
  bzero(&rxPacket, MAC802154_HEADER_LENGTH);

  debug("RXCallback(%d): ", dataLength);

  if (mode == modeSniffer) {
    if ((queue_head+1)%QUEUE_LEN == queue_tail) {
      dropped++;
      dwNewReceive(dev);
      dwSetDefaults(dev);
      dwStartReceive(dev);
      return;
    }

    // Queue the received packet, the main loop will print it on the console
    dwGetData(dev, (uint8_t*)&snifferPacketQueue[queue_head], dataLength);
    snifferPacketLength[queue_head] = dataLength;
    queue_head = (queue_head+1)%QUEUE_LEN;
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
    return;
  }

  dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

  if (memcmp(rxPacket.destAddress, address, 8)) {
    debug("Not for me! for %02x with %02x\r\n", rxPacket.destAddress[0], rxPacket.payload[0]);
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
    return;
  }

  dwGetReceiveTimestamp(dev, &arival);

  memcpy(txPacket.destAddress, rxPacket.sourceAddress, 8);
  memcpy(txPacket.sourceAddress, rxPacket.destAddress, 8);

  switch(rxPacket.payload[TYPE]) {
    // Anchor received messages
    case POLL:
      debug("POLL from %02x at %04x\r\n", rxPacket.sourceAddress[0], (unsigned int)arival.low32);

      poll_rx = arival;

      txPacket.payload[TYPE] = ANSWER;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];

      dwNewTransmit(dev);
      dwSetDefaults(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);
      break;
    case FINAL:
    {
      reportPayload_t *report = (reportPayload_t *)(txPacket.payload+2);

      debug("FINAL\r\n");

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

      answer_rx = arival;

      txPacket.payload[0] = FINAL;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];

      dwNewTransmit(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);
      break;
    case REPORT:
    {
      reportPayload_t *report = (reportPayload_t *)(rxPacket.payload+2);
      double tround1, treply1, treply2, tround2, tprop_ctn, tprop, distance;

      printf("REPORT\r\n");

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

dwDevice_t dwm_device;
dwDevice_t *dwm = &dwm_device;

int main() {
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  int result;
  int i;
  char ch;
  bool selftestPasses = true;

  /* Configure the system clock */
  SystemClock_Config();

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

  // Initializing pressure sensor (if present ...)
  lps25hInit(&hi2c1);
  printf("TEST\t: Initializing pressure sensor ... ");
  if (lps25hTestConnection()) {
    printf("[OK]\r\n");
    lps25hSetEnabled(true);
  } else {
    printf("[FAIL] (%u)\r\n", (unsigned int)hi2c1.ErrorCode);
    selftestPasses = false;
  }

  printf("TEST\t: Pressure sensor self-test ... ");
  if (lps25hSelfTest()) {
    printf("[OK]\r\n");
  } else {
    printf("[FAIL]\r\n");
    selftestPasses = false;
  }

  // Initializing i2c eeprom
  eepromInit(&hi2c1);
  printf("TEST\t: EEPROM self-test ... ");
  if (eepromTest()) {
    printf("[OK]\r\n");
  } else {
    printf("[FAIL]\r\n");
    selftestPasses = false;
  }

  // Initialising radio

  printf("TEST\t: Initialize DWM1000 ... ");
  dwInit(dwm, &dwOps);       // Init libdw
  dwOpsInit(dwm);
  result = dwConfigure(dwm); // Configure the dw1000 chip
  if (result == 0) {
    printf("[OK]\r\n");
    dwEnableAllLeds(dwm);
  } else {
    printf("[ERROR]: %s\r\n", dwStrError(result));
    selftestPasses = false;
  }

  if (!selftestPasses) {
    printf("TEST\t: One or more self-tests failed, blocking startup!\r\n");
    while(1);
  }

  cfgInit();

  if (cfgReadU8(cfgAddress, &address[0])) {
    printf("CONFIG\t: Address is 0x%X\r\n", address[0]);
  } else {
    printf("CONFIG\t: Address not found!\r\n");
  }

  if (cfgReadU8(cfgMode, &mode)) {
    printf("CONFIG\t: Mode is ");
    switch (mode) {
      case modeAnchor: printf("Anchor\r\n"); break;
      case modeTag: printf("Tag\r\n"); break;
      case modeSniffer: printf("Sniffer\r\n"); break;
      default: printf("UNKNOWN\r\n"); break;
    }
  } else {
    printf("Device mode: Not found!\r\n");
  }

  uint8_t anchorListSize = 0;
  if (cfgFieldSize(cfgAnchorlist, &anchorListSize)) {
    if (cfgReadU8list(cfgAnchorlist, (uint8_t*)&anchors, anchorListSize)) {
      printf("CONFIG\t: Tag mode anchor list (%i): ", anchorListSize);
      for (i = 0; i < anchorListSize; i++) {
        printf("0x%02X ", anchors[i]);
      }
      printf("\r\n");
    } else {
      printf("CONFIG\t: Tag mode anchor list: Not found!\r\n");
    }
  }

  HAL_Delay(500);

  ledOff(ledRanging);
  ledOff(ledSync);
  ledOff(ledMode);

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

  printf("SYSTEM\t: Node started ...\r\n");

  // Initialize the packet in the TX buffer
  MAC80215_PACKET_INIT(txPacket, MAC802154_TYPE_DATA);
  txPacket.pan = 0xbccf;

  if (mode == modeAnchor || mode == modeSniffer) {
    dwNewReceive(dwm);
    dwSetDefaults(dwm);
    dwStartReceive(dwm);
  }

  // Main loop ...
  while(1) {
    // Measure pressure
    if (mode != modeSniffer && has_pressure) {
      if(lps25hGetData(&pressure, &temperature, &asl)) {
        pressure_ok = true;
        ledOn(ledRanging);
      } else {
        printf("Fail reading pressure\r\n");
        printf("pressure not ok\r\n");
        ledOff(ledRanging);
      }
    }

    // Accepts serial commands
    if (HAL_UART_Receive(&huart1, (uint8_t*)&ch, 1, 0) == HAL_OK) {
      bool configChanged = false;
      if (ch >= '0' && ch <= '9') {
        printf("Updating address from 0x%02X to 0x%02X\r\n", address[0], ch - '0');
        cfgWriteU8(cfgAddress, ch - '0');
        if (cfgReadU8(cfgAddress, &address[0])) {
          printf("Device address: 0x%X\r\n", address[0]);
        } else {
          printf("Device address: Not found!\r\n");
        }
        configChanged = true;
      }
      if (ch == 'A' || ch == 'T' || ch == 'S' || ch == 'a' || ch == 't' || ch == 's') {
        // Print current mode
        if (cfgReadU8(cfgMode, &mode)) {
          printf("Previous device mode: ");
          switch (mode) {
            case modeAnchor: printf("Anchor\r\n"); break;
            case modeTag: printf("Tag\r\n"); break;
            case modeSniffer: printf("Sniffer\r\n"); break;
            default: printf("UNKNOWN\r\n"); break;
          }
        } else {
          printf("Previous device mode: Not found!\r\n");
        }

        // Write new changes
        switch (ch) {
          case 'A': case 'a': cfgWriteU8(cfgMode, modeAnchor); break;
          case 'T': case 't': cfgWriteU8(cfgMode, modeTag); break;
          case 'S': case 's': cfgWriteU8(cfgMode, modeSniffer); break;
        }

        // Print out to verify
        if (cfgReadU8(cfgMode, &mode)) {
          printf("New device mode: ");
          switch (mode) {
            case modeAnchor: printf("Anchor\r\n"); break;
            case modeTag: printf("Tag\r\n"); break;
            case modeSniffer: printf("Sniffer\r\n"); break;
            default: printf("UNKNOWN\r\n"); break;
          }
        } else {
          printf("New device mode: Not found!\r\n");
        }
        configChanged = true;
      }

      if (ch == 'D' || ch == 'd') {
        configChanged = true;
        printf("Resetting EEPROM configuration...");
        if (cfgReset())
          printf("OK\r\n");
        else
          printf("ERROR\r\n");
      }
      if (configChanged) {
        printf("EEPROM configuration changed, restart for it to take effect!\r\n");
      }

    }

    if (mode == modeSniffer) {
      static uint32_t prevDropped = 0;
      ledOn(ledMode);
      ledOff(ledSync);
      ledOff(ledRanging);

      if (dropped != prevDropped) {
        printf("Dropped!\r\n");
        prevDropped = dropped;
      }

      if (queue_tail != queue_head) {
        printf("From %02x to %02x data ", snifferPacketQueue[queue_tail].sourceAddress[0],
                                          snifferPacketQueue[queue_tail].destAddress[0]);
        for (int i=0; i<(snifferPacketLength[queue_tail] - MAC802154_HEADER_LENGTH); i++) {
          printf("0x%02x ", snifferPacketQueue[queue_tail].payload[i]);
        }
        queue_tail = (queue_tail+1)%QUEUE_LEN;
        printf("\r\n");
      }

      continue;
    }

    ledOff(ledMode);

    ledOn(ledSync);
    HAL_Delay(200);
    ledOff(ledSync);
    HAL_Delay(200);

    if (mode == modeTag) {
       for (i=0; i<anchorListSize; i++) {
         printf ("Interrogating anchor %d\r\n", anchors[i]);
         base_address[0] = anchors[i];
         dwIdle(dwm);

         txPacket.payload[TYPE] = POLL;
         txPacket.payload[SEQ] = ++curr_seq;

         memcpy(txPacket.sourceAddress, address, 8);
         memcpy(txPacket.destAddress, base_address, 8);

         dwNewTransmit(dwm);
         dwSetDefaults(dwm);
         dwSetData(dwm, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2);

         dwWaitForResponse(dwm, true);
         dwStartTransmit(dwm);

         HAL_Delay(30);
       }
     }
  }

  return 0;
}

/* Function required to use "printf" to print on serial console */
int _write (int fd, const void *buf, size_t count)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, count, HAL_MAX_DELAY);
  return count;
}
