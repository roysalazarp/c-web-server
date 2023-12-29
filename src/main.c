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
#define PORT 8080

volatile sig_atomic_t should_exit = 0;

void sigint_handler(int signo) {
  if (signo == SIGINT) {
    should_exit = 1;
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

int extract_request_headers (char *request_headers, int server_socket_file_descriptor, int client_socket_file_descriptor) {
    if (recv(client_socket_file_descriptor, request_headers, REQUEST_HEADERS_BUFFER_SIZE, 0) == -1) {
        log_error("Failed read N bytes into BUF from socket FD\n");
        return -1;
    }

    return 0;
}

int filetype_request (char *path, char *extension) {
    size_t extension_length = strlen(extension) + 1;
    size_t path_length = strlen(path) + 1;

    if (extension_length > path_length) {
        return -1;
    }

    if (strncmp(path + path_length - extension_length, extension, extension_length) == 0) {
        return 0;
    }

    return -1;
}

int main () {
    // used to gracefully exit program to check memory leaks with valgrind
    signal(SIGINT, sigint_handler);

    int server_socket_file_descriptor = create_and_configure_server_socket(PORT);
    if (server_socket_file_descriptor == -1) {
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        if (should_exit) {
            close(server_socket_file_descriptor);
            break;
        }

        int client_socket_file_descriptor = accept_client_connection(server_socket_file_descriptor);
        if (client_socket_file_descriptor == -1) {
            close(server_socket_file_descriptor);
            exit(EXIT_FAILURE);
        }

        char *request_headers;
        request_headers = (char*)malloc((REQUEST_HEADERS_BUFFER_SIZE * (sizeof *request_headers)) + 1);
        if (request_headers == NULL) {
            log_error("Failed to allocate memory for request_headers\n");
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            exit(EXIT_FAILURE);
        }
        
        request_headers[0] = '\0'; // set memory to empty string

        if (extract_request_headers(request_headers, server_socket_file_descriptor, client_socket_file_descriptor) == -1) {
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            free(request_headers);
            request_headers = NULL;
            exit(EXIT_FAILURE);
        }

        request_headers[REQUEST_HEADERS_BUFFER_SIZE] = '\0';

        const char *first_space = strchr(request_headers, ' ');
        const char *second_space = strchr(first_space + 1, ' ');

        size_t method_length = first_space - request_headers;

        char *method;
        method = malloc(method_length * (sizeof *method) + 1);
        
        size_t url_length = second_space - (first_space + 1);

        char *url;
        url = malloc(url_length * (sizeof *url) + 1);

        if (method == NULL || url == NULL) {
            log_error("Failed to allocate memory for method, url or protocol\n");
            close(server_socket_file_descriptor);
            close(client_socket_file_descriptor);
            free(request_headers);
            request_headers = NULL;
            exit(EXIT_FAILURE);
        }

        strncpy(method, request_headers, method_length);
        method[method_length] = '\0';

        strncpy(url, first_space + 1, url_length);
        url[url_length] = '\0';

        if (filetype_request(url, ".css") == 0) {
            char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\n\r\n";

            if (serve_static(client_socket_file_descriptor, url, headers) == -1) {
                close(server_socket_file_descriptor);
                close(client_socket_file_descriptor);
                free(request_headers);
                request_headers = NULL;
                free(url);
                url = NULL;
                free(method);
                method = NULL;
                exit(EXIT_FAILURE);
            }
        }

        if (strcmp(url, "/") == 0) {
            if (strcmp(method, "GET") == 0) {
                if (home_get(client_socket_file_descriptor, request_headers) == -1) {
                    close(server_socket_file_descriptor);
                    close(client_socket_file_descriptor);
                    free(request_headers);
                    request_headers = NULL;
                    free(url);
                    url = NULL;
                    free(method);
                    method = NULL;
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
                free(url);
                url = NULL;
                free(method);
                method = NULL;
                exit(EXIT_FAILURE);
            }
        }
        
        free(request_headers);
        request_headers = NULL;
        free(url);
        url = NULL;
        free(method);
        method = NULL;
    }
}