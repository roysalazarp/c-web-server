#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/limits.h>

#include "utils/utils.h"

#define HEADERS_BUFFER_SIZE 4096

char* read_template_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        log_error("Error opening template file");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) == -1) {
        log_error("Failed to seek to a certain position on STREAM");
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);

    if (file_size == -1) {
        log_error("Failed to return the current position of STREAM");
        fclose(file);
        return NULL;
    }

    rewind(file);

    char* content = (char*)malloc((size_t)(file_size + 1));

    if (content == NULL) {
        log_error("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(content, sizeof(char), file_size, file);

    if (ferror(file) != 0) {
        log_error("Error reading from file");
        fclose(file);
        return NULL;
    }

    content[bytes_read] = '\0'; // Null-terminate the string
    fclose(file);

    return content;
}

char* build_template_path(const char* path) {
    char resolved_path[PATH_MAX];

    if (realpath(".", resolved_path) == NULL) {
        log_error("Failed to get the absolute path of the current working directory");
        return NULL;
    }

    char* template_path = (char*)malloc(PATH_MAX);
    if (template_path == NULL) {
        log_error("Memory allocation failed");
        return NULL;
    }

    if (snprintf(template_path, PATH_MAX, "%s/%s", resolved_path, path) >= PATH_MAX) {
        log_error("Formatted string truncated");
        return NULL;
    }

    return template_path;
}

char* build_http_response(char* content, const char* headers) {
    size_t response_length = strlen(content) + strlen(headers);

    char* http_response = (char *)malloc(response_length);
    if (http_response == NULL) {
        log_error("Memory allocation failed");
        return NULL;
    }

    // TODO: Check for snprintf errors
    // Generate the HTTP response
    snprintf(http_response, response_length, "%s%s", headers, content);

    return http_response;
}

void home_get (int client_socket_file_descriptor, char* request_headers) {
    char* host = retrieve_header(request_headers, "Host");
    free(request_headers);

    if (host != NULL) {
        printf("%s\n", host);
    }

    free(host);

    char* template_path = build_template_path("src/web/pages/home.html");
    if (template_path == NULL) {
        log_error("Failed to build template path");
        free(template_path);
        exit(EXIT_FAILURE);
    }

    char* template_content = read_template_file(template_path);
    free(template_path);
    if (template_content == NULL) {
        log_error("Failed to read template file");
        exit(EXIT_FAILURE);
    }

    char headers[100] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    char* http_response = build_http_response(template_content, headers);
    free(template_content);
    if (http_response == NULL) {
        log_error("Failed to build http response");
        exit(EXIT_FAILURE);
    }

    // TODO: Check for strlen erros
    size_t http_response_length = strlen(http_response);

    // Send the HTTP response
    if (send(client_socket_file_descriptor, http_response, http_response_length, 0) == -1) {
        log_error("Failed send buffer");
        free(http_response);
        exit(EXIT_FAILURE);
    }

    free(http_response);

    close(client_socket_file_descriptor);
}

void home_post (int client_socket_file_descriptor, char* request_headers) {}
void home_put (int client_socket_file_descriptor, char* request_headers) {}
void home_patch (int client_socket_file_descriptor, char* request_headers) {}
