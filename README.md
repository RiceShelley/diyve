# DIYVE

Diyve is an open source DIY dive computer built on a RP2040. Learn more at [diyve.org](https://diyve.org).

## WARNING

Experimental project not intended as a primary or sole dive computer.

---

## What it does

- Records depth and temperature at 1 Hz to an SD card during dives
- Displays live depth (ft), temperature (F), elapsed time, and max depth on an e-paper screen
- Exports dive logs to [UDDF 3.2.0](https://www.streit.cc/extern/uddf_v321/en/index.html) for import into [Subsurface](https://subsurface-divelog.org/) and other dive log software
- Four screens navigated with two buttons: Menu, Dive, Logs, Settings
- Real-time clock keeps accurate timestamps even when powered off

---

## Hardware

### Bill of Materials

| # | Part | Role | Price |
|---|------|------|-------|
| 1 | [Raspberry Pi Pico (RP2040)](https://www.amazon.com/dp/B0BDLHMQ9C) | Main MCU - dual-core ARM Cortex-M0+ | $15 |
| 2 | [MS5803 pressure sensor](https://www.amazon.com/dp/B0BP8JYWGN) | Depth + temperature via I2C | $70 |
| 3 | [DS3231 RTC](https://www.amazon.com/dp/B09KPC8JZQ) | Accurate timestamps | $8 |
| 4 | [Waveshare 1.54" e-paper (200x200)](https://www.amazon.com/dp/B072Q4WTWH) | Display | $18 |
| 5 | [SD card breakout](https://www.amazon.com/dp/B0BV8ZQ81F) | FAT filesystem logging | $7 |
| 6 | [SD card](https://www.amazon.com/dp/B0CYT2KL98) | Storage | $10 |
| 7 | [LiPo battery](https://www.amazon.com/dp/B07BTLN9SX) | Power | $8 |
| 8 | [TP4056 LiPo charger](https://www.amazon.com/dp/B07PKND8KG) | Charging | $6 |
| 9 | [Schottky diode](https://www.amazon.com/dp/B0FC2CTR7F) | Reverse-polarity protection | $6 |
| 10 | [Slide switch](https://www.amazon.com/dp/B0FG9872XQ) | Power on/off | $9 |
| 11 | [2x momentary buttons](https://www.amazon.com/dp/B09R3ZPWJ7) | Navigation (TOP) + select (SIDE) | $7 |
| 12 | [Perfboard (52x72.5 mm)](https://www.amazon.com/dp/B0CZ6W18F6) | Custom PCB base | $5 |
| 13 | [Pin headers](https://www.amazon.com/dp/B0FCTSSLK9) | Connections | $8 |
| 14 | [Wire](https://www.amazon.com/dp/B09Y8GWYYD) | Wiring | $13 |
| 15 | [GoPro housing](https://www.amazon.com/dp/B08LD4VXGL) | Waterproof enclosure | $19 |
| 16 | [Two-part epoxy](https://www.amazon.com/dp/B001Z3C3AG) | Sealing | $6 |
| - | [RP2040 Debug Probe](https://www.amazon.com/dp/B0CQJB5FC5) | Flashing (dev only) | $30 |

**Total (excluding debug probe): ~$215**

### GPIO Pinout

#### Buttons
| GPIO | Function |
|------|----------|
| GPIO 28 | TOP button - navigate (active low, pull-up) |
| GPIO 27 | SIDE button - select / long-press (active low, pull-up) |

#### I2C Sensors
| GPIO | Function |
|------|----------|
| GPIO 2 | MS5803 SDA (I2C1) |
| GPIO 3 | MS5803 SCL (I2C1) |
| Default SDA/SCL | DS3231 (I2C0) |

#### SD Card (SPI0)
| GPIO | Function |
|------|----------|
| GPIO 16 | MISO |
| GPIO 17 | CS |
| GPIO 18 | SCK |
| GPIO 19 | MOSI |

#### E-Paper Display (SPI1)
| GPIO | Function |
|------|----------|
| GPIO 10 | CLK |
| GPIO 11 | MOSI |
| GPIO 13 | CS |
| GPIO 6  | DC |
| GPIO 7  | RST |
| GPIO 8  | BUSY |

All pin assignments are defined in `src/include/diyve_pins.h`.

---

## Firmware

**Language:** C
**Build system:** CMake + Pico SDK 2.2.0
**Output:** `diyve.uf2` / `diyve.elf`

### Dependencies (fetched automatically via CMake `FetchContent`)

- [`no-OS-FatFS-SD-SPI-RPi-Pico`](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico) - FatFS SD driver (carlk3)
- [`pico-ds3231`](https://github.com/antgon/pico-ds3231) - DS3231 RTC driver (antgon)

### Prerequisites

- [Pico SDK 2.2.0](https://github.com/raspberrypi/pico-sdk) installed and on `PICO_SDK_PATH`
- CMake >= 3.13
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- OpenOCD with CMSIS-DAP support (for flashing)
- Python 3 (for RTC sync after flashing)

### Build

```bash
./build.sh
```

This runs `cmake -B build -S .` followed by `make -C build/`.

### Flash

```bash
./program.sh
```

This builds, flashes via OpenOCD + CMSIS-DAP, and automatically syncs the RTC from your PC's clock over UART.

To use a different serial port:

```bash
cmake -B build -DSYNC_PORT=/dev/ttyUSB0
cmake --build build -t flash
```

---

## Source Layout

```
src/
  main.c                          - init: stdlib, GPIO, RTC, MS5803, EPD, then GUI loop
  include/diyve_pins.h            - all GPIO assignments
  ms5803_i2c/                     - pressure sensor driver, 2nd-order temp compensation
  dive_logger/                    - FatFS SD logging, 15-byte binary records
  display/                        - EPD init + unit conversion helpers (C -> F, m -> ft)
  gui/
    gui.c                         - screen framework
    screen_menu.c                 - Menu screen (clock, pressure, SD status)
    screen_dive.c                 - Dive screen (live depth/temp/time/max)
    screen_logs.c                 - Logs screen (browse .bin files)
    screen_settings.c             - Settings screen
  vendor/epd_waveshare_1in54/     - Waveshare driver + GUI_Paint + fonts 8-24px

tools/
  dive_log_to_uddf.py             - binary log -> UDDF 3.2.0 XML
  sync_time.py                    - sync RTC from PC over UART on boot

mechanical/
  main_holder.scad                - main housing (52x72.5 mm)
  button_holder.scad              - button T-slot mechanism
  pressure_sensor_enclosure.scad  - waterproof sensor box
  plastic_clip.scad               - wrist attachment clip

test_data/                        - real dive logs from prototype testing
```

---

## Dive Logging

Dives are logged at 1 Hz as 15-byte binary records to `.bin` files on the SD card.

**Record format:** `<HBBBBBff`
- `uint16` year
- `uint8` month, day, hour, minute, second
- `float32` temperature (C)
- `float32` depth (m)

**Depth calculation:**
```
gauge_pressure = abs_pressure - (101325 Pa + 200 Pa offset)
depth_m        = gauge_pressure / (1025 * 9.81)
```
Water density: 1025 kg/m3 (saltwater)

**Button timing:**
- Poll: 50 ms
- Debounce: 150 ms
- Long-press threshold: 1000 ms

**E-paper refresh:** partial mode (~100 ms vs full ~1 s)

---

## Exporting Dive Logs

Convert a binary log to UDDF for Subsurface or other dive log software:

```bash
python3 tools/dive_log_to_uddf.py -i dive_004.bin -o dive_004.uddf
```

Output is UDDF 3.2.0 XML, importable directly into [Subsurface](https://subsurface-divelog.org/).

---

## Mechanical

The enclosure is a GoPro housing sealed with two-part epoxy. OpenSCAD source files for all custom-printed parts are in `mechanical/`.

---

## License

MIT - see [LICENSE](LICENSE).
