#ifndef HANDLERS_H
#define HANDLERS_H

int serve_static (int client_socket_file_descriptor, char *path, const char *headers);

int home_get (int client_socket_file_descriptor, char *request_headers);
// void home_post (int client_socket_file_descriptor, char *request_headers);
// void home_put (int client_socket_file_descriptor, char *request_headers);
// void home_patch (int client_socket_file_descriptor, char *request_headers);

// void about_get (int client_socket_file_descriptor, char *request_headers);
// void about_post (int client_socket_file_descriptor, char *request_headers);

// void method_not_supported (int client_socket_file_descriptor, char *request_headers);

int not_found (int client_socket_file_descriptor, char *request_headers);

#endif