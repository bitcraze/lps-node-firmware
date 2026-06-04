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
/* service_config.c: service dispatch configuration adapter */

#include "service_config.h"

#include "cfg.h"
#include "uwb.h"

void serviceConfigInitDispatchConfig(serviceDispatchConfig_t *dispatchConfig) {
  if (dispatchConfig == NULL) {
    return;
  }

  uwbConfig_t *uwbConfig = uwbGetConfig();

  dispatchConfig->nodeId = uwbConfig->address[0];
  dispatchConfig->readConfig = serviceConfigReadSnapshot;
  dispatchConfig->writePosition = serviceConfigWritePosition;
  dispatchConfig->writeRadioMode = serviceConfigWriteRadioMode;
  dispatchConfig->writeTxPower = serviceConfigWriteTxPower;
}

bool serviceConfigReadSnapshot(serviceConfigPayload_t *snapshot) {
  if (snapshot == NULL) {
    return false;
  }

  uwbConfig_t *uwbConfig = uwbGetConfig();

  snapshot->nodeId = uwbConfig->address[0];
  snapshot->mode = uwbConfig->mode;
  snapshot->positionEnabled = uwbConfig->positionEnabled;
  snapshot->position[0] = uwbConfig->position[0];
  snapshot->position[1] = uwbConfig->position[1];
  snapshot->position[2] = uwbConfig->position[2];
  snapshot->smartPower = uwbConfig->smartPower;
  snapshot->forceTxPower = uwbConfig->forceTxPower;
  snapshot->txPower = uwbConfig->txPower;
  snapshot->lowBitrate = uwbConfig->lowBitrate;
  snapshot->longPreamble = uwbConfig->longPreamble;
  snapshot->serviceProtocolVersion = SERVICE_PROTOCOL_VERSION;

  return true;
}

bool serviceConfigWritePosition(const float position[3]) {
  if (position == NULL) {
    return false;
  }

  float positionCopy[3] = {
    position[0],
    position[1],
    position[2],
  };

  if (!cfgWriteFP32list(cfgAnchorPos, positionCopy, 3)) {
    return false;
  }

  uwbConfig_t *uwbConfig = uwbGetConfig();

  uwbConfig->position[0] = position[0];
  uwbConfig->position[1] = position[1];
  uwbConfig->position[2] = position[2];
  uwbConfig->positionEnabled = true;

  return true;
}

bool serviceConfigWriteRadioMode(uint8_t radioMode) {
  if (radioMode > 3) {
    return false;
  }

  // cfg has no atomic multi-field write or rollback; only success means all fields were applied.
  bool lowBitrateWritten = cfgWriteU8(cfgLowBitrate, radioMode & 1);
  bool longPreambleWritten = cfgWriteU8(cfgLongPreamble, (radioMode & 2) >> 1);

  return lowBitrateWritten && longPreambleWritten;
}

bool serviceConfigWriteTxPower(const serviceSetTxPowerPayload_t *payload) {
  if (payload == NULL) {
    return false;
  }

  // cfg has no atomic multi-field write or rollback; only success means all fields were applied.
  bool smartPowerWritten = cfgWriteU8(cfgSmartPower, payload->enableSmartPower != 0 ? 1 : 0);
  bool forceTxPowerWritten = cfgWriteU8(cfgForceTxPower, payload->forceTxPower != 0 ? 1 : 0);
  bool txPowerWritten = cfgWriteU32(cfgTxPower, payload->txPower);

  return smartPowerWritten && forceTxPowerWritten && txPowerWritten;
}
