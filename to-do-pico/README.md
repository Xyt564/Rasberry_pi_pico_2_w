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

## Limitations (Important)

Because this runs on a microcontroller with very limited resources:

* **Maximum of 2 tasks**
* Task names are limited to **14 characters**
* Tasks are **not saved** (everything is lost on reboot)
* No scrolling, no fancy UI, just serial text
* Blocking input (this is intentional and simple)

This is expected behavior — not a bug.

---

## How to Use

### 1. Install the uf2 file from repo.
### hold the bootceel until the drive appears in the folder app.
### then put the uf2 file in there then the drive should auto disappear and rebbot if succesful 

### 2. Open the serial terminal FIRST

Before plugging in the Pico 2 W, run this command:

```bash
screen /dev/ttyACM0 115200
```

> This ensures the serial connection is ready when the Pico boots.

---

### 2. Plug in the Pico 2 W

After plugging it in:

* Wait about **5 seconds**
* You should see:

```
READY
```

If you don’t see anything, unplug it, close `screen`, and try again (to close screen do ctrl+A then press k then Y.

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
  * `[X]` means done
  * `[ ]` means not done

* **2 – Add**

  * Adds a task (if space is available)
  * Max 2 tasks total

* **3 – Done**

  * Marks a task as completed

* **4 – Del**

  * Deletes a task
  * Tasks shift up automatically if needed

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

This is a **toy project / learning project**, not a full task manager.
If it breaks, locks up, or behaves weirdly — that’s kind of the point 😄

---
