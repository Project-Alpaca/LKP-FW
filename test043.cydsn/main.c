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

// 0..(I2C_RW_BOUNDARY-1) is RW
#define I2C_PROTO_RW_BOUNDARY 1

//#define BIT_CTL_SCAN_EN 0
#define BIT_CTL_INTR_EN 1
#define BIT_CTL_INTR_TRIG 7

typedef struct {
    /* control and configuration regs - writable */
    // 0: scan_en (currently disabled) 1: interrupt_en 7: interrupt_trig
    uint8_t ctl;
    // TODO
    /* data regs - read-only */
    // Version
    uint8_t ver_major;
    uint8_t ver_minor;
    // Bit-field of trigger state
    uint8_t key0;
    uint8_t key1;
    uint8_t key2;
    uint8_t key3;
    // Raw signal (TODO)
    //uint8_t key_raw[32];
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
    i2cregs.ver_major = 1;
    i2cregs.ver_minor = 0;

    // Initialize I2C
    I2C_Start();
    I2C_EzI2CSetBuffer1(sizeof(i2cregs), I2C_PROTO_RW_BOUNDARY, (volatile uint8_t *) &i2cregs);

    // Initialize CapSense
    Slider_Start();
    Slider_ScanAllWidgets();

    /* Main loop */
    for(;;) {
        // Assert interrupt pin if necessary
        Pin_Interrupt_Out_Write((i2cregs.ctl & (1 << BIT_CTL_INTR_TRIG)) ? 1 : 0);
        if (Slider_IsBusy() == Slider_NOT_BUSY) {
            // Apply advanced filters
            Slider_ProcessAllWidgets();
            bool dirty = false;
            // Check for all individual widgets
            for (uint8_t i=0; i<(sizeof(SEG_MAPPING) / sizeof(uint32_t)); i++) {
                int8_t bit = (Slider_IsWidgetActive(SEG_MAPPING[i]) ? 1 : 0);
                uint8_t state_bit_pos = i & 7;
                volatile uint8_t *reg = &i2cregs.key0 + (i >> 3);
                int8_t bit_orig = ((*reg >> state_bit_pos) & 1);
                if (bit_orig != bit) {
                    dirty = true;
                    *reg ^= 1 << state_bit_pos;
                }
            }
            // Re-arm the widget scanner
            Slider_ScanAllWidgets();

            // Sync the interrupt bit with dirty bit 
            if (dirty && ((i2cregs.ctl >> BIT_CTL_INTR_EN) & 1)) {
                i2cregs.ctl |= 1 << BIT_CTL_INTR_TRIG;
            }
        }
    }
    /* Should never jump to here */
    return 0;
}

/* [] END OF FILE */
