#include <string.h>
#include <sys/socket.h>

void endpoints_notFound(int channel, const char* request_headers) {
  char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                    "<html><body><h1>404 Not Found</h1></body></html>";
  send(channel, response, strlen(response), 0);
}
