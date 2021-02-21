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

#include "analog_slider.h"
#include "utils.h"

uint8_t calculate_analog_sensor_value(uint32_t sensor) {
    uint32_t scratch = 0;
    uint32 ceil, floor = 0;
    scratch = SLIDER_SENSOR_REG(sensor).diff;
    if (unlikely(scratch < SLIDER_WIDGET_REG.fingerTh)) {
        return 0;
    }
    floor = SLIDER_WIDGET_REG.fingerTh + SLIDER_SENSOR_REG(sensor).bsln[0u];
    ceil = 1u << SLIDER_WIDGET_REG.resolution;
    // Touch status is defined as diff > (fingerTh + hysterisis). So we calculate the analog value using
    // ((diff-fingerTh) / (ceil-floor)) * SENSOR_MAX_VALUE and take the lower 8-bit.
    scratch -= SLIDER_WIDGET_REG.fingerTh;
    scratch *= SENSOR_MAX_VALUE;
    scratch /= ceil - floor;
    return (uint8_t) (scratch & 0xff);
}

/* [] END OF FILE */
