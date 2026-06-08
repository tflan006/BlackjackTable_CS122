#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "http_server.h"
#include "hardware/spi.h"
#include "Player.h"
#include "lvgl_BlackjackApp.h"

extern "C" {
    #include "dhcpserver.h"
}

Player players[MAX_PLAYERS];
   
int main() {
    stdio_init_all();

    //while (!stdio_usb_connected()) {
    //    sleep_ms(10);
    //}
    printf("Starting Blackjack Access Point...\n");

    if (cyw43_arch_init()) {
        printf("Error: WiFi init failed!\n");
        return 1;
    }

    cyw43_arch_enable_ap_mode("BlackjackTable", "password123", CYW43_AUTH_WPA2_AES_PSK);
    printf("Access Point active. SSID: BlackjackTable\n");

    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr,  192, 168, 4, 1);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw,      192, 168, 4, 1);
    netif_set_addr(netif_default, &ipaddr, &netmask, &gw);

    static dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gw, &netmask);
    printf("DHCP Server online.\n");

    if (!http_server_init()) {
        printf("Error: HTTP server failed to start!\n");
        return 1;
    }
    printf("HTTP Server online on port 80.\n");
    printf("Open on your phone: http://192.168.4.1\n");

    ucr::bcoe::SPIDisplay spi_display(480, 272, 1000000, 20);
    spi_display.begin();
    spi_display.clear();

    BlackjackApp app(&spi_display);
    app.run();
    
    return 0;
}