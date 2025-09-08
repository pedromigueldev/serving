#ifndef _SERVING_H_
#define _SERVING_H_

#include "chaining.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#define SERVING_PACKET_SIZE 2048

typedef unsigned long ulong;
typedef struct serving_t serving;
typedef void (*serving_endpoint_func)(void);
typedef struct serving_t_endpoints serving_endpoints;
typedef struct serving_t_request serving_t_request_http1_1 ;

enum SERVING_METHOD {
    SERVING_METHOD_GET = 0,
    SERVING_METHOD_POST,
    SERVING_METHOD_PUT,
    SERVING_METHOD_DELETE,
    SERVING_METHOD_HEAD,
    SERVING_METHOD_OPTIONS,
    SERVING_METHOD_TRACE,
    SERVING_METHOD_CONNECT,
    SERVING_METHOD_PATCH,
    SERVING_METHOD_MAX
};

#define MAX_ENDPOINTS_QUANTITY 50
struct serving_t_endpoints {
    size_t items;
    enum SERVING_METHOD methods[MAX_ENDPOINTS_QUANTITY];
    chainstr paths[MAX_ENDPOINTS_QUANTITY];
    serving_endpoint_func endpoint_func[MAX_ENDPOINTS_QUANTITY];
};

struct serving_t {
	struct sockaddr_in address;
	ulong interface;
    int domain, service, protocol, port, backlog, socket;
    serving_endpoints endpoints;
};

struct serving_t_request {
    Chaining * url;
    Chaining * method;
    chainstr body;
    struct {
        chainstr header;
        chainstr Host;
        chainstr Hostname;
        chainstr Accept;
        chainstr AcceptEconding;
        chainstr AcceptLanguage;
        chainstr AcceptCharset;
        chainstr UserAgent;
        chainstr ContentLength;
        chainstr ContentType;
        chainstr ContentEncoding;
        chainstr Authorization;
        chainstr Connection;
        chainstr Origin;
        chainstr Referer;
        chainstr Cookie;
    } header;
};

int serving_server_run (serving* server_config, const int PORT);
void serving_endpoint_set(serving* server_config, enum SERVING_METHOD method, const char url[static 1], serving_endpoint_func endpoint_func);

#endif
