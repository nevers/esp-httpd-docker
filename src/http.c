#include "http.h"
#include "osapi.h"
#include "mem.h"

ICACHE_FLASH_ATTR static void handleIncomingConnection(void* arg);
ICACHE_FLASH_ATTR static void httpdReceive(void* arg, char* data, unsigned short length);
ICACHE_FLASH_ATTR static void httpdReconnect(void*, sint8 err);
ICACHE_FLASH_ATTR static void httpdDisconnect(void* arg);
void printRemoteIp(struct espconn* connection);
bool parseHttpRequest(const char* data, HttpRequest* request);
char* findLast(const char* string, const char* pattern);
HttpMethod getHttpMethod(const char* data);
void substring(char* string, char* endChar, char* result);

int handlers_size = 0;
int handlers_max_size = HTTP_DEFAULT_REQUEST_HANDLERS;
http_request_handler** handlers = NULL;

void http_init() {
    println("[http] init");
    static struct espconn connection;
    static esp_tcp protocol;
    connection.type = ESPCONN_TCP;
    connection.state = ESPCONN_NONE;
    connection.proto.tcp = &protocol;
    connection.proto.tcp->local_port= 80;
    espconn_regist_connectcb(&connection, handleIncomingConnection);
    espconn_accept(&connection);
    println("[http] listening for GET requests");

    handlers = (http_request_handler**) os_zalloc(sizeof(http_request_handler*) * handlers_max_size);
}

void httpd_stop() {
    os_free(handlers);
}

void http_add_request_handler(http_request_handler* handler) {
    handlers[handlers_size] = handler;
    handlers_size++;
}

static ICACHE_FLASH_ATTR void handleIncomingConnection(void* arg) {
    struct espconn* connection = arg;
    espconn_regist_reconcb(connection, httpdReconnect);
    espconn_regist_disconcb(connection, httpdDisconnect);
    espconn_regist_recvcb(connection, httpdReceive);
}

static ICACHE_FLASH_ATTR void httpdReconnect(void* arg, sint8 err) {
    struct espconn* connection = arg;
    print("[http] ");
    printRemoteIp(connection);
    println(" reconnect");
}

static ICACHE_FLASH_ATTR void httpdDisconnect(void* arg) {
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


static ICACHE_FLASH_ATTR void httpdReceive(void* connection, char* data, unsigned short length) {
    HttpRequest request; 
    if(!parseHttpRequest(data, &request)) {
        println("[http] error parsing request");
        http_send_bad_request(connection);
        return;        
    }

    print("[http] accepted request from: ");
    println(request.host);
    
    int i; 
    for(i = 0; i < handlers_size; i++) 
        (*handlers[i])(&request, connection);
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

void http_send(struct espconn* connection, const char* data) {
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

void http_send_bad_request(struct espconn* connection) {
    char data[256];
    os_memset(data, 0, 256);
    uint16 length = 0;
    os_sprintf(data, "HTTP/1.0 400 BadRequest\r\nContent-Length: 0\r\nServer: esp-sensor\r\n\n");
    length = os_strlen(data);
    espconn_sent(connection, data, length);
}
