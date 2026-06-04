/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2026, Bitcraze AB
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
/* service_config.h: service dispatch configuration adapter */

#ifndef __SERVICE_CONFIG_H__
#define __SERVICE_CONFIG_H__

#include <stdbool.h>
#include <stdint.h>

#include "service_dispatch.h"
#include "service_protocol.h"

void serviceConfigInitDispatchConfig(serviceDispatchConfig_t *dispatchConfig);
bool serviceConfigReadSnapshot(serviceConfigPayload_t *snapshot);
bool serviceConfigWritePosition(const float position[3]);
bool serviceConfigWriteRadioMode(uint8_t radioMode);
bool serviceConfigWriteTxPower(const serviceSetTxPowerPayload_t *payload);

#endif // __SERVICE_CONFIG_H__
