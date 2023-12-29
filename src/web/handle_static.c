#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/limits.h>

#include "utils/utils.h"

int serve_static (int client_socket_file_descriptor, char *path, const char *headers) {
    char *template_path;
    template_path = (char*)malloc(PATH_MAX * (sizeof *template_path) + 1);
    if (template_path == NULL) {
        log_error("Failed to allocate memory for template_path\n");
        return -1;
    }

    template_path[0] = '\0';

    if (build_template_path(template_path, path) == -1) {
        free(template_path);
        template_path = NULL;
        return -1;
    }

    long int file_size = calculate_file_bytes_length(template_path);
    if (file_size == -1) {
        free(template_path);
        template_path = NULL;
        return -1;
    }

    char *static_content;
    static_content = (char*)malloc(((size_t)(file_size)) * (sizeof *static_content) + 1);
    if (static_content == NULL) {
        log_error("Failed to allocate memory for static_content\n");
        free(template_path);
        template_path = NULL;
        return -1;
    }

    static_content[0] = '\0';

    if (read_file(static_content, template_path, file_size) == -1) {
        free(template_path);
        template_path = NULL;
        free(static_content);
        static_content = NULL;
        return -1;
    }

    static_content[file_size] = '\0';

    free(template_path);
    template_path = NULL;
    
    size_t response_length = calculate_combined_length(2, static_content, headers);
    if (response_length == -1) {
        free(static_content);
        static_content = NULL;
        return -1;
    }
    
    char *http_response;
    http_response = (char*)malloc(response_length * (sizeof *http_response) + 1);
    if (http_response == NULL) {
        log_error("Failed to allocate memory for http_response\n");
        free(static_content);
        static_content = NULL;
        return -1;
    }

    http_response[0] = '\0';

    if (sprintf(http_response, "%s%s", headers, static_content) < 0) {
        log_error("Did't store the result in a specified buffer correctly\n");
        free(static_content);
        static_content = NULL;
        free(http_response);
        http_response = NULL;
        return -1;
    }

    free(static_content);
    static_content = NULL;

    // Send the HTTP response
    if (send(client_socket_file_descriptor, http_response, response_length, 0) == -1) {
        log_error("Failed send HTTP response\n");
        free(http_response);
        http_response = NULL;
        return -1;
    }

    free(http_response);
    http_response = NULL;

    close(client_socket_file_descriptor);
    return 0;
}