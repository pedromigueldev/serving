#include "./serving.h"
#include "chaining.h"
#include "chaining_arena.h"
#include "./chaining_array.h"
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

// static Chain_bucket Raw_request_arena;
static Chain_bucket Request_arena;
static Chain_bucket Endpoints_arena;
static serving* __server;

int __request_read(int connection_fd, Chaining ** buffer);
int __server_make(serving* server, const int PORT);
int __server_wait(serving* server, int* connection_fd);
int __parse_http1_1_request (Chaining* source[static 1], serving_t_request_http1_1 * destination);

void serving_endpoint_set(serving* server_config, const char method[static 1], const char url[static 1], serving_endpoint_func endpoint_func) {
    if (Endpoints_arena == nullptr)
        Endpoints_arena = Chain_bucket_new(sizeof(char) *  SERVING_PACKET_SIZE * 2);

    server_config->endpoints.methods[server_config->endpoints.items] = Chaining_new_arena(&Endpoints_arena, method);
    server_config->endpoints.paths[server_config->endpoints.items] = Chaining_new_arena(&Endpoints_arena, url);
    server_config->endpoints.endpoint_func[server_config->endpoints.items] = endpoint_func;
    server_config->endpoints.items++;
    return;
}

int serving_server_run (serving* server_config, const int PORT) {
    int connection_fd = -1;
    __server = server_config;

    if(__server_make(__server, PORT)) {
        perror("Failed to make server...\n");
        return 1;
    }

    do {
        Request_arena = Chain_bucket_new(sizeof(char) *  SERVING_PACKET_SIZE * 3);

        CHAINING_STR_AFREE raw_request_buffer = Chaining_new("");

        if(__server_wait(__server, &connection_fd)) {
            perror("ERROR: Failed to launch server...\n");
            goto ret_error;
        };

        if (__request_read(connection_fd, &raw_request_buffer)) {
            perror("ERROR: Read request failed\n");
            goto ret_error;
        }

        serving_t_request_http1_1 raw_request_parsed = {0};
        if (__parse_http1_1_request(&raw_request_buffer, &raw_request_parsed)) {
            perror("ERROR: Parse request failed\n");
            goto ret_error;
        }

        close(connection_fd);
        Bucket_free(&Request_arena);
    } while(false);

    Bucket_free(&Endpoints_arena);
    close(__server->socket);
    return 0;

    ret_error:
    Bucket_free(&Request_arena);
    Bucket_free(&Endpoints_arena);
    close(connection_fd);
    close(__server->socket);
    return 1;
}

int __server_make(serving* server, const int PORT) {

    *server = (serving) {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .backlog = 10,
        .port = PORT,
        .endpoints = server->endpoints
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

int __server_wait(serving* server, int* connection_fd) {

    int address_length = sizeof(server->address);
    struct sockaddr* addrs = (struct sockaddr*)&server->address;
    socklen_t* socklen = (socklen_t*)&address_length;

    printf("============ WAITING FOR CONNECTION ============\n");

    if((*connection_fd = accept(server->socket, addrs, socklen)) < 0) {
        perror("ERROR: Failed to accept new connection...\n");
        return 1;
    }
    return 0;
}

int __request_read(int connection_fd, Chaining ** buffer) {
    int bytes = 1;
    char packet[SERVING_PACKET_SIZE];

    fd_set read_fds;
    struct timeval timeout;

    int flags = fcntl(connection_fd, F_GETFL, 0);
    fcntl(connection_fd, F_SETFL, flags | O_NONBLOCK);

    FD_ZERO(&read_fds);
    FD_SET(connection_fd, &read_fds);

    timeout = (struct timeval) {
        .tv_usec = 300000
    };

    int activity = select(connection_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (activity < 0) {
        perror("select error");
    } else if (activity == 0) {
        printf("Timeout occurred, no data available.\n");
    } else if (activity) {
        do {
            bytes = recv(connection_fd, packet, sizeof(packet), MSG_DONTWAIT);
            if (bytes > 0) {
                Chaining_append_raw(buffer, packet, bytes);
            } else {
                if (errno == EAGAIN && errno == EWOULDBLOCK) {
                    break;
                } else {
                    perror("recv error");
                    break;
                }
            }
        } while (1);
    }
    Chaining_print(*buffer);
    return 0;
}

int __parse_http1_1_request (Chaining_str source[static 1], struct serving_t_request * destination) {
    Chain_bucket Temp_header_bucket = Chain_bucket_new((*source)->size);

    CHAINING_STR_AFREE body = Chaining_look_for((*source), "\r\n\r\n", false);
    if (body == nullptr)
        return 1;

    CHAINING_STR_AFREE headers = CHAINING_STR_NEW((*source)->string, .len = (*source)->size - body->size);
    if (headers== nullptr)
        return 1;

    Chaining_str_array HTTP1_1Headers_lines = {0};
    if (CHAIN_EXPLODE(&HTTP1_1Headers_lines, headers, "\r\n", .bucket = Temp_header_bucket)) return 1;

	Chaining_str_array first_line_header = {0};
	if (CHAIN_EXPLODE(&first_line_header, HTTP1_1Headers_lines->array[0], " ", .bucket = Request_arena)) return 1;
	if (!Chaining_includes(first_line_header->array[2], "HTTP/1.1")) return 1;


	*destination = (struct serving_t_request) {
        .url = first_line_header->array[1],
        .method = first_line_header->array[0],
    };

    for (size_t i = 0; i < HTTP1_1Headers_lines->size; i++) {
        Chaining_str_array temp = {0};
        Chaining_explode_in_bucket(Request_arena, HTTP1_1Headers_lines->array[i], &temp, ": ", true);

        if(Chaining_includes(temp->array[0], "Hostname"))
            destination->header.Hostname = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Host"))
            destination->header.Host = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Accept-Encoding"))
            destination->header.AcceptEconding = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Accept"))
            destination->header.Accept = temp->array[1];
        else if(Chaining_includes(temp->array[0], "User-Agent"))
            destination->header.UserAgent = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Content-Encoding"))
            destination->header.ContentEncoding = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Content-Length"))
            destination->header.ContentLength = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Content-Type"))
            destination->header.ContentType = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Authorization"))
            destination->header.Authorization = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Connection"))
            destination->header.Connection = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Origin"))
            destination->header.Origin = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Referer") || Chaining_includes(temp->array[0], "Referrer"))
            destination->header.Referer = temp->array[1];
        else if(Chaining_includes(temp->array[0], "Cookie"))
            destination->header.Cookie = temp->array[1];
        free(temp);
    }

    if (destination->header.Host == nullptr && destination->header.Hostname == nullptr && destination->header.ContentLength == nullptr)
        return 1;

    if(destination->header.ContentLength != nullptr) {
        // create atoi function that accepts string size;
        destination->body = Chaining_clone_arena(&Request_arena, &body);

    }

    free(HTTP1_1Headers_lines); // temp arrays
    free(first_line_header);    // temp arrays
    free(Temp_header_bucket);
    return 0;
};
