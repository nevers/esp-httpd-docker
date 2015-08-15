#include "user_config.h"
#include "user_interface.h"
#include "eagle_soc.h"
#include "osapi.h"
#include "uart_register.h"
#include "espconn.h"

#include <string.h>

void disableDebugMessages();
void nop(char);
void wifi_callback(System_Event_t *event);
void println(uint8_t message[]);
void print(uint8_t* message);
void print_char(uint8_t character);
void initHttpd();
LOCAL void ICACHE_FLASH_ATTR handleIncomingConnection(void* arg);
void http_receive(void* arg);
void http_reconnect(void*, sint8 err);
void http_disconnect(void* arg);

void ICACHE_FLASH_ATTR user_init() {
    disableDebugMessages();
    uart_div_modify(0, UART_CLK_FREQ/115200); // Set the UART baud rate
    print("\033[2J"); // Clear the screen
    println("\n\r--- begin ---");
    println("Initialising wifi");
    char ssid[32] = SSID;
    char pwd[64] = PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

    println("Setting wifi configuration");
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_callback);

    initHttpd();
}

void disableDebugMessages() {
    os_install_putc1(nop);    
}

void nop(char c) {
}

void println(uint8_t message[]) {
    print(message);
    print("\n\r");
}

void print(uint8_t* message) {
    int i;
    for(i = 0; i < strlen(message); i++)
        print_char(message[i]);
}

void print_char(uint8_t character) {
    while (true) {
        uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
            break;
    }
    WRITE_PERI_REG(UART_FIFO(0) , character);
}

void wifi_callback(System_Event_t *evt) {
    println("Got wifi callback");
    switch(evt->event) {
        case EVENT_STAMODE_CONNECTED:
            println("Connected");
            break;

         case EVENT_STAMODE_DISCONNECTED:
            println("Disconnected");
            break;

         case EVENT_STAMODE_GOT_IP:
            ;
            char ip[90];
            os_sprintf(ip, "Got ip: " IPSTR, IP2STR(&evt->event_info.got_ip.ip));
            println(ip);
            break;
    }
}

void initHttpd() {
    println("Initialising httpd");
    LOCAL struct espconn connection;
    println(".");
    LOCAL esp_tcp protocol;
    println(".");
    connection.type = ESPCONN_TCP;
    println(".");
    connection.state = ESPCONN_NONE;
    println(".");
    connection.proto.tcp = &protocol;
    println(".");
    connection.proto.tcp->local_port= 80;
    espconn_regist_connectcb(&connection, handleIncomingConnection);
    espconn_accept(&connection);
    println("Listening ");
}

LOCAL void ICACHE_FLASH_ATTR handleIncomingConnection(void* arg) {
} 
