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
/* cfg.h: node configuration handling */
#ifndef __CFG_H__
#define __CFG_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  cfgAddress = 0,
  cfgMode,
  cfgAnchorlist,
  cfgAnchorPos,
  cfgSmartPower,
  cfgForceTxPower,
  cfgTxPower,
  cfgLowBitrate,
  cfgLongPreamble,
} ConfigField;

void cfgInit();

bool cfgReset(void);

bool cfgFieldSize(ConfigField field, uint8_t * size);

bool cfgReadU8(ConfigField field, uint8_t * value);

bool cfgWriteU8(ConfigField field, uint8_t data);

bool cfgReadU32(ConfigField field, uint32_t * value);

bool cfgWriteU32(ConfigField field, uint32_t value);

bool cfgReadU8list(ConfigField field, uint8_t list[], uint8_t lenth);

bool cfgWriteU8list(ConfigField field, uint8_t list[], uint8_t lenth);

bool cfgReadFP32listLength(ConfigField field, uint8_t * size);

bool cfgReadFP32list(ConfigField field, float list[], uint8_t length);

bool cfgWriteFP32list(ConfigField field, float list[], uint8_t length);

void cfgSetBinaryMode(bool enable);

bool cfgIsBinaryMode();

#endif
