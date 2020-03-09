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

#define NUM_PHYSICAL_LEDS LED_COLUMNS
#define NUM_EFFECTIVE_PIXELS 32u

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

volatile slider_protocol_t i2cregs = {
    .rw = {
        .ctl = 0,
    },
    .ro = {
        .ver_major = 1,
        .ver_minor = 0,
        .keys = {0, 0, 0, 0},
    },
};

static void clearLED(uint32_t color);
static void setLEDWithInterpol(uint32_t offset, uint32_t color);
static void updateLEDWithInterpol();

#if (NUM_EFFECTIVE_PIXELS == NUM_PHYSICAL_LEDS)
    // TODO
    static void clearLED(uint32_t color) {
        LED_MemClear(color);
    }
    static void setLEDWithInterpol(uint32 offset, uint32_t color) {
        LED_Pixel((int32_t) offset, 0, color);
    }
    static void updateLEDWithInterpol() {
        LED_Trigger(1);
    }
#else
    #define _MAX_PIXEL (NUM_EFFECTIVE_PIXELS - 1)
    #define _MAX_LED (NUM_PHYSICAL_LEDS - 1)
    #define _LED_COMMON_DENOM (_MAX_PIXEL * _MAX_LED)
    uint32_t _effective_pixels[NUM_EFFECTIVE_PIXELS] = {0};
    static void clearLED(uint32_t color) {
        for (uint32_t i=0; i<NUM_EFFECTIVE_PIXELS; i++) {
            _effective_pixels[i] = color;
        }
    }
    static void setLEDWithInterpol(uint32_t offset, uint32_t color) {
        _effective_pixels[offset] = color;
    }
    static void updateLEDWithInterpol() {
        // Loop through all physical LEDs
        for (uint32_t led_idx=0; led_idx<NUM_PHYSICAL_LEDS; led_idx++) {
            // Rebase the physical LED # to common denominator
            uint32_t int_pos = led_idx * _LED_COMMON_DENOM / _MAX_LED;
            // Determine the interval
            uint32_t pixel0 = int_pos * _MAX_PIXEL / _LED_COMMON_DENOM;
            uint32_t pixel1 = pixel0 + 1;
            // Rebase the (now integer) interval to the common denominator
            uint32_t pixel0_pos = pixel0 * _LED_COMMON_DENOM / _MAX_PIXEL;
            uint32_t pixel1_pos = pixel1 * _LED_COMMON_DENOM / _MAX_PIXEL;
            if (pixel1 >= NUM_EFFECTIVE_PIXELS) {
                // Copy the pixel if we are at the 1.0 case
                LED_Pixel(_MAX_LED, 0, _effective_pixels[_MAX_PIXEL]);
            } else {
                // Actually do the interpolation
                uint32_t pixel_interpol = 0;
                // Copy pixel0 and pixel1 for easier manipulation
                uint32_t pixel0_data = _effective_pixels[pixel0];
                uint32_t pixel1_data = _effective_pixels[pixel1];
                uint8_t bitpos = 0;
                while (pixel0_data != 0 || pixel1_data != 0) {
                    // Read color
                    uint32_t color0 = pixel0_data & 0xff;
                    uint32_t color1 = pixel1_data & 0xff;
                    // Do the interpolation
                    uint32_t color_interpol;
                    color_interpol = (color0 * (pixel1_pos - int_pos) + color1 * (int_pos - pixel0_pos)) / (pixel1_pos - pixel0_pos);
                    // Set the interpolated value
                    pixel_interpol |= ((color_interpol & 0xff) << bitpos);
                    // Update bitpos and shift out the processed byte
                    pixel0_data >>= 8;
                    pixel1_data >>= 8;
                    bitpos += 8;
                }
                // Copy to LED display buffer
                LED_Pixel((int32_t) led_idx, 0, pixel_interpol);
            }
        }
        LED_Trigger(1);
    }
#endif

void setup() {
    /* Setup */
    // Initialize I2C
    I2C_Start();
    I2C_EzI2CSetBuffer1(sizeof(i2cregs), sizeof(i2cregs.rw), (volatile uint8_t *) &i2cregs);

    // Initialize LED
    LED_Start();
    // Hard code the dim level to 1/2 for now
    LED_Dim(1);
    // Set initial LED states
    if (LED_Ready()) {
        clearLED(LED_BG);
        updateLEDWithInterpol();
    }
    // Initialize CapSense
    Slider_Start();
    Slider_ScanAllWidgets();
}

void loop() {
    // Assert interrupt pin if necessary
    Pin_Interrupt_Out_Write((i2cregs.rw.ctl & BIT_CTL_INTR_TRIG) ? PIN_LOW : PIN_HIZ);
    if (Slider_IsBusy() == Slider_NOT_BUSY) {
        // Apply filters
        Slider_ProcessAllWidgets();
        bool dirty = false;
        // Check for all individual sensors
        if (Slider_IsWidgetActive(Slider_SEGMENTS_WDGT_ID)) {
            Pin_Status_LED_Write(PIN_HIGH);
            // TODO: might be possible to get the value from the widget config struct.
            for (uint32_t sensor=0; sensor<Slider_TOTAL_CSD_SENSORS; sensor++) {
                uint8_t state_bit_pos = sensor & 7;
                uint8_t state_reg_offset = sensor >> 3;
                uint8_t bit = (Slider_IsSensorActive(Slider_SEGMENTS_WDGT_ID, sensor) ? 1 : 0);

                volatile uint8_t *reg = &i2cregs.ro.keys[state_reg_offset];

                uint8_t bit_orig = ((*reg >> state_bit_pos) & 1);
                if (bit_orig != bit) {
                    dirty = true;
                    // Flip the bit
                    *reg ^= 1 << state_bit_pos;
                    if (bit) {
                        setLEDWithInterpol(sensor, LED_CURSOR);
                    } else {
                        setLEDWithInterpol(sensor, LED_BG);
                    }
                }
            }
        // No sensor is active, check if any of them were active
        } else if (i2cregs.ro.keys_active) {
            Pin_Status_LED_Write(PIN_LOW);
            // Nothing active, zeroing
            i2cregs.ro.keys_active = 0;
            // Clear LED
            clearLED(LED_BG);
            dirty = true;
        }
        // Re-arm the widget scanner
        Slider_ScanAllWidgets();

        // Update the LED
        if (dirty && LED_Ready()) {
            updateLEDWithInterpol();
        }

        // Sync the interrupt bit with dirty bit 
        if (dirty && (i2cregs.rw.ctl & BIT_CTL_INTR_EN)) {
            i2cregs.rw.ctl |= BIT_CTL_INTR_TRIG;
        }
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
