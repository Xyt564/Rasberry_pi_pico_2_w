# Raspberry Pi Pico 2 W Operating System

**Version 2.0**

A **shell-based operating system** built from scratch for the **Raspberry Pi Pico 2 W**, written in **C/C++**, running **bare-metal** (no Linux, no RTOS), with networking, filesystem support, a multitasking shell, built-in applications, and a local web server.

This project exists to push the Pico 2 W **far beyond what it’s “supposed” to do**.

---

## Overview

Pico OS is a minimal but powerful operating-system–style environment designed specifically for the **RP2350 (Pico 2 W)**.
It runs entirely on the microcontroller, communicates over USB serial, and exposes a command-driven shell with real utilities, games, and networking features.

Despite extreme hardware limitations, Pico OS includes:

* Dual-core execution
* Flash-backed filesystem
* WiFi + NTP time sync
* Local HTTP web server
* Multiple built-in applications
* Custom ASCII system information display

No Linux. No shortcuts. Just bare metal.

---

## Key Features

### Core System

* **Shell-based OS environment** over USB serial (TTY)
* **Dual-core support**

  * Core 0: Shell, command handling, user apps
  * Core 1: Background services & system tasks
* **Multitasking design** within strict RAM limits
* **Watchdog integration** for stability

### Filesystem

* **LittleFS** mounted directly on flash
* Persistent storage across reboots
* File operations:

  * `ls`
  * `cat`
  * `delete`
  * Lightweight `nano`-style editor

### Networking

* **WiFi support** using CYW43 + lwIP
* **NTP time synchronization**
* Network-aware applications (scanner, server)

### HTTP Server

* Built-in **local web server**
* Serves **HTML/CSS** directly from LittleFS
* Can be exposed to the internet using **Cloudflare Tunnel**
* Runs entirely on the Pico 2 W

### Applications

* `neofetch`-style ASCII system info (Raspberry Pi logo)
* `nmap`-like TCP port scanner
* ASCII text converter
* Games:

  * **Tetris**
  * **Snake**

---

## Example Boot Output

```
║     Raspberry Pi Pico 2 W Operating System   ║
║                  Version 2.0                  ║
╚═══════════════════════════════════════════════╝

Booting...

[OK] Initializing hardware
[..] Mounting filesystem
[OK] Filesystem ready
[..] Starting WiFi driver
[OK] WiFi driver ready
[OK] Initializing system clock
[OK] WiFi credentials loaded

Boot complete!
Type 'help' for available commands
Type 'neofetch' for a cool system overview

Starting background tasks...
```

---

## Hardware Constraints (Why This Is Hard)

The Raspberry Pi Pico 2 W has:

* **520 KB SRAM total**
* **4 MB flash**
* No MMU
* No virtual memory
* No OS safety net

This means:

* Stack and heap sizes must be **manually tuned**
* Multicore requires **explicit stack allocation**
* Filesystem, WiFi, and apps all fight for the same memory
* One bad allocation = panic or hard fault

---

## Why This Exists

### Reason Why

Today, my braces were removed.
My teeth felt weird. Distractingly weird.

So instead of painkillers, I decided to focus.

Nine hours later, this exists:

* A shell-based OS
* Multicore scheduling
* Networking
* Games
* A working HTTP server
* A custom ASCII neofetch just for fun

---

## Quote Section (For fun)

> “That won’t run on a microcontroller.”
> — Someone who didn’t try

> “It’s not about needing to do it. It’s about proving that you can.”
> — Project philosophy

> “If the hardware says no, optimise harder.”
> — Embedded development rule #1

---

## Building the OS

### One-Command Build (Recommended)

The `build.sh` script **does everything for you**:

* Finds or clones the Pico SDK
* Clones LittleFS
* Configures CMake
* Builds the UF2
* Generates a flash helper script

```bash
chmod +x build.sh
./build.sh
```

Output:

```
build/pico_os.uf2
```

---

## Flashing to the Pico 2 W

### Manual Flash

1. Hold **BOOTSEL**
2. Plug in Pico 2 w
3. Copy `pico_os.uf2` to the RP2350 drive

### Quick Flash

1. Hold **BOOTSEL**
2. Plug in Pico 2 w
3. then run: 
```bash
./flash.sh
```

---

## Connecting to the Shell

```bash
screen /dev/ttyACM0 115200
```

or

```bash
minicom -D /dev/ttyACM0 -b 115200
```

---

## Exposing the Web Server to the Internet

Pico OS includes a local HTTP server that can be made publicly accessible using **Cloudflare Tunnel**.

### Option 1: Quick Tunnel (No Login Required)

This is the **recommended method**.

#### Install Cloudflared

```bash
sudo apt install cloudflared
```

#### Start Tunnel

```bash
cloudflared tunnel --url http://PICO_LOCAL_IP:80
```

Cloudflare will generate a temporary public URL:

```
https://random-name.trycloudflare.com
```

No account.
No DNS.
No configuration.

Perfect for testing and demos.

---

### Option 2: Named Tunnel (Advanced)

Use this if you want:

* A permanent URL
* A custom domain

This requires a Cloudflare account and domain.

(Quick Tunnel is intentionally the default.)

---

## Security Notes

* Pico OS is **not hardened**
* No TLS termination on-device
* No authentication layer
* Exposing it publicly is for **experimentation and demos only**

If you expose it:

* Use Cloudflare protections
* Treat it as read-only content
* Do not host sensitive data

---

## Why This Works on a Microcontroller

Because:

* Memory is aggressively controlled
* Features are purpose-built
* No unnecessary abstractions
* Every byte is intentional

This is not Linux scaled down.
It’s an OS designed **for the hardware**, not despite it.

---

## Final Notes

This project exists because:

* Someone said it couldn’t be done
* Embedded systems reward stubbornness
* Challenges are more fun than comfort

If something doesn’t work, it’s not a failure—it’s an invitation.

---

**Pico OS v2.0**
Built because “impossible” isn’t a technical term.

---
