/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_Analog_Slider.h"
#include "`$INSTANCE_NAME`_Utils.h"

uint8_t `$INSTANCE_NAME`_CalculateAnalogSensorValue(uint32_t sensor) {
    uint32_t scratch = 0;
    uint32 ceil, floor = 0;
    scratch = `$INSTANCE_NAME`_SLIDER_SENSOR_REG(sensor).diff;
    if (`$INSTANCE_NAME`_unlikely(scratch < `$INSTANCE_NAME`_SLIDER_WIDGET_REG.fingerTh)) {
        return 0;
    }
    floor = `$INSTANCE_NAME`_SLIDER_WIDGET_REG.fingerTh + `$INSTANCE_NAME`_SLIDER_SENSOR_REG(sensor).bsln[0u];
    ceil = 1u << `$INSTANCE_NAME`_SLIDER_WIDGET_REG.resolution;
    // Touch status is defined as diff > (fingerTh + hysterisis). So we calculate the analog value using
    // ((diff-fingerTh) / (ceil-floor)) * SENSOR_MAX_VALUE and take the lower 8-bit.
    scratch -= `$INSTANCE_NAME`_SLIDER_WIDGET_REG.fingerTh;
    scratch *= `$INSTANCE_NAME`_SENSOR_MAX_VALUE;
    scratch /= ceil - floor;
    return (uint8_t) (scratch & 0xff);
}

/* [] END OF FILE */
