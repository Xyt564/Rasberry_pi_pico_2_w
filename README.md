# 🟢 Raspberry Pi Pico 2 W Projects

Welcome to my collection of **Raspberry Pi Pico 2 W projects**.

This repository is where I store and share the C++ projects I build for my Pico 2 W.
It’s a mix of **personal storage**, **experiments**, and **shareable examples** for anyone interested in embedded development on constrained hardware.

---

## 📦 Source Code Always Included

All projects in this repository include their **full C/C++ source code**.

Every project folder contains:

* Complete C/C++ source files
* Build files (`CMakeLists.txt`, scripts, config headers, etc.)
* Comments explaining how things work
* Project-specific instructions in that folder’s `README.md`

This makes it easy to:

* Clone the repository
* Read and learn from the code
* Build projects yourself
* Modify behaviour (HTML, logic, scan ranges, etc.)
* Understand how Pico 2 W projects work end-to-end

---

## ⚠️ Important: Precompiled `.uf2` Files Are Project-Dependent

**Not all projects include a precompiled `.uf2` file.**

This is **intentional**.

### Why Some Projects Have No UF2

Projects that use **WiFi or networking** (for example: web servers or scanners) require:

* WiFi SSID & password
* Network-specific configuration
* SDK features that must be set at build time

A precompiled `.uf2` would:

* **Not connect to your WiFi**
* Often fail silently
* Cause confusion

For these projects, you **must build the firmware yourself** after editing the source.

Each project’s README clearly states whether:

* A `.uf2` is provided
* Or the project is **source-only by design**

---

## ⚡ Flashing a `.uf2` (When Provided)

If a project **does include a UF2**, flashing is easy:

1. Hold the **BOOTSEL** button on your Pico 2 W
2. Plug it into your computer via USB
3. Release BOOTSEL when it appears as a USB drive
4. Copy the `.uf2` file onto the Pico
5. The Pico will reboot and run the project automatically

No extra tools required.

---

## 🛠 Building Projects From Source (Common for WiFi Projects)

Many projects include:

* `check.sh` – verifies your environment and dependencies
* `build.sh` – builds the firmware and generates a `.uf2`

Typical workflow:

```text
1. Run check.sh
2. Fix anything it reports missing
3. Edit source (WiFi credentials, config, etc.)
4. Run build.sh
5. Flash the generated UF2
```

**Always read the project’s README first.**
It will tell you exactly what is required.

---

## 🔍 What You’ll Find Here

* Embedded C++ projects for **Raspberry Pi Pico 2 W**
* Networking demos (web servers, scanners)
* Simple utilities and experiments
* Learning-focused code
* Realistic designs that respect hardware limits

---

## ⚙️ Hardware Limitations & Realistic Expectations

The Raspberry Pi Pico 2 W is a **bare-metal microcontroller**, not a full computer.

### Core Constraints

* **CPU & Memory**

  * Dual-core ARM Cortex-M0+ @ 133 MHz
  * **264 KB RAM**, **2 MB flash**
  * No virtual memory or protection

* **No Operating System**

  * No Linux / Windows
  * No processes or users
  * No shell or background services

* **Networking Limits**

  * Basic TCP/UDP only
  * No HTTPS natively
  * One active connection recommended for stability

* **Storage**

  * No filesystem by default
  * All assets must fit in flash memory

### What This Means

* Projects are **intentionally simple**
* Static HTML over dynamic web apps
* Minimal logic over heavy frameworks
* Reliability over features

Trying to turn the Pico 2 W into a “real server” will usually end in crashes or instability.

---

## 🧠 Project Philosophy

These projects are built with one goal:

> **Do things that make sense on a microcontroller.**

That means:

* Small, focused programs
* Clear tradeoffs
* No unnecessary abstraction
* Code you can actually understand

---

## 🔮 Future Plans

* More Pico 2 W C++ projects
* MicroPython experiments later
* Practical tools with realistic limits
* Learning-first approach over polish

---

## 📜 License

* Free to use, modify, and learn from
* If you want something custom, open an issue or request
* No guarantees, but I’ll try to help when I can

---

### ✅ Final Note

If a project:

* Uses WiFi
* Touches networking
* Needs credentials

**Expect to build it yourself.**

That’s not a bug — it’s how embedded development works.

---
