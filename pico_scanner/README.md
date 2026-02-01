# 🧪 Raspberry Pi Pico 2 W – TCP Port Scanner

A **minimal TCP port scanner written in C/C++ for the Raspberry Pi Pico 2 W**.

This project demonstrates basic TCP networking and port scanning on **extremely constrained embedded hardware**.
It is intentionally lightweight, serial-based, and designed to work within the Pico 2 W’s limits.

---

## 🚀 Features

* TCP connect-based port scanning
* Custom port range scanning (`start-end`)
* Serial command interface
* Live progress feedback
* Designed specifically for Pico 2 W constraints

---

## 🖥 Example Output

```
> SCAN 127.0.0.1 1-1024
Scanning 127.0.0.1 ports 1-1024...
Progress: 97.7% (port 1001)

=== Scan Complete ===
Scanned 1024 ports in 3 ms
Found 0 open port(s)
```

---

## ⚙️ How It Works

The scanner attempts a **TCP connection** to each port in the specified range.

* If the connection succeeds → port is marked **open**
* If it fails or times out → port is **closed**

All interaction is done via **USB serial**, keeping the implementation simple and reliable.

This is **not a replacement for tools like Nmap** — it’s an embedded proof-of-concept and learning project.

---

## 🧠 Hardware Limitations (Important)

The Pico 2 W is **very limited hardware**, and this project is designed around those limits.

### Constraints

* **2 CPU cores**
* **Very limited RAM**
* **No operating system**
* **WiFi stack uses a large portion of memory**
* **No true parallel socket handling**

### Implications

* Scanning is **sequential**, not parallel
* Large port ranges can take a **long time**
* Long TCP timeouts significantly increase scan duration
* Aggressive scanning may cause slowdowns or instability

### Recommendations

* Keep scan ranges **small** (e.g. `1–1024`)
* Expect **longer wait times** for large ranges
* Best suited for **testing, demos, and learning**, not real-world audits

The simplicity is intentional — adding more “advanced” features would make the scanner unreliable on this hardware.

---

## 📂 Repository Contents

This repository is **source-only by design**.

```
.
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── lwipopts.h
├── scanner.cpp
├── build.sh
└── README.md
```

### Why No Precompiled UF2?

WiFi credentials and network configuration must be set **at build time**.

A precompiled `.uf2` would **not connect to your WiFi**, making it unusable for others.
For this reason, users must build the firmware locally.

---

## 🛠 Building the Project

### Requirements

* Raspberry Pi Pico SDK
* CMake
* Make or Ninja
* ARM GCC toolchain
* Linux / macOS (Windows via WSL works)

---

### 🔧 Clone the Repository

```bash
git clone git@github.com:Xyt564/Raspberry_pi_pico_2_w.git
cd Raspberry_pi_pico_2_w
```

---

### 🔧 Using `build.sh`

A build script is included to simplify compilation.

1. Make the script executable:

```bash
chmod +x build.sh
```

2. Run the build:

```bash
./build.sh
```

The script will:

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

Connect via a **serial terminal** to use the scanner.

---

## ⚠️ WiFi Configuration

WiFi credentials **must be configured before building**.

Edit the relevant source or configuration values to match your network **before running `build.sh`**.
Without valid WiFi settings, the scanner will not function.

---

## 📂 Source Code

The **full source code is included** in this repository.

Users are encouraged to:

* Read the code
* Modify it
* Experiment with different scan ranges and timeouts

This project is intended to be educational.

---

## 🛠 Maintenance & Support

This project is provided **as-is**.

* ❌ No active maintenance
* ❌ No guaranteed updates
* ❌ No long-term support

Issues and requests are welcome, but fixes are **not guaranteed**.

---

## ⚠️ Disclaimer

This project is for **educational purposes only**.
Only scan systems you own or have explicit permission to test.

---

## 📌 Version

**v1.0 – Initial Release**

---
