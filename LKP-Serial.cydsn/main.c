/* ========================================
 *
 * Copyright dogtopus, 2019
 * Released to public domain
 * SPDX-License-Identifier: Unlicense
 *
 * ========================================
*/
#include "project.h"
#include "utils.h"
#include "analog_slider.h"
#include <stdbool.h>

// Protocol: https://gist.github.com/dogtopus/b61992cfc383434deac5fab11a458597
typedef enum {
    cmd_none = 0x00,
    cmd_input_report = 0x01,
    cmd_led_report,
    cmd_enable_slider_report,
    cmd_disable_slider_report,
    cmd_unk_0x09 = 0x09,
    cmd_unk_0x0a,
    cmd_reset = 0x10,
    cmd_exception = 0xee,
    cmd_get_hw_info = 0xf0,
} lkps_command_t;

typedef enum {
    exc_wrong_checksum = 0x1u,
    exc_bus_error = 0x2u,
    exc_internal = 0xedu,
} lkps_exception_t;

typedef enum {
    state_recv_sync,
    state_recv_cmd,
    state_recv_len,
    state_recv_args,
    state_recv_sum,
} lkps_state_t;

typedef struct {
    char model[8];
    uint8_t device_class;
    char chip_pn[5];
    uint8_t unk_0xe;
    uint8_t fw_ver;
    uint8_t unk_0x10;
    uint8_t unk_0x11;
} __attribute__((packed)) lkps_hw_info_t;

const lkps_hw_info_t hw_info = {
    .model = {'1', '5', '2', '7', '5', ' ', ' ', ' '},
    .device_class = 0xa0u,
    .chip_pn = {'0', '6', '6', '8', '7'},
    .unk_0xe = 0,
    .fw_ver = 144,
    .unk_0x10 = 0,
    .unk_0x11 = 0,
};

#define SYNC 0xffu
#define ESC 0xfdu

#define LKPS_LED_MAX_GLOBAL_BRIGHTNESS 63u

#define PIN_HIGH 1
#define PIN_HIZ 1
#define PIN_LOW 0

#define NUM_PHYSICAL_LEDS LED_COLUMNS
#define NUM_EFFECTIVE_PIXELS 32u

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


lkps_state_t rx_state = state_recv_cmd;
uint8_t rx_checksum = 0;
uint8_t tx_checksum = 0;
uint8_t sensor_value_analog[Slider_TOTAL_CSD_SENSORS] = {0};

bool auto_report = false;
bool is_escaping = false;
lkps_command_t rx_current_cmd = cmd_none;


void put_sync() {
    UART_UartPutChar(SYNC);
    tx_checksum = 1;
}

void put_byte(uint8_t data) {
    if (unlikely(data == SYNC || data == ESC)) {
        UART_UartPutChar(ESC);
        UART_UartPutChar(data - 1);
    } else {
        UART_UartPutChar(data);
    }
    tx_checksum -= data;
}

void put_checksum() {
    put_byte(tx_checksum);
}

void put_cmd_with_args(lkps_command_t cmd, const void *buf, uint8_t len) {
    put_sync();

    put_byte((uint8_t) (cmd & 0xffu));
    put_byte(len);

    for (uint8_t off=0; off<len; off++) {
        put_byte(((uint8_t *) buf)[off]);
    }

    put_checksum();
}

void put_cmd_no_args(lkps_command_t cmd) {
    put_sync();

    put_byte((uint8_t) (cmd & 0xffu));
    put_byte(0u);

    put_checksum();
}

void raise_exception(lkps_exception_t code1) {
    put_sync();

    put_byte((uint8_t) (cmd_exception & 0xffu));
    put_byte(2);

    put_byte(0xffu);
    put_byte((uint8_t) (code1 & 0xffu));

    put_checksum();
}

void reset_parser() {
    rx_state = state_recv_cmd;
    rx_checksum = 1;
    is_escaping = false;
    rx_current_cmd = cmd_none;
}

