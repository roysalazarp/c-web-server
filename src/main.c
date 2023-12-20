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

// Returns a file descriptor for the new socket, or -1 for errors.
int create_and_configure_server_socket (uint16_t port) {
    int server_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket_file_descriptor == -1) {
        log_error("Failed to create server socket\n");
        return -1;
    }

    if (setsockopt(server_socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        log_error("Failed to set local address for immediately reuse upon socker closed\n");
        close(server_socket_file_descriptor);
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,              // IPv4
        .sin_port = htons(port),            // Convert the port number from host byte order to network byte order (big-endian)
        .sin_addr.s_addr = INADDR_ANY       // Listen on all available network interfaces (IPv4 addresses)
    };

    if (bind(server_socket_file_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        log_error("Failed to bind socket to address and port\n");
        close(server_socket_file_descriptor);
        return -1;
    }

    if (listen(server_socket_file_descriptor, MAX_CONNECTIONS) == -1) {
        log_error("Failed to set up socket to listen for incoming connections\n");
        close(server_socket_file_descriptor);
        return -1;
    }

    printf("Server listening on port: %d...\n", port);
    return server_socket_file_descriptor;
}

// Returns the new socket's descriptor, or -1 for errors.
int accept_client_connection (int server_socket_file_descriptor) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket_file_descriptor = accept(server_socket_file_descriptor, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket_file_descriptor == -1) {
        log_error("Failed to create client socket\n");
        return -1;
    }

    return client_socket_file_descriptor;
}

int extract_request_headers (char* request_headers, int server_socket_file_descriptor, int client_socket_file_descriptor) {
    if (recv(client_socket_file_descriptor, request_headers, REQUEST_HEADERS_BUFFER_SIZE, 0) == -1) {
        log_error("Failed read N bytes into BUF from socket FD\n");
        return -1;
    }

    return 0;
}

int main () {
    // used to gracefully exit program to check memory leaks with valgrind
    signal(SIGINT, sigint_handler);

    uint16_t port = 8080;

    int server_socket_file_descriptor = create_and_configure_server_socket(port);
    if (server_socket_file_descriptor == -1) {
        exit(EXIT_FAILURE);
    }
    
    while (running) {
        int client_socket_file_descriptor = accept_client_connection(server_socket_file_descriptor);
        if (client_socket_file_descriptor == -1) {
            close(server_socket_file_descriptor);
            exit(EXIT_FAILURE);
        }

        char* request_headers = (char*)malloc(REQUEST_HEADERS_BUFFER_SIZE * sizeof(char));
        if (request_headers == NULL) {
            log_error("Failed to allocate memory for request_headers\n");
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            exit(EXIT_FAILURE);
        }
        
        request_headers[0] = '\0';

        if (extract_request_headers(request_headers, server_socket_file_descriptor, client_socket_file_descriptor) == -1) {
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            free(request_headers);
            request_headers = NULL;
            exit(EXIT_FAILURE);
        }

        char method[10];
        char url[256];
    
        if (sscanf(request_headers, "%9s %255s\n", method, url) != 2) {
            log_error("Failed to fill variables\n");
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            free(request_headers);
            request_headers = NULL;
            exit(EXIT_FAILURE);
        }

        if (strcmp(url, "/") == 0) {
            if (strcmp(method, "GET") == 0) {
                if (home_get(client_socket_file_descriptor, request_headers) == -1) {
                    close(server_socket_file_descriptor);
                    close(client_socket_file_descriptor);
                    free(request_headers);
                    exit(EXIT_FAILURE);
                }
            } // else if (strcmp(method, "POST") == 0) {
            //     home_post(client_socket_file_descriptor, request_headers);
            // } else if  (strcmp(method, "PUT") == 0) {
            //     home_put(client_socket_file_descriptor, request_headers);
            // } else if (strcmp(method, "PATCH") == 0) {
            //     home_patch(client_socket_file_descriptor, request_headers);
            // } else {
            //     method_not_supported(client_socket_file_descriptor, request_headers);
            // }
        // } else if (strcmp(url, "/about") == 0) {
            // if (strcmp(method, "GET") == 0) {
            //     about_get(client_socket_file_descriptor, request_headers);
            // } else if (strcmp(method, "POST") == 0) {
            //     about_post(client_socket_file_descriptor, request_headers);
            // } else {
            //     method_not_supported(client_socket_file_descriptor, request_headers);
            // }
        } else {
            if (not_found(client_socket_file_descriptor, request_headers) == -1) {
                close(server_socket_file_descriptor);
                close(client_socket_file_descriptor);
                free(request_headers);
                request_headers = NULL;
                exit(EXIT_FAILURE);
            }
        }

        if (!running) {
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            free(request_headers);
            request_headers = NULL;
            exit(EXIT_FAILURE);
            break;
        }
    }
}