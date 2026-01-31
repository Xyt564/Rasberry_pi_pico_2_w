## Raspberry Pi Pico W LED Example

This is a simple project for the **Raspberry Pi Pico W** that demonstrates controlling the onboard LED using **C++**. I wrote this to practice C++ and get familiar with the Pico 2 W’s WiFi chip and GPIO features.

## What it Does

* Initializes the standard I/O for debugging.
* Initializes the WiFi chip (required for controlling the onboard LED on the Pico W).
* Turns the onboard LED **on**.
* Keeps the program running indefinitely.

Basically, when you flash the `.uf2` file to your Pico W, the onboard LED will light up and stay on.

## Hardware

* Raspberry Pi Pico 2 W

## Software

* Pico SDK (C/C++ development)
* `pico/stdlib.h` and `pico/cyw43_arch.h` libraries for LED and WiFi functionality
* CMake for building the project

## How to Use

1. To do this, press and hold the **BOOTSEL** button on your Pico W while plugging it into your computer.
2. Drag and drop the `.uf2` file onto the Pico, which should appear as a USB drive.
3. Once flashed, the onboard LED will turn **on** and remain on.
4. You can view debug messages (like WiFi initialization) via USB serial if you want.

## Code Overview

```cpp
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
    stdio_init_all();                 // Initialize standard I/O
    if (cyw43_arch_init()) {          // Initialize WiFi chip
        printf("WiFi init failed\n");
        return -1;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // Turn on LED

    while (true) {                    // Keep program running
        sleep_ms(1000);
    }

    cyw43_arch_deinit();              // Cleanup (never reached)
    return 0;
}
```

## Learning Notes

* This project helped me understand:

  * Basic C++ syntax and structure
  * Controlling GPIO pins on the Pico W
  * Working with the Pico W SDK
  * How the onboard LED is tied to the WiFi chip

## License

Feel free to use and theres not really much to modify for this. if you would like the full code just put a request in and I will add it in.

---
