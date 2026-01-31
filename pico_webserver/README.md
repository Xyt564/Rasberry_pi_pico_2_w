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

You are **not** expected to rewrite the C/C++.

### ✔ Safe to edit

```c
const char *html_content = "...";
```

You can:

* Change text
* Change styles
* Add/remove sections
* Replace links
* Make it your own portfolio / info page

### ❌ Don’t touch unless you know what you’re doing

* TCP callbacks
* lwIP logic
* WiFi setup
* Memory handling

Keeping the C/C++ logic the same ensures stability on the Pico 2 W.

---

## 🌐 Exposing It to the Internet (Free)

This server is designed for **local networks**, but you can expose it **publicly for free** using **Cloudflare Tunnel (cloudflared)**.

### How it works (high level)

1. Pico 2 W runs the web server on your LAN
2. A separate machine (PC / server / laptop) runs `cloudflared`
3. Cloudflare creates a secure tunnel to your Pico
4. You get a public HTTPS URL — **no port forwarding**

This is:

* Free
* Secure
* Fast to set up
* No router config required

> The Pico itself does **not** run cloudflared — it just serves HTTP.

---

## 🔧 How It Works (Under the Hood)

1. Pico 2 W connects to WiFi using the CYW43 driver
2. A TCP server is opened on port **80**
3. When a client connects:

   * The request is checked for `GET`
   * A minimal HTTP response is constructed
   * HTML is sent directly from memory
4. Connection is closed cleanly after sending

No threading, no async framework — just **raw TCP callbacks**.

---

## 📦 Tech Stack

* Raspberry Pi Pico SDK
* CYW43 WiFi stack
* lwIP TCP/IP stack
* C / C++
* Bare-metal embedded networking

---

## 🧠 Design Philosophy

This server is intentionally:

* Minimal
* Static
* Limited

That’s not a flaw — it’s **necessary**.

The Pico 2 W has:

* Limited RAM
* No MMU
* No filesystem
* No process isolation

If this were more complex, it **wouldn’t run reliably**.

So the design is:

> *“Do one thing, do it well, and don’t waste memory.”*

---

## 🚀 Use Cases

* Embedded status page
* Device info page
* Mini portfolio
* Internal tools
* Learning lwIP + embedded networking
* Quick demo servers

---

## 📝 Notes

* Default port: **80**
* WPA2 WiFi only
* One client at a time (by design)
* Not intended for production workloads

---

## 📜 License

Use it, modify it, break it, learn from it.
If you improve it — even better.

---

Got you — this fits really nicely as a short **Security / Safety** section. I’ll keep it honest and technical (no cringe “unhackable” claims), but still clearly explain *why* this is low-risk.

You can drop this straight into the README.

---

### 🔐 Security & Safety Notes

This project is **inherently low-risk by design**, mainly because it runs on a **bare-metal microcontroller**, not a traditional computer.

### 🧠 No Operating System

* The Pico 2 W runs **without an OS**
* No users, no processes, no shell
* No package manager, services, or background daemons
* Nothing to “log into” or escalate privileges on

There is simply **no attack surface** like you’d find on Linux or Windows.

---

### 📦 Static, Read-Only Behaviour

* Serves a **single static HTML page**
* No file uploads
* No form handling
* No dynamic execution
* No filesystem

Requests are handled in memory and discarded immediately.

---

### 🌍 Local Network by Default

By default, the server:

* Is only accessible on your **local network**
* Is not publicly exposed
* Requires no port forwarding

This alone already makes it very hard to interact with from outside.

---

### ☁️ Cloudflare Tunnel Protection (Optional)

If you expose it using **Cloudflare Tunnel (cloudflared)**:

* The Pico is **never directly exposed** to the internet
* Cloudflare sits in front of it as a reverse proxy
* You get:

  * HTTPS
  * DDoS protection
  * Rate limiting
  * Bot filtering

The microcontroller only ever sees traffic from Cloudflare.

---

### 🔒 Realistic Threat Model

Could it theoretically have bugs? Yes — like any C++ code.

But in practice:

* There’s no OS to compromise
* No persistence mechanism
* No remote code execution path
* A crash just resets the device

Worst case: the server stops responding and you reboot it.

---

### ✅ Summary

This setup is safe because:

* Bare-metal microcontroller
* No OS
* No user input handling
* Local-only by default
* Cloudflare protection when exposed

It’s about as low-risk as a networked device can realistically be.

---
