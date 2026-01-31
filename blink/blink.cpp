#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
    // Initialize stdio (optional, for debugging via USB)
    stdio_init_all();
    
    // Initialize the WiFi chip (required for Pico W LED control)
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        return -1;
    }
    
    // Turn on the LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    
    // Keep the program running
    while (true) {
        sleep_ms(1000);
    }
    
    // Cleanup (won't actually reach here due to infinite loop)
    cyw43_arch_deinit();
    return 0;
}
