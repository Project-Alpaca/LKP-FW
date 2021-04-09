/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_API_Common.h"

#if (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_NATIVE)

//#define BIT_CTL_SCAN_EN 1
#define BIT_CTL_INTR_EN 1 << 1
#define BIT_CTL_INTR_TRIG 1 << 7

#define PIN_HIGH 1
#define PIN_HIZ 1
#define PIN_LOW 0

// 0BRG when using WS281x, WBGR when using SK6812RGBW
static const uint32_t LED_BG = 0x121212;
static const uint32_t LED_CURSOR = 0xffffff;

#define PROTOCOL_VER_MAJOR 2u
#define PROTOCOL_VER_MINOR 0u
#define PROTOCOL_VER_REV 0u

static const uint8_t VER_CHECK_QUERY = 0x1;

typedef struct {
    /* control and configuration regs - writable */
    union {
        // bank 0: ver
        struct {
            // Version check registers - write to this and set bit 0 of check to 1 to perform a protocol version check
            // Loosely follows Semantic Versioning (https://semver.org/)
            // major: breaking addition/deletion or drastic behavior change (repurposing registers, etc.)
            uint8_t ver_major;
            // minor: backwards compatible addition/deletion
            uint8_t ver_minor;
            // rev: minor behavior change
            uint8_t ver_rev;
            // 0: query current version
            uint8_t check;
        };
        uint32_t ver;
    };
    union {
        // bank 1: config 0
        struct {
            // 0: scan_en (currently disabled) 1: interrupt_en 7: interrupt_trig
            uint8_t ctl;
            uint8_t rfu5;
            uint8_t rfu6;
            uint8_t rfu7;
        };
        uint32_t config0;
    };
    // LED lighting rule (TODO)
} __attribute__((packed)) slider_protocol_rw_t;

typedef struct {
    /* data regs - read-only */
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
    // Analog signal
    uint8_t keys_analog[32];
} __attribute__((packed)) slider_protocol_ro_t;

typedef struct {
    slider_protocol_rw_t rw;
    slider_protocol_ro_t ro;
} __attribute__((packed)) slider_protocol_t;

static volatile slider_protocol_t i2cregs = {
    .rw = {
        .ver_major = PROTOCOL_VER_MAJOR,
        .ver_minor = PROTOCOL_VER_MINOR,
        .ver_rev = PROTOCOL_VER_REV,
        .check = 0,
        .config0 = 0,
    },
    .ro = {
        .keys = {0, 0, 0, 0},
        .keys_analog = {0},
    },
};

static void update_slider_regs() {
    // Assert interrupt pin if necessary
    `$INSTANCE_NAME`_IO_Pin_Interrupt_Out_Write((i2cregs.rw.ctl & BIT_CTL_INTR_TRIG) ? PIN_LOW : PIN_HIZ);
    if (`$INSTANCE_NAME`_Slider_IsBusy() == `$INSTANCE_NAME`_Slider_NOT_BUSY) {
        // Apply filters
        `$INSTANCE_NAME`_Slider_ProcessAllWidgets();
        bool dirty = false;
        // Check for all individual sensors
        if (`$INSTANCE_NAME`_Slider_IsWidgetActive(`$INSTANCE_NAME`_Slider_SEGMENTS_WDGT_ID)) {
            `$INSTANCE_NAME`_IO_Pin_Status_LED_Write(PIN_HIGH);
            // TODO: might be possible to get the value from the widget config struct.
            for (uint32_t sensor=0; sensor<`$INSTANCE_NAME`_Slider_SEGMENTS_NUM_SENSORS; sensor++) {
                uint8_t state_bit_pos = sensor & 7;
                uint8_t state_reg_offset = sensor >> 3;
                uint8_t bit = (`$INSTANCE_NAME`_Slider_IsSensorActive(`$INSTANCE_NAME`_Slider_SEGMENTS_WDGT_ID, sensor) ? 1 : 0);

                volatile uint8_t *reg = &i2cregs.ro.keys[state_reg_offset];

                uint8_t bit_orig = ((*reg >> state_bit_pos) & 1);
                if (bit_orig != bit) {
                    dirty = true;
                    // Flip the bit
                    *reg ^= 1 << state_bit_pos;
                    if (bit) {
                        `$INSTANCE_NAME`_SetLED(sensor, LED_CURSOR);
                    } else {
                        `$INSTANCE_NAME`_SetLED(sensor, LED_BG);
                    }
                }
                if (bit) {
                    i2cregs.ro.keys_analog[sensor] = `$INSTANCE_NAME`_CalculateAnalogSensorValue(sensor);
                } else {
                    i2cregs.ro.keys_analog[sensor] = 0;
                }
            }
        // No sensor is active, check if any of them were active
        } else if (i2cregs.ro.keys_active) {
            `$INSTANCE_NAME`_IO_Pin_Status_LED_Write(PIN_LOW);
            // Nothing active, zeroing
            i2cregs.ro.keys_active = 0;
            // Clear analog register.
            `$INSTANCE_NAME`_memset_v(i2cregs.ro.keys_analog, 0, sizeof(i2cregs.ro.keys_analog));
            // Clear LED
            `$INSTANCE_NAME`_ClearLED(LED_BG);
            dirty = true;
        }
        // Re-arm the widget scanner
        `$INSTANCE_NAME`_Slider_ScanAllWidgets();

        // Update the LED
        if (dirty && `$INSTANCE_NAME`_IO_LED_Ready()) {
            `$INSTANCE_NAME`_CommitLED();
        }

        // Sync the interrupt bit with dirty bit 
        if (dirty && (i2cregs.rw.ctl & BIT_CTL_INTR_EN)) {
            i2cregs.rw.ctl |= BIT_CTL_INTR_TRIG;
        }
    }
}

void `$INSTANCE_NAME`_Start() {
    // Initialize I2C
    `$INSTANCE_NAME`_IO_I2C_Start();
    `$INSTANCE_NAME`_IO_I2C_EzI2CSetBuffer1(sizeof(i2cregs), sizeof(i2cregs.rw), (volatile uint8_t *) &i2cregs);

    // Initialize LED
    `$INSTANCE_NAME`_IO_LED_Start();
    // Hard code the dim level to 1/2 for now
    `$INSTANCE_NAME`_IO_LED_Dim(1);
    // Set initial LED states
    while (!`$INSTANCE_NAME`_IO_LED_Ready());
    `$INSTANCE_NAME`_ClearLED(LED_BG);
    `$INSTANCE_NAME`_CommitLED();

    // Initialize CapSense
    `$INSTANCE_NAME`_Slider_Start();
    `$INSTANCE_NAME`_Slider_ScanAllWidgets();
}

void `$INSTANCE_NAME`_Task() {
    update_slider_regs();
}

#endif // (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_NATIVE)

/* [] END OF FILE */
