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
#include "service_shell.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service_controller.h"
#include "service_protocol.h"

#define SERVICE_SHELL_LINE_LENGTH 80
#define SERVICE_SHELL_DEFAULT_TX_POWER 0x07274767UL
#define SERVICE_SHELL_MAX_PRINTABLE_POSITION_M 2147483.0f
// Keep command handlers in separate stack frames under -O3; the main task stack
// is intentionally small and the formatted service replies use large buffers.
#define SERVICE_SHELL_NOINLINE __attribute__((noinline))

typedef struct {
  bool active;
  bool previousWasCr;
  bool lineOverflow;
  char line[SERVICE_SHELL_LINE_LENGTH];
  uint8_t lineLength;
  serviceShellWriteFn_t write;
  serviceShellFlushFn_t flush;
  bool echo;
} serviceShellContext_t;

static serviceShellContext_t ctx;

static void writeText(const char *text) {
  if (ctx.write != NULL) {
    ctx.write(text);
  }
  if (ctx.flush != NULL) {
    ctx.flush();
  }
}

static void echoChar(char ch) {
  char text[2] = { ch, '\0' };
  writeText(text);
}

static void writePrompt(void) {
  writeText("svc> ");
}

static bool parseU8(const char *text, unsigned int *value) {
  char tail;

  if (sscanf(text, "%u %c", value, &tail) != 1) {
    return false;
  }

  return *value <= UINT8_MAX;
}

static const char *skipSpaces(const char *text) {
  while (*text == ' ') {
    text++;
  }
  return text;
}

static bool serviceShellTxPowerFromDb(float db, uint32_t *txPower) {
  uint8_t coarse = 0;
  uint8_t fine = 0;
  uint8_t powerByte;

  if (txPower == NULL || !isfinite(db) || db <= 0.0f || db > 33.5f) {
    return false;
  }

  float halfStepsFloat = db * 2.0f;
  uint8_t halfSteps = (uint8_t)(halfStepsFloat + 0.5f);
  if (fabsf(halfStepsFloat - (float)halfSteps) > 0.001f) {
    return false;
  }

  while (coarse <= 6 && (uint8_t)((6 - coarse) * 6) > halfSteps) {
    coarse++;
  }

  while ((uint8_t)((6 - coarse) * 6 + fine) < halfSteps) {
    fine++;
  }

  powerByte = (uint8_t)((coarse << 5) | fine);
  *txPower = (uint32_t)powerByte | ((uint32_t)powerByte << 8) | ((uint32_t)powerByte << 16) | ((uint32_t)powerByte << 24);
  return true;
}

static bool parseDecimalFloatToken(const char **cursor, float *value) {
  const char *text = skipSpaces(*cursor);
  float sign = 1.0f;
  float result = 0.0f;
  float scale = 0.1f;
  bool hasDigits = false;

  if (*text == '-') {
    sign = -1.0f;
    text++;
  } else if (*text == '+') {
    text++;
  }

  while (*text >= '0' && *text <= '9') {
    hasDigits = true;
    result = result * 10.0f + (float)(*text - '0');
    text++;
  }

  if (*text == '.') {
    text++;
    while (*text >= '0' && *text <= '9') {
      hasDigits = true;
      result += (float)(*text - '0') * scale;
      scale *= 0.1f;
      text++;
    }
  }

  if (!hasDigits) {
    return false;
  }

  *value = sign * result;
  *cursor = text;
  return true;
}

static void SERVICE_SHELL_NOINLINE writeControllerError(serviceControllerResult_t result, uint8_t targetId) {
  char output[64];

  switch (result) {
    case serviceControllerResultTimeout:
      snprintf(output, sizeof(output), "ERR code=timeout target=%u attempts=%u\r\n", targetId, SERVICE_CONTROLLER_ATTEMPTS);
      writeText(output);
      break;
    case serviceControllerResultBusy:
      snprintf(output, sizeof(output), "ERR code=busy target=%u\r\n", targetId);
      writeText(output);
      break;
    case serviceControllerResultWrongMode:
      writeText("ERR code=wrong_mode need=service\r\n");
      break;
    case serviceControllerResultOk:
    default:
      break;
  }
}

