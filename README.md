# Wake Up Test

This test code shows how to configure deep sleep with the fastest wake up time pissible as far as I know (currently 44ms).
Example is based on GPIO example from ESP-IDF

## GPIO functions:

### Watcher:

| GPIO     | Direction | Configuration                         |  Description                                            |
| -------- | --------- | ------------------------------------- | ------------------------------------------------------- |
| GPIO2    | output    |                                       | Sends wake up signal to sleeper                         |
| GPIO18   | input     | pulled up, interrupt from rising edge | Receives ACK from sleeper                               |
| GPIO0    | input     | pulled up, interrupt from rising edge | Tied to boot button on board, triggers wake up sequence |

### Sleper:

| GPIO     | Direction | Configuration               |  Description                                       |
| -------- | --------- | --------------------------- | -------------------------------------------------- |
| GPIO18   | output    |                             | Sends wake up ACK signal to sleeper                |
| GPIO2    | input     | tied to EXT0 wake up source | When pulled from high to low it wakes the board up |

## Flashing

When flashing the ESP32, it is usally necessary to disconnect pin GPIO2 from the MCU, otherwise the idf will not detect the board.

## Test

 1. Connect watcher GPIO18 with sleeper GPIO18
 2. Connect watcher GPIO2 with sleeper GPIO2
 3. Generate pulses on GPIO0 (boot button) to test wake up times. 

### Hardware Required

- Two development boards with ESP32/ESP32-S2/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
- Two USB cables for Power supply and programming
- Some jumper wires to connect GPIOs.

## Change between light and deep sleep

The default behavior of the system is going to light sleep, to test deep sleep define the macro DEEP_SLEEP in sleeper.c.

## Deep sleep exit time optimisations
- Serial flasher config
    + flash SPI mode -> QUIO (improvement of ~5ms)
    + flash SPI speed -> 80MHz (improvement of ~5ms)

- Component config 
    + default log verbosity -> no output (improvement of ~120 ms)

- Bootloader config
    + bootloader log verbosity -> warning (improvement of ~100ms)
    + bootloader optimisation level -> optimize for performance (-O2) (improvement of ~ 5ms)
    + skip image validation when exiting deep sleep (improvement of ~55ms)

## Deep sleep vs light sleep

| Metric       | Deep sleep | Light sleep |  Comments                                                                                 |
| ------------ | ---------- | ----------- | ----------------------------------------------------------------------------------------- |
| Wake up time | 44ms       | 4ms         | Wake up from deep sleep implies full reset, while, from light sleep, it resumes execution |
| Consumption  | 9mA - 11uA | 15mA - 2mA  | Consumption is tied to the board used, mainly to the built in voltage regulators          |

Find a more detailed consumption analysis among diferent boards [here](https://diyi0t.com/reduce-the-esp32-power-consumption/).
