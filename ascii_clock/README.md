# Pico 2 W ASCII Clock ⏰

A **terminal-based ASCII clock** for the **Raspberry Pi Pico 2 W**, written in C++.

The clock retrieves time over **WiFi using NTP**, displays it as large ASCII digits over **USB serial or UART**, and provides a **manual fallback** if WiFi/NTP is unavailable.

This project is designed to be simple, reliable, and fully embedded-friendly — no dynamic memory abuse or unnecessary UI libraries, just solid firmware.

---

## ✨ Features

* 📡 **WiFi + NTP time synchronization** (pool.ntp.org)
* 🕒 **Manual time fallback** if WiFi fails
* 🖥️ **Centered ASCII clock** (assumes 80‑column terminal)
* 💡 **Onboard LED status indicators**

  * Slow blink → Connecting to WiFi
  * Fast blink → WiFi connected
* 🔌 **Two build modes**:

  * USB CDC serial (default)
  * Hardware UART (GPIO 0 / GPIO 1)
* 🧱 Specifically designed for the **Pico 2 W**

---

## 📁 Project Structure

```
ascii_clock/
├── ascii_clock.cpp        # Main source code
├── CMakeLists.txt         # Pico SDK CMake configuration
├── build.sh               # Interactive build script
├── lwipopts.h             # Required lwIP configuration
├── pico_sdk_import.cmake  # Pico SDK import helper
└── build/                 # Build output (generated)
```

---

## 🔧 Requirements

Before building, ensure you have:

* **Raspberry Pi Pico SDK** installed
* A **Pico 2 W** board
* CMake, Make, and a working ARM toolchain

### Installing Pico SDK

Clone the SDK and initialize submodules:

```bash
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
```

Export the SDK path:

```bash
export PICO_SDK_PATH=~/pico-sdk
```

> ⚠️ The build will fail if `PICO_SDK_PATH` is not set.

---

## 🛠️ Building

An interactive build script is provided for convenience.

### Steps:

1. Open `ascii_clock.cpp` and add your WiFi credentials:

```cpp
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

2. Make the build script executable:

```bash
chmod +x build.sh
```

3. Run the build script:

```bash
./build.sh
```

4. Select the build version:

```
1) USB Serial version (default / recommended)
2) UART Serial version (GPIO 0/1)
```

The script will:

* Remove previous build output (remove the `#` before `rm -rf build` if needed)
* Configure CMake
* Compile the firmware
* Generate `ascii_clock.uf2`

---

## 🚀 Flashing to the Pico

1. Hold **BOOTSEL** on the Pico while plugging it into USB
2. Release **BOOTSEL**
3. Copy the UF2 file to the Pico:

```bash
cp build/ascii_clock.uf2 /media/$USER/RPI-RP2/
```

The Pico will reboot automatically.

---

## 🖥️ Viewing Output

### USB Serial (default build)

* **Linux**:

```bash
screen /dev/ttyACM0 115200
```

* **macOS**:

```bash
screen /dev/tty.usbmodem* 115200
```

* **Windows**:
  Use PuTTY, select the correct COM port, baud rate **115200**.

---

### UART Version

If using UART mode, wire as follows:

| Pico GPIO | Connect To           |
| --------- | -------------------- |
| GPIO 0    | RX (USB‑TTL adapter) |
| GPIO 1    | TX (USB‑TTL adapter) |
| GND       | GND                  |

* Baud rate: **115200**

---

## 💡 LED Status Indicators

The onboard LED provides status feedback:

* ⏳ **Slow blink** → Connecting to WiFi
* ✅ **Fast blink** → WiFi connected successfully
* ❌ **Long blinks** → WiFi or NTP failure

LED feedback is active even without a serial terminal connected.

---

## ⏰ Manual Time Fallback

If WiFi or NTP is unavailable, the clock will use a hardcoded fallback time:

```cpp
init_time(2026, 2, 1, 6, 10, 48, 0);
```

Format:

```
init_time(Year, Month, Day, DayOfWeek, Hour, Minute, Second)
```

DayOfWeek mapping:

```
0 = Sunday
1 = Monday
...
6 = Saturday
```

---

## 🖼️ Example Output

```
            Pico 2 W ASCII Clock

      ###   ###      ###   ###      ###   ###
     #   # #   #    #   # #   #    #   # #   #
     #   #   ##        ##   ##        ##   ##
     #   # #   #    #   # #   #    #   # #   #
      ###   ###      ###   ###      ###   ###

            2026-02-01  (Day 6)
            Time source: NTP
```

---

## 🧠 Design Notes

* Uses lwIP directly (without SNTP helper)
* Avoids heap-heavy abstractions for reliability
* `goto` is used intentionally for clean error handling
* Assumes an 80‑column terminal (standard for serial output)

> That’s not a bug — it’s how embedded development works.

---

## 📜 License

This project is open for personal or educational use. You may:

* Use it
* Fork it
* Modify it
* Report issues or request features

Have fun and explore embedded development! 👾

---
