// edit as you see fit to meet ur own requirements

#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// WiFi credentials - UPDATE THESE!
const char WIFI_SSID[] = "YOUR_SSID";
const char WIFI_PASSWORD[] = "YOUR_PASSWORD";

#define TCP_PORT 80
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"

// Minimal HTML page (u can edit from here. if u dont know how to just look for the words and change them or use Ai)
const char *html_content = 
    "<!DOCTYPE html>"
    "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Xyt564| Pico Server</title>"
    "<style>"
    "body{font-family:system-ui,sans-serif;max-width:800px;margin:40px auto;padding:20px;background:#0a0a0a;color:#e0e0e0;line-height:1.6}"
    "h1{color:#60a5fa;border-bottom:2px solid #1e40af;padding-bottom:10px}"
    "h2{color:#93c5fd;margin-top:30px}"
    ".intro{background:#1a1a1a;padding:20px;border-radius:8px;border-left:4px solid #60a5fa;margin:20px 0}"
    ".skills{display:flex;flex-wrap:wrap;gap:8px;margin:15px 0}"
    ".skill{background:#1e40af;color:#fff;padding:6px 12px;border-radius:4px;font-size:14px}"
    ".footer{margin-top:40px;padding-top:20px;border-top:1px solid #333;color:#888;text-align:center;font-size:14px}"
    "a{color:#60a5fa;text-decoration:none}"
    "a:hover{text-decoration:underline}"
    ".status{color:#4ade80;font-size:12px}"
    "</style></head><body>"
    "<div class='status'>🟢 Served from Raspberry Pi Pico 2 W</div>"
    "<h1>Xyt564</h1>"
    "<div class='intro'>"
    "<strong>Self-Taught Developer & Tinkerer</strong><br>"
    "19-year-old college student passionate about systems programming, embedded systems, and cybersecurity. "
    "From building custom programming languages to running home servers on old laptops, I love turning curiosity into working code."
    "</div>"
    "<h2>Featured Projects</h2>"
    "<p><strong>Star Lang</strong> - Custom programming language with lexer, parser, and interpreter built from scratch in C++</p>"
    "<p><strong>M5StickC Plus2 Firmware</strong> - Custom firmware for ESP32-based device with WiFi, Bluetooth, and optimized power management</p>"
    "<p><strong>Home Server</strong> - Repurposed laptop running Linux with file storage, media streaming, and self-hosted services</p>"
    "<p><strong>Custom Game</strong> - Game built in C++ with SDL2, exploring game loops, rendering, and physics from the ground up</p>"
    "<h2>Tech Stack</h2>"
    "<div class='skills'>"
    "<span class='skill'>Python</span>"
    "<span class='skill'>C/C++</span>"
    "<span class='skill'>Rust</span>"
    "<span class='skill'>TypeScript</span>"
    "<span class='skill'>Assembly</span>"
    "<span class='skill'>Embedded Systems</span>"
    "<span class='skill'>Linux</span>"
    "<span class='skill'>Security</span>"
    "</div>"
    "<h2>About</h2>"
    "<p>2+ years of hands-on experience in systems programming, cybersecurity, and building practical tools. "
    "I approach development with a security-first mindset and value understanding my stack end-to-end.</p>"
    "<p><strong>GitHub:</strong> <a href='https://github.com/Xyt564' target='_blank'>Xyt564</a></p>"
    "<div class='footer'>"
    "Optimized for microcontrollers | Built with Pico SDK | "
    "<a href='https://v0-xyt564.vercel.app/' target='_blank'>Full Portfolio</a>"
    "</div>"
    "</body></html>";

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
} TCP_SERVER_T;

static err_t tcp_close_client_connection(struct tcp_pcb *client_pcb, TCP_SERVER_T *state) {
    tcp_arg(client_pcb, NULL);
    tcp_poll(client_pcb, NULL, 0);
    tcp_sent(client_pcb, NULL);
    tcp_recv(client_pcb, NULL);
    tcp_err(client_pcb, NULL);
    err_t err = tcp_close(client_pcb);
    if (err != ERR_OK) {
        tcp_abort(client_pcb);
        return ERR_ABRT;
    }
    return ERR_OK;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    state->complete = true;
    return tcp_close_client_connection(pcb, state);
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (!p) {
        return tcp_close_client_connection(pcb, state);
    }

    if (p->tot_len > 0) {
        // Check if it's a GET request
        if (strncmp(HTTP_GET, (char*)p->payload, strlen(HTTP_GET)) == 0) {
            size_t content_len = strlen(html_content);
            size_t header_len = snprintf(NULL, 0, HTTP_RESPONSE_HEADERS, 200, content_len);
            size_t total_len = header_len + content_len;
            
            char *response = (char*)malloc(total_len + 1);
            if (response) {
                snprintf(response, header_len + 1, HTTP_RESPONSE_HEADERS, 200, content_len);
                strcat(response, html_content);
                
                err_t write_err = tcp_write(pcb, response, total_len, TCP_WRITE_FLAG_COPY);
                if (write_err == ERR_OK) {
                    tcp_output(pcb);
                }
                free(response);
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }
    
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, NULL, POLL_TIME_S * 2);
    
    return ERR_OK;
}

static bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        tcp_close(pcb);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

int main() {
    stdio_init_all();
    
    TCP_SERVER_T *state = (TCP_SERVER_T*)calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        return 1;
    }

    // Initialize WiFi chip
    if (cyw43_arch_init()) {
        printf("Failed to initialize WiFi\n");
        return 1;
    }

    // Enable station mode
    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    
    // Connect to WiFi
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to WiFi\n");
        return 1;
    }
    
    printf("Connected to WiFi!\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    
    // Blink LED once to indicate server is starting
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(500);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    
    // Start the server
    if (!tcp_server_open(state)) {
        printf("Failed to open server\n");
        return 1;
    }
    
    printf("Server running on port %d\n", TCP_PORT);
    
    // Keep the server running
    while(true) {
        #if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        sleep_ms(1);
        #else
        sleep_ms(1000);
        #endif
    }

    cyw43_arch_deinit();
    free(state);
    return 0;
}
