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
/* uwb_sniffer.c: Uwb sniffer implementation */

#include "uwb.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "cfg.h"
#include "led.h"

#include "libdw1000.h"

#include "dwOps.h"
#include "mac.h"


static uint32_t twrAnchorOnEvent(dwDevice_t *dev, uwbEvent_t event)
{
  static dwTime_t arrival;
  static packet_t rxPacket;

  if (event == eventPacketReceived) {
    int dataLength = dwGetDataLength(dev);
    dwGetRawReceiveTimestamp(dev, &arrival);
    dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);

    if (cfgIsBinaryMode()) {
      write(STDOUT_FILENO, "\xbc", 1);
      write(STDOUT_FILENO, &arrival.full, 5);
      write(STDOUT_FILENO, &rxPacket.sourceAddress[0], 1);
      write(STDOUT_FILENO, &rxPacket.destAddress[0], 1);
      dataLength -= MAC802154_HEADER_LENGTH;
      write(STDOUT_FILENO, &dataLength, 2);
      write(STDOUT_FILENO, rxPacket.payload, dataLength);
      write(STDOUT_FILENO, &dataLength, 2);  // Length repeated for sync detection
    } else {
      printf("From %02x to %02x @%02x%08x: ", rxPacket.sourceAddress[0],
                                            rxPacket.destAddress[0],
                                            (unsigned int) arrival.high8,
                                            (unsigned int) arrival.low32);
      for (int i=0; i<(dataLength - MAC802154_HEADER_LENGTH); i++) {
        printf("%02x", rxPacket.payload[i]);
      }
      printf("\r\n");
    }

  } else {
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
  }

  return MAX_TIMEOUT;
}

static void twrAnchorInit(uwbConfig_t * newconfig, dwDevice_t *dev)
{
  // Set the LED for anchor mode
  ledBlink(ledMode, false);
}

uwbAlgorithm_t uwbSnifferAlgorithm = {
  .init = twrAnchorInit,
  .onEvent = twrAnchorOnEvent,
};
