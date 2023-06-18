# LKP-FW

LKP firmware.

## Building and flashing

1. **IMPORTANT:** Due to the nature of PSoC Creator projects, it is recommended to create a dedicated copy of this repository before building to prevent polluting the upstream codebase when creating a PR.
2. Open the main workspace (LKP.cywrk) in PSoC Creator.
3. Within PSoC Creator, open the project targeting the correct board (e.g. LKP-FW-Core for LKP-Core) and set it as active project.
4. Open the TopDesign schematic, change LKPd's protocol and LED parameters to reflect the actual requirements.
5. Save the project and build.
6. Connect KitProg with target board.
7. Use the menu option Debug -\> Program to flash the firmware to the board.

## Protocols

The firmware supports 3 different protocols, each has different LED behaviors. Protocols are configurable at build time by changing LKPd's parameters.

### Native I2C

Uses an I2C-based custom protocol with an interrupt signal output. LEDs are controlled on the LKP side rather than the host. Provides digital ON/OFF readings for all 32 sensor electrodes. Note that by default LKP expects 5V devices, so level shifter (e.g. LKP-IF-Qwiic) is required when interfacing with 1.8V or 3.3V devices if such devices are not 5V tolerant or when the pull-up resistors are hard-wired to the board.

I2C is configured to communicate at up to 1MHz (Fm+) mode, device address 0x08 with 16-bit register addressing.

Protocol is subject to change in the future.

### 837-15275/15330

Uses [837-15275/15330][sega-slider] (i.e. official Project DIVA Arcade/Chunithm touch slider made by SEGA) protocol. LEDs are fully controlled from the host side. Provides analog readings derived from raw counts for all sensor electrodes. Note that by default LKP expects 5V devices, so level shifter (e.g. LKP-IF-Qwiic) is required on the RX side when interfacing with 1.8V or 3.3V devices and TX side if devices are not 5V tolerant.

### CapSense Tuner

Exposes the CapSense DSRAM over I2C for peek/poke. Intended to be used with PSoC CapSense Tuner for troubleshooting. LED pin is unused and floating.

I2C is configured to communicate at up to 1MHz (Fm+) mode, device address 0x08 with 16-bit register addressing.

## Legacy projects

LKP-{Native,Serial,Tuner} are legacy projects targeting LKP-Core only. They are kept for historical purpose and sometimes for quick prototyping of new features. Use of these projects for production are not recommended.

[sega-slider]: https://gist.github.com/dogtopus/b61992cfc383434deac5fab11a458597
