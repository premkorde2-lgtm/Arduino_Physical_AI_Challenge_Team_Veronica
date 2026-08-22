#  Wearable Watch Custom PCB Firmware & Flashing Guide

This directory contains the firmware running on the custom ESP32 wearable watch board. The system integrates real-time IMU motion tracking, GPS location parsing, BLE Air Mouse control, and direct Telegram safety alerting over Wi-Fi.

---

##  Custom PCB Flashing Instructions

Since this is a custom PCB without an integrated USB-to-UART auto-reset circuit, flashing requires an external programming bridge.

### Flashing Hardware Setup
* **Flashing Bridge:** Use a standard USB-to-UART converter (or a secondary ESP32 development board).
* **ESP32 Programmer Hack:** If using an external ESP32 board, connect its **BOOT pin to GND** and utilize its onboard UART bridge IC strictly for TX/RX flashing.
* **Boot Mode Entry:** Hold the custom PCB **BOOT (GPIO 0)** button low during power-up/reset to enter UART download mode.

```text
[ Flashing Tool / ESP32 Bridge ]            [ Custom Watch PCB ]
              TXD ─────────────────────────► RXD 
              RXD ◄───────────────────────── TXD 
              GND ─────────────────────────► GND
             3.3V ─────────────────────────► 3.3V
GPIO 0 ──► [BOOT Button] ──► GND
================================================================================
                    ESP32 WEARABLE WATCH - HARDWARE PINOUT
================================================================================

[ 1. MPU-6050 ACCELEROMETER / GYROSCOPE (I2C) ]
--------------------------------------------------------------------------------
ESP32 Pin       MPU-6050 Pin    Protocol / Notes
GPIO 21   <---> SDA             I2C Data
GPIO 22   <---> SCL             I2C Clock
3.3V      <---> VCC             Power Supply
GND       <---> GND             Common Ground

[ 2. GPS MODULE (UART 2) ]
--------------------------------------------------------------------------------
ESP32 Pin       GPS Module Pin  Protocol / Notes
GPIO 16   <---  TX              Hardware Serial RX2 (Cross-connected)
GPIO 17   --->  RX              Hardware Serial TX2 (Cross-connected)
3.3V/5V   <---> VCC             Power Supply
GND       <---> GND             Common Ground
* Settings: 9600 Baud | 8N1 | NMEA $GNRMC Protocol


[ 3. DIGITAL INPUT BUTTONS ]
--------------------------------------------------------------------------------
ESP32 Pin       Component       Circuit Configuration
GPIO 32   <---> MENU Button     GPIO 32 ─── Button ─── 3.3V (INPUT_PULLDOWN)
GPIO 5    <---> NEXT Button     GPIO 5  ─── Button ─── 3.3V (INPUT_PULLDOWN / Mouse Click)


[ 4. CAUTION MODE DIGITAL OUTPUTS ]
--------------------------------------------------------------------------------
ESP32 Pin       Target Load     Notes
GPIO 19   --->  Output 1        Toggles every 3s (Use MOSFET/Transistor driver)
GPIO 26   --->  Output 2        Toggles every 3s (Use MOSFET/Transistor driver)


[ 5. TFT DISPLAY (SPI) ]
--------------------------------------------------------------------------------
Protocol: SPI
* Pin assignments are configured in the TFT_eSPI library (User_Setup.h).


================================================================================
                  CUSTOM PCB UART FLASHING WIRING (NO AUTO-RESET)
================================================================================

[ ESP32 Programmer Hack / USB-to-UART Bridge ]
--------------------------------------------------------------------------------
Programmer Pin                  Custom Watch PCB Pin
TXD                     --->    RXD (GPIO 3)
RXD                     <---    TXD (GPIO 1)
GND                     <--->   GND
3.3V                    <--->   3.3V
GPIO 0 (BOOT)           --->    Press/Hold BOOT Button to GND during reset
--------------------------------------------------------------------------------



## Wireless & System Protocol Stack

                           ┌───────────────────────────┐
                           │      Custom ESP32 Watch   │
                           └─────────────┬─────────────┘
                                         │
        ┌────────────────────────────────┼────────────────────────────────┐
        │ I²C                            │ UART (9600 Baud)               │ SPI
        ▼                                ▼                                ▼
  ┌───────────┐                    ┌───────────┐                    ┌───────────┐
  │ MPU-6050  │                    │ GPS ($GNRMC)│                   │ TFT SPI   │
  └─────┬─────┘                    └───────────┘                    └───────────┘
        │
        ├─► BLE HID Interface ─────────► Air Mouse Motion (Cursor Control)
        │
        └─► Wi-Fi (802.11) ────────────► HTTPS / TLS (Port 443) ──► Telegram API

