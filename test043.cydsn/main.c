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

//#define BIT_CTL_SCAN_EN 1
#define BIT_CTL_INTR_EN 1 << 1
#define BIT_CTL_INTR_TRIG 1 << 7

#define PIN_HIGH 1
#define PIN_HIZ 1
#define PIN_LOW 0

// 0BRG when using WS281x, WBGR when using SK6812RGBW
#define LED_BG 0x121212
#define LED_CURSOR 0xffffff

typedef struct {
    /* control and configuration regs - writable */
    // 0: scan_en (currently disabled) 1: interrupt_en 7: interrupt_trig
    uint8_t ctl;
    // TODO
} __attribute__((packed)) slider_protocol_rw_t;

typedef struct {
    /* data regs - read-only */
    // Version
    uint8_t ver_major;
    uint8_t ver_minor;
    // Bit-field of trigger state
    union {
        struct {
            uint8_t key0;
            uint8_t key1;
            uint8_t key2;
            uint8_t key3;
        };
        uint8_t keys[4];
        uint32_t keys_active;
    };
    // LED lighting rule (TODO)
    // Raw signal (TODO)
    //uint8_t key_raw[32];
} __attribute__((packed)) slider_protocol_ro_t;

typedef struct {
    slider_protocol_rw_t rw;
    slider_protocol_ro_t ro;
} __attribute__((packed)) slider_protocol_t;

volatile slider_protocol_t i2cregs = {0};

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

int main(void) {
    /* Enable global interrupts. */
    CyGlobalIntEnable;

    /* Setup */
    // Set protocol version
    i2cregs.ro.ver_major = 1;
    i2cregs.ro.ver_minor = 0;

    // Initialize I2C
    I2C_Start();
    I2C_EzI2CSetBuffer1(sizeof(i2cregs), sizeof(i2cregs.rw), (volatile uint8_t *) &i2cregs);

    // Initialize LED
    LED_Start();
    // Hard code the dim level to 1/2 for now
    LED_Dim(1);

    // Initialize CapSense
    Slider_Start();
    Slider_ScanAllWidgets();

    /* Main loop */
    for(;;) {
        // Assert interrupt pin if necessary
        Pin_Interrupt_Out_Write((i2cregs.rw.ctl & BIT_CTL_INTR_TRIG) ? PIN_LOW : PIN_HIZ);
        if (Slider_IsBusy() == Slider_NOT_BUSY) {
            // Apply advanced filters
            Slider_ProcessAllWidgets();
            bool dirty = false;
            // Check for all individual sensors
            if (Slider_IsWidgetActive(Slider_SEGMENTS_WDGT_ID)) {
                Pin_Status_LED_Write(PIN_HIGH);
                for (uint8_t i=0; i<(sizeof(SEG_MAPPING) / sizeof(uint32_t)); i++) {
                    uint8_t bit = (Slider_IsSensorActive(Slider_SEGMENTS_WDGT_ID, SEG_MAPPING[i]) ? 1 : 0);
                    uint8_t state_bit_pos = i & 7;
                    volatile uint8_t *reg = &i2cregs.ro.keys[i >> 3];
                    uint8_t bit_orig = ((*reg >> state_bit_pos) & 1);
                    if (bit_orig != bit) {
                        dirty = true;
                        // Flip the bit
                        *reg ^= 1 << state_bit_pos;
                        if (bit) {
                            LED_Pixel(i, 0, LED_CURSOR);
                        } else {
                            LED_Pixel(i, 0, LED_BG);
                        }
                    }
                }
            // No sensor is active, check if any of them were active
            } else if (i2cregs.ro.keys_active) {
                Pin_Status_LED_Write(PIN_LOW);
                // Nothing active, zeroing
                i2cregs.ro.keys_active = 0;
                // Clear LED
                LED_DisplayClear(LED_BG);
                dirty = true;
            }
            // Re-arm the widget scanner
            Slider_ScanAllWidgets();

            // Update the LED
            LED_Trigger(1);

            // Sync the interrupt bit with dirty bit 
            if (dirty && (i2cregs.rw.ctl & BIT_CTL_INTR_EN)) {
                i2cregs.rw.ctl |= BIT_CTL_INTR_TRIG;
            }
        }
    }
    /* Should never jump to here */
    return 0;
}

/* [] END OF FILE */
