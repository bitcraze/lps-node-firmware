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
#ifndef __MAIN_TASK_CONFIG_H__
#define __MAIN_TASK_CONFIG_H__

#include "FreeRTOSConfig.h"

// The service shell get path formats a full configuration snapshot and uses
// newlib printf helpers. Hardware testing showed configMINIMAL_STACK_SIZE + 150
// can overflow the main task stack while running `get <id>`.
#define MAIN_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 190)

#endif // __MAIN_TASK_CONFIG_H__
