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

#define MAX_ENDPOINTS_QUANTITY 50
struct serving_t_endpoints {
    size_t items;
    Chaining_str methods[MAX_ENDPOINTS_QUANTITY];
    Chaining_str paths[MAX_ENDPOINTS_QUANTITY];
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
    Chaining_str body;
    struct {
        Chaining_str header;
        Chaining_str Host;
        Chaining_str Hostname;
        Chaining_str Accept;
        Chaining_str AcceptEconding;
        Chaining_str AcceptLanguage;
        Chaining_str AcceptCharset;
        Chaining_str UserAgent;
        Chaining_str ContentLength;
        Chaining_str ContentType;
        Chaining_str ContentEncoding;
        Chaining_str Authorization;
        Chaining_str Connection;
        Chaining_str Origin;
        Chaining_str Referer;
        Chaining_str Cookie;
    } header;
};

int serving_server_run (serving* server_config, const int PORT);
void serving_endpoint_set(serving* server_config, const char method[static 1], const char url[static 1], serving_endpoint_func endpoint_func);

#endif
