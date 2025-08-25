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

#define SERVING_PACKET_SIZE 800

typedef unsigned long ulong;
typedef struct serving_t serving;
typedef void (*serving_endpoint_func)(void);
typedef struct serving_t_endpoints serving_endpoints;

struct serving_t_endpoints {
    size_t size;
    size_t capacity;
    Chaining_str * methods;
    Chaining_str * paths;
    serving_endpoint_func * endpoint_func;
};

struct serving_t {
	struct sockaddr_in address;
	ulong interface;
    int domain, service, protocol, port, backlog, socket;
    serving_endpoints endpoints;
};


int serving_server_run (serving* server_config, const int PORT);
void serving_endpoint_set(serving* server_config, const char method[static 1], const char url[static 1], serving_endpoint_func endpoint_func);

#endif