void parse_request(uint8_t data) {
    static uint32_t led_scratch = 0x0u;
    static uint8_t brightness_scratch = 0;
    static uint8_t args_bytes_remaining = 0;
    static uint8_t current_args_offset = 0;

    switch (rx_state) {
        case state_recv_cmd:
            rx_current_cmd = data;
            rx_state = state_recv_len;
            break;
        case state_recv_len:
            args_bytes_remaining = data;
            current_args_offset = 0;
            // TODO maybe verify length here?
            // Go directly to checksum verification if there's no args
            if (args_bytes_remaining != 0) {
                rx_state = state_recv_args;
            } else {
                rx_state = state_recv_sum;
            }
            break;
        case state_recv_args: {
            switch (rx_current_cmd) {
                // Receive arguments
                case cmd_led_report: {
                    // brightness modifier
                    if (current_args_offset == 0) {
                        brightness_scratch = data;
                    } else {
                        uint8_t led_index = (current_args_offset - 1) / 3;
                        uint8_t led_color = (current_args_offset - 1) % 3;
                        uint32_t scratch_value = data;
                        scratch_value *= brightness_scratch;
                        scratch_value /= LKPS_LED_MAX_GLOBAL_BRIGHTNESS;
                        led_scratch |= (scratch_value & 0xffu) << (8 * (3 - led_color));
                        if (led_color == 2) {
                            setLEDWithInterpol(led_index, led_scratch);
                            led_scratch = 0;
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            // Update offsets and check for end of args
            args_bytes_remaining -= 1;
            current_args_offset += 1;
            if (args_bytes_remaining == 0) {
                rx_state = state_recv_sum;
            }
            break;
        }
        case state_recv_sum: {
            if (likely(rx_checksum == 0)) {
                switch (rx_current_cmd) {
                    case cmd_led_report:
                        // Sync current LED buffer.
                        updateLEDWithInterpol();
                        break;
                    case cmd_unk_0x09:
                    case cmd_unk_0x0a:
                        put_cmd_no_args(rx_current_cmd);
                        break;
                    case cmd_enable_slider_report:
                        auto_report = true;
                        break;
                    case cmd_disable_slider_report:
                        auto_report = false;
                        break;
                    case cmd_input_report:
                        put_cmd_with_args(cmd_input_report, &sensor_value_analog, sizeof(sensor_value_analog));
                        break;
                    case cmd_get_hw_info:
                        put_cmd_with_args(cmd_get_hw_info, &hw_info, sizeof(hw_info));
                        break;
                    case cmd_exception:
                        // TODO what to do?
                        break;
                    case cmd_reset:
                        // Clear LED
                        clearLED(0);
                        updateLEDWithInterpol();
                        // Reset slider
                        Slider_Stop();
                        Slider_Start();
                        // Respond
                        put_cmd_no_args(cmd_reset);
                        break;
                    default:
                        break;
                }
            } else {
                raise_exception(exc_wrong_checksum);
            }
            rx_state = state_recv_sync;
            break;
        }
        // Ignore garbage
        case state_recv_sync:
            break;
    }
}

void handle_request() {
    uint32_t next = 0;

    // Attempt to drain the buffer
    while (UART_SpiUartGetRxBufferSize() > 0) {
        next = UART_UartGetByte();
        // This shouldn't happen?
        if (next & 0xffffff00u) {
            reset_parser();
            raise_exception(exc_bus_error);
            // TODO anything else?
            break;
        }
        uint8_t next_byte = (uint8_t) (next & 0xff);
        // Handle ESC and SYNC
        if (next_byte == ESC) {
            is_escaping = true;
        } else if (next_byte == SYNC) {
            reset_parser();
            // Checksum is implicitly updated here
        } else {
            if (is_escaping) {
                is_escaping = false;
                next_byte += 1;
            }
            rx_checksum -= next_byte;
            parse_request(next_byte);
        }
    }
}

void prepare_input_report() {
    if (Slider_IsBusy() == Slider_NOT_BUSY) {
        // Apply advanced filters
        Slider_ProcessAllWidgets();
        // Check for all individual sensors
        if (Slider_IsWidgetActive(Slider_SEGMENTS_WDGT_ID)) {
            Pin_Status_LED_Write(PIN_HIGH);
            for (uint8_t sensor=0; sensor<=Slider_SEGMENTS_NUM_SENSORS; sensor++) {
                uint8_t bit = (Slider_IsSensorActive(Slider_SEGMENTS_WDGT_ID, sensor) ? 1 : 0);
                if (likely(!bit)) {
                    sensor_value_analog[sensor] = 0;
                } else {
                    sensor_value_analog[sensor] = calculate_analog_sensor_value(sensor);
                }
            }
            if (auto_report) {
                put_cmd_with_args(cmd_input_report, &sensor_value_analog, sizeof(sensor_value_analog));
            }
        } else {
            Pin_Status_LED_Write(PIN_LOW);
        }
        // Re-arm the widget scanner
        Slider_ScanAllWidgets();
    }
}


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

    reset_parser();
}

void loop() {
    prepare_input_report();
    handle_request();
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
