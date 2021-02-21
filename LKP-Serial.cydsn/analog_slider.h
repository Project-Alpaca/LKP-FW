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

#include "project.h"

#define SLIDER_DSRAM Slider_dsRam
#define SLIDER_WIDGET_NAME segments

#define SLIDER_WIDGET_REG SLIDER_DSRAM.wdgtList.SLIDER_WIDGET_NAME
#define SLIDER_SENSOR_REG(id) SLIDER_DSRAM.snsList.SLIDER_WIDGET_NAME[id]

#define SENSOR_MAX_VALUE ((uint8_t) 0xfcu)

extern uint8_t calculate_analog_sensor_value(uint32_t sensor);

/* [] END OF FILE */
