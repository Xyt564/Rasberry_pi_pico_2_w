/**
 * Raspberry Pi Pico 2 W Operating System
 * Full-featured OS with shell, filesystem, networking, and utilities
 * Communicates over USB serial (TTY)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/util/datetime.h"
#include "hardware/watchdog.h"
#include "hardware/flash.h"
#include "lwip/apps/sntp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// LittleFS includes
extern "C" {
#include "lfs.h"
}

// System configuration
#define MAX_COMMAND_LEN 256
#define MAX_ARGS 16
#define MAX_PROCESSES 8
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - (512 * 1024)) // Last 512KB for filesystem
#define LFS_BLOCK_SIZE 4096
#define LFS_BLOCK_COUNT 128

// ANSI color codes for terminal
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_CLEAR_SCREEN "\033[2J\033[H"

// Process structure
struct Process {
    char name[32];
    bool running;
    uint32_t start_time;
    void (*func)(void);
};

// Todo item structure
struct TodoItem {
    char text[100];
    bool completed;
    bool active;
};

// Global variables
static char command_buffer[MAX_COMMAND_LEN];
static int cmd_index = 0;
static Process processes[MAX_PROCESSES];
static int process_count = 0;
static absolute_time_t boot_time;
static bool wifi_connected = false;
static char wifi_ssid[64] = "";
static char wifi_password[64] = "";
static char timezone_str[32] = "GMT";
static int timezone_offset = 0; // UK timezone (will be +1 during BST)
static bool ntp_synced = false;
static time_t system_time_offset = 0; // Offset from boot time to actual time
static absolute_time_t time_sync_base; // When we last synced time

// LittleFS variables
static lfs_t lfs;
static struct lfs_config lfs_cfg;
static uint8_t lfs_read_buffer[LFS_BLOCK_SIZE];
static uint8_t lfs_prog_buffer[LFS_BLOCK_SIZE];
static uint8_t lfs_lookahead_buffer[128];

// Log system
#define MAX_LOG_ENTRIES 50
static char log_entries[MAX_LOG_ENTRIES][128];
static int log_index = 0;
static int log_count = 0;

// Todo list
static TodoItem todos[2] = {
    {"", false, false},
    {"", false, false}
};

// Forward declarations
void shell_loop();
void log_message(const char* msg);
void print_prompt();
void execute_command(char* cmd);
char* read_line(const char* prompt, bool echo);

// Panic handler - prints panic info over USB serial
void panic_handler(const char *fmt, ...) {
    // Try to print panic message over USB
    printf("\r\n\r\n");
    printf("╔════════════════════════════════════════╗\r\n");
    printf("║           SYSTEM PANIC!                ║\r\n");
    printf("╔════════════════════════════════════════╗\r\n");
    printf("\r\n");
    
    va_list args;
    va_start(args, fmt);
    printf("PANIC: ");
    vprintf(fmt, args);
    printf("\r\n");
    va_end(args);
    
    printf("\r\n");
    printf("System halted. Please reboot (unplug/replug).\r\n");
    printf("\r\n");
    
    // Flash LED rapidly to indicate panic
    while (true) {
        busy_wait_ms(100);
    }
}

// Helper function to get current time
time_t get_current_time() {
    if (system_time_offset == 0) {
        return 0; // No time set yet
    }
    uint64_t elapsed_ms = absolute_time_diff_us(time_sync_base, get_absolute_time()) / 1000;
    return system_time_offset + (elapsed_ms / 1000);
}

// Helper function to set current time
void set_current_time(time_t new_time) {
    system_time_offset = new_time;
    time_sync_base = get_absolute_time();
    ntp_synced = true;
}

// LittleFS flash operations
int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size) {
    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;
    memcpy(buffer, (uint8_t*)XIP_BASE + addr, size);
    return 0;
}

int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                   lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, (const uint8_t*)buffer, size);
    restore_interrupts(ints);
    return 0;
}

int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr, c->block_size);
    restore_interrupts(ints);
    return 0;
}

int lfs_flash_sync(const struct lfs_config *c) {
    return 0;
}

// Initialize LittleFS
void init_filesystem() {
    lfs_cfg.read = lfs_flash_read;
    lfs_cfg.prog = lfs_flash_prog;
    lfs_cfg.erase = lfs_flash_erase;
    lfs_cfg.sync = lfs_flash_sync;
    lfs_cfg.read_size = 1;
    lfs_cfg.prog_size = FLASH_PAGE_SIZE;
    lfs_cfg.block_size = LFS_BLOCK_SIZE;
    lfs_cfg.block_count = LFS_BLOCK_COUNT;
    lfs_cfg.cache_size = 256;
    lfs_cfg.lookahead_size = 128;
    lfs_cfg.block_cycles = 500;
    lfs_cfg.read_buffer = lfs_read_buffer;
    lfs_cfg.prog_buffer = lfs_prog_buffer;
    lfs_cfg.lookahead_buffer = lfs_lookahead_buffer;

    int err = lfs_mount(&lfs, &lfs_cfg);
    if (err) {
        printf("Formatting filesystem...\r\n");
        err = lfs_format(&lfs, &lfs_cfg);
        if (err) {
            printf("ERROR: Failed to format filesystem (code %d)\r\n", err);
            log_message("ERROR: Filesystem format failed");
            return;
        }
        err = lfs_mount(&lfs, &lfs_cfg);
        if (err) {
            printf("ERROR: Failed to mount filesystem after format (code %d)\r\n", err);
            log_message("ERROR: Filesystem mount failed");
            return;
        }
    }
    
    log_message("Filesystem mounted successfully");
}

// Log message function
void log_message(const char* msg) {
    time_t now = get_current_time();
    if (now == 0) {
        // No time set yet, use uptime
        uint32_t uptime_sec = to_ms_since_boot(get_absolute_time()) / 1000;
        snprintf(log_entries[log_index], sizeof(log_entries[0]), 
                 "[+%05lus] %s", (unsigned long)uptime_sec, msg);
    } else {
        struct tm *t = localtime(&now);
        snprintf(log_entries[log_index], sizeof(log_entries[0]), 
                 "[%02d:%02d:%02d] %s", t->tm_hour, t->tm_min, t->tm_sec, msg);
    }
    log_index = (log_index + 1) % MAX_LOG_ENTRIES;
    if (log_count < MAX_LOG_ENTRIES) log_count++;
}

// NTP time sync callback
void ntp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p->tot_len >= 48) {
        uint8_t *data = (uint8_t*)p->payload;
        uint32_t ntp_time = (data[40] << 24) | (data[41] << 16) | (data[42] << 8) | data[43];
        time_t unix_time = ntp_time - 2208988800UL + (timezone_offset * 3600); // NTP to Unix epoch with timezone
        
        set_current_time(unix_time);
        log_message("NTP time synchronized");
        printf(ANSI_GREEN "Time synchronized successfully!\n" ANSI_RESET);
    }
    pbuf_free(p);
}

void sync_ntp_time() {
    if (!wifi_connected) {
        printf(ANSI_YELLOW "WiFi not connected. Cannot sync time.\n" ANSI_RESET);
        return;
    }
    
    printf("Syncing time with NTP server...\n");
    
    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf(ANSI_RED "Failed to create UDP socket\n" ANSI_RESET);
        return;
    }
    
    ip_addr_t ntp_server;
    if (!ipaddr_aton("129.6.15.28", &ntp_server)) { // NIST time server
        printf(ANSI_RED "Failed to resolve NTP server\n" ANSI_RESET);
        udp_remove(pcb);
        return;
    }
    
    udp_recv(pcb, ntp_recv_callback, NULL);
    
    uint8_t ntp_packet[48] = {0};
    ntp_packet[0] = 0x1B; // LI=0, VN=3, Mode=3 (client)
    
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_RAM);
    if (p) {
        memcpy(p->payload, ntp_packet, 48);
        udp_sendto(pcb, p, &ntp_server, 123);
        pbuf_free(p);
        
        // Wait for response
        sleep_ms(2000);
    }
}

// WiFi connection
void connect_wifi() {
    if (strlen(wifi_ssid) == 0) {
        printf(ANSI_RED "No WiFi credentials configured\n" ANSI_RESET);
        printf("Use 'setting' command to configure WiFi\n");
        return;
    }
    
    printf("Connecting to WiFi: %s...\n", wifi_ssid);
    
    if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, 
                                            CYW43_AUTH_WPA2_AES_PSK, 30000) == 0) {
        wifi_connected = true;
        printf(ANSI_GREEN "WiFi connected!\n" ANSI_RESET);
        log_message("WiFi connected");
        sleep_ms(1000);
        sync_ntp_time();
    } else {
        wifi_connected = false;
        printf(ANSI_RED "WiFi connection failed\n" ANSI_RESET);
        log_message("WiFi connection failed");
    }
}

// Process management
int add_process(const char* name, void (*func)(void)) {
    if (process_count >= MAX_PROCESSES) return -1;
    
    strncpy(processes[process_count].name, name, sizeof(processes[0].name) - 1);
    processes[process_count].running = true;
    processes[process_count].start_time = to_ms_since_boot(get_absolute_time());
    processes[process_count].func = func;
    process_count++;
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Started process: %s", name);
    log_message(msg);
    
    return process_count - 1;
}

void stop_process(const char* name) {
    for (int i = 0; i < process_count; i++) {
        if (strcmp(processes[i].name, name) == 0 && processes[i].running) {
            processes[i].running = false;
            char msg[64];
            snprintf(msg, sizeof(msg), "Stopped process: %s", name);
            log_message(msg);
            printf("Process '%s' stopped\n", name);
            return;
        }
    }
    printf("Process '%s' not found\n", name);
}

void list_processes() {
    printf("\n=== Running Processes ===\n");
    bool found = false;
    for (int i = 0; i < process_count; i++) {
        if (processes[i].running) {
            uint32_t runtime = to_ms_since_boot(get_absolute_time()) - processes[i].start_time;
            printf("  %s (runtime: %lu ms)\n", processes[i].name, (unsigned long)runtime);
            found = true;
        }
    }
    if (!found) {
        printf("  No processes running\n");
    }
    printf("\n");
}

// Improved input reading with echo control
char* read_line(const char* prompt, bool echo) {
    static char input_buffer[MAX_COMMAND_LEN];
    int index = 0;
    
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }
    
    while (true) {
        int c = getchar_timeout_us(1000000); // 1 second timeout
        
        if (c == PICO_ERROR_TIMEOUT) {
            continue;
        }
        
        if (c == '\r' || c == '\n') {
            input_buffer[index] = '\0';
            printf("\r\n");
            fflush(stdout);
            return input_buffer;
        } else if (c == 127 || c == 8) { // Backspace
            if (index > 0) {
                index--;
                printf("\b \b"); // Always show backspace visually
                fflush(stdout);
            }
        } else if (c >= 32 && c < 127) {
            if (index < MAX_COMMAND_LEN - 1) {
                input_buffer[index++] = c;
                if (echo) {
                    putchar(c);
                } else {
                    putchar('*'); // Show asterisk for password
                }
                fflush(stdout);
            }
        }
    }
}

// System info display
void show_system_info() {
    uint32_t uptime_sec = to_ms_since_boot(get_absolute_time()) / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    
    printf("\n=== System Information ===\n");
    printf("Platform: Raspberry Pi Pico 2 W\n");
    printf("CPU: Dual-core ARM Cortex-M33\n");
    printf("RAM: 520 KB SRAM\n");
    printf("Flash: 4 MB\n");
    printf("Uptime: %02lu:%02lu:%02lu\n", 
           (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);
    printf("WiFi: %s\n", wifi_connected ? ANSI_GREEN "Connected" ANSI_RESET : ANSI_RED "Disconnected" ANSI_RESET);
    if (wifi_connected) {
        printf("SSID: %s\n", wifi_ssid);
    }
    printf("Time Synced: %s\n", ntp_synced ? ANSI_GREEN "Yes" ANSI_RESET : ANSI_YELLOW "No" ANSI_RESET);
    printf("\n");
}

// File operations
void list_files() {
    printf("\n=== Files ===\n");
    lfs_dir_t dir;
    if (lfs_dir_open(&lfs, &dir, "/") >= 0) {
        struct lfs_info info;
        while (lfs_dir_read(&lfs, &dir, &info) > 0) {
            if (strcmp(info.name, ".") != 0 && strcmp(info.name, "..") != 0) {
                if (info.type == LFS_TYPE_DIR) {
                    printf("  [DIR]  %s\n", info.name);
                } else {
                    printf("  [FILE] %s (%lu bytes)\n", info.name, (unsigned long)info.size);
                }
            }
        }
        lfs_dir_close(&lfs, &dir);
    }
    printf("\n");
}

void view_file(const char* filename) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, filename, LFS_O_RDONLY) >= 0) {
        printf("\n=== %s ===\n", filename);
        char buf[256];
        int len;
        while ((len = lfs_file_read(&lfs, &file, buf, sizeof(buf) - 1)) > 0) {
            buf[len] = '\0';
            printf("%s", buf);
        }
        printf("\n");
        lfs_file_close(&lfs, &file);
    } else {
        printf(ANSI_RED "Error: Could not open file '%s'\n" ANSI_RESET, filename);
    }
}

void nano_editor(const char* filename) {
    printf("\n=== Nano Editor: %s ===\n", filename);
    printf("Enter text (type 'SAVE' on a new line to save and exit):\n\n");
    
    char content[2048];
    int content_len = 0;
    
    while ((size_t)content_len < sizeof(content) - 1) {
        char* line = read_line(NULL, true);
        
        if (strcmp(line, "SAVE") == 0) {
            break;
        }
        
        int line_len = strlen(line);
        if ((size_t)(content_len + line_len + 1) < sizeof(content)) {
            memcpy(content + content_len, line, line_len);
            content_len += line_len;
            content[content_len++] = '\n';
        }
    }
    
    content[content_len] = '\0';
    
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) >= 0) {
        lfs_file_write(&lfs, &file, content, content_len);
        lfs_file_close(&lfs, &file);
        printf(ANSI_GREEN "File saved successfully!\n" ANSI_RESET);
    } else {
        printf(ANSI_RED "Error: Could not save file\n" ANSI_RESET);
    }
}

void delete_file(const char* filename) {
    if (lfs_remove(&lfs, filename) >= 0) {
        printf(ANSI_GREEN "File '%s' deleted\n" ANSI_RESET, filename);
    } else {
        printf(ANSI_RED "Error: Could not delete file '%s'\n" ANSI_RESET, filename);
    }
}

void show_storage_info() {
    printf("\n=== Storage Information ===\n");
    printf("Block size: %lu bytes\n", (unsigned long)lfs_cfg.block_size);
    printf("Block count: %lu\n", (unsigned long)lfs_cfg.block_count);
    printf("Total size: %lu KB\n", (unsigned long)(lfs_cfg.block_size * lfs_cfg.block_count / 1024));
    printf("\n");
}

// Network operations
void show_ip() {
    if (!wifi_connected) {
        printf(ANSI_YELLOW "WiFi not connected\n" ANSI_RESET);
        return;
    }
    
    printf("\n=== Network Information ===\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    printf("Netmask: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif_list)));
    printf("Gateway: %s\n", ip4addr_ntoa(netif_ip4_gw(netif_list)));
    printf("\n");
}

void ping_test(const char* host) {
    printf("Ping functionality would ping: %s\n", host);
    printf("(Full ICMP implementation requires additional code)\n");
}

// Time operations
void show_time() {
    printf("\n=== Current Time ===\n");
    time_t now = get_current_time();
    if (now == 0) {
        printf(ANSI_YELLOW "Time not set yet. Connect to WiFi for NTP sync.\n" ANSI_RESET);
        printf("Use 'wifi' command to connect, or 'setting' to configure WiFi.\n");
    } else {
        struct tm *t = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %Z", t);
        printf("Current time: %s\n", time_str);
        printf("Timezone: %s (UTC%+d)\n", timezone_str, timezone_offset);
    }
    printf("\n");
}

void view_log() {
    printf("\n=== System Log ===\n");
    int start = (log_count == MAX_LOG_ENTRIES) ? log_index : 0;
    for (int i = 0; i < log_count; i++) {
        int idx = (start + i) % MAX_LOG_ENTRIES;
        printf("%s\n", log_entries[idx]);
    }
    printf("\n");
}

void show_ram() {
    extern char __StackLimit, __bss_end__;
    uint32_t free_ram = &__StackLimit - &__bss_end__;
    
    printf("\n=== RAM Information ===\n");
    printf("Total RAM: 520 KB\n");
    printf("Approximate free RAM: %lu bytes\n", (unsigned long)free_ram);
    printf("\n");
}

// Timer application
void timer_app() {
    printf(ANSI_CLEAR_SCREEN);
    printf(ANSI_BOLD ANSI_CYAN "=== Timer Application ===\n" ANSI_RESET);
    printf("\n1. Stopwatch\n");
    printf("2. Countdown Timer\n");
    printf("3. Exit\n\n");
    
    char* choice = read_line("Enter choice: ", true);
    
    if (strcmp(choice, "1") == 0) {
        // Stopwatch
        printf("\nStopwatch started! Press Enter to stop...\n\n");
        absolute_time_t start = get_absolute_time();
        
        while (true) {
            int c = getchar_timeout_us(100000); // Check every 100ms
            if (c == '\r' || c == '\n') {
                break;
            }
            
            uint64_t elapsed_ms = absolute_time_diff_us(start, get_absolute_time()) / 1000;
            uint32_t seconds = elapsed_ms / 1000;
            uint32_t ms = elapsed_ms % 1000;
            uint32_t minutes = seconds / 60;
            seconds = seconds % 60;
            
            printf("\r" ANSI_BOLD ANSI_GREEN "%02lu:%02lu.%03lu" ANSI_RESET, 
                   (unsigned long)minutes, (unsigned long)seconds, (unsigned long)ms);
            fflush(stdout);
        }
        
        printf("\n\nStopwatch stopped!\n");
        
    } else if (strcmp(choice, "2") == 0) {
        // Countdown
        char* seconds_str = read_line("Enter seconds to countdown: ", true);
        int total_seconds = atoi(seconds_str);
        
        if (total_seconds <= 0) {
            printf(ANSI_RED "Invalid time!\n" ANSI_RESET);
            return;
        }
        
        printf("\nCountdown started!\n\n");
        
        for (int i = total_seconds; i >= 0; i--) {
            printf("\r" ANSI_BOLD ANSI_YELLOW "Time remaining: %d seconds   " ANSI_RESET, i);
            fflush(stdout);
            sleep_ms(1000);
        }
        
        printf("\n\n" ANSI_BOLD ANSI_GREEN "Time's up! ⏰\n" ANSI_RESET);
    }
    
    printf("\nPress Enter to continue...");
    read_line(NULL, true);
}

// Todo application
void todo_app() {
    while (true) {
        printf(ANSI_CLEAR_SCREEN);
        printf(ANSI_BOLD ANSI_CYAN "=== Todo List ===\n" ANSI_RESET);
        printf("\n");
        
        // Display todos
        for (int i = 0; i < 2; i++) {
            if (todos[i].active) {
                if (todos[i].completed) {
                    printf("  %d. [" ANSI_GREEN "✓" ANSI_RESET "] %s\n", i + 1, todos[i].text);
                } else {
                    printf("  %d. [ ] %s\n", i + 1, todos[i].text);
                }
            } else {
                printf("  %d. " ANSI_YELLOW "(empty)" ANSI_RESET "\n", i + 1);
            }
        }
        
        printf("\n" ANSI_BOLD "Actions:\n" ANSI_RESET);
        printf("1. Add/Edit Task 1\n");
        printf("2. Add/Edit Task 2\n");
        printf("3. Mark Task as Complete\n");
        printf("4. Delete Task\n");
        printf("5. Exit\n\n");
        
        char* choice = read_line("Enter choice: ", true);
        
        if (strcmp(choice, "1") == 0 || strcmp(choice, "2") == 0) {
            int idx = atoi(choice) - 1;
            char* text = read_line("Enter task description: ", true);
            strncpy(todos[idx].text, text, sizeof(todos[idx].text) - 1);
            todos[idx].text[sizeof(todos[idx].text) - 1] = '\0';
            todos[idx].active = true;
            todos[idx].completed = false;
            printf(ANSI_GREEN "Task saved!\n" ANSI_RESET);
            sleep_ms(1000);
            
        } else if (strcmp(choice, "3") == 0) {
            char* task_num = read_line("Which task to mark complete (1 or 2): ", true);
            int idx = atoi(task_num) - 1;
            if (idx >= 0 && idx < 2 && todos[idx].active) {
                todos[idx].completed = !todos[idx].completed;
                printf(ANSI_GREEN "Task updated!\n" ANSI_RESET);
            } else {
                printf(ANSI_RED "Invalid task!\n" ANSI_RESET);
            }
            sleep_ms(1000);
            
        } else if (strcmp(choice, "4") == 0) {
            char* task_num = read_line("Which task to delete (1 or 2): ", true);
            int idx = atoi(task_num) - 1;
            if (idx >= 0 && idx < 2 && todos[idx].active) {
                todos[idx].active = false;
                todos[idx].completed = false;
                todos[idx].text[0] = '\0';
                printf(ANSI_GREEN "Task deleted!\n" ANSI_RESET);
            } else {
                printf(ANSI_RED "Invalid task!\n" ANSI_RESET);
            }
            sleep_ms(1000);
            
        } else if (strcmp(choice, "5") == 0) {
            break;
        }
    }
}

// Neofetch-style system info
void neofetch() {
    printf(ANSI_CLEAR_SCREEN);
    
    uint32_t uptime_sec = to_ms_since_boot(get_absolute_time()) / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    
    time_t now = get_current_time();
    char time_str[32] = "Not synced";
    if (now != 0) {
        struct tm *t = localtime(&now);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    }
    
    // ASCII art Raspberry Pi logo
    printf(ANSI_RED "      .~~.   .~~.      " ANSI_RESET "  pico@pico-os\n");
    printf(ANSI_RED "     '. \\ ' ' / .'     " ANSI_RESET "  -------------\n");
    printf(ANSI_RED "      .~ .~~~..~.      " ANSI_RESET "  " ANSI_BOLD "OS:" ANSI_RESET " Pico OS v1.0\n");
    printf(ANSI_RED "     : .~.'~'.~. :     " ANSI_RESET "  " ANSI_BOLD "Host:" ANSI_RESET " Raspberry Pi Pico 2 W\n");
    printf(ANSI_RED "    ~ (   ) (   ) ~    " ANSI_RESET "  " ANSI_BOLD "CPU:" ANSI_RESET " Dual-core ARM Cortex-M33\n");
    printf(ANSI_RED "   ( : '~'.~.'~' : )   " ANSI_RESET "  " ANSI_BOLD "Memory:" ANSI_RESET " 520 KB SRAM\n");
    printf(ANSI_RED "    ~ .~ (   ) ~. ~    " ANSI_RESET "  " ANSI_BOLD "Flash:" ANSI_RESET " 4 MB\n");
    printf(ANSI_RED "     (  : '~' :  )     " ANSI_RESET "  " ANSI_BOLD "Uptime:" ANSI_RESET " %luh %lum\n", 
           (unsigned long)hours, (unsigned long)minutes);
    printf(ANSI_RED "      '~ .~~~. ~'      " ANSI_RESET "  " ANSI_BOLD "WiFi:" ANSI_RESET " %s\n",
           wifi_connected ? ANSI_GREEN "Connected" ANSI_RESET : ANSI_RED "Disconnected" ANSI_RESET);
    printf(ANSI_RED "          '~'          " ANSI_RESET "  " ANSI_BOLD "Time:" ANSI_RESET " %s\n", time_str);
    printf("\n");
    printf("                         " ANSI_RED "█" ANSI_GREEN "█" ANSI_YELLOW "█" ANSI_BLUE "█" ANSI_MAGENTA "█" ANSI_CYAN "█" ANSI_RESET "\n");
    printf("\n");
}

// Settings menu with fixed input
void settings_menu() {
    while (true) {
        printf("\n=== Settings ===\n");
        printf("1. WiFi Configuration\n");
        printf("2. Time/Timezone Settings\n");
        printf("3. View Current Settings\n");
        printf("4. Exit\n\n");
        
        char* choice = read_line("Enter choice: ", true);
        
        if (strcmp(choice, "1") == 0) {
            // WiFi configuration with proper echo
            char* ssid = read_line("Enter WiFi SSID: ", true);  // Show SSID as typed
            strncpy(wifi_ssid, ssid, sizeof(wifi_ssid) - 1);
            wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
            
            char* pass = read_line("Enter WiFi Password: ", false);  // Hide password
            strncpy(wifi_password, pass, sizeof(wifi_password) - 1);
            wifi_password[sizeof(wifi_password) - 1] = '\0';
            
            // Save to filesystem
            lfs_file_t file;
            if (lfs_file_open(&lfs, &file, "wifi.cfg", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) >= 0) {
                char buf[128];
                int len = snprintf(buf, sizeof(buf), "%s\n%s", wifi_ssid, wifi_password);
                lfs_file_write(&lfs, &file, buf, len);
                lfs_file_close(&lfs, &file);
                printf(ANSI_GREEN "WiFi credentials saved\n" ANSI_RESET);
            }
            
            char* connect = read_line("Connect now? (y/n): ", true);
            if (connect[0] == 'y' || connect[0] == 'Y') {
                connect_wifi();
            }
            
        } else if (strcmp(choice, "2") == 0) {
            printf("\nCurrent timezone: %s (UTC%+d)\n", timezone_str, timezone_offset);
            char* offset_str = read_line("Enter timezone offset from UTC (-12 to +14): ", true);
            timezone_offset = atoi(offset_str);
            if (timezone_offset < -12) timezone_offset = -12;
            if (timezone_offset > 14) timezone_offset = 14;
            snprintf(timezone_str, sizeof(timezone_str), "UTC%+d", timezone_offset);
            printf(ANSI_GREEN "Timezone set to %s\n" ANSI_RESET, timezone_str);
            
            if (wifi_connected) {
                char* sync = read_line("Sync time now? (y/n): ", true);
                if (sync[0] == 'y' || sync[0] == 'Y') {
                    sync_ntp_time();
                }
            }
            
        } else if (strcmp(choice, "3") == 0) {
            printf("\n=== Current Settings ===\n");
            printf("WiFi SSID: %s\n", strlen(wifi_ssid) > 0 ? wifi_ssid : ANSI_YELLOW "(not set)" ANSI_RESET);
            printf("WiFi Status: %s\n", wifi_connected ? ANSI_GREEN "Connected" ANSI_RESET : ANSI_RED "Disconnected" ANSI_RESET);
            printf("Timezone: %s\n", timezone_str);
            printf("NTP Synced: %s\n", ntp_synced ? ANSI_GREEN "Yes" ANSI_RESET : ANSI_YELLOW "No" ANSI_RESET);
            read_line("\nPress Enter to continue...", true);
            
        } else if (strcmp(choice, "4") == 0) {
            break;
        }
    }
}

// Help command
void show_help() {
    printf("\n" ANSI_BOLD "Available Commands:\n" ANSI_RESET);
    
    printf(ANSI_BOLD "Applications:" ANSI_RESET "\n");
    printf("  neofetch         - Show system info with ASCII art\n");
    printf("  timer            - Stopwatch and countdown timer\n");
    printf("  todo             - Manage your todo list (2 tasks)\n\n");
    
    printf(ANSI_BOLD "System:" ANSI_RESET "\n");
    printf("  help             - Show this help message\n");
    printf("  sysinfo          - Display system information\n");
    printf("  clear            - Clear the screen\n");
    printf("  reboot           - Reboot the system\n");
    printf("  ps               - List running processes\n");
    printf("  stop <process>   - Stop a process\n\n");
    
    printf(ANSI_BOLD "Files:" ANSI_RESET "\n");
    printf("  ls               - List files\n");
    printf("  cat <file>       - View file contents\n");
    printf("  nano <file>      - Edit file\n");
    printf("  make <file>      - Create new file\n");
    printf("  delete <file>    - Delete file\n");
    printf("  showspace        - Show storage information\n\n");
    
    printf(ANSI_BOLD "Network:" ANSI_RESET "\n");
    printf("  ipa              - Show IP address\n");
    printf("  ping <host>      - Ping a host\n");
    printf("  wifi             - Connect to WiFi\n\n");
    
    printf(ANSI_BOLD "System Info:" ANSI_RESET "\n");
    printf("  time             - Show current time\n");
    printf("  viewlog          - View system log\n");
    printf("  showram          - Show RAM usage\n\n");
    
    printf(ANSI_BOLD "Configuration:" ANSI_RESET "\n");
    printf("  setting          - Open settings menu\n\n");
}

// Command parser
void execute_command(char* cmd) {
    // Trim whitespace
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;
    
    char* args[MAX_ARGS];
    int argc = 0;
    
    // Split command into arguments
    char* token = strtok(cmd, " ");
    while (token && argc < MAX_ARGS) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    
    if (argc == 0) return;
    
    // Execute commands
    if (strcmp(args[0], "help") == 0) {
        show_help();
    } else if (strcmp(args[0], "neofetch") == 0) {
        neofetch();
        read_line("\nPress Enter to continue...", true);
    } else if (strcmp(args[0], "timer") == 0) {
        timer_app();
    } else if (strcmp(args[0], "todo") == 0) {
        todo_app();
    } else if (strcmp(args[0], "sysinfo") == 0) {
        show_system_info();
    } else if (strcmp(args[0], "clear") == 0) {
        printf(ANSI_CLEAR_SCREEN);
    } else if (strcmp(args[0], "reboot") == 0) {
        printf("Rebooting...\n");
        sleep_ms(1000);
        watchdog_enable(1, 1);
        while (1);
    } else if (strcmp(args[0], "ps") == 0) {
        list_processes();
    } else if (strcmp(args[0], "stop") == 0) {
        if (argc < 2) {
            printf("Usage: stop <process_name>\n");
        } else {
            stop_process(args[1]);
        }
    } else if (strcmp(args[0], "ls") == 0) {
        list_files();
    } else if (strcmp(args[0], "cat") == 0) {
        if (argc < 2) {
            printf("Usage: cat <filename>\n");
        } else {
            view_file(args[1]);
        }
    } else if (strcmp(args[0], "nano") == 0) {
        if (argc < 2) {
            printf("Usage: nano <filename>\n");
        } else {
            nano_editor(args[1]);
        }
    } else if (strcmp(args[0], "make") == 0) {
        if (argc < 2) {
            printf("Usage: make <filename>\n");
        } else {
            nano_editor(args[1]);
        }
    } else if (strcmp(args[0], "delete") == 0) {
        if (argc < 2) {
            printf("Usage: delete <filename>\n");
        } else {
            delete_file(args[1]);
        }
    } else if (strcmp(args[0], "showspace") == 0) {
        show_storage_info();
    } else if (strcmp(args[0], "ipa") == 0) {
        show_ip();
    } else if (strcmp(args[0], "ping") == 0) {
        if (argc < 2) {
            printf("Usage: ping <host>\n");
        } else {
            ping_test(args[1]);
        }
    } else if (strcmp(args[0], "wifi") == 0) {
        connect_wifi();
    } else if (strcmp(args[0], "time") == 0) {
        show_time();
    } else if (strcmp(args[0], "viewlog") == 0) {
        view_log();
    } else if (strcmp(args[0], "showram") == 0) {
        show_ram();
    } else if (strcmp(args[0], "setting") == 0) {
        settings_menu();
    } else {
        printf(ANSI_RED "Unknown command: %s" ANSI_RESET "\n", args[0]);
        printf("Type 'help' for available commands\n");
    }
}

// Print shell prompt
void print_prompt() {
    time_t now = get_current_time();
    if (now == 0) {
        // No time set, show uptime
        uint32_t uptime_sec = to_ms_since_boot(get_absolute_time()) / 1000;
        printf(ANSI_GREEN "+%05lus" ANSI_RESET " " 
               ANSI_BOLD ANSI_BLUE "pico@os" ANSI_RESET 
               ":" ANSI_CYAN "~" ANSI_RESET "$ ", (unsigned long)uptime_sec);
    } else {
        struct tm *t = localtime(&now);
        printf(ANSI_GREEN "%02d:%02d:%02d" ANSI_RESET " " 
               ANSI_BOLD ANSI_BLUE "pico@os" ANSI_RESET 
               ":" ANSI_CYAN "~" ANSI_RESET "$ ", 
               t->tm_hour, t->tm_min, t->tm_sec);
    }
    fflush(stdout);
}

// Main shell loop
void shell_loop() {
    print_prompt();
    
    while (true) {
        int c = getchar_timeout_us(0);
        
        if (c == PICO_ERROR_TIMEOUT) {
            sleep_ms(10);
            continue;
        }
        
        if (c == '\r' || c == '\n') {
            command_buffer[cmd_index] = '\0';
            printf("\r\n");
            
            if (cmd_index > 0) {
                execute_command(command_buffer);
            }
            
            cmd_index = 0;
            command_buffer[0] = '\0';
            print_prompt();
        } else if (c == 127 || c == 8) { // Backspace
            if (cmd_index > 0) {
                cmd_index--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (c >= 32 && c < 127) {
            if (cmd_index < MAX_COMMAND_LEN - 1) {
                command_buffer[cmd_index++] = c;
                putchar(c);
                fflush(stdout);
            }
        }
    }
}

// Background NTP sync task
void ntp_sync_task() {
    while (true) {
        if (wifi_connected && ntp_synced) {
            sleep_ms(3600000); // Sync every hour
            sync_ntp_time();
        } else {
            sleep_ms(5000);
        }
    }
}

// Boot sequence
void boot_sequence() {
    printf("\r\n\r\n");
    printf("╔═══════════════════════════════════════════════╗\r\n");
    printf("║     Raspberry Pi Pico 2 W Operating System   ║\r\n");
    printf("║                  Version 1.0                  ║\r\n");
    printf("╚═══════════════════════════════════════════════╝\r\n");
    printf("\r\n");
    
    printf("Booting...\r\n\r\n");
    
    log_message("System booting");
    
    printf("[OK] Initializing hardware\r\n");
    
    printf("[..] Mounting filesystem\r\n");
    init_filesystem();
    printf("[OK] Filesystem ready\r\n");
    
    printf("[..] Starting WiFi driver\r\n");
    int wifi_init = cyw43_arch_init();
    if (wifi_init != 0) {
        printf("[WARN] WiFi driver init failed (code %d)\r\n", wifi_init);
        printf("[WARN] WiFi features will be unavailable\r\n");
        log_message("WARNING: WiFi init failed");
    } else {
        cyw43_arch_enable_sta_mode();
        printf("[OK] WiFi driver ready\r\n");
    }
    
    printf("[OK] Initializing system clock\r\n");
    // Initialize time tracking
    time_sync_base = get_absolute_time();
    system_time_offset = 0; // Will be set by NTP
    
    // Load WiFi config
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, "wifi.cfg", LFS_O_RDONLY) >= 0) {
        char buf[128];
        int len = lfs_file_read(&lfs, &file, buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            char* newline = strchr(buf, '\n');
            if (newline) {
                *newline = '\0';
                size_t ssid_len = strlen(buf);
                if (ssid_len < sizeof(wifi_ssid)) {
                    memcpy(wifi_ssid, buf, ssid_len);
                    wifi_ssid[ssid_len] = '\0';
                }
                size_t pass_len = strlen(newline + 1);
                if (pass_len < sizeof(wifi_password)) {
                    memcpy(wifi_password, newline + 1, pass_len);
                    wifi_password[pass_len] = '\0';
                }
                printf("[OK] WiFi credentials loaded\r\n");
            }
        }
        lfs_file_close(&lfs, &file);
    }
    
    printf("\r\nBoot complete!\r\n");
    printf("Type 'help' for available commands\r\n");
    printf("Type 'neofetch' for a cool system overview\r\n\r\n");
    
    log_message("Boot complete");
    boot_time = get_absolute_time();
}

// Main function
int main() {
    // Initialize all stdio types
    stdio_init_all();
    
    // Critical: Wait for USB to enumerate
    // The Pico needs time to set up USB CDC
    busy_wait_ms(2000);
    
    // Send test pattern to verify USB is working
    for (int i = 0; i < 10; i++) {
        printf("\r\n");
        busy_wait_ms(50);
    }
    
    printf("╔════════════════════════════════╗\r\n");
    printf("║   USB SERIAL ACTIVE - TEST OK  ║\r\n");
    printf("╚════════════════════════════════╝\r\n");
    printf("\r\n");
    busy_wait_ms(500);
    
    printf("Pico OS initializing...\r\n");
    printf("If you see this, USB serial is working!\r\n\r\n");
    busy_wait_ms(500);
    
    // Boot sequence with error checking
    boot_sequence();
    
    printf("Starting background tasks...\r\n");
    
    // Start background tasks
    int ntp_pid = add_process("ntp_sync", ntp_sync_task);
    if (ntp_pid < 0) {
        printf("WARNING: Failed to start NTP sync task\r\n");
    }
    
    printf("Entering shell...\r\n\r\n");
    busy_wait_ms(300);
    
    // Enter shell loop
    shell_loop();
    
    // Should never reach here
    panic_handler("Shell loop exited unexpectedly");
    
    return 0;
}
