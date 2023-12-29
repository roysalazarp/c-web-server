#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int not_found (int client_socket_file_descriptor, char *request_headers) {
    char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>404 Not Found</h1></body></html>";
    send(client_socket_file_descriptor, response, strlen(response), 0);

    close(client_socket_file_descriptor);
    return 0;
}
