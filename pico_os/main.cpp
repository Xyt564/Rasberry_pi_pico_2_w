/**
 * Pico 2 W Unified System - Optimized Version
 * Web-based terminal with integrated applications
 * Features: TODO app, LED blink, NTP clock
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"

// ============== CONFIGURATION ==============
const char WIFI_SSID[] = "YOUR_SSID";
const char WIFI_PASSWORD[] = "YOUR_PASS";
#define TCP_PORT 80
#define OUTPUT_BUFFER_SIZE 16384
#define CMD_BUFFER_SIZE 512

// ============== FORWARD DECLARATIONS ==============
void process_command(const char* cmd);
void output_write(const char* str);
void output_printf(const char* fmt, ...);

// ============== SYSTEM STATE ==============
static struct {
    char ip_addr[16];
    absolute_time_t boot_time;
} sys_state;

// ============== OUTPUT BUFFER (Ring Buffer) ==============
static struct {
    char buffer[OUTPUT_BUFFER_SIZE];
    uint16_t write_pos;
    uint16_t read_pos;
    bool overflow;
} output_buf;

void output_clear() {
    output_buf.write_pos = 0;
    output_buf.read_pos = 0;
    output_buf.overflow = false;
}

void output_write(const char* str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        output_buf.buffer[output_buf.write_pos] = str[i];
        output_buf.write_pos = (output_buf.write_pos + 1) % OUTPUT_BUFFER_SIZE;
        
        if (output_buf.write_pos == output_buf.read_pos) {
            output_buf.overflow = true;
            output_buf.read_pos = (output_buf.read_pos + 1) % OUTPUT_BUFFER_SIZE;
        }
    }
}

void output_printf(const char* fmt, ...) {
    char temp[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);
    output_write(temp);
}

size_t output_read_all(char* dest, size_t max_len) {
    size_t count = 0;
    while (output_buf.read_pos != output_buf.write_pos && count < max_len - 1) {
        dest[count++] = output_buf.buffer[output_buf.read_pos];
        output_buf.read_pos = (output_buf.read_pos + 1) % OUTPUT_BUFFER_SIZE;
    }
    dest[count] = '\0';
    return count;
}

// ============== TO-DO APP ==============
static struct {
    char task1[15];
    char task2[15];
    bool done1;
    bool done2;
    int count;
    bool running;
} todo_state = {"", "", false, false, 0, false};

void todo_show_commands() {
    output_write("\nAvailable commands:\n");
    output_write("  list       - Show all tasks\n");
    output_write("  add <task> - Add a new task\n");
    output_write("  done <n>   - Mark task as complete\n");
    output_write("  del <n>    - Delete a task\n");
    output_write("  stop       - Exit TODO app\n\n");
}

void todo_init() {
    todo_state.running = true;
    output_write("\n=== TO-DO APP STARTED ===\n");
    todo_show_commands();
}

void todo_list() {
    output_write("\n=== TO-DO LIST ===\n");
    if (todo_state.count >= 1) 
        output_printf("1. [%c] %s\n", todo_state.done1 ? 'X' : ' ', todo_state.task1);
    if (todo_state.count >= 2) 
        output_printf("2. [%c] %s\n", todo_state.done2 ? 'X' : ' ', todo_state.task2);
    if (todo_state.count == 0) 
        output_write("No tasks.\n");
    todo_show_commands();
}

void todo_add(const char* task) {
    if (todo_state.count == 0) {
        strncpy(todo_state.task1, task, 14);
        todo_state.task1[14] = '\0';
        todo_state.done1 = false;
        todo_state.count = 1;
        output_write("Task 1 added.\n");
    } else if (todo_state.count == 1) {
        strncpy(todo_state.task2, task, 14);
        todo_state.task2[14] = '\0';
        todo_state.done2 = false;
        todo_state.count = 2;
        output_write("Task 2 added.\n");
    } else {
        output_write("List full (max 2 tasks).\n");
    }
    todo_show_commands();
}

void todo_done(int n) {
    if (n == 1 && todo_state.count >= 1) {
        todo_state.done1 = true;
        output_write("Task 1 marked done.\n");
    } else if (n == 2 && todo_state.count >= 2) {
        todo_state.done2 = true;
        output_write("Task 2 marked done.\n");
    } else {
        output_write("Invalid task number.\n");
    }
    todo_show_commands();
}

void todo_del(int n) {
    if (n == 1 && todo_state.count >= 1) {
        if (todo_state.count == 2) {
            strcpy(todo_state.task1, todo_state.task2);
            todo_state.done1 = todo_state.done2;
        }
        todo_state.count--;
        output_write("Task 1 deleted.\n");
    } else if (n == 2 && todo_state.count == 2) {
        todo_state.count--;
        output_write("Task 2 deleted.\n");
    } else {
        output_write("Invalid task number.\n");
    }
    todo_show_commands();
}

void todo_stop() {
    todo_state.running = false;
    output_write("TO-DO app stopped.\n");
}

// ============== BLINK APP ==============
static struct {
    bool running;
    absolute_time_t last_toggle;
    bool led_state;
    uint32_t interval_ms;
} blink_state = {false, 0, false, 500};

void blink_show_commands() {
    output_write("\nAvailable commands:\n");
    output_write("  speed <ms> - Change blink interval (50-5000ms)\n");
    output_write("  stop       - Exit blink app\n\n");
}

void blink_init() {
    blink_state.running = true;
    blink_state.last_toggle = get_absolute_time();
    blink_state.led_state = false;
    blink_state.interval_ms = 500;
    output_write("\n=== LED BLINK APP STARTED ===\n");
    output_printf("LED blinking at %dms interval.\n", blink_state.interval_ms);
    blink_show_commands();
}

void blink_set_speed(uint32_t ms) {
    if (ms < 50 || ms > 5000) {
        output_write("Speed must be between 50-5000ms.\n");
        blink_show_commands();
        return;
    }
    blink_state.interval_ms = ms;
    output_printf("Blink interval set to %dms.\n", ms);
    blink_show_commands();
}

void blink_stop() {
    blink_state.running = false;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    output_write("LED blink stopped.\n");
}

void blink_tick() {
    if (!blink_state.running) return;
    
    int64_t elapsed = absolute_time_diff_us(blink_state.last_toggle, get_absolute_time());
    if (elapsed > (blink_state.interval_ms * 1000)) {
        blink_state.led_state = !blink_state.led_state;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blink_state.led_state);
        blink_state.last_toggle = get_absolute_time();
    }
}

// ============== NTP TIME & CLOCK APP ==============

// NTP packet structure (same as ascii_clock)
typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum, poll, precision;
    uint32_t root_delay, root_dispersion, ref_id;
    uint32_t ref_ts[2], org_ts[2], rx_ts[2], tx_ts[2];
} ntp_packet_t;

static struct {
    bool synced;
    bool initialized;
    bool request_sent;
    ip_addr_t server_addr;
    struct udp_pcb *pcb;
    time_t base_time;           // Unix timestamp from NTP
    absolute_time_t base_tick;  // Pico monotonic clock at the moment NTP arrived
} ntp_state = {false, false, false, {0}, NULL, 0, 0};

// Returns current Unix time using the NTP baseline + elapsed monotonic time
static time_t get_current_time(void) {
    if (!ntp_state.synced) return 0;
    int64_t elapsed = absolute_time_diff_us(ntp_state.base_tick, get_absolute_time()) / 1000000;
    return ntp_state.base_time + (time_t)elapsed;
}

// UDP receive callback — fires when the NTP reply comes back
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    (void)arg; (void)pcb; (void)addr; (void)port;
    if (p->tot_len == sizeof(ntp_packet_t)) {
        ntp_packet_t *pkt = (ntp_packet_t*)p->payload;
        uint32_t sec1900 = ntohl(pkt->tx_ts[0]);
        ntp_state.base_time = (time_t)(sec1900 - 2208988800U);
        ntp_state.base_tick = get_absolute_time();
        ntp_state.synced = true;
    }
    pbuf_free(p);
}

// DNS callback — stores the resolved IP once it comes back
static void ntp_dns_found(const char *name, const ip_addr_t *ip, void *arg) {
    (void)name; (void)arg;
    if (ip) ntp_state.server_addr = *ip;
}

// Send the actual NTP request packet
static void ntp_send_request(void) {
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ntp_packet_t), PBUF_RAM);
    if (!p) return;
    ntp_packet_t *pkt = (ntp_packet_t*)p->payload;
    memset(pkt, 0, sizeof(*pkt));
    pkt->li_vn_mode = 0x1b;  // LI=0, VN=3, Mode=3 (client)
    udp_sendto(ntp_state.pcb, p, &ntp_state.server_addr, 123);
    pbuf_free(p);
    ntp_state.request_sent = true;
}

void ntp_init() {
    if (ntp_state.initialized) return;

    ntp_state.pcb = udp_new();
    if (!ntp_state.pcb) return;
    udp_recv(ntp_state.pcb, ntp_recv, NULL);

    // Kick off DNS resolution for pool.ntp.org; once it resolves
    // ntp_dns_found stores the IP and we send the request on the next tick
    dns_gethostbyname("pool.ntp.org", &ntp_state.server_addr, ntp_dns_found, NULL);

    ntp_state.initialized = true;
}

// Called from the main loop — handles the async DNS -> send -> (recv fires via callback) flow
void ntp_tick() {
    if (!ntp_state.initialized || ntp_state.synced || ntp_state.request_sent) return;

    // DNS resolved? (ip will be non-zero once ntp_dns_found fires)
    if (ip_addr_isany(&ntp_state.server_addr)) return;

    // Got an IP, send the request
    ntp_send_request();
}

void ntp_check_sync() {
    static bool notified = false;
    if (ntp_state.synced && !notified) {
        notified = true;
        output_write("NTP time synced successfully!\n\n");
    }
}

static struct {
    bool running;
} clock_state = {false};

void clock_show_commands() {
    output_write("\nAvailable commands:\n");
    output_write("  show - Display current time\n");
    output_write("  stop - Exit clock app\n\n");
}

void clock_init() {
    clock_state.running = true;
    output_write("\n=== CLOCK APP STARTED ===\n");
    
    if (!ntp_state.initialized) {
        output_write("Initializing NTP time sync...\n");
        ntp_init();
        output_write("(Time sync may take 5-10 seconds)\n");
    }
    
    clock_show_commands();
}

void clock_display() {
    ntp_check_sync();
    
    if (!ntp_state.synced) {
        output_write("\nWaiting for NTP time sync...\n");
        output_write("Please wait a few seconds and try again.\n");
        clock_show_commands();
        return;
    }
    
    time_t now = get_current_time();
    struct tm *timeinfo = localtime(&now);
    
    output_write("\n========================================\n");
    output_write("       CURRENT TIME            \n");
    output_write("========================================\n");
    output_printf("  Time: %02d:%02d:%02d              \n", 
                  timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    output_printf("  Date: %04d-%02d-%02d          \n",
                  timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    output_printf("  Day:  %s                  \n", days[timeinfo->tm_wday]);
    output_write("========================================\n");
    
    clock_show_commands();
}

void clock_stop() {
    clock_state.running = false;
    output_write("Clock stopped.\n");
}

void clock_tick() {
    if (!clock_state.running) return;
    ntp_check_sync();
}

// ============== COMMAND PROCESSOR ==============
enum AppType { APP_NONE, APP_TODO, APP_BLINK, APP_CLOCK };
static enum AppType current_app = APP_NONE;

void process_command(const char* cmd) {
    char command[64] = {0};
    char arg1[128] = {0};
    
    sscanf(cmd, "%63s %127[^\n]", command, arg1);
    
    for (char* p = command; *p; p++) *p = tolower(*p);
    
    // ========== HELP COMMAND ==========
    if (strcmp(command, "help") == 0) {
        output_write("\n=== AVAILABLE COMMANDS ===\n");
        output_write("help       - Show this help\n");
        output_write("clear      - Clear terminal\n");
        output_write("status     - System status\n");
        output_write("apps       - List applications\n");
        output_write("run <app>  - Start application\n");
        output_write("stop       - Stop current app\n");
        output_write("current    - Show running app\n");
        output_write("reboot     - Restart system\n\n");
        return;
    }
    
    // ========== CLEAR COMMAND ==========
    if (strcmp(command, "clear") == 0) {
        output_clear();
        output_write("Terminal cleared.\n\n");
        return;
    }
    
    // ========== STATUS COMMAND ==========
    if (strcmp(command, "status") == 0) {
        int64_t uptime = absolute_time_diff_us(sys_state.boot_time, get_absolute_time()) / 1000000;
        output_write("\n=== SYSTEM STATUS ===\n");
        output_printf("IP Address: %s\n", sys_state.ip_addr);
        output_printf("Uptime: %lld seconds\n", uptime);
        output_printf("NTP Synced: %s\n", ntp_state.synced ? "Yes" : "No");
        output_write("Status: Running\n\n");
        return;
    }
    
    // ========== APPS COMMAND ==========
    if (strcmp(command, "apps") == 0) {
        output_write("\n=== AVAILABLE APPLICATIONS ===\n");
        output_write("1. todo  - Task manager (max 2 tasks)\n");
        output_write("2. blink - Control LED blinking\n");
        output_write("3. clock - Real-time clock (NTP synced)\n");
        output_write("\nUse 'run <app>' to start an application.\n\n");
        return;
    }
    
    // ========== CURRENT APP COMMAND ==========
    if (strcmp(command, "current") == 0) {
        if (current_app == APP_NONE || 
            (current_app == APP_TODO && !todo_state.running) ||
            (current_app == APP_BLINK && !blink_state.running) ||
            (current_app == APP_CLOCK && !clock_state.running)) {
            output_write("No application currently running.\n");
        } else {
            const char* app_name = "";
            if (current_app == APP_TODO) app_name = "todo";
            else if (current_app == APP_BLINK) app_name = "blink";
            else if (current_app == APP_CLOCK) app_name = "clock";
            output_printf("Current application: %s\n", app_name);
        }
        return;
    }
    
    // ========== RUN COMMAND ==========
    if (strcmp(command, "run") == 0) {
        if (strlen(arg1) == 0) {
            output_write("Usage: run <app>\n");
            output_write("Available apps: todo, blink, clock\n\n");
            return;
        }
        
        // Stop current app first
        if (current_app == APP_TODO && todo_state.running) todo_stop();
        if (current_app == APP_BLINK && blink_state.running) blink_stop();
        if (current_app == APP_CLOCK && clock_state.running) clock_stop();
        
        // Convert app name to lowercase
        for (char* p = arg1; *p; p++) *p = tolower(*p);
        
        if (strcmp(arg1, "todo") == 0) {
            current_app = APP_TODO;
            todo_init();
        } else if (strcmp(arg1, "blink") == 0) {
            current_app = APP_BLINK;
            blink_init();
        } else if (strcmp(arg1, "clock") == 0) {
            current_app = APP_CLOCK;
            clock_init();
        } else {
            output_printf("Unknown app: %s\n", arg1);
            output_write("Use 'apps' to see available applications.\n\n");
        }
        return;
    }
    
    // ========== STOP COMMAND ==========
    if (strcmp(command, "stop") == 0) {
        bool stopped = false;
        if (current_app == APP_TODO && todo_state.running) {
            todo_stop();
            stopped = true;
        }
        if (current_app == APP_BLINK && blink_state.running) {
            blink_stop();
            stopped = true;
        }
        if (current_app == APP_CLOCK && clock_state.running) {
            clock_stop();
            stopped = true;
        }
        
        if (!stopped) {
            output_write("No application running.\n");
        }
        current_app = APP_NONE;
        return;
    }
    
    // ========== REBOOT COMMAND ==========
    if (strcmp(command, "reboot") == 0) {
        output_write("Rebooting system...\n");
        sleep_ms(500);
        watchdog_enable(1, 1);
        while(1);
    }
    
    // ========== TODO APP COMMANDS ==========
    if (current_app == APP_TODO && todo_state.running) {
        if (strcmp(command, "list") == 0) {
            todo_list();
            return;
        }
        
        if (strcmp(command, "add") == 0) {
            if (strlen(arg1) == 0) {
                output_write("Usage: add <task_name>\n");
                todo_show_commands();
                return;
            }
            todo_add(arg1);
            return;
        }
        
        if (strcmp(command, "done") == 0) {
            int n = atoi(arg1);
            todo_done(n);
            return;
        }
        
        if (strcmp(command, "del") == 0) {
            int n = atoi(arg1);
            todo_del(n);
            return;
        }
    }
    
    // ========== BLINK APP COMMANDS ==========
    if (current_app == APP_BLINK && blink_state.running) {
        if (strcmp(command, "speed") == 0) {
            uint32_t ms = atoi(arg1);
            blink_set_speed(ms);
            return;
        }
    }
    
    // ========== CLOCK APP COMMANDS ==========
    if (current_app == APP_CLOCK && clock_state.running) {
        if (strcmp(command, "show") == 0) {
            clock_display();
            return;
        }
    }
    
    // Unknown command
    output_printf("Unknown command: %s\n", command);
    output_write("Type 'help' for available commands.\n");
}

// ============== WEB SERVER ==============
typedef struct {
    struct tcp_pcb *server_pcb;
} TCP_SERVER_T;

static TCP_SERVER_T *tcp_server_state = NULL;

// Compact terminal HTML
const char *terminal_html = 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Pico Terminal</title><style>"
    "body{margin:0;padding:20px;background:#0a0a0a;color:#0f0;font-family:monospace}"
    "#output{background:#000;border:2px solid #0f0;padding:15px;height:400px;overflow-y:auto;white-space:pre-wrap;margin-bottom:10px}"
    "#cmd{background:#000;color:#0f0;border:2px solid #0f0;padding:10px;width:calc(100% - 24px);font-family:monospace;font-size:14px}"
    "button{background:#0f0;color:#000;border:none;padding:10px 20px;margin:5px;cursor:pointer;font-family:monospace}"
    "button:hover{background:#0a0}"
    "h1{color:#0f0;text-align:center;text-shadow:0 0 10px #0f0}"
    ".status{color:#0f0;text-align:center;margin:10px 0}"
    "</style></head><body>"
    "<h1>PICO 2 W TERMINAL</h1>"
    "<div class='status'>Connected to: __IP__</div>"
    "<div id='output'></div>"
    "<input type='text' id='cmd' placeholder='Enter command (type help)...' autofocus>"
    "<div style='text-align:center'>"
    "<button onclick='sendCmd()'>Execute</button>"
    "<button onclick='sendCmd(\"help\")'>Help</button>"
    "<button onclick='sendCmd(\"status\")'>Status</button>"
    "<button onclick='sendCmd(\"clear\")'>Clear</button>"
    "</div>"
    "<script>"
    "let cmdInput=document.getElementById('cmd');"
    "let output=document.getElementById('output');"
    "function sendCmd(cmd){fetch('/cmd',{method:'POST',body:cmd||cmdInput.value}).then(r=>r.text()).then(t=>{output.textContent=t;output.scrollTop=output.scrollHeight});if(!cmd)cmdInput.value='';}"
    "cmdInput.addEventListener('keypress',e=>{if(e.key==='Enter')sendCmd()});"
    "setInterval(()=>fetch('/output').then(r=>r.text()).then(t=>{if(t)output.textContent=t;output.scrollTop=output.scrollHeight}),1000);"
    "</script></body></html>";

static err_t tcp_close_client_connection(struct tcp_pcb *client_pcb) {
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
    return tcp_close_client_connection(pcb);
}

static void send_http_response(struct tcp_pcb *pcb, const char* content, const char* content_type) {
    size_t content_len = strlen(content);
    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n\r\n",
        content_len, content_type);
    
    tcp_write(pcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    tcp_write(pcb, content, content_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) {
        return tcp_close_client_connection(pcb);
    }

    if (p->tot_len > 0) {
        char *request = (char*)malloc(p->tot_len + 1);
        if (request) {
            pbuf_copy_partial(p, request, p->tot_len, 0);
            request[p->tot_len] = '\0';
            
            // Check request type - ORDER MATTERS!
            if (strncmp(request, "GET /output", 11) == 0) {
                // Return terminal output only
                char output[OUTPUT_BUFFER_SIZE];
                output_read_all(output, sizeof(output));
                send_http_response(pcb, output, "text/plain");
            }
            else if (strncmp(request, "POST /cmd", 9) == 0) {
                // Process command and return output
                char *body = strstr(request, "\r\n\r\n");
                if (body) {
                    body += 4;
                    process_command(body);
                }
                char output[OUTPUT_BUFFER_SIZE];
                output_read_all(output, sizeof(output));
                send_http_response(pcb, output, "text/plain");
            }
            else if (strncmp(request, "GET /", 5) == 0) {
                // Serve terminal HTML page
                char *html = (char*)malloc(strlen(terminal_html) + 20);
                if (html) {
                    strcpy(html, terminal_html);
                    char *ip_pos = strstr(html, "__IP__");
                    if (ip_pos) {
                        char temp[2048];
                        *ip_pos = '\0';
                        snprintf(temp, sizeof(temp), "%s%s%s", html, sys_state.ip_addr, ip_pos + 6);
                        strcpy(html, temp);
                    }
                    send_http_response(pcb, html, "text/html");
                    free(html);
                }
            }
            
            free(request);
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
    
    tcp_arg(client_pcb, arg);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    
    return ERR_OK;
}

static bool tcp_server_open(TCP_SERVER_T *state) {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        tcp_close(pcb);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 2);
    if (!state->server_pcb) {
        if (pcb) tcp_close(pcb);
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

// ============== MAIN ==============
int main() {
    stdio_init_all();
    
    // Initialize system state
    sys_state.boot_time = get_absolute_time();
    output_clear();
    
    // Initialize WiFi
    if (cyw43_arch_init()) {
        printf("Failed to initialize WiFi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    
    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect\n");
        return 1;
    }
    
    // Store IP address
    strncpy(sys_state.ip_addr, ip4addr_ntoa(netif_ip4_addr(netif_list)), 15);
    printf("Connected! IP: %s\n", sys_state.ip_addr);
    
    // Initialize NTP
    ntp_init();
    
    // Startup blink
    for (int i = 0; i < 3; i++) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
    }
    
    // Start web server
    tcp_server_state = (TCP_SERVER_T*)calloc(1, sizeof(TCP_SERVER_T));
    if (!tcp_server_state || !tcp_server_open(tcp_server_state)) {
        printf("Failed to start server\n");
        return 1;
    }
    
    printf("Server running on port %d\n", TCP_PORT);
    printf("Terminal: http://%s/\n", sys_state.ip_addr);
    
    // Welcome message in terminal buffer
    output_write("========================================\n");
    output_write("  PICO 2 W UNIFIED SYSTEM v2.0\n");
    output_write("========================================\n\n");
    output_write("System booted successfully.\n");
    output_printf("IP Address: %s\n", sys_state.ip_addr);
    output_write("NTP time sync in progress...\n\n");
    output_write("Type 'help' to see available commands.\n");
    output_write("Type 'apps' to see available applications.\n\n");
    
    // Main loop
    while (true) {
        // Update background tasks
        blink_tick();
        clock_tick();
        ntp_tick();
        
        #if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        sleep_ms(1);
        #else
        sleep_ms(10);
        #endif
    }

    cyw43_arch_deinit();
    return 0;
}
