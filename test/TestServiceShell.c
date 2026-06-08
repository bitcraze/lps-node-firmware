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
#include <string.h>

#include "unity.h"
#include "service_shell.h"
#include "service_controller.h"
#include "service_protocol.h"

#define OUTPUT_BUFFER_SIZE 256
#define RECORDED_PAYLOAD_SIZE 32

static char output[OUTPUT_BUFFER_SIZE];
static size_t outputLength;
static bool transactionCalled;
static uint8_t recordedTargetId;
static uint8_t recordedCommandId;
static uint8_t recordedPayloadLength;
static uint8_t recordedPayload[RECORDED_PAYLOAD_SIZE];
static bool recordedReplyPayloadPresent;
static bool recordedReplyPayloadLengthPresent;
static uint8_t recordedReplyPayloadCapacity;
static serviceControllerResult_t transactionResult;
static uint8_t transactionReplyStatus;
static uint8_t transactionReplyFlags;
static uint16_t transactionRequestId;
static unsigned int flushCount;
static size_t lastFlushOutputLength;

serviceControllerResult_t serviceControllerTransaction(uint8_t targetId, uint8_t commandId, const uint8_t *payload, uint8_t payloadLength, uint8_t *replyPayload, uint8_t *replyPayloadLength, uint8_t *replyStatus, uint8_t *replyFlags, uint16_t *requestId) {
  transactionCalled = true;
  recordedTargetId = targetId;
  recordedCommandId = commandId;
  recordedPayloadLength = payloadLength;

  if (payload != NULL && payloadLength <= sizeof(recordedPayload)) {
    memcpy(recordedPayload, payload, payloadLength);
  }

  (void)replyPayload;
  recordedReplyPayloadPresent = replyPayload != NULL;
  recordedReplyPayloadLengthPresent = replyPayloadLength != NULL;

  if (replyPayloadLength != NULL) {
    recordedReplyPayloadCapacity = *replyPayloadLength;
  }

  if (commandId == SERVICE_COMMAND_GET_CONFIG && replyPayload != NULL && replyPayloadLength != NULL && *replyPayloadLength >= sizeof(serviceConfigPayload_t)) {
    serviceConfigPayload_t reply = {
      .nodeId = targetId,
      .mode = 4,
      .positionEnabled = 1,
      .position = {1.25f, 2.5f, 3.75f},
      .smartPower = 1,
      .forceTxPower = 0,
      .txPower = 0x07274767UL,
      .lowBitrate = 1,
      .longPreamble = 0,
      .channel = 5,
      .serviceProtocolVersion = SERVICE_PROTOCOL_VERSION,
    };
    memcpy(replyPayload, &reply, sizeof(reply));
    *replyPayloadLength = sizeof(reply);
  }

  if (replyStatus != NULL) {
    *replyStatus = transactionReplyStatus;
  }

  if (replyFlags != NULL) {
    *replyFlags = transactionReplyFlags;
  }

  if (requestId != NULL) {
    *requestId = transactionRequestId;
  }

  return transactionResult;
}

static void writeOutput(const char *text) {
  size_t textLength = strlen(text);

  if (outputLength + textLength < sizeof(output)) {
    memcpy(&output[outputLength], text, textLength + 1);
    outputLength += textLength;
  }
}

static void flushOutput(void) {
  flushCount++;
  lastFlushOutputLength = outputLength;
}

static void sendLine(const char *line) {
  while (*line != '\0') {
    serviceShellProcessChar(*line++);
  }
  serviceShellProcessChar('\r');
}

static void sendBytes(const char *bytes, size_t length) {
  for (size_t i = 0; i < length; i++) {
    serviceShellProcessChar(bytes[i]);
  }
}

void setUp(void) {
  memset(output, 0, sizeof(output));
  memset(recordedPayload, 0, sizeof(recordedPayload));
  outputLength = 0;
  transactionCalled = false;
  recordedTargetId = 0;
  recordedCommandId = 0;
  recordedPayloadLength = 0;
  recordedReplyPayloadPresent = false;
  recordedReplyPayloadLengthPresent = false;
  recordedReplyPayloadCapacity = 0;
  transactionResult = serviceControllerResultOk;
  transactionReplyStatus = SERVICE_STATUS_OK;
  transactionReplyFlags = SERVICE_REPLY_FLAG_ACTIVE;
  transactionRequestId = 0x1234;
  flushCount = 0;
  lastFlushOutputLength = 0;

  if (serviceShellIsActive()) {
    sendLine("exit");
  }
}

void tearDown(void) {
  if (serviceShellIsActive()) {
    sendLine("exit");
  }
}

void test_serviceShellShouldEnterServiceModeAndPrintReadyAndPrompt(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ", output);
}

void test_serviceShellShouldRejectEntryOutsideServiceMode(void) {
  TEST_ASSERT_FALSE(serviceShellEnter(false, writeOutput));

  TEST_ASSERT_FALSE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("ERR code=wrong_mode need=service\r\n", output);
}

void test_serviceShellShouldNotEchoMachineInput(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("help");

  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK commands=help,local,radio,get,set,exit\r\nsvc> ", output);
}

void test_serviceShellShouldEchoHumanInput(void) {
  TEST_ASSERT_TRUE(serviceShellEnterHuman(true, writeOutput, flushOutput));

  sendLine("help");

  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> help\r\nOK commands=help,local,radio,get,set,exit\r\nsvc> ", output);
}

