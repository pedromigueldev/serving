#ifndef _SERVING_H_
#define _SERVING_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>
#include <stddef.h>

#define SERVING_PACKET_SIZE 800

typedef unsigned long ulong;
typedef struct serving_t serving;

struct serving_t {
	struct sockaddr_in address;
	ulong interface;
    int domain, service, protocol, port, backlog, socket;
};

int serving_server_run (serving* server_config, const int PORT);

#endif
