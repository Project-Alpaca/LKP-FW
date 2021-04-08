/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#pragma once

#include "`$INSTANCE_NAME`_API_Common.h"

#define `$INSTANCE_NAME`_NUM_PHYSICAL_LEDS `$INSTANCE_NAME`_IO_LED_COLUMNS
#define `$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS 32u

void `$INSTANCE_NAME`_ClearLED(uint32_t color);
void `$INSTANCE_NAME`_SetLED(uint32_t offset, uint32_t color);
void `$INSTANCE_NAME`_CommitLED();

/* [] END OF FILE */
