#ifndef __HTTP_D_H__
#define __HTTP_D_H__

#define HTTP_URL_LENGTH_MAX 1024
#define HTTP_HOST_LENGTH_MAX 16
#define HTTP_DEFAULT_REQUEST_HANDLERS 16

#include "user_interface.h"
#include "espconn.h"

typedef enum HttpMethod {
    GET, POST, PUT, DELETE, HEAD, TRACE, CONNECT, UNKNOWN
} HttpMethod;

typedef struct HttpRequest {
    enum HttpMethod method;
    char url[HTTP_URL_LENGTH_MAX];
    char host[HTTP_HOST_LENGTH_MAX];
} HttpRequest;

typedef void http_request_handler(HttpRequest* request, struct espconn* connection);

void http_init();
void http_stop();
void http_add_request_handler(http_request_handler* handler);
void http_send(struct espconn* connection, const char* data);
void http_send_bad_request(struct espconn* connection);

#endif
