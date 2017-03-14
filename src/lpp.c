/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2017, Bitcraze AB
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
#include <stdio.h>

#include "stm32f0xx_hal.h"

#include "lpp.h"

#include "cfg.h"
#include "uwb.h"

#include "bootmode.h"

#define debug(...) // printf(__VA_ARGS__)

void lppHandleShortPacket(char *data, size_t length)
{
  if (length < 1) return;

  int type  = data[0];

  debug("Handling LPP short packet of type %02x, length %d\r\n", type, length);
  debug("Raw data: ");
  for (int i=0; i<length; i++) {
    printf("%02x ", data[i]);
  }
  debug("\r\n");

  switch(type) {
    case LPP_SHORT_ANCHOR_POSITION:
    {
      struct lppShortAnchorPosition_s* newpos = (struct lppShortAnchorPosition_s*)&data[1];

      if (length != 3*sizeof(float) + 1) {
        debug("LPP: Wrong set-anchor position length\r\n");
        break;
      }

      cfgWriteFP32list(cfgAnchorPos, newpos->position, 3);
      uwbConfig_t *uwbConfig = uwbGetConfig();
      uwbConfig->position[0] = newpos->position[0];
      uwbConfig->position[1] = newpos->position[1];
      uwbConfig->position[2] = newpos->position[2];
      uwbConfig->positionEnabled = true;

      printf("Setting new anchor position to %f, %f, %f\r\n", newpos->position[0],
                                                              newpos->position[1],
                                                              newpos->position[2]);
      break;
    }
    case LPP_SHORT_REBOOT:
    {
      struct lppShortReboot_s* rebootInfo = (struct lppShortReboot_s*)&data[1];

      // Set boot flags
      if (rebootInfo->bootMode == LPP_SHORT_REBOOT_TO_BOOTLOADER) {
        bootmodeSetBootloaderModeFlag();
      } else if (rebootInfo->bootMode == LPP_SHORT_REBOOT_TO_FIRMWARE) {
        bootmodeClearBootloaderModeFlag();
      }

      // Then resets!
      NVIC_SystemReset();

      break;
    }
  }
}
