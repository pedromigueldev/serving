#include "./serving.h"
#include "chaining.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct serving_t_request {
    Chaining * url;
    Chaining * method;
    Chaining * header;
};


int serving_request_read(int connection_fd, Chaining ** buffer);
int serving_server_make(serving* server, const int PORT);
int serving_server_wait(serving* server, int* connection_fd);
int __parse_request (Chaining* from[static 1], struct serving_t_request * to);

int serving_server_run (serving* server_config, const int PORT) {
    int connection_fd = -1;

    if(serving_server_make(server_config, PORT)) {
        perror("Failed to make server...\n");
        return 1;
    }

    do {
        Chaining* raw_request_buffer = Chaining_new("");
        struct serving_t_request raw_request_parsed = {0};

        if(serving_server_wait(server_config, &connection_fd)) {
            perror("ERROR: Failed to launch server...\n");
            break;
        };

        if (serving_request_read(connection_fd, &raw_request_buffer)) {
            perror("ERROR: Read request failed\n");
            close(connection_fd);
            break;
        }

        if (__parse_request(&raw_request_buffer, &raw_request_parsed)) {
            perror("ERROR: Parse request failed\n");
            close(connection_fd);
            break;
        }

        close(connection_fd);
        free(raw_request_buffer);
        free(raw_request_parsed.header);
        free(raw_request_parsed.method);
        free(raw_request_parsed.url);
    } while(false);

    close(server_config->socket);
    return 0;
}

int serving_server_make(serving* server, const int PORT) {

    *server = (serving) {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .backlog = 10,
        .port = PORT,
    };

    server->address.sin_family = server->domain;
    server->address.sin_port = htons(server->port);
    server->address.sin_addr.s_addr = htonl(server->interface);

    if ((server->socket = socket(server->domain, server->service, server->protocol)) < 0){
        perror("Fail to connect to socket...\n");
        return 1;
    }

    const int enable = 1;
    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Setsockopt(SO_REUSEADDR) failed\n");
        return 1;
    }

    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        perror("Setsockopt(SO_REUSEPORT) failed\n");
        return 1;
    }

    if ((bind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address))) < 0) {
        perror("Failed to bind socket...\n");
        return 1;
    }

    if ((listen(server->socket, server->backlog)) < 0){
        perror("Failed to start listening...\n");
        return 1;
    }

    return 0;
};


int serving_server_wait(serving* server, int* connection_fd) {
    int address_length = sizeof(server->address);

    printf("============ WAITING FOR CONNECTION ============\n");
    if((*connection_fd = accept(
            server->socket,
            (struct sockaddr*)&server->address,
            (socklen_t*)&address_length)) < 0)
    {
        perror("ERROR: Failed to accept new connection...\n");
        return 1;
    }
    return 0;
}

int serving_request_read(int connection_fd, Chaining ** buffer) {
    int bytes = 1;
    char packet[SERVING_PACKET_SIZE];

    do {
        bytes = recv(connection_fd, packet, SERVING_PACKET_SIZE, 0);
        Chaining_append_raw(buffer, packet, (size_t)bytes);
    } while ((size_t)bytes >= (*buffer)->size);

    if (bytes < 0) {
        perror("ERROR: Failed to create request buffer\n");
        return 1;
    }

    Chaining_print(*buffer);
    return 0;
}

int __parse_request (Chaining* from[static 1], struct serving_t_request * to) {

    CHAINING_STR_AFREE temp = Chaining_clone(from);
    char* url;
    char* head;
    char* method;

    char* body = strstr(temp->string, "\r\n\r\n");
    head = temp->string;
    head[body-head] = '\0';

    CHAINING_STR_AFREE temp2 = Chaining_clone(from);
    char* token = strtok(temp2->string, " ");
    method = token;

    token = strtok(NULL, " ");
    url = token;

    *to = (struct serving_t_request) {
        .header = Chaining_new(head),
        .url = Chaining_new(url),
        .method = Chaining_new(method),
    };
    return 0;
};
