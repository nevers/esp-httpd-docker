#include "user_config.h"
#include "user_interface.h"
#include "eagle_soc.h"
#include "osapi.h"
#include "uart_register.h"
#include "espconn.h"
#include "mem.h"

void disableDebugMessages();
void nop(char);
void wifi_callback(System_Event_t *event);
void println(uint8_t message[]);
void print(uint8_t* message);
void print_char(uint8_t character);
void print_int(int number);
void initHttpd();
void printRemoteIp(struct espconn* connection);
LOCAL ICACHE_FLASH_ATTR void handleIncomingConnection(void* arg);
LOCAL ICACHE_FLASH_ATTR void httpdReceive(void* arg, char* data, unsigned short length);
LOCAL ICACHE_FLASH_ATTR void httpdReconnect(void*, sint8 err);
LOCAL ICACHE_FLASH_ATTR void httpdDisconnect(void* arg);

#define HTTP_URL_LENGTH_MAX 1024
#define HTTP_HOST_LENGTH_MAX 16

typedef enum HttpMethod {
    GET, POST, PUT, DELETE, HEAD, TRACE, CONNECT, UNKNOWN
} HttpMethod;

typedef struct HttpRequest {
    enum HttpMethod method;
    char url[HTTP_URL_LENGTH_MAX];
    char host[HTTP_HOST_LENGTH_MAX];
} HttpRequest;

bool parseHttpRequest(const char* data, HttpRequest* request);
char* findLast(const char* string, const char* pattern);
HttpMethod getHttpMethod(const char* data);
void substring(char* string, char* endChar, char* result);
void send_bad_request(struct espconn* connection);

void ICACHE_FLASH_ATTR user_init() {
    disableDebugMessages();
    uart_div_modify(0, UART_CLK_FREQ/115200); // Set the UART baud rate
    print("\033[2J"); // Clear the screen
    println("\n\r--- begin ---");
    println("[wifi] init");
    char ssid[32] = SSID;
    char pwd[64] = PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

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

void print_int(int number) {
    char numberString[8];
    os_sprintf(numberString, "%d", number);
    print(numberString);
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
    switch(evt->event) {
        case EVENT_STAMODE_CONNECTED:
            println("[wifi] connected");
            break;

         case EVENT_STAMODE_DISCONNECTED:
            println("[wifi] disconnected");
            break;

         case EVENT_STAMODE_GOT_IP:
            print("[wifi] got ip: ");
            char ip[16];
            os_sprintf(ip, IPSTR, IP2STR(&evt->event_info.got_ip.ip));
            println(ip);
            break;
    }
}

void initHttpd() {
    println("[http] init");
    LOCAL struct espconn connection;
    LOCAL esp_tcp protocol;
    connection.type = ESPCONN_TCP;
    connection.state = ESPCONN_NONE;
    connection.proto.tcp = &protocol;
    connection.proto.tcp->local_port= 80;
    espconn_regist_connectcb(&connection, handleIncomingConnection);
    espconn_accept(&connection);
    println("[http] listening for GET requests");
}

LOCAL ICACHE_FLASH_ATTR void handleIncomingConnection(void* arg) {
    struct espconn* connection = arg;
    espconn_regist_reconcb(connection, httpdReconnect);
    espconn_regist_disconcb(connection, httpdDisconnect);
    espconn_regist_recvcb(connection, httpdReceive);
}

LOCAL ICACHE_FLASH_ATTR void httpdReconnect(void* arg, sint8 err) {
    struct espconn* connection = arg;
    print("[http] ");
    printRemoteIp(connection);
    println(" reconnect");
}


LOCAL ICACHE_FLASH_ATTR void httpdDisconnect(void* arg) {
    struct espconn* connection = arg;
    print("[http] ");
    printRemoteIp(connection);
    println(" disconnected");
}

void printRemoteIp(struct espconn* connection) {
    print_int(connection->proto.tcp->remote_ip[0]);
    print(".");
    print_int(connection->proto.tcp->remote_ip[1]);
    print(".");
    print_int(connection->proto.tcp->remote_ip[2]);
    print(".");
    print_int(connection->proto.tcp->remote_ip[3]);
}

void send_ok(struct espconn* connection, const char* data) {
    int total_length = 0;
    char header[256]; //FIXME this this should be a constant and we need a check for not exceeding it.
    os_memset(header, 0, 256); 
    char* response = NULL;

    os_sprintf(header, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n", os_strlen(data));
    os_sprintf(header + os_strlen(header), "Content-type: text/plain\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache\r\n\r\n");
    total_length = os_strlen(header) + os_strlen(data);
    response = (char *) os_zalloc(total_length);
    os_memcpy(response, header, os_strlen(header));
    os_memcpy(response + os_strlen(header), data, os_strlen(data));
    espconn_sent(connection, response, total_length);
    os_free(response);
}

LOCAL ICACHE_FLASH_ATTR void httpdReceive(void* connection, char* data, unsigned short length) {
    HttpRequest request; 
    if(!parseHttpRequest(data, &request)) {
        println("[http] error parsing request");
        send_bad_request(connection);
        return;        
    }

    print("[http] accepted request from: ");
    println(request.host);


    uint8_t buffer[16];
    uint16 value = system_adc_read();
    os_sprintf(buffer, "[adcr] %d\n", value);
    send_ok(connection, buffer);
    println("[http] response sent");
}

bool parseHttpRequest(const char* data, HttpRequest* request) {
    char* urlBegin = (char*) findLast(data, " ");
    char* hostBegin = (char*) findLast(data, "Host: ");
    HttpMethod method = getHttpMethod(data);
    if(urlBegin == NULL || hostBegin == NULL || method == UNKNOWN) return false;

    request->method = method;
    substring(urlBegin, " ", request->url);
    substring(hostBegin, "\n", request->host);
    return true;
}

char* findLast(const char* string, const char* pattern) {
    char* first = (char*) os_strstr(string, pattern);
    return (first == NULL) ? NULL : first + strlen(pattern);
}

HttpMethod getHttpMethod(const char* data) {
    HttpMethod method = UNKNOWN;
    if(os_strncmp(data, "GET ", 4) == 0)
        method = GET;
    else if(os_strncmp(data, "POST ", 5) == 0)
        method = POST;
    else if(os_strncmp(data, "PUT ", 4) == 0)
        method = PUT;
    else if(os_strncmp(data, "DELETE ", 7) == 0)
       method = DELETE;
    else if(os_strncmp(data, "HEAD ", 5) == 0)
        method = HEAD;
    else if(os_strncmp(data, "TRACE ", 6) == 0)
        method = TRACE;
    else if(os_strncmp(data, "CONNECT ", 8) == 0)
        method = CONNECT;
    return method;
}

void substring(char* string, char* endChar, char* result) {
   char* end = (char*) os_strstr(string, endChar); 
   int length = end - string; //FIXME length should not exceed the size of url (1024)
   os_memcpy(result, string, length);
   result[length] = '\0';
}

void send_bad_request(struct espconn* connection) {
    char data[256];
    os_memset(data, 0, 256);
    uint16 length = 0;
    os_sprintf(data, "HTTP/1.0 400 BadRequest\r\nContent-Length: 0\r\nServer: esp-sensor\r\n\n");
    length = os_strlen(data);
    espconn_sent(connection, data, length);
}
