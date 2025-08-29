#include "./serving.h"
#include "chaining.h"
#include "chaining_arena.h"
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

static Chain_bucket Raw_request_arena;
static Chain_bucket Request_arena;
static Chain_bucket Endpoints_arena;
static serving* __server;

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

int __request_read(int connection_fd, Chaining ** buffer);
int __server_make(serving* server, const int PORT);
int __server_wait(serving* server, int* connection_fd);
int __parse_http1_1_request (Chaining* from[static 1], struct serving_t_request * to);

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
        Raw_request_arena = Chain_bucket_new(sizeof(char) * SERVING_PACKET_SIZE * 3);

        Chaining_str raw_request_buffer = Chaining_new_arena(&Raw_request_arena, "");
        if(__server_wait(__server, &connection_fd)) {
            perror("ERROR: Failed to launch server...\n");
            break;
        };

        if (__request_read(connection_fd, &raw_request_buffer)) {
            perror("ERROR: Read request failed\n");
            close(connection_fd);
            break;
        }

        struct serving_t_request raw_request_parsed = {0};
        if (__parse_http1_1_request(&raw_request_buffer, &raw_request_parsed)) {
            perror("ERROR: Parse request failed\n");
            close(connection_fd);
            break;
        }

        close(connection_fd);
        Bucket_free(&Request_arena);
        Bucket_free(&Raw_request_arena);
    } while(false);

    Bucket_free(&Endpoints_arena);
    close(__server->socket);
    return 0;
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

    int flags = fcntl(connection_fd, F_GETFL, 0);
    fcntl(connection_fd, F_SETFL, flags | O_NONBLOCK);

    do {
        bytes = recv(connection_fd, packet, SERVING_PACKET_SIZE, MSG_DONTWAIT);

        if (bytes > 0) {
            Chaining_append_raw_arena(&Raw_request_arena, buffer, packet, bytes);
        } else if (0 > bytes) {
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv error");
                break;
            }
        }

    } while ((size_t)bytes >= (*buffer)->size);

    Chaining_print(*buffer);
    return 0;
}

int __parse_http1_1_request (Chaining* from[static 1], struct serving_t_request * to) {

    Chaining_str temp = Chaining_clone_arena(&Request_arena ,from);
    char* url;
    char* method;

    CHAINING_STR_AFREE body = Chaining_look_for(temp, "\r\n\r\n", false);
    CHAINING_STR_AFREE head = Chaining_new_len(temp->string, temp->size - body->size);

    method = strtok(temp->string, " ");
    url = strtok(NULL, " ");

    *to = (struct serving_t_request) {
        .body = CHAINING_STR_NEW(body->string, .len = body->size, .bucket = Request_arena),
        .url = CHAINING_STR_NEW(url, .bucket = Request_arena),
        .method = CHAINING_STR_NEW(method, .bucket = Request_arena),
    };

    if (
        (!Chaining_includes(head, "HTTP/1.1")) &&
        (!Chaining_includes(head, "Hostname: ")) &&
        (!Chaining_includes(head, "Host: ")) &&
        (!Chaining_includes(head, "Content-Length: "))
    ) return 1;

    char* token;
    while ((token = strtok(NULL, "\r\n")) != nullptr) {
        CHAINING_STR_AFREE string = CHAINING_STR_NEW(token);

        if(Chaining_includes(string, "Host: "))
            to->header.Host = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Hostname: "))
            to->header.Hostname = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Accept: "))
            to->header.Accept = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Accept-Encoding: "))
            to->header.AcceptEconding = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "User-Agent: "))
            to->header.UserAgent = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Content-Encoding: "))
            to->header.ContentEncoding = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Content-Length: "))
            to->header.ContentLength = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Content-Type: "))
            to->header.ContentType = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Authorization: "))
            to->header.Authorization = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Connection: "))
            to->header.Connection = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Origin: "))
            to->header.Origin = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Referer: ") || Chaining_includes(string, "Referrer: "))
            to->header.Referer = CHAINING_STR_NEW(token, .bucket = Request_arena);
        else if(Chaining_includes(string, "Cookie: "))
            to->header.Cookie = CHAINING_STR_NEW(token, .bucket = Request_arena);
    }

    return 0;
};
