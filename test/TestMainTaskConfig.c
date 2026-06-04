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
 */
#include "unity.h"

#include "FreeRTOSConfig.h"
#include "main_task_config.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_mainTaskStackShouldLeaveRoomForServiceShellGetFormatting(void) {
  TEST_ASSERT_TRUE(MAIN_TASK_STACK_SIZE >= configMINIMAL_STACK_SIZE + 190);
}
