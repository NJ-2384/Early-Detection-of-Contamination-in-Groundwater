# Tarang

ESP32 firmware for a water-quality monitoring node. It reads temperature, pH and TDS sensors, gets a GPS fix from an A7672S 4G modem, and pushes the readings to a [Blynk](https://blynk.io) dashboard over HTTPS.

## Repository layout

| Sketch | Purpose |
|---|---|
| [Main/](Main/Main.ino) | Full firmware: reads all sensors, uploads to Blynk via the modem |
| [Temperature/](Temperature/Temperature.ino) | Stand-alone DS18B20 test (°C / °F over serial) |
| [PH/](PH/PH.ino) | Stand-alone analog pH sensor test and calibration |
| [TDS/](TDS/TDS.ino) | Stand-alone TDS sensor test (median-filtered, temperature-compensated) |
| [GSM_Serial/](GSM_Serial/GSM_Serial.ino) | Serial-monitor-to-modem AT command bridge for debugging |

Each folder is an independent Arduino sketch. Use the single-sensor sketches to verify wiring and calibrate before flashing `Main`.

## Hardware

- ESP32 board
- DS18B20 waterproof temperature sensor (1-Wire)
- Analog pH meter module
- Analog TDS meter module (Keyestudio KS0429 / Gravity-style)
- A7672S 4G LTE modem with GNSS
- Relay (used by `Main` to control modem power)

### Pin map (as used in `Main`)

| Signal | Pin |
|---|---|
| TDS analog out | `A0` |
| pH analog out | `A3` |
| DS18B20 data | GPIO 4 |
| Relay | GPIO 19 |
| Modem RX / TX (Serial2) | GPIO 17 / GPIO 16 |

The modem runs at 115200 baud, 8N1.

## Software setup

1. Install the Arduino IDE (or `arduino-cli`) with the ESP32 board package.
2. Install libraries: **OneWire** and **DallasTemperature**.
3. Open a sketch folder, select your ESP32 board and port, and upload.
4. Open the serial monitor at **115200 baud**.

## How `Main` works

Each loop iteration:

1. Reads temperature (DS18B20), pH (analog) and TDS (30-sample median filter, compensated to 25 °C).
2. Prints the values to serial.
3. Initializes the modem HTTP stack and GNSS via AT commands.
4. Sends each value as an HTTP GET to the Blynk external API:

   | Blynk pin | Value |
   |---|---|
   | `v0` | Temperature (°C) |
   | `v1` | TDS (ppm) |
   | `v2` | pH |
   | `v3` | Latitude |
   | `v4` | Longitude |

5. Waits 5 s and repeats.

Oxygen and iron levels are currently hard-coded placeholder values printed to serial only; they are not measured or uploaded.

## Calibration

- **pH**: adjust `calibration_value` in `PH/PH.ino` (and the matching value in `Main`'s `PH()`), using a pH 7 buffer solution.
- **TDS**: `temperature` is fixed at 25 °C in `TDS/TDS.ino`; `Main` also uses the 25 °C default rather than the live DS18B20 reading.
- ADC reference is set by `VREF` (3.3 V).

## Security note

`Main/Main.ino` contains a hard-coded Blynk auth token in its URLs. Move it to a constant or untracked config header, and rotate the token in the Blynk console, before making this repository public.

## Credits

TDS and temperature sketches are adapted from the Keyestudio KS0429 example, [Random Nerd Tutorials](https://randomnerdtutorials.com), and the Dallas Temperature library example.