void test_serviceShellShouldFlushHumanPromptWhenItIsEmitted(void) {
  TEST_ASSERT_TRUE(serviceShellEnterHuman(true, writeOutput, flushOutput));

  TEST_ASSERT_TRUE(flushCount > 0);
  TEST_ASSERT_EQUAL(strlen("SVC READY version=1\r\nsvc> "), lastFlushOutputLength);
}

void test_serviceShellShouldExitOnExitCommand(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("exit");

  TEST_ASSERT_FALSE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK exit\r\n", output);
}

void test_serviceShellShouldSendSetRadioTransaction(void) {
  serviceSetRadioModePayload_t payload;

  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set radio 3 1");

  TEST_ASSERT_TRUE(transactionCalled);
  TEST_ASSERT_EQUAL_UINT8(3, recordedTargetId);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_COMMAND_SET_RADIO_MODE, recordedCommandId);
  TEST_ASSERT_EQUAL_UINT8(sizeof(serviceSetRadioModePayload_t), recordedPayloadLength);
  memcpy(&payload, recordedPayload, sizeof(payload));
  TEST_ASSERT_EQUAL_UINT8(1, payload.radioMode);
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK req=4660 target=3 status=active reset=0\r\nsvc> ", output);
}

void test_serviceShellShouldSendSetChannelTransaction(void) {
  serviceSetUwbChannelPayload_t payload;

  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set channel 3 7");

  TEST_ASSERT_TRUE(transactionCalled);
  TEST_ASSERT_EQUAL_UINT8(3, recordedTargetId);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_COMMAND_SET_UWB_CHANNEL, recordedCommandId);
  TEST_ASSERT_EQUAL_UINT8(sizeof(serviceSetUwbChannelPayload_t), recordedPayloadLength);
  memcpy(&payload, recordedPayload, sizeof(payload));
  TEST_ASSERT_EQUAL_UINT8(7, payload.channel);
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK req=4660 target=3 status=active reset=0\r\nsvc> ", output);
}

void test_serviceShellShouldRejectInvalidChannel(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set channel 3 6");

  TEST_ASSERT_FALSE(transactionCalled);
  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ERR code=bad_value field=channel\r\nsvc> ", output);
}

void test_serviceShellShouldSendGetTransactionWithReplyCapacity(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("get 5");

  TEST_ASSERT_TRUE(transactionCalled);
  TEST_ASSERT_EQUAL_UINT8(5, recordedTargetId);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_COMMAND_GET_CONFIG, recordedCommandId);
  TEST_ASSERT_EQUAL_UINT8(0, recordedPayloadLength);
  TEST_ASSERT_TRUE(recordedReplyPayloadPresent);
  TEST_ASSERT_TRUE(recordedReplyPayloadLengthPresent);
  TEST_ASSERT_EQUAL_UINT8(sizeof(serviceConfigPayload_t), recordedReplyPayloadCapacity);
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK req=4660 target=5 node=5 mode=4 pos_enabled=1 x=1.250 y=2.500 z=3.750 smart_power=1 force_tx_power=0 tx_power=0x07274767 radio=1 low_bitrate=1 long_preamble=0 channel=5 version=1\r\nsvc> ", output);
}

void test_serviceShellShouldRejectNulInsideLineWithoutExiting(void) {
  const char command[] = { 'e', 'x', 'i', 't', '\0', 'j', 'u', 'n', 'k', '\r' };

  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendBytes(command, sizeof(command));

  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ERR code=bad_value field=line\r\nsvc> ", output);
}

void test_serviceShellShouldRejectNonFinitePosition(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set pos 3 nan 1 2");

  TEST_ASSERT_FALSE(transactionCalled);
  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ERR code=bad_value field=pos\r\nsvc> ", output);
}

void test_serviceShellShouldRejectNonFinitePowerDb(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set power 3 nan");

  TEST_ASSERT_FALSE(transactionCalled);
  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ERR code=bad_value field=power_db\r\nsvc> ", output);
}

void test_serviceShellShouldSendSetPowerTransactionForPowerDb(void) {
  serviceSetTxPowerPayload_t payload;

  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set power 3 15.5");

  TEST_ASSERT_TRUE(transactionCalled);
  TEST_ASSERT_EQUAL_UINT8(3, recordedTargetId);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_COMMAND_SET_TX_POWER, recordedCommandId);
  TEST_ASSERT_EQUAL_UINT8(sizeof(serviceSetTxPowerPayload_t), recordedPayloadLength);
  memcpy(&payload, recordedPayload, sizeof(payload));
  TEST_ASSERT_EQUAL_UINT8(0, payload.enableSmartPower);
  TEST_ASSERT_EQUAL_UINT8(1, payload.forceTxPower);
  TEST_ASSERT_EQUAL_UINT32(0x21212121UL, payload.txPower);
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> OK req=4660 target=3 status=active reset=0\r\nsvc> ", output);
}

void test_serviceShellShouldRejectPowerDbOutsideHalfDbSteps(void) {
  TEST_ASSERT_TRUE(serviceShellEnter(true, writeOutput));

  sendLine("set power 3 15.25");

  TEST_ASSERT_FALSE(transactionCalled);
  TEST_ASSERT_TRUE(serviceShellIsActive());
  TEST_ASSERT_EQUAL_STRING("SVC READY version=1\r\nsvc> ERR code=bad_value field=power_db\r\nsvc> ", output);
}