static void SERVICE_SHELL_NOINLINE writeRemoteResult(serviceControllerResult_t result, uint8_t targetId, uint8_t replyStatus, uint8_t replyFlags, uint16_t requestId) {
  char output[80];

  if (result != serviceControllerResultOk) {
    writeControllerError(result, targetId);
    return;
  }

  if (replyStatus != SERVICE_STATUS_OK) {
    snprintf(output, sizeof(output), "ERR code=remote_status status=%u target=%u req=%u\r\n", replyStatus, targetId, requestId);
    writeText(output);
    return;
  }

  if ((replyFlags & SERVICE_REPLY_FLAG_RESET_PENDING) != 0) {
    snprintf(output, sizeof(output), "OK req=%u target=%u status=written reset=1\r\n", requestId, targetId);
  } else {
    snprintf(output, sizeof(output), "OK req=%u target=%u status=active reset=0\r\n", requestId, targetId);
  }

  writeText(output);
}

static void SERVICE_SHELL_NOINLINE runTransaction(uint8_t targetId, uint8_t commandId, const void *payload, uint8_t payloadLength, void *replyPayload, uint8_t *replyPayloadLength) {
  uint8_t replyStatus = 0;
  uint8_t replyFlags = 0;
  uint16_t requestId = 0;
  serviceControllerResult_t result;

  result = serviceControllerTransaction(targetId, commandId, payload, payloadLength, replyPayload, replyPayloadLength, &replyStatus, &replyFlags, &requestId);
  writeRemoteResult(result, targetId, replyStatus, replyFlags, requestId);
}

static void formatPosition(char *output, size_t outputLength, float value) {
  if (!isfinite(value) || value > SERVICE_SHELL_MAX_PRINTABLE_POSITION_M || value < -SERVICE_SHELL_MAX_PRINTABLE_POSITION_M) {
    snprintf(output, outputLength, "nan");
    return;
  }

  int32_t scaled = (int32_t)(value >= 0.0f ? value * 1000.0f + 0.5f : value * 1000.0f - 0.5f);
  uint32_t absolute = scaled < 0 ? (uint32_t)-scaled : (uint32_t)scaled;

  snprintf(output, outputLength, "%s%lu.%03lu", scaled < 0 ? "-" : "", (unsigned long)(absolute / 1000), (unsigned long)(absolute % 1000));
}

static bool isValidUwbChannel(unsigned int channel) {
  return channel == 1 || channel == 2 || channel == 3 || channel == 4 || channel == 5 || channel == 7;
}

static void SERVICE_SHELL_NOINLINE writeConfigSnapshot(uint8_t targetId, uint16_t requestId, const serviceConfigPayload_t *snapshot) {
  char output[96];
  char x[16];
  char y[16];
  char z[16];
  uint8_t radioMode = (snapshot->lowBitrate ? 1 : 0) | (snapshot->longPreamble ? 2 : 0);

  formatPosition(x, sizeof(x), snapshot->position[0]);
  formatPosition(y, sizeof(y), snapshot->position[1]);
  formatPosition(z, sizeof(z), snapshot->position[2]);

  snprintf(output, sizeof(output),
           "OK req=%u target=%u node=%u mode=%u pos_enabled=%u ",
           requestId,
           targetId,
           snapshot->nodeId,
           snapshot->mode,
           snapshot->positionEnabled);
  writeText(output);

  snprintf(output, sizeof(output), "x=%s y=%s z=%s ", x, y, z);
  writeText(output);

  snprintf(output, sizeof(output),
           "smart_power=%u force_tx_power=%u tx_power=0x%08lx radio=%u ",
           snapshot->smartPower,
           snapshot->forceTxPower,
           (unsigned long)snapshot->txPower,
           radioMode);
  writeText(output);

  snprintf(output, sizeof(output),
           "low_bitrate=%u long_preamble=%u channel=%u version=%u\r\n",
           snapshot->lowBitrate,
           snapshot->longPreamble,
           snapshot->channel,
           snapshot->serviceProtocolVersion);
  writeText(output);
}

