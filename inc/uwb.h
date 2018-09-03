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
/* uwb.h: Uwb radio implementation */

#ifndef __UWB_H__
#define __UWB_H__

#include <stdbool.h>
#include <stdint.h>
#include "cfg.h"
#include <libdw1000.h>

#define MAX_ANCHORS 6

typedef struct uwbConfig_s {
  uint8_t mode;
  uint8_t address[8];
  uint8_t anchorListSize;
  uint8_t anchors[MAX_ANCHORS];
  float position[3];
  bool positionEnabled;

  bool smartPower;
  bool forceTxPower;
  uint32_t txPower;

  bool lowBitrate;
  bool longPreamble;
} uwbConfig_t;

#define MODE_ANCHOR 0
#define MODE_TAG 1
#define MODE_SNIFFER 2
#define MODE_TDOA_ANCHOR2 3
#define MODE_TDOA_ANCHOR3 4

typedef enum uwbEvent_e {
  eventTimeout,
  eventPacketReceived,
  eventPacketSent,
  eventReceiveTimeout,
  eventReceiveFailed,
} uwbEvent_t;

// Callback for one uwb algorithm
typedef struct uwbAlgorithm_s {
  void (*init)(uwbConfig_t * config, dwDevice_t *dev);
  uint32_t (*onEvent)(dwDevice_t *dev, uwbEvent_t event);
} uwbAlgorithm_t;

#include <FreeRTOS.h>

#define MAX_TIMEOUT portMAX_DELAY

void uwbInit();
bool uwbTest();
void uwbStart();
char * uwbStrError();
struct uwbConfig_s * uwbGetConfig();
int uwbAlgorithmCount();
char * uwbAlgorithmName(unsigned int id);

#endif //__UWB_H__
