# 🟢 Raspberry Pi Pico 2 W – Minimal Web Server

A **basic HTTP web server** written in **C/C++ using the Pico SDK and lwIP**, designed **specifically for the Raspberry Pi Pico 2 W**.

This project serves a single HTML page over WiFi and is meant to be **simple, lightweight, and hackable** — not a full-featured web framework.

---

## ⚠️ Hardware Requirement (Important)

**This project is for the Raspberry Pi Pico 2 W ONLY.**

It will **not work** on:

* Raspberry Pi Pico (no WiFi)
* Raspberry Pi Pico W (older WiFi stack & different constraints)

The Pico **2 W** uses updated WiFi hardware and SDK support that this project relies on.

---

## ✨ Features

* Minimal HTTP server running on **port 80**
* Serves a static HTML page
* Built directly on **lwIP TCP callbacks**
* No filesystem, no SPIFFS, no SD card
* Extremely low overhead
* Designed to run continuously

---

## 📄 Customising the Website

You are **expected** to edit the HTML.
You are **not** expected to rewrite the networking code.

### ✔ Safe to edit

```c
const char *html_content = "...";
```

You can:

* Change text
* Change styles
* Add/remove sections
* Replace links
* Make it your own portfolio or info page

### ❌ Avoid editing unless you know what you’re doing

* TCP callbacks
* lwIP logic
* WiFi setup
* Memory handling

---

## ❌ Why There Is No Precompiled UF2

A precompiled `.uf2` is **not provided by design**.

WiFi credentials and SDK configuration are required **at build time**.
A prebuilt firmware would not connect to your network and would likely fail to run.

This project must be **built locally**.

---

## 📂 Repository Contents

```
.
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── lwipopts.h
├── pico_webserver.cpp
├── check.sh
├── build.sh
└── README.md
```

---

## 🛠 Build Instructions (IMPORTANT)

### ⚠️ Read This First

**You MUST run `check.sh` before building.**

Even as the project author, this script was required to ensure everything was set up correctly.
If required dependencies are missing, the project **will not build**.

---

### 🔍 Step 1: Run `check.sh`

This script verifies:

* Pico SDK is installed
* Required SDK submodules are present
* Toolchain availability
* Environment variables are set correctly

Run:

```bash
chmod +x check.sh
./check.sh
```

* If the check **passes**, you can continue
* If anything is missing, the script will **tell you exactly what to fix**

Do **not** skip this step.

---

### 🌐 Step 2: Configure WiFi

Edit the source file and set your WiFi credentials **before building**:

```c
const char WIFI_SSID[] = "YOUR_SSID";
const char WIFI_PASSWORD[] = "YOUR_PASSWORD";
```

---

### ▶️ Step 3: Build with `build.sh`

Once `check.sh` reports everything is OK:

```bash
chmod +x build.sh
./build.sh
```

This will:

* Create a `build/` directory
* Configure the project with CMake
* Compile the firmware
* Generate a `.uf2` file in `build/`

---

## 📦 Flashing the Pico

1. Hold the **BOOTSEL** button on the Pico 2 W
2. Plug it into your computer via USB
3. Release BOOTSEL when it appears as a USB drive
4. Copy the generated `.uf2` file from the `build/` directory onto the Pico
5. The Pico will reboot automatically

---

## 🌐 Accessing the Server

Then Open a serial terminal to view logs.

Once connected, the Pico will print its IP address:

```
http://<pico-ip-address>/
```

---

## 🧠 Design Philosophy

This server is intentionally:

* Minimal
* Static
* Limited

The Pico 2 W has:

* Limited RAM
* Only **2 CPU cores**
* No OS
* No filesystem

More complexity = less reliability.

---

## 📝 Notes & Limitations

* Default port: **80**
* WPA2 WiFi only
* One client at a time (by design but for me it did manage to run on my 3 phones at the same time)
* No HTTPS
* Not intended for production use

---

## 🛠 Maintenance

This project is provided **as-is**.

* No active maintenance
* No guaranteed updates
* Fixes can be attempted on request

---

## 🔐 Security Notes

* Bare-metal microcontroller
* No OS or shell
* Static responses only
* No user input handling

A crash just resets the device.

---

## 📜 License

Use it, modify it, break it, learn from it.

---

### ✅ TL;DR

```text
1. Run check.sh
2. Fix anything it complains about
3. Set WiFi credentials
4. Run build.sh
5. Flash UF2
6. Done
```