static void SERVICE_SHELL_NOINLINE handleGet(const char *args) {
  unsigned int target;
  serviceConfigPayload_t reply;
  uint8_t replyLength = sizeof(reply);
  uint8_t replyStatus = 0;
  uint8_t replyFlags = 0;
  uint16_t requestId = 0;
  serviceControllerResult_t result;

  if (!parseU8(args, &target)) {
    writeText("ERR code=bad_value field=id\r\n");
    return;
  }

  result = serviceControllerTransaction((uint8_t)target, SERVICE_COMMAND_GET_CONFIG, NULL, 0, (uint8_t *)&reply, &replyLength, &replyStatus, &replyFlags, &requestId);

  if (result != serviceControllerResultOk) {
    writeControllerError(result, (uint8_t)target);
    return;
  }

  if (replyStatus != SERVICE_STATUS_OK) {
    writeRemoteResult(result, (uint8_t)target, replyStatus, replyFlags, requestId);
    return;
  }

  if (replyLength != sizeof(reply)) {
    char output[64];
    snprintf(output, sizeof(output), "ERR code=bad_length target=%u req=%u length=%u\r\n", target, requestId, replyLength);
    writeText(output);
    return;
  }

  writeConfigSnapshot((uint8_t)target, requestId, &reply);
}

static void SERVICE_SHELL_NOINLINE handleSetPosition(const char *args) {
  unsigned long target;
  char *end;
  const char *cursor;
  serviceSetPositionPayload_t payload;

  target = strtoul(args, &end, 10);
  cursor = end;

  if (end == args || target > UINT8_MAX ||
      !parseDecimalFloatToken(&cursor, &payload.position[0]) ||
      !parseDecimalFloatToken(&cursor, &payload.position[1]) ||
      !parseDecimalFloatToken(&cursor, &payload.position[2]) ||
      *skipSpaces(cursor) != '\0' ||
      !isfinite(payload.position[0]) || !isfinite(payload.position[1]) || !isfinite(payload.position[2])) {
    writeText("ERR code=bad_value field=pos\r\n");
    return;
  }

  runTransaction((uint8_t)target, SERVICE_COMMAND_SET_POSITION, &payload, sizeof(payload), NULL, NULL);
}

static void SERVICE_SHELL_NOINLINE handleSetRadio(const char *args) {
  unsigned int target;
  unsigned int mode;
  char tail;
  serviceSetRadioModePayload_t payload;

  if (sscanf(args, "%u %u %c", &target, &mode, &tail) != 2 || target > UINT8_MAX || mode > 3) {
    writeText("ERR code=bad_value field=radio\r\n");
    return;
  }

  payload.radioMode = (uint8_t)mode;

  runTransaction((uint8_t)target, SERVICE_COMMAND_SET_RADIO_MODE, &payload, sizeof(payload), NULL, NULL);
}

static void SERVICE_SHELL_NOINLINE handleSetChannel(const char *args) {
  unsigned int target;
  unsigned int channel;
  char tail;
  serviceSetUwbChannelPayload_t payload;

  if (sscanf(args, "%u %u %c", &target, &channel, &tail) != 2 || target > UINT8_MAX || !isValidUwbChannel(channel)) {
    writeText("ERR code=bad_value field=channel\r\n");
    return;
  }

  payload.channel = (uint8_t)channel;

  runTransaction((uint8_t)target, SERVICE_COMMAND_SET_UWB_CHANNEL, &payload, sizeof(payload), NULL, NULL);
}

static void SERVICE_SHELL_NOINLINE handleSetPower(const char *args) {
  unsigned int target;
  char value[16];
  char tail;
  serviceSetTxPowerPayload_t payload;

  if (sscanf(args, "%u %15s %c", &target, value, &tail) != 2 || target > UINT8_MAX) {
    writeText("ERR code=bad_value field=power\r\n");
    return;
  }

  if (strcmp(value, "default") == 0) {
    payload.enableSmartPower = 1;
    payload.forceTxPower = 0;
    payload.txPower = SERVICE_SHELL_DEFAULT_TX_POWER;
    runTransaction((uint8_t)target, SERVICE_COMMAND_SET_TX_POWER, &payload, sizeof(payload), NULL, NULL);
    return;
  }

  float db;
  uint32_t txPower;
  if (sscanf(value, "%f %c", &db, &tail) != 1 || !serviceShellTxPowerFromDb(db, &txPower)) {
    writeText("ERR code=bad_value field=power_db\r\n");
    return;
  }

  payload.enableSmartPower = 0;
  payload.forceTxPower = 1;
  payload.txPower = txPower;
  runTransaction((uint8_t)target, SERVICE_COMMAND_SET_TX_POWER, &payload, sizeof(payload), NULL, NULL);
}

