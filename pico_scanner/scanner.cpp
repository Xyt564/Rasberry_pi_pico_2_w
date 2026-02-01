#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"

// WiFi credentials - CHANGE THESE!
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASS";

// Server configuration
#define SERVER_PORT 9999
#define SCAN_TIMEOUT_MS 500

class PortScanner {
private:
    struct tcp_pcb* server_pcb;
    struct tcp_pcb* client_pcb;
    
    ip_addr_t target_ip;
    uint16_t current_port;
    uint16_t start_port;
    uint16_t end_port;
    std::vector<uint16_t> open_ports;
    bool scanning;
    absolute_time_t scan_start_time;
    
public:
    PortScanner() : server_pcb(nullptr), client_pcb(nullptr), 
                    current_port(0), start_port(0), end_port(0), 
                    scanning(false) {}
    
    void send_message(const char* msg) {
        if (client_pcb && msg) {
            tcp_write(client_pcb, msg, strlen(msg), TCP_WRITE_FLAG_COPY);
            tcp_output(client_pcb);
        }
    }
    
    void send_message(const std::string& msg) {
        send_message(msg.c_str());
    }
    
    // Static callback wrappers
    static err_t static_accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err) {
        return ((PortScanner*)arg)->accept_callback(newpcb, err);
    }
    
    static err_t static_recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
        return ((PortScanner*)arg)->recv_callback(tpcb, p, err);
    }
    
    static err_t static_scan_connected(void* arg, struct tcp_pcb* tpcb, err_t err) {
        return ((PortScanner*)arg)->scan_connected_callback(tpcb, err);
    }
    
    static void static_scan_error(void* arg, err_t err) {
        ((PortScanner*)arg)->scan_error_callback(err);
    }
    
    // Accept new client connection
    err_t accept_callback(struct tcp_pcb* newpcb, err_t err) {
        if (err != ERR_OK || newpcb == nullptr) {
            return ERR_VAL;
        }
        
        printf("Client connected\n");
        client_pcb = newpcb;
        tcp_arg(client_pcb, this);
        tcp_recv(client_pcb, static_recv_callback);
        
        send_message("=== Pico Port Scanner v1.0 ===\n");
        send_message("Usage: SCAN <target_ip> <start_port>-<end_port>\n");
        send_message("Example: SCAN 192.168.1.1 1-1024\n");
        send_message("> ");
        
        return ERR_OK;
    }
    
    // Receive data from client
    err_t recv_callback(struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
        if (!p) {
            // Connection closed
            printf("Client disconnected\n");
            tcp_close(tpcb);
            client_pcb = nullptr;
            scanning = false;
            return ERR_OK;
        }
        
        if (p->tot_len > 0) {
            char buffer[256] = {0};
            pbuf_copy_partial(p, buffer, p->tot_len, 0);
            buffer[p->tot_len] = '\0';
            
            printf("Received: %s\n", buffer);
            parse_command(buffer);
        }
        
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        
        return ERR_OK;
    }
    
    // Parse command from client
    void parse_command(const char* cmd) {
        char command[16], ip_str[32], ports[32];
        
        if (sscanf(cmd, "%15s %31s %31s", command, ip_str, ports) != 3) {
            send_message("Invalid format. Use: SCAN <ip> <start>-<end>\n> ");
            return;
        }
        
        if (strcasecmp(command, "SCAN") != 0) {
            send_message("Unknown command. Use SCAN\n> ");
            return;
        }
        
        // Parse IP address
        if (!ipaddr_aton(ip_str, &target_ip)) {
            send_message("Invalid IP address\n> ");
            return;
        }
        
        // Parse port range
        if (sscanf(ports, "%hu-%hu", &start_port, &end_port) != 2) {
            send_message("Invalid port range. Use format: 1-1024\n> ");
            return;
        }
        
        if (start_port < 1 || end_port > 65535 || start_port > end_port) {
            send_message("Invalid port range (1-65535)\n> ");
            return;
        }
        
        // Start scanning
        start_scan();
    }
    
    void start_scan() {
        scanning = true;
        current_port = start_port;
        open_ports.clear();
        scan_start_time = get_absolute_time();
        
        char msg[128];
        snprintf(msg, sizeof(msg), "Scanning %s ports %d-%d...\n", 
                 ipaddr_ntoa(&target_ip), start_port, end_port);
        send_message(msg);
        
        scan_next_port();
    }
    
    void scan_next_port() {
        if (!scanning || current_port > end_port) {
            finish_scan();
            return;
        }
        
        // Progress update every 100 ports
        if ((current_port - start_port) % 100 == 0) {
            float progress = ((float)(current_port - start_port) / 
                            (end_port - start_port + 1)) * 100.0f;
            char msg[64];
            snprintf(msg, sizeof(msg), "Progress: %.1f%% (port %d)\r", 
                    progress, current_port);
            send_message(msg);
        }
        
        // Create new TCP connection for port scan
        struct tcp_pcb* scan_pcb = tcp_new();
        if (!scan_pcb) {
            current_port++;
            scan_next_port();
            return;
        }
        
        tcp_arg(scan_pcb, this);
        tcp_err(scan_pcb, static_scan_error);
        
        // Attempt to connect
        err_t err = tcp_connect(scan_pcb, &target_ip, current_port, static_scan_connected);
        
        if (err != ERR_OK) {
            tcp_abort(scan_pcb);
            current_port++;
            scan_next_port();
        }
    }
    
    err_t scan_connected_callback(struct tcp_pcb* tpcb, err_t err) {
        if (err == ERR_OK) {
            // Port is open!
            char msg[64];
            snprintf(msg, sizeof(msg), "\n[+] Port %d OPEN\n", current_port);
            send_message(msg);
            open_ports.push_back(current_port);
        }
        
        tcp_abort(tpcb);
        current_port++;
        
        // Small delay to avoid overwhelming
        sleep_ms(5);
        scan_next_port();
        
        return ERR_OK;
    }
    
    void scan_error_callback(err_t err) {
        // Port closed or unreachable
        current_port++;
        scan_next_port();
    }
    
    void finish_scan() {
        if (!scanning) return;
        
        scanning = false;
        int64_t elapsed = absolute_time_diff_us(scan_start_time, get_absolute_time()) / 1000;
        
        char msg[256];
        snprintf(msg, sizeof(msg), "\n\n=== Scan Complete ===\n");
        send_message(msg);
        
        snprintf(msg, sizeof(msg), "Scanned %d ports in %lld ms\n", 
                (end_port - start_port + 1), elapsed);
        send_message(msg);
        
        snprintf(msg, sizeof(msg), "Found %zu open port(s)\n", open_ports.size());
        send_message(msg);
        
        if (!open_ports.empty()) {
            send_message("Open ports: ");
            for (size_t i = 0; i < open_ports.size(); i++) {
                snprintf(msg, sizeof(msg), "%d ", open_ports[i]);
                send_message(msg);
            }
            send_message("\n");
        }
        
        send_message("\n> ");
    }
    
    bool start_server() {
        server_pcb = tcp_new();
        if (!server_pcb) {
            printf("Failed to create server PCB\n");
            return false;
        }
        
        err_t err = tcp_bind(server_pcb, IP_ADDR_ANY, SERVER_PORT);
        if (err != ERR_OK) {
            printf("Failed to bind to port %d\n", SERVER_PORT);
            return false;
        }
        
        server_pcb = tcp_listen(server_pcb);
        tcp_arg(server_pcb, this);
        tcp_accept(server_pcb, static_accept_callback);
        
        printf("Server listening on port %d\n", SERVER_PORT);
        return true;
    }
};

// Global scanner instance
PortScanner* scanner = nullptr;

int main() {
    stdio_init_all();
    
    // Initialize Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Failed to initialize WiFi\n");
        return 1;
    }
    
    // Blink LED once on startup
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(500);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    
    printf("Pico Port Scanner Starting...\n");
    
    // Enable station mode
    cyw43_arch_enable_sta_mode();
    
    printf("Connecting to WiFi '%s'...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to WiFi\n");
        return 1;
    }
    
    printf("Connected to WiFi!\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    
    // Create and start scanner
    scanner = new PortScanner();
    if (!scanner->start_server()) {
        printf("Failed to start server\n");
        return 1;
    }
    
    printf("\nReady! Connect with: nc %s %d\n", 
           ip4addr_ntoa(netif_ip4_addr(netif_list)), SERVER_PORT);
    
    // Main loop
    while (true) {
        sleep_ms(100);
    }
    
    cyw43_arch_deinit();
    return 0;
}
