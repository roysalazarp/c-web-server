#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include <stdarg.h>

#include "utils/utils.h"
#include "template_engine/template_engine.h"

int home_get (int client_socket_file_descriptor, char *request_headers) {
    char *template_path;
    template_path = (char*)malloc(PATH_MAX * (sizeof *template_path) + 1);
    if (template_path == NULL) {
        log_error("Failed to allocate memory for template_path\n");
        return -1;
    }
    
    template_path[0] = '\0';

    if (build_template_path(template_path, "/src/web/pages/home.html") == -1) {
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

    char *template_content;
    template_content = (char*)malloc(((size_t)(file_size)) * (sizeof *template_content) + 1);

    if (template_content == NULL) {
        log_error("Failed to allocate memory for template_content\n");
        free(template_path);
        template_path = NULL;
        return -1;
    }

    template_content[0] = '\0';

    if (read_file(template_content, template_path, file_size) == -1) {
        free(template_path);
        template_path = NULL;
        free(template_content);
        template_content = NULL;
        return -1;
    }

    template_content[file_size] = '\0';

    free(template_path);
    template_path = NULL;

    // char *country[3] = { "v0", "Finland", NULL };
    // if (render_val(country[0], country[1], &template_content) == -1) {
    //     free(template_content);
    //     template_content = NULL;
    //     return -1;
    // }

    // char *phone_number[3] = { "v1", "441317957", NULL };
    // if (render_val(phone_number[0], phone_number[1], &template_content) == -1) {
    //     free(template_content);
    //     template_content = NULL;
    //     return -1;
    // }
    
    // char *phone_prefix[3] = { "v2", "+358", NULL };
    // if (render_val(phone_prefix[0], phone_prefix[1], &template_content) == -1) {
    //     free(template_content);
    //     template_content = NULL;
    //     return -1;
    // }

    // char *menu_list[7] = { "for0", "for0->v0", "home", "about", "contact", "careers", NULL };
    // if (render_for(menu_list, &template_content, 7) == -1) {
    //     free(template_content);
    //     template_content = NULL;
    //     return -1;
    // }

    char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    
    size_t response_length = calculate_combined_length(2, template_content, headers);
    if (response_length == -1) {
        free(template_content);
        template_content = NULL;
        return -1;
    }
    
    char *http_response;
    http_response = (char*)malloc(response_length * (sizeof *http_response) + 1);
    if (http_response == NULL) {
        log_error("Failed to allocate memory for http_response\n");
        free(template_content);
        template_content = NULL;
        return -1;
    }

    http_response[0] = '\0';

    if (sprintf(http_response, "%s%s", headers, template_content) < 0) {
        log_error("Did't store the result in a specified buffer correctly\n");
        free(template_content);
        template_content = NULL;
        free(http_response);
        http_response = NULL;
        return -1;
    }

    http_response[response_length] = '\0';

    free(template_content);
    template_content = NULL;

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

void home_post (int client_socket_file_descriptor, char *request_headers) {}
void home_put (int client_socket_file_descriptor, char *request_headers) {}
void home_patch (int client_socket_file_descriptor, char *request_headers) {}
