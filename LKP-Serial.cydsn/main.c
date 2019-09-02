/* ========================================
 *
 * Copyright dogtopus, 2019
 * Released to public domain
 * SPDX-License-Identifier: Unlicense
 *
 * ========================================
*/
#include "project.h"
#include <stdbool.h>

typedef enum {
    cmd_input_report = 0x01,
    cmd_led_report,
    cmd_enable_slider_report,
    cmd_disable_slider_report,
    cmd_unk_0x09 = 0x09,
    cmd_unk_0x0a,
    cmd_ping = 0x10,
    cmd_exception = 0xee,
    cmd_get_hw_info = 0xf0,
} lkps_command_t;

typedef enum {
    state_recv_sync,
    state_recv_cmd,
    state_recv_len,
    state_recv_args,
    state_recv_sum,
} lkps_state_t;

#define PIN_HIGH 1
#define PIN_HIZ 1
#define PIN_LOW 0

const uint32_t SEG_MAPPING[] = {
    Slider_SEGMENTS_SNS0_ID,
    Slider_SEGMENTS_SNS1_ID,
    Slider_SEGMENTS_SNS2_ID,
    Slider_SEGMENTS_SNS3_ID,
    Slider_SEGMENTS_SNS4_ID,
    Slider_SEGMENTS_SNS5_ID,
    Slider_SEGMENTS_SNS6_ID,
    Slider_SEGMENTS_SNS7_ID,
    Slider_SEGMENTS_SNS8_ID,
    Slider_SEGMENTS_SNS9_ID,
    Slider_SEGMENTS_SNS10_ID,
    Slider_SEGMENTS_SNS11_ID,
    Slider_SEGMENTS_SNS12_ID,
    Slider_SEGMENTS_SNS13_ID,
    Slider_SEGMENTS_SNS14_ID,
    Slider_SEGMENTS_SNS15_ID,
    Slider_SEGMENTS_SNS16_ID,
    Slider_SEGMENTS_SNS17_ID,
    Slider_SEGMENTS_SNS18_ID,
    Slider_SEGMENTS_SNS19_ID,
    Slider_SEGMENTS_SNS20_ID,
    Slider_SEGMENTS_SNS21_ID,
    Slider_SEGMENTS_SNS22_ID,
    Slider_SEGMENTS_SNS23_ID,
    Slider_SEGMENTS_SNS24_ID,
    Slider_SEGMENTS_SNS25_ID,
    Slider_SEGMENTS_SNS26_ID,
    Slider_SEGMENTS_SNS27_ID,
    Slider_SEGMENTS_SNS28_ID,
    Slider_SEGMENTS_SNS29_ID,
    Slider_SEGMENTS_SNS30_ID,
    Slider_SEGMENTS_SNS31_ID,
};

void setup() {
    /* Setup */
    // Initialize I2C
    UART_Start();

    // Initialize LED
    LED_Start();
    // Hard code the dim level to 1/2 for now
    LED_Dim(1);
    LED_DisplayClear(0x0);
    LED_Trigger(1);

    // Initialize CapSense
    Slider_Start();
    Slider_ScanAllWidgets();
}

void prepare_input_report() {
    if (Slider_IsBusy() == Slider_NOT_BUSY) {
        // Apply advanced filters
        Slider_ProcessAllWidgets();
        // Check for all individual sensors
        if (Slider_IsWidgetActive(Slider_SEGMENTS_WDGT_ID)) {
            Pin_Status_LED_Write(PIN_HIGH);
            for (uint8_t i=0; i<(sizeof(SEG_MAPPING) / sizeof(uint32_t)); i++) {
                uint8_t bit = (Slider_IsSensorActive(Slider_SEGMENTS_WDGT_ID, SEG_MAPPING[i]) ? 1 : 0);
                // TODO update input report buffer
            }
        }
        // Re-arm the widget scanner
        Slider_ScanAllWidgets();

        // Update the LED
        LED_Trigger(1);
    }
}

void loop() {
    prepare_input_report();
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
