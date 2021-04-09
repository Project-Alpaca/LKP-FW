/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_API_Common.h"

#if (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_SERIAL)

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

static const lkps_hw_info_t hw_info = {
    .model = {'1', '5', '2', '7', '5', ' ', ' ', ' '},
    .device_class = 0xa0u,
    .chip_pn = {'0', '6', '6', '8', '7'},
    .unk_0xe = 0xffu,
    .fw_ver = 144,
    .unk_0x10 = 0,
    .unk_0x11 = 0x64u,
};

#define SYNC 0xffu
#define ESC 0xfdu

#define LKPS_LED_MAX_GLOBAL_BRIGHTNESS 63u

#define PIN_HIGH 1
#define PIN_HIZ 1
#define PIN_LOW 0

static lkps_state_t rx_state = state_recv_sync;
static uint8_t rx_checksum = 0;
static uint8_t tx_checksum = 0;
static uint8_t sensor_value_analog[`$INSTANCE_NAME`_Slider_TOTAL_CSD_SENSORS] = {0};

static bool auto_report = false;
static bool is_escaping = false;
static lkps_command_t rx_current_cmd = cmd_none;
static bool led_data_ready = false;


static void put_sync() {
    `$INSTANCE_NAME`_IO_UART_UartPutChar(SYNC);
    tx_checksum = 1;
}

static void put_byte(uint8_t data) {
    if (`$INSTANCE_NAME`_unlikely(data == SYNC || data == ESC)) {
        `$INSTANCE_NAME`_IO_UART_UartPutChar(ESC);
        `$INSTANCE_NAME`_IO_UART_UartPutChar(data - 1);
    } else {
        `$INSTANCE_NAME`_IO_UART_UartPutChar(data);
    }
    tx_checksum -= data;
}

static void put_checksum() {
    put_byte(tx_checksum);
}

static void put_cmd_with_args(lkps_command_t cmd, const void *buf, uint8_t len) {
    put_sync();

    put_byte((uint8_t) (cmd & 0xffu));
    put_byte(len);

    for (uint8_t off=0; off<len; off++) {
        put_byte(((uint8_t *) buf)[off]);
    }

    put_checksum();
}

static void put_cmd_no_args(lkps_command_t cmd) {
    put_sync();

    put_byte((uint8_t) (cmd & 0xffu));
    put_byte(0u);

    put_checksum();
}

static void raise_exception(lkps_exception_t code1) {
    put_sync();

    put_byte((uint8_t) (cmd_exception & 0xffu));
    put_byte(2);

    put_byte(0xffu);
    put_byte((uint8_t) (code1 & 0xffu));

    put_checksum();
}

static void reset_parser() {
    rx_state = state_recv_cmd;
    rx_checksum = 1;
    is_escaping = false;
    rx_current_cmd = cmd_none;
}

