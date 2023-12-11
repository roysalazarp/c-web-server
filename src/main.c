#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "utils/utils.h"
#include "web/handlers.h"

#define REQUEST_HEADERS_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 100

volatile sig_atomic_t running = 1;

void sigint_handler(int signo) {
  if (signo == SIGINT) {
    running = 0;
  }
}

int create_and_configure_server_socket (uint16_t port) {
    int server_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket_file_descriptor == -1) {
        log_error("Creation of server socket failed");
        exit(EXIT_FAILURE);
    }

    // Reuse of a local address immediately after the socket is closed
    if (setsockopt(server_socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        log_error("setsockopt(SO_REUSEADDR) failed");
        close(server_socket_file_descriptor);
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,              // IPv4
        .sin_port = htons(port),            // Convert the port number from host byte order to network byte order (big-endian)
        .sin_addr.s_addr = INADDR_ANY       // Listen on all available network interfaces (IPv4 addresses)
    };

    // Bind socket to address and port
    if (bind(server_socket_file_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        log_error("Socket binding failed");
        close(server_socket_file_descriptor);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket_file_descriptor, MAX_CONNECTIONS) == -1) {
        log_error("Listen failed");
        close(server_socket_file_descriptor);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port: %d...\n", port);

    return server_socket_file_descriptor;
}

int accept_client_connection (int server_socket_file_descriptor) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket_file_descriptor = accept(server_socket_file_descriptor, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_socket_file_descriptor == -1) {
        log_error("Creation of client socket failed");
        close(server_socket_file_descriptor);
        exit(EXIT_FAILURE);
    }

    return client_socket_file_descriptor;
}

char* extract_request_headers (int server_socket_file_descriptor, int client_socket_file_descriptor) {
    char* request_headers = (char*)malloc(REQUEST_HEADERS_BUFFER_SIZE);

    if (request_headers == NULL) {
        log_error("Failed to allocate memory");
        close(server_socket_file_descriptor);
        exit(EXIT_FAILURE);
    }

    if (recv(client_socket_file_descriptor, request_headers, REQUEST_HEADERS_BUFFER_SIZE, 0) == -1) {
      return NULL;
    }

    return request_headers;
}

int main () {
    // used to gracefully exit program to check memory leaks with valgrind
    signal(SIGINT, sigint_handler);

    uint16_t port = 8080;

    int server_socket_file_descriptor = create_and_configure_server_socket(port);
    
    while (running) {
        int client_socket_file_descriptor = accept_client_connection(server_socket_file_descriptor);
        char* request_headers = extract_request_headers(server_socket_file_descriptor, client_socket_file_descriptor);

        if (request_headers == NULL) {
            log_error("Request headers is NULL");
            exit(EXIT_FAILURE);
        }

        char method[10];
        char url[256];
    
        sscanf(request_headers, "%9s %255s\n", method, url);

        if (strcmp(url, "/") == 0) {
            if (strcmp(method, "GET") == 0) {
                home_get(client_socket_file_descriptor, request_headers);
            } else if (strcmp(method, "POST") == 0) {
                home_post(client_socket_file_descriptor, request_headers);
            } else if  (strcmp(method, "PUT") == 0) {
                home_put(client_socket_file_descriptor, request_headers);
            } else if (strcmp(method, "PATCH") == 0) {
                home_patch(client_socket_file_descriptor, request_headers);
            } else {
                method_not_supported(client_socket_file_descriptor, request_headers);
            }
        } else if (strcmp(url, "/about") == 0) {
            if (strcmp(method, "GET") == 0) {
                about_get(client_socket_file_descriptor, request_headers);
            } else if (strcmp(method, "POST") == 0) {
                about_post(client_socket_file_descriptor, request_headers);
            } else {
                method_not_supported(client_socket_file_descriptor, request_headers);
            }
        } else {
            not_found(client_socket_file_descriptor, request_headers);
        }

        if (!running) {
          break;
        }
    }
}