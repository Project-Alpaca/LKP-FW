/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#pragma once

#include "`$INSTANCE_NAME`_API_Common.h"

#define `$INSTANCE_NAME`_NUM_PHYSICAL_LEDS `$INSTANCE_NAME`_IO_LED_COLUMNS
#define `$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS 32u

extern void `$INSTANCE_NAME`_ClearLED(uint32_t color);
extern void `$INSTANCE_NAME`_SetLED(uint32_t offset, uint32_t color);
extern void `$INSTANCE_NAME`_CommitLED();

/* [] END OF FILE */
