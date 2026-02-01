# Pico 2 W ASCII Clock ⏰

A **terminal-based ASCII clock** for the **Raspberry Pi Pico 2 W**, written in C++.

It syncs time over **WiFi using NTP**, displays it as large ASCII digits over **USB serial or UART**, and **falls back to manual time** if WiFi/NTP isn’t available.

This project is intentionally simple, reliable, and very embedded-friendly — no dynamic memory abuse, no fancy UI libraries, just solid firmware.

---

## ✨ Features

* 📡 **WiFi + NTP time sync** (pool.ntp.org)
* 🕒 **Manual time fallback** if WiFi fails
* 🖥️ **Centered ASCII clock** (assumes 80‑column terminal)
* 💡 **Onboard LED feedback**

  * Slow blink → connecting to WiFi
  * Fast blink → WiFi connected
* 🔌 **Two build modes**:

  * USB CDC serial (default)
  * Hardware UART (GPIO 0 / GPIO 1)
* 🧱 Designed specifically for **Pico 2 W**

---

## 📁 Project Structure

```
ascii_clock/
├── ascii_clock.cpp        # Main source code
├── CMakeLists.txt         # Pico SDK CMake config
├── build.sh               # Interactive build script
├── lwipopts.h             # Required lwIP configuration
├── pico_sdk_import.cmake  # Pico SDK import helper
└── build/                 # Build output (generated)
```

---

## 🔧 Requirements

Before building, you need:

* **Raspberry Pi Pico SDK** installed
* A **Pico 2 W** board
* CMake, Make, and a working ARM toolchain

### Pico SDK

Clone the SDK somewhere sensible:

```bash
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
```

Then export the SDK path:

```bash
export PICO_SDK_PATH=~/pico-sdk
```

> ⚠️ The build will fail if `PICO_SDK_PATH` is not set.

---

## 🛠️ Building

An interactive build script is provided.

### Run the build (Make sure you add your WiFi credentials in the source before building.)

```bash
chmod +x build.sh
./build.sh
```

You’ll be asked which version to build:

```
1) USB Serial version (default / recommended)
2) UART Serial version (GPIO 0/1)
```

The script will:

* Clean old builds (Remove the hastag before the rm -rf build if u have previous build attempts)
* Configure CMake
* Compile the firmware
* Generate `ascii_clock.uf2`

---

## 🚀 Flashing to the Pico

1. Hold **BOOTSEL** on the Pico
2. Plug it into USB
3. Release BOOTSEL
4. Copy the UF2 file:

```bash
cp build/ascii_clock.uf2 /media/$USER/RPI-RP2/
```

The Pico will reboot automatically.

---

## 🖥️ Viewing Output

### USB Serial (default / recommended build)

* **Linux**:

  ```bash
  screen /dev/ttyACM0 115200
  ```
* **macOS**:

  ```bash
  screen /dev/tty.usbmodem* 115200
  ```
* **Windows**:

  * Use PuTTY
  * Select the COM port
  * Baud rate: **115200**

---

### UART Version

If you build the UART version, wire as follows:

| Pico GPIO | Connect To           |
| --------- | -------------------- |
| GPIO 0    | RX (USB‑TTL adapter) |
| GPIO 1    | TX (USB‑TTL adapter) |
| GND       | GND                  |

* Baud rate: **115200**

---

## 📡 WiFi Configuration

Edit these lines in `ascii_clock.cpp`:

```cpp
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

build only after changing credentials.

---

## 💡 LED Status Indicators

The onboard LED provides instant feedback:

* ⏳ **Slow blink** → Connecting to WiFi
* ✅ **Fast blink** → WiFi connected successfully
* ❌ **Long blinks** → WiFi or NTP failure

This is useful even if no serial terminal is open.

---

## ⏰ Manual Time Fallback

If WiFi or NTP fails, the clock uses a hardcoded fallback time.

You can edit it here:

```cpp
init_time(2026, 2, 1, 6, 10, 48, 0);
```

Format:

```
init_time(Year, Month, Day, DayOfWeek, Hour, Minute, Second)
```

DayOfWeek:

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

* Uses lwIP directly (no SNTP helper)
* No heap-heavy abstractions
* `goto` is used intentionally for clean error fallback
* Assumes an 80‑column terminal (standard for serial)

> That’s not a bug — that’s embedded development.

---

## 📜 License

Use it, fork it, break it, improve it.
If you find bugs or want features, open an issue.

Have fun 👾
