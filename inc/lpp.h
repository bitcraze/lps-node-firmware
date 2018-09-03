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
#ifndef __LPP_H__
#define __LPP_H__

#include <stddef.h>
#include <stdint.h>

void lppHandleShortPacket(char *data, size_t length);

#define SHORT_LPP 0xF0

#define LPP_SHORT_ANCHOR_POSITION 0x01
#define LPP_SHORT_REBOOT 0x02
#define LPP_SHORT_MODE 0x03
#define LPP_SHORT_UWB 0x04
#define LPP_SHORT_UWB_MODE 0x05

struct lppShortAnchorPosition_s {
  float position[3];
} __attribute__((packed));

#define LPP_SHORT_REBOOT_TO_BOOTLOADER 0x00
#define LPP_SHORT_REBOOT_TO_FIRMWARE 0x01

struct lppShortReboot_s {
  uint8_t bootMode;
} __attribute__((packed));

#define LPP_SHORT_MODE_TWR 0x01
#define LPP_SHORT_MODE_TDOA2 0x02
#define LPP_SHORT_MODE_TDOA3 0x03

struct lppShortMode_s {
  uint8_t mode;
} __attribute__((packed));

struct lppShortUWB_s {
  uint8_t enableSmartPower :1;
  uint8_t forceTxPower :1;
  uint32_t txPower;
} __attribute__((packed));

struct lppShortUWBMode_s {
  uint8_t enableLowBitrate :1;
  uint8_t enableLongPreamble :1;
} __attribute__((packed));

#endif //__LPP_H__
