#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include <stdarg.h>

#include "utils/utils.h"
#include "template_engine/template_engine.h"

#define HEADERS_BUFFER_SIZE 4096

long int calculate_file_bytes_length (char* file_path) {
    FILE* file = fopen(file_path, "rb");

    if (file == NULL) {
        log_error("Error opening file");
        return -1;
    }

    if (fseek(file, 0, SEEK_END) == -1) {
        log_error("Failed to move the file position indicator to the end of the file");
        fclose(file);
        return -1;
    }

    long int file_size = ftell(file);
    if (file_size == -1) {
        log_error("Failed to determine the current file position indicator of a file");
        fclose(file);
        return -1;
    }

    rewind(file);
    fclose(file);
    
    return file_size;
}

int read_file (char* file_content, char* file_path, long file_size) {
    FILE* file = fopen(file_path, "rb");

    if (file == NULL) {
        log_error("Error opening file");
        return -1;
    }

    size_t bytes_read = fread(file_content, sizeof(char), (size_t)file_size, file);
    if (bytes_read != (size_t)file_size) {
        if (feof(file)) {
            log_error("End of file reached before reading all elements");
        } 

        if (ferror(file)) {
            perror("An error occurred during the fread operation");
        }

        fclose(file);
        return -1;
    }

    file_content[bytes_read] = '\0';
    fclose(file);

    return 0;
}

int build_template_path (char* buffer, const char* path) {
    char cwd[PATH_MAX];

    if (realpath(".", cwd) == NULL) {
        log_error("Failed to get the absolute path of the current working directory");
        return -1;
    }

    if (snprintf(buffer, PATH_MAX, "%s/%s", cwd, path) >= PATH_MAX) {
        log_error("Formatted string truncated");
        return -1;
    }
    
    return 0;
}

size_t calculate_combined_length (int num_strings, ...) {
    if (num_strings <= 0) {
        log_error("Invalid number of strings");
        return -1;
    }
    
    size_t sum = 0;

    va_list args;
    va_start(args, num_strings);

    for (int i = 0; i < num_strings; ++i) {
        char *current_str = va_arg(args, char*);
        if (current_str == NULL) {
            va_end(args);
            log_error("string is NULL");
            return -1;
        }
        sum += strlen(current_str);
    }

    va_end(args);

    return sum;
}

int home_get (int client_socket_file_descriptor, char* request_headers) {
    char* template_path = (char*)malloc(PATH_MAX);
    if (template_path == NULL) {
        log_error("Failed to allocate memory for template_path");
        return -1;
    }

    if (build_template_path(template_path, "src/web/pages/home.html") == -1) {
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

    char* template_content = (char*)malloc((size_t)(file_size));
    if (template_content == NULL) {
        log_error("Failed to allocate memory for template_content");
        free(template_path);
        template_path = NULL;
        return -1;
    }

    if (read_file(template_content, template_path, file_size) == -1) {
        free(template_path);
        template_path = NULL;
        free(template_content);
        template_content = NULL;
        return -1;
    }

    free(template_path);
    template_path = NULL;

    printf("%s\n", template_content);

    char* country[2] = { "v0", "Finland" };
    if (render_val(country[0], country[1], template_content) == -1) {
        free(template_content);
        template_content = NULL;
        return -1;
    }

    char* phone_number[2] = { "v1", "441317957" };
    if (render_val(phone_number[0], phone_number[1], template_content) == -1) {
        free(template_content);
        template_content = NULL;
        return -1;
    }
    
    char* phone_prefix[2] = { "v2", "+358" };
    if (render_val(phone_prefix[0], phone_prefix[1], template_content) == -1) {
        free(template_content);
        template_content = NULL;
        return -1;
    }

    // char* menu_list[6] = { "for0", "for0->v0", "home", "about", "contact", "careers" };
    // if (render_for(menu_list, template_content, 6) == NULL) {
    //     free(template_content);
    //     return -1;
    // }

    printf("%s\n", template_content);

    char headers[100] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    
    size_t response_length = calculate_combined_length(2, template_content, headers);
    if (response_length == -1) {
        free(template_content);
        template_content = NULL;
        return -1;
    }
    
    char* http_response = (char *)malloc(response_length);
    if (http_response == NULL) {
        log_error("Failed to allocate memory for http_response");
        free(template_content);
        template_content = NULL;
        return -1;
    }

    if (snprintf(http_response, response_length, "%s%s", headers, template_content) != response_length) {
        log_error("Did't store the result in a specified buffer correctly");
        free(template_content);
        template_content = NULL;
        free(http_response);
        http_response = NULL;
        return -1;
    }

    free(template_content);
    template_content = NULL;

    // Send the HTTP response
    if (send(client_socket_file_descriptor, http_response, response_length, 0) == -1) {
        log_error("Failed send HTTP response");
        free(http_response);
        http_response = NULL;
        return -1;
    }

    free(http_response);
    http_response = NULL;

    close(client_socket_file_descriptor);
    return 0;
}

void home_post (int client_socket_file_descriptor, char* request_headers) {}
void home_put (int client_socket_file_descriptor, char* request_headers) {}
void home_patch (int client_socket_file_descriptor, char* request_headers) {}
