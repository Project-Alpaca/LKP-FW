# LKP-FW

LKP firmware.

## Variations

There are 2 variations of firmware, each implements different communication protocol and has different LED behaviors.

### Native

Uses an I2C-based custom protocol with an interrupt signal output. LEDs are controlled on the LKP side rather than the host. Provides digital ON/OFF readings for all 32 sensor electrodes. Uses open-drain I/O and therefore compatible with 1.8V-5V logic.

Protocol is subject to change in the future.

### Serial

Uses [837-15275/15330][sega-slider] (i.e. official Project DIVA Arcade/Chunithm touch slider made by SEGA) protocol. LEDs are fully controlled from the host side. Provides analog readings derived from raw counts for all sensor electrodes. Note that the TX pin sends out 5V signal, so level shifter is required when interfacing with 1.8V or 3.3V devices without 5V tolerance.

[sega-slider]: https://gist.github.com/dogtopus/b61992cfc383434deac5fab11a458597
