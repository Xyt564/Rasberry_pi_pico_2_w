/**
 * ASCII Clock for Raspberry Pi Pico 2 W
 * USB or UART serial output
 * NTP time sync with LED feedback
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#ifdef USE_UART
#include "hardware/uart.h"
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#endif

// ================= WIFI CONFIG =================
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASS"
// ==============================================

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_DELTA 2208988800
#define UK_TIMEZONE_OFFSET 0

// ===== TERMINAL LAYOUT =====
#define TERMINAL_WIDTH 80
#define CLOCK_WIDTH 43
#define CLOCK_PADDING ((TERMINAL_WIDTH - CLOCK_WIDTH) / 2)

// ================= TIME =================
typedef struct {
    int year, month, day, dotw, hour, min, sec;
} datetime_t;

datetime_t current_time;
bool time_synced = false;

// ================= ASCII DIGITS =================
const char* DIGITS[10][5] = {
    {" ### ", "#   #", "#   #", "#   #", " ### "},
    {"  #  ", " ##  ", "  #  ", "  #  ", "#####"},
    {" ### ", "#   #", "  ## ", " #   ", "#####"},
    {" ### ", "#   #", "  ## ", "#   #", " ### "},
    {"#   #", "#   #", "#####", "    #", "    #"},
    {"#####", "#    ", "#### ", "    #", "#### "},
    {" ### ", "#    ", "#### ", "#   #", " ### "},
    {"#####", "    #", "   # ", "  #  ", " #   "},
    {" ### ", "#   #", " ### ", "#   #", " ### "},
    {" ### ", "#   #", " ####", "    #", " ### "}
};

const char* COLON[5] = {"  ", " #", "  ", " #", "  "};

// ================= NTP =================
typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum, poll, precision;
    uint32_t root_delay, root_dispersion, ref_id;
    uint32_t ref_ts[2], org_ts[2], rx_ts[2], tx_ts[2];
} ntp_packet_t;

static struct {
    ip_addr_t addr;
    struct udp_pcb *pcb;
} ntp_state;

// ================= HELPERS =================
void pad() {
    for (int i = 0; i < CLOCK_PADDING; i++) putchar(' ');
}

void wifi_blink(int times, int on_ms, int off_ms) {
    for (int i = 0; i < times; i++) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(on_ms);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(off_ms);
    }
}

bool is_leap_year(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int days_in_month(int m, int y) {
    static const int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    return (m == 2 && is_leap_year(y)) ? 29 : d[m - 1];
}

void tick_time() {
    if (++current_time.sec < 60) return;
    current_time.sec = 0;
    if (++current_time.min < 60) return;
    current_time.min = 0;
    if (++current_time.hour < 24) return;
    current_time.hour = 0;
    current_time.day++;
    current_time.dotw = (current_time.dotw + 1) % 7;
    if (current_time.day > days_in_month(current_time.month, current_time.year)) {
        current_time.day = 1;
        if (++current_time.month > 12) {
            current_time.month = 1;
            current_time.year++;
        }
    }
}

void init_time(int y,int m,int d,int w,int h,int mi,int s) {
    current_time = {y,m,d,w,h,mi,s};
}

void timestamp_to_datetime(time_t t) {
    struct tm *tm = gmtime(&t);
    current_time.year = tm->tm_year + 1900;
    current_time.month = tm->tm_mon + 1;
    current_time.day = tm->tm_mday;
    current_time.dotw = tm->tm_wday;
    current_time.hour = tm->tm_hour;
    current_time.min = tm->tm_min;
    current_time.sec = tm->tm_sec;
}

// ================= NTP CALLBACKS =================
static void ntp_recv(void*, udp_pcb*, pbuf *p, const ip_addr_t*, u16_t) {
    if (p->tot_len == sizeof(ntp_packet_t)) {
        auto *pkt = (ntp_packet_t*)p->payload;
        uint32_t sec1900 = ntohl(pkt->tx_ts[0]);
        time_t unix_time = sec1900 - NTP_DELTA + UK_TIMEZONE_OFFSET;
        timestamp_to_datetime(unix_time);
        time_synced = true;
    }
    pbuf_free(p);
}

static void dns_found(const char*, const ip_addr_t *ip, void*) {
    if (ip) ntp_state.addr = *ip;
}

bool ntp_init() {
    ntp_state.pcb = udp_new();
    if (!ntp_state.pcb) return false;
    udp_recv(ntp_state.pcb, ntp_recv, nullptr);
    dns_gethostbyname(NTP_SERVER, &ntp_state.addr, dns_found, nullptr);
    return true;
}

void ntp_request() {
    pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ntp_packet_t), PBUF_RAM);
    auto *pkt = (ntp_packet_t*)p->payload;
    memset(pkt, 0, sizeof(*pkt));
    pkt->li_vn_mode = 0x1b;
    udp_sendto(ntp_state.pcb, p, &ntp_state.addr, NTP_PORT);
    pbuf_free(p);
}

// ================= DISPLAY =================
void clear_screen() { printf("\033[2J\033[H"); }

void display_clock() {
    printf("\033[H\n");

    int h1=current_time.hour/10, h2=current_time.hour%10;
    int m1=current_time.min/10,  m2=current_time.min%10;
    int s1=current_time.sec/10,  s2=current_time.sec%10;

    pad(); printf("Pico 2 W ASCII Clock\n\n");

    for (int r=0;r<5;r++) {
        pad();
        printf("%s %s %s %s %s %s %s %s\n",
            DIGITS[h1][r], DIGITS[h2][r], COLON[r],
            DIGITS[m1][r], DIGITS[m2][r], COLON[r],
            DIGITS[s1][r], DIGITS[s2][r]);
    }

    pad();
    printf("\n%04d-%02d-%02d  (Day %d)\n",
        current_time.year,
        current_time.month,
        current_time.day,
        current_time.dotw);

    pad();
    printf(time_synced ? "Time source: NTP\n" : "Time source: MANUAL\n");
}

// ================= MAIN =================
int main() {
#ifdef USE_UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    stdio_uart_init_full(UART_ID, BAUD_RATE, UART_TX_PIN, UART_RX_PIN);
#else
    stdio_init_all();
    sleep_ms(2000);
#endif

    int wifi_result = 0;

    if (cyw43_arch_init()) goto manual_time;
    cyw43_arch_enable_sta_mode();

    wifi_blink(2, 300, 300); // connecting indicator

    wifi_result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK, 30000
    );

    if (wifi_result != 0) {
        wifi_blink(3, 500, 300); // failure
        goto manual_time;
    }

    wifi_blink(5, 80, 80); // success

    if (!ntp_init()) goto manual_time;
    sleep_ms(2000);
    ntp_request();

    for (int i=0;i<50 && !time_synced;i++) sleep_ms(100);
    if (!time_synced) goto manual_time;

    goto start_clock;

manual_time:
    init_time(2026,2,1,6,10,48,0);

start_clock:
    clear_screen();
    absolute_time_t last = get_absolute_time();

    while (true) {
        display_clock();
        sleep_until(delayed_by_ms(last, 1000));
        last = get_absolute_time();
        tick_time();
    }
}
