/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_API_Common.h"

#if (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_TUNER)

void `$INSTANCE_NAME`_Start() {
    // Initialize I2C
    `$INSTANCE_NAME`_IO_I2C_Tuner_Start();
    `$INSTANCE_NAME`_IO_I2C_Tuner_EzI2CSetBuffer1(sizeof(`$INSTANCE_NAME`_Slider_dsRam), sizeof(`$INSTANCE_NAME`_Slider_dsRam), (uint8_t *) &`$INSTANCE_NAME`_Slider_dsRam);

    // Initialize CapSense
    `$INSTANCE_NAME`_Slider_Start();
    `$INSTANCE_NAME`_Slider_ScanAllWidgets();
}

void `$INSTANCE_NAME`_Task() {
    if (`$INSTANCE_NAME`_Slider_IsBusy() == `$INSTANCE_NAME`_Slider_NOT_BUSY) {
        `$INSTANCE_NAME`_Slider_ProcessAllWidgets();
        `$INSTANCE_NAME`_Slider_RunTuner();
        `$INSTANCE_NAME`_Slider_ScanAllWidgets();
    }
}

#endif // (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_TUNER)
/* [] END OF FILE */
