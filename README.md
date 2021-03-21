# LKP-FW

LKP firmware.

## Variations

There are 3 variations of firmware, each implements different communication protocol and has different LED behaviors.

### Native

Uses an I2C-based custom protocol with an interrupt signal output. LEDs are controlled on the LKP side rather than the host. Provides digital ON/OFF readings for all 32 sensor electrodes. Note that by default LKP expects 5V devices, so level shifter (e.g. LKP-IF-Qwiic) is required when interfacing with 1.8V or 3.3V devices if such devices are not 5V tolerant or when the pull-up resistors are hard-wired to the board.

I2C is configured to communicate at 400kHz (Fm) mode, device address 0x08 with 16-bit register addressing.

Protocol is subject to change in the future.

### Serial

Uses [837-15275/15330][sega-slider] (i.e. official Project DIVA Arcade/Chunithm touch slider made by SEGA) protocol. LEDs are fully controlled from the host side. Provides analog readings derived from raw counts for all sensor electrodes. Note that by default LKP expects 5V devices, so level shifter (e.g. LKP-IF-Qwiic) is required on the RX side when interfacing with 1.8V or 3.3V devices and TX side if devices are not 5V tolerant.

### Tuner

Exposes the CapSense DSRAM over I2C for peek/poke. Intended to be used with PSoC CapSense Tuner for troubleshooting. LED pin is unused and floating.

I2C is configured to communicate at 1MHz (Fm+) mode, device address 0x08 with 16-bit register addressing.

[sega-slider]: https://gist.github.com/dogtopus/b61992cfc383434deac5fab11a458597
