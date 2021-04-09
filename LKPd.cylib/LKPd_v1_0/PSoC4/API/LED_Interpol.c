/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_LED_Interpol.h"

#if (`$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS == `$INSTANCE_NAME`_NUM_PHYSICAL_LEDS)
    // TODO
    void `$INSTANCE_NAME`_ClearLED(uint32_t color) {
        `$INSTANCE_NAME`_IO_LED_MemClear(color);
    }
    void `$INSTANCE_NAME`_SetLED(uint32 offset, uint32_t color) {
        `$INSTANCE_NAME`_IO_LED_Pixel((int32_t) offset, 0, color);
    }
    void `$INSTANCE_NAME`_CommitLED() {
        `$INSTANCE_NAME`_IO_LED_Trigger(1);
    }
#else
    #define _MAX_PIXEL (`$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS - 1)
    #define _MAX_LED (`$INSTANCE_NAME`_NUM_PHYSICAL_LEDS - 1)
    #define _LED_COMMON_DENOM (_MAX_PIXEL * _MAX_LED)
    static uint32_t _effective_pixels[`$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS] = {0};
    void `$INSTANCE_NAME`_ClearLED(uint32_t color) {
        for (uint32_t i=0; i<`$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS; i++) {
            _effective_pixels[i] = color;
        }
    }
    void `$INSTANCE_NAME`_SetLED(uint32_t offset, uint32_t color) {
        _effective_pixels[offset] = color;
    }
    void `$INSTANCE_NAME`_CommitLED() {
        // Loop through all physical LEDs
        for (uint32_t led_idx=0; led_idx<`$INSTANCE_NAME`_NUM_PHYSICAL_LEDS; led_idx++) {
            // Rebase the physical LED # to common denominator
            uint32_t int_pos = led_idx * _LED_COMMON_DENOM / _MAX_LED;
            // Determine the interval
            uint32_t pixel0 = int_pos * _MAX_PIXEL / _LED_COMMON_DENOM;
            uint32_t pixel1 = pixel0 + 1;
            // Rebase the (now integer) interval to the common denominator
            uint32_t pixel0_pos = pixel0 * _LED_COMMON_DENOM / _MAX_PIXEL;
            uint32_t pixel1_pos = pixel1 * _LED_COMMON_DENOM / _MAX_PIXEL;
            if (pixel1 >= `$INSTANCE_NAME`_NUM_EFFECTIVE_PIXELS) {
                // Copy the pixel if we are at the 1.0 case
                `$INSTANCE_NAME`_IO_LED_Pixel(_MAX_LED, 0, _effective_pixels[_MAX_PIXEL]);
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
                `$INSTANCE_NAME`_IO_LED_Pixel((int32_t) led_idx, 0, pixel_interpol);
            }
        }
        `$INSTANCE_NAME`_IO_LED_Trigger(1);
    }
#endif
/* [] END OF FILE */
