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

#define SLIDER_DSRAM Slider_dsRam
#define SLIDER_WIDGET_NAME segments

#define SLIDER_WIDGET_REG SLIDER_DSRAM.wdgtList.SLIDER_WIDGET_NAME
#define SLIDER_SENSOR_REG(id) SLIDER_DSRAM.snsList.SLIDER_WIDGET_NAME[id]

#define SENSOR_MAX_VALUE ((uint8_t) 0xfcu)

extern uint8_t calculate_analog_sensor_value(uint32_t sensor);

/* [] END OF FILE */
