/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#pragma once

#include "project.h"

#define `$INSTANCE_NAME`_SLIDER_DSRAM `$INSTANCE_NAME`_Slider_dsRam
#define `$INSTANCE_NAME`_SLIDER_WIDGET_NAME segments

#define `$INSTANCE_NAME`_SLIDER_WIDGET_REG `$INSTANCE_NAME`_SLIDER_DSRAM.wdgtList.`$INSTANCE_NAME`_SLIDER_WIDGET_NAME
#define `$INSTANCE_NAME`_SLIDER_SENSOR_REG(id) `$INSTANCE_NAME`_SLIDER_DSRAM.snsList.`$INSTANCE_NAME`_SLIDER_WIDGET_NAME[id]

#define `$INSTANCE_NAME`_SENSOR_MAX_VALUE ((uint8_t) 0xfcu)

extern uint8_t `$INSTANCE_NAME`_CalculateAnalogSensorValue(uint32_t sensor);

/* [] END OF FILE */
