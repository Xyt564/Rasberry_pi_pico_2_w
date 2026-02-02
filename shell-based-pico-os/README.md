# Pico Shell OS (TTY)

> A shell-based, OS-style environment for the **Raspberry Pi Pico 2 W**,
> running entirely over **USB serial (TTY)**.
> No Linux. No framebuffer. No excuses.

This project turns the Pico 2 W into a **tiny operating-system–like shell** with a filesystem, networking, background tasks, built-in apps, and system info — all running **bare metal** on hardware

---

## 🚀 What This Is

This is **not** a demo or proof-of-concept.

It is:

* A real interactive shell over USB CDC
* Persistent filesystem (LittleFS)
* WiFi support + NTP time sync
* Background system tasks
* System logging
* Built-in applications
* ANSI-colored terminal UI
* An ASCII `neofetch` clone (because why not)

All on a board with **520 KB of RAM**.

---

## ✨ Features

### 🖥️ Shell

* Interactive command-line shell over USB serial
* ANSI colors, timestamps, dynamic prompt
* Command parsing and arguments
* Core commands: `help`, `clear`, `reboot`, `ps`, `stop`, `neofetch`, and more

### 📁 Filesystem (LittleFS)

* Persistent flash-based storage
* Auto-format on first boot
* File listing, viewing, creation, deletion
* Nano-style text editor over serial
* Storage usage reporting

### 🌐 Networking

* WiFi via CYW43
* Saved WiFi credentials (persisted)
* IP / gateway / netmask display
* NTP time synchronization
* Background hourly NTP sync task

### ⏰ Time & System Info

* Real-time clock via NTP
* Timezone configuration
* Uptime tracking
* RAM usage reporting
* Circular system log with timestamps

### 🧠 Built-in Apps

* **Timer**

  * Stopwatch
  * Countdown timer
* **Todo List**

  * Simple persistent task manager

Apps behave like tiny userspace programs — not demos.

---

## 🧱 Hardware Target

* **Board:** Raspberry Pi Pico 2 W
* **MCU:** RP2350 (Dual-core Cortex-M33)
* **RAM:** 520 KB SRAM
* **Flash:** 4 MB
* **Interface:** USB CDC (TTY)

This project is **Pico 2 W only**.

---

## ⚠️ Hardware Limitations (and Why This Is Hard)

Let’s be blunt:

* **520 KB total RAM**
* No MMU
* No virtual memory
* No process isolation
* Flash write limits
* USB, WiFi, filesystem, and shell all share the same memory
* Everything runs bare metal

Most people stop here and say:

> “That’s impossible.”

This project exists because I didn’t.

Every feature had to be carefully designed:

* Minimal buffers
* Tight data structures
* No wasted abstractions
* No unnecessary memory usage

If this system were “fully featured,” it would not run.
That’s not a flaw — that’s embedded reality.

---

## 🔧 Building (The Easy Part)

### Important:

**`build.sh`**** essentially does everything for you.**

You don’t need to manually:

* Install the Pico SDK
* Fetch LittleFS
* Configure toolchains
* Fight CMake paths

Just run:

```bash
chmod +x build.sh
./build.sh
```

The script will:

* Locate or clone the Pico SDK
* Clone LittleFS automatically
* Configure CMake correctly
* Build the project
* Generate the UF2
* Create a `flash.sh` helper script

If it builds, you’re done.

---

## 🔥 Flashing

### Automatic (recommended)

1. Hold **BOOTSEL**
2. Plug in the pico 2 w
3. then run

```bash
./flash.sh
```

### Manual

1. Hold **BOOTSEL**
2. Plug in the pico 2 w
3. Copy `build/pico_shell_os.uf2` to the RP2350 drive

---

## 🔌 Connecting to the Shell

```bash
screen /dev/ttyACM0 115200
```

or

```bash
minicom -D /dev/ttyACM0 -b 115200
```

If you see the boot banner

## 🖼️ ASCII Neofetch (Example Output)

This is what the built-in `neofetch` command looks like in the shell.
*(In the actual terminal, the Raspberry Pi logo is rendered in red using ANSI colors.)*

```
      .~~.   .~~.        pico@pico-os
     '. \ ' ' / .'       -------------
      .~ .~~~..~.        OS: Pico OS v1.0
     : .~.'~'.~. :       Host: Raspberry Pi Pico 2 W
    ~ (   ) (   ) ~      CPU: Dual-core ARM Cortex-M33
   ( : '~'.~.'~' : )     Memory: 520 KB SRAM
    ~ .~ (   ) ~. ~      Flash: 4 MB
     (  : '~' :  )       Uptime: 0h 0m
      '~ .~~~. ~'        WiFi: Disconnected
          '~'            Time: Not synced

                         ██████

```

After displaying system info, the shell waits for user input:

```
Press Enter to continue...

```

Is it useful? Debatable.
Is it funny? Absolutely.
Is it there purely so you can flex a Pico over serial? Yes.

---

## 🤔 Why This Exists

Honestly?

Today I got my **braces taken off**.

My teeth felt *incredibly* weird — uncomfortable enough that I needed a distraction — so I sat down and started coding.

Six hours later:

* A full **shell-based OS-style system**
* Running bare metal
* On a **Pico 2 W**
* With networking, storage, apps, and system tools

Somewhere along the way I also made an ASCII `neofetch` clone purely *for the funnies*, because if you’re going to do something people say is impossible, you might as well make it look cool when you show it off.

This project exists because:

* I wanted a distraction
* I wanted to push the Pico 2 W to its limits
* And people kept saying: *“That can’t be done on a microcontroller”*

Turns out it can — if you’re stubborn enough.

Looking back, it’s a bit absurd.

Most people would’ve taken painkillers and called it a day.
Instead, I sat there with post-braces discomfort and somehow ended up building something that many experienced developers will confidently tell you is *“not possible”* on this hardware.

Not because it was easy.
Not because the Pico suddenly got more RAM.
But because I kept going anyway.

That’s kind of the whole theme of this project.

---

## 🧠 Why It Matters

People say:

> “You can’t build an OS on a Pico.”

Sometimes the correct response is:

> “I already did.”

---

## 📜 License

MIT License
Use it, break it, learn from it.

---

## 🫡 Final Note

This is a **shell-first microcontroller OS**, built under extreme constraints — and it runs anyway.

If you’re here to learn, welcome.
If you’re here to doubt, try fitting this into **520 KB** first.
