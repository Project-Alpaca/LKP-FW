/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/
#include "project.h"
#include "utils.h"
#include <stdbool.h>

void setup() {
    /* Setup */
    // Initialize I2C
    I2C_Start();
    I2C_EzI2CSetBuffer1(sizeof(Slider_dsRam), sizeof(Slider_dsRam), (uint8_t *) &Slider_dsRam);

    // Initialize CapSense
    Slider_Start();
    Slider_ScanAllWidgets();
}

void loop() {
    if (Slider_IsBusy() == Slider_NOT_BUSY) {
        Slider_ProcessAllWidgets();
        Slider_RunTuner();
        Slider_ScanAllWidgets();
    }
}

int main(void) {
    /* Enable global interrupts. */
    CyGlobalIntEnable;

    setup();

    /* Main loop */
    for(;;) {
        loop();
    }
    /* Should never jump to here */
    return 0;
}

/* [] END OF FILE */