static void parse_request(uint8_t data) {
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
                    led_data_ready = false;
                    // brightness modifier
                    if (current_args_offset == 0) {
                        brightness_scratch = data;
                    } else {
                        uint8_t led_index = (current_args_offset - 1) / 3;
                        uint8_t led_color = (current_args_offset - 1) % 3;
                        uint32_t scratch_value = data;
                        scratch_value *= brightness_scratch;
                        scratch_value /= LKPS_LED_MAX_GLOBAL_BRIGHTNESS;
                        led_scratch |= (scratch_value & 0xffu) << (8 * (2 - led_color));
                        if (led_color == 2) {
                            `$INSTANCE_NAME`_SetLED(led_index, led_scratch);
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
            if (`$INSTANCE_NAME`_likely(rx_checksum == 0)) {
                switch (rx_current_cmd) {
                    case cmd_led_report:
                        // Sync current LED buffer.
                        if (`$INSTANCE_NAME`_IO_LED_Ready()) {
                            // update immediately when possible
                            `$INSTANCE_NAME`_CommitLED();
                        } else {
                            // deferred update
                            led_data_ready = true;
                        }
                        break;
                    case cmd_unk_0x09:
                    case cmd_unk_0x0a:
                        put_cmd_no_args(rx_current_cmd);
                        break;
                    case cmd_enable_slider_report:
                        auto_report = true;
                        // Immediately reply the current report buffer
                        put_cmd_with_args(cmd_input_report, &sensor_value_analog, sizeof(sensor_value_analog));
                        break;
                    case cmd_disable_slider_report:
                        auto_report = false;
                        put_cmd_no_args(cmd_disable_slider_report);
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
                        `$INSTANCE_NAME`_ClearLED(0);
                        `$INSTANCE_NAME`_CommitLED();
                        // Reset slider
                        `$INSTANCE_NAME`_Slider_Stop();
                        `$INSTANCE_NAME`_Slider_Start();
                        `$INSTANCE_NAME`_Slider_ScanAllWidgets();
                        // Respond
                        put_cmd_no_args(cmd_reset);
                        // Drain RX buffer to hopefully prevent unnecessary double resets
                        `$INSTANCE_NAME`_IO_UART_SpiUartClearRxBuffer();
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

static void handle_request() {
    uint32_t next = 0;

    // Attempt to drain the buffer
    while (`$INSTANCE_NAME`_IO_UART_SpiUartGetRxBufferSize() > 0) {
        next = `$INSTANCE_NAME`_IO_UART_UartGetByte();
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

static void prepare_input_report() {
    if (`$INSTANCE_NAME`_Slider_IsBusy() == `$INSTANCE_NAME`_Slider_NOT_BUSY) {
        // Apply advanced filters
        `$INSTANCE_NAME`_Slider_ProcessAllWidgets();
        // Check for all individual sensors
        if (`$INSTANCE_NAME`_Slider_IsWidgetActive(`$INSTANCE_NAME`_Slider_SEGMENTS_WDGT_ID)) {
            `$INSTANCE_NAME`_IO_Pin_Status_LED_Write(PIN_HIGH);
            for (uint8_t sensor=0; sensor<=`$INSTANCE_NAME`_Slider_SEGMENTS_NUM_SENSORS; sensor++) {
                uint8_t bit = (`$INSTANCE_NAME`_Slider_IsSensorActive(`$INSTANCE_NAME`_Slider_SEGMENTS_WDGT_ID, sensor) ? 1 : 0);
                if (`$INSTANCE_NAME`_likely(!bit)) {
                    sensor_value_analog[sensor] = 0;
                } else {
                    sensor_value_analog[sensor] = `$INSTANCE_NAME`_CalculateAnalogSensorValue(sensor);
                }
            }
        } else {
            // Clear report buffer
            memset(sensor_value_analog, 0, sizeof(sensor_value_analog));
            `$INSTANCE_NAME`_IO_Pin_Status_LED_Write(PIN_LOW);
        }

        // Re-arm the widget scanner
        `$INSTANCE_NAME`_Slider_ScanAllWidgets();

        // Handle auto report
        if (auto_report) {
            put_cmd_with_args(cmd_input_report, &sensor_value_analog, sizeof(sensor_value_analog));
        }
    }
}

static void check_deferred_led_update() {
    if (led_data_ready && `$INSTANCE_NAME`_IO_LED_Ready()) {
        `$INSTANCE_NAME`_CommitLED();
        led_data_ready = false;
    }
}

void `$INSTANCE_NAME`_Start() {
    // Initialize UART
    `$INSTANCE_NAME`_IO_UART_Start();

    // Initialize LED
    `$INSTANCE_NAME`_IO_LED_Start();
    // Hard code the dim level to 1/2 for now
    `$INSTANCE_NAME`_IO_LED_Dim(1);
    // Set initial LED states
    while (!`$INSTANCE_NAME`_IO_LED_Ready());
    `$INSTANCE_NAME`_ClearLED(0);
    `$INSTANCE_NAME`_CommitLED();

    // Initialize CapSense
    `$INSTANCE_NAME`_Slider_Start();
    `$INSTANCE_NAME`_Slider_ScanAllWidgets();
}

void `$INSTANCE_NAME`_Task() {
    prepare_input_report();
    handle_request();
    check_deferred_led_update();
}

#endif // (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_SERIAL)

/* [] END OF FILE */