static void handleSet(const char *args) {
  if (strncmp(args, "pos ", 4) == 0) {
    handleSetPosition(args + 4);
  } else if (strncmp(args, "radio ", 6) == 0) {
    handleSetRadio(args + 6);
  } else if (strncmp(args, "channel ", 8) == 0) {
    handleSetChannel(args + 8);
  } else if (strncmp(args, "power ", 6) == 0) {
    handleSetPower(args + 6);
  } else {
    writeText("ERR code=unknown_command\r\n");
  }
}

static void handleRadio(const char *args) {
  unsigned int mode;

  if (args[0] != '\0' && (!parseU8(args, &mode) || mode > 3)) {
    writeText("ERR code=bad_value field=radio\r\n");
    return;
  }

  writeText("OK radio command=unsupported\r\n");
}

static void SERVICE_SHELL_NOINLINE processLine(void) {
  if (ctx.lineOverflow) {
    writeText("ERR code=bad_value field=line\r\n");
  } else if (strcmp(ctx.line, "help") == 0) {
    writeText("OK commands=help,local,radio,get,set,exit\r\n");
  } else if (strcmp(ctx.line, "local") == 0) {
    writeText("OK local mode=service\r\n");
  } else if (strcmp(ctx.line, "radio") == 0) {
    handleRadio("");
  } else if (strncmp(ctx.line, "radio ", 6) == 0) {
    handleRadio(ctx.line + 6);
  } else if (strncmp(ctx.line, "get ", 4) == 0) {
    handleGet(ctx.line + 4);
  } else if (strncmp(ctx.line, "set ", 4) == 0) {
    handleSet(ctx.line + 4);
  } else if (strcmp(ctx.line, "exit") == 0) {
    writeText("OK exit\r\n");
    ctx.active = false;
  } else if (ctx.line[0] != '\0') {
    writeText("ERR code=unknown_command\r\n");
  }

  ctx.lineLength = 0;
  ctx.line[0] = '\0';
  ctx.lineOverflow = false;

  if (ctx.active) {
    writePrompt();
  }
}

static bool serviceShellEnterWithOptions(bool isServiceMode, serviceShellWriteFn_t writeFn, serviceShellFlushFn_t flushFn, bool echo) {
  memset(&ctx, 0, sizeof(ctx));
  ctx.write = writeFn;
  ctx.flush = flushFn;
  ctx.echo = echo;

  if (!isServiceMode) {
    writeText("ERR code=wrong_mode need=service\r\n");
    ctx.write = NULL;
    ctx.flush = NULL;
    return false;
  }

  ctx.active = true;
  writeText("SVC READY version=1\r\n");
  writePrompt();
  return true;
}

bool serviceShellEnter(bool isServiceMode, serviceShellWriteFn_t writeFn) {
  return serviceShellEnterWithOptions(isServiceMode, writeFn, NULL, false);
}

bool serviceShellEnterHuman(bool isServiceMode, serviceShellWriteFn_t writeFn, serviceShellFlushFn_t flushFn) {
  return serviceShellEnterWithOptions(isServiceMode, writeFn, flushFn, true);
}

bool serviceShellIsActive(void) {
  return ctx.active;
}

bool serviceShellProcessChar(char ch) {
  if (!ctx.active) {
    return false;
  }

  if (ch == '\n' && ctx.previousWasCr) {
    ctx.previousWasCr = false;
    return ctx.active;
  }

  ctx.previousWasCr = (ch == '\r');

  if (ch == '\r' || ch == '\n') {
    if (ctx.echo) {
      writeText("\r\n");
    }
    processLine();
    return ctx.active;
  }

  if ((uint8_t)ch < ' ') {
    ctx.lineOverflow = true;
    return ctx.active;
  }

  if (ctx.echo) {
    echoChar(ch);
  }

  if (ctx.lineLength < sizeof(ctx.line) - 1) {
    ctx.line[ctx.lineLength++] = ch;
    ctx.line[ctx.lineLength] = '\0';
  } else {
    ctx.lineOverflow = true;
  }

  return ctx.active;
}
