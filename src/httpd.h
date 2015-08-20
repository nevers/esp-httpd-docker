#ifndef __HTTP_D_H__
#define __HTTP_D_H__

#define HTTP_URL_LENGTH_MAX 1024
#define HTTP_HOST_LENGTH_MAX 16

#include "user_interface.h"
#include "espconn.h"

void initHttpd();

typedef enum HttpMethod {
    GET, POST, PUT, DELETE, HEAD, TRACE, CONNECT, UNKNOWN
} HttpMethod;

typedef struct HttpRequest {
    enum HttpMethod method;
    char url[HTTP_URL_LENGTH_MAX];
    char host[HTTP_HOST_LENGTH_MAX];
} HttpRequest;

#endif
