# Pico 2 W Unified System

> A tiny OS-style environment for the **Raspberry Pi Pico 2 W**
> Web terminal. Multiple apps. NTP time. No Linux. No mercy.

This project is a **minimal operating-system–like framework** built entirely for the **Raspberry Pi Pico 2 W** using the Pico SDK.

It provides a web-based terminal, a command system, and multiple built-in applications — all running on a **microcontroller with no MMU, limited RAM, and strict hardware constraints**.

If you’re expecting Linux, you’re in the wrong repo.
If you’re into pushing embedded hardware way past “blink an LED”, welcome.

---

## 🔥 What This Is

In embedded terms, this project functions as:

* A **system core** (boot, init, uptime, shared services)
* A **command processor / shell**
* A **single-app-at-a-time execution model**
* A **web-based terminal UI**
* A set of **modular applications** sharing system resources

It boots, connects to Wi-Fi, starts a TCP web server, and lets you control the system live from your browser.

That counts.

---

## 🚫 Why This Can’t Be a “Real” OS (Hardware Reality)

This project intentionally **does not** try to be a traditional operating system — because the hardware physically won’t allow it.

The Raspberry Pi Pico 2 W has:

* ❌ **No MMU**
  → No memory protection, no virtual memory, no process isolation
* ❌ **Limited RAM**
  → Everything must fit at once, including networking stacks
* ❌ **No preemptive multitasking**
  → All execution is cooperative
* ❌ **No storage-backed paging or swap**
* ❌ **Bare-metal execution**
  → No kernel/user separation

Because of this:

* There are **no processes**, only cooperative applications
* Apps **share memory intentionally**
* Only **one app runs at a time** to keep RAM predictable
* Safety comes from **design discipline**, not hardware protection

Trying to fake a “real OS” here would just produce something slower, larger, and less stable.

This project stays honest.

---

## 🧠 Architecture Overview

### Core Design Choices

* **Single main loop**
  All background work is handled via lightweight `*_tick()` functions.
* **Shared output ring buffer**
  System + app output is streamed to both serial and web interfaces.
* **Command-based app control**
  Commands are parsed once and routed based on the active app.
* **No dynamic scheduling**
  Keeps timing predictable and avoids hard-to-debug race conditions.

This makes the system **simple, deterministic, and debuggable** — which matters on a microcontroller.

---

## 🧩 Built-In Applications

### 📝 TO-DO App

* Max 2 tasks (intentional RAM limit)
* Add, list, complete, and delete tasks
* Fully command-driven

### 💡 Blink App

* Controls the Pico 2 W onboard LED
* Adjustable blink speed (50–5000 ms)
* Runs non-blocking in the background

### 🕒 Clock App

* NTP-synced real time using `pool.ntp.org`
* UDP + DNS via lwIP
* Maintains time using the Pico’s monotonic clock
* Displays time, date, and weekday

---

## 🌐 Web Terminal

* Runs on port **80**
* Served directly from flash (no filesystem)
* Ultra-light HTML/CSS/JS
* Live command execution
* Periodic output polling

Access it at:

```
http://<pico-ip>/
```

---

## 📡 Networking

* Wi-Fi STA mode (Pico 2 W only)
* TCP server via lwIP
* UDP for NTP
* DNS resolution for time servers

Networking runs in **threadsafe background mode**, keeping the main loop responsive.

---

## ✅ Stability & Stress Testing

This system is intentionally designed to be **boring in the best way**.

### Stability Characteristics

* No blocking calls in the main loop
* No uncontrolled memory growth
* Minimal dynamic allocation (mostly at startup)
* Predictable execution flow

### Stress Testing Performed

* Continuous web terminal polling
* Rapid command execution via browser
* Repeated app switching (run/stop loops)
* Extended uptime testing
* Concurrent background tasks (blink + NTP + web traffic)

The system remains responsive and stable under sustained use, with no crashes or lockups observed during testing.

If it crashes, it reboots cleanly.
If it runs, it runs indefinitely.

---

## ⚙️ Build System

### Requirements

* Raspberry Pi Pico SDK
* CMake ≥ 3.13
* Raspberry Pi Pico 2 W

### Build

```bash
chmod +x build.sh
./build.sh
```

Produces:

* `.uf2`
* `.elf`
* `.bin`

---

## 🧪 Hardware Support

**Supported**

* ✅ Raspberry Pi Pico 2 W

**Not Supported**

* ❌ Pico
* ❌ Pico W
* ❌ Any other RP2040 / RP2350 boards

This project relies on Pico 2 W–specific Wi-Fi and GPIO mappings.

---

## 📈 Optimizations

* `-Os` size optimization
* Link-time dead code elimination
* Manual stack & heap sizing
* Cooperative execution model
* No filesystem dependency

> If it gets bigger, it stops fitting.
> If it stops fitting, it stops existing.

---

## 📌 Status

* Version: **2.0**
* Fully functional
* Stable under stress
* Built for learning, experimentation, and embedded OS design

---

## 🧨 Final Notes

This project exists because:

* Embedded systems are fun
* Constraints force better design
* The Pico 2 W deserved more than “hello world”

If someone says “this isn’t a real OS” — they’re right.
If someone says “this is impressive for a microcontroller” — they’re also right.

Both can be true.

---

## Quote for the goats Energy

> “Four hours, a dream and a Pico 2 W then came a full OS running. Stable, stress-tested, untouchable. Limits exist? Cute. I went anyway.”
> “They said it was impossible. I said, hold my Pico — OS incoming.”

---
