## Raspberry Pi Pico 2 W – Minimal Task Manager

A **very simple serial-based task manager** written in C for the **Raspberry Pi Pico 2 W only**.

This project runs over USB serial and lets you manage **up to 2 tasks** directly from a terminal. It’s intentionally minimal due to **hardware and memory limitations** and is mainly meant as a small embedded experiment / demo.

⚠️ **This is for the Raspberry Pi Pico 2 W ONLY**
It is **not tested** and **not guaranteed to work** on:

* Pico (original)
* Pico W
* Any other RP2040 / RP2350 board

---

## Features

* Add up to **2 tasks**
* List tasks
* Mark tasks as **done**
* Delete tasks
* Runs entirely over **USB serial**
* No dynamic memory
* No filesystem
* No Wi-Fi (even though it’s a Pico 2 W)

---

## Why This Is So Limited (Hardware Reality)

This project runs on a **bare-metal microcontroller**, not a computer.

The Pico 2 W has:

* No operating system
* Very limited RAM and flash compared to a PC
* No filesystem
* No real user input devices
* No display
* No background processes

Because of this:

* Tasks are stored in **fixed-size arrays**
* There is **no dynamic allocation**
* Nothing can be saved permanently
* Input is blocking and linear
* The program does exactly one thing at a time

These limitations are **intentional** and reflect how embedded systems actually work.
This isn’t bad design — it’s realistic design for the hardware.

---

## Limitations (Important)

Because this runs on a microcontroller with very limited resources:

* **Maximum of 2 tasks**
* Task names are limited to **14 characters**
* Tasks are **not saved** (everything is lost on reboot)
* No scrolling, no fancy UI, just serial text
* Blocking input (intentional and simple)

This is expected behavior — **not a bug**.

---

## How to Use

### 1. Flash the UF2

1. Hold the **BOOTSEL** button on the Pico 2 W
2. Plug it into your computer
3. Release BOOTSEL when the drive appears
4. Copy the `.uf2` file from the repo onto the drive
5. The drive will automatically disappear and reboot if successful

---

### 2. Open the serial terminal **FIRST**

Before plugging in (or rebooting) the Pico 2 W, run:

```bash
screen /dev/ttyACM0 115200
```

This ensures the serial connection is ready when the Pico boots.

---

### 3. Wait for startup

After plugging it in or rebooting:

* Wait about **5 seconds**
* You should see:

```
READY
```

If you don’t:

* Unplug the Pico
* Close `screen` (`Ctrl+A`, then `K`, then `Y`)
* Try again

---

## Menu Options

Once running, you’ll see:

```
1=List 2=Add 3=Done 4=Del
>
```

### Options Explained

* **1 – List**

  * Shows all current tasks
  * `[X]` = done
  * `[ ]` = not done

* **2 – Add**

  * Adds a task if space is available
  * Hard limit of 2 tasks

* **3 – Done**

  * Marks a task as completed

* **4 – Del**

  * Deletes a task
  * Task 2 shifts into slot 1 if needed

---

## Example Output

```
READY

1=List 2=Add 3=Done 4=Del
>2
Task: homework
OK

>1
Tasks:
1. [ ] homework
```

---

## Build Notes

* Uses `pico/stdlib`
* Uses `stdio` over USB
* No external libraries
* Designed to be compiled with the Pico SDK

---

## Why Pico 2 W Only?

This was written and tested specifically for the **Pico 2 W environment**, including:

* USB serial behavior
* SDK configuration
* Timing expectations

Other boards **may behave differently** or not work at all.

---

## Disclaimer

This is a **toy / learning project**, not a real task manager.
If it breaks, locks up, or behaves weirdly — that’s part of learning embedded systems 😄

---
