#include <string.h>
#include <sys/socket.h>

#include "router.h"

void not_found(int client_socket) {
    char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>404 Not Found</h1></body></html>";
    send(client_socket, response, strlen(response), 0);
}
