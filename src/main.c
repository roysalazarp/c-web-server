#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "router.h"

#define PORT 8080
#define MAX_CONNECTIONS 100

int main() {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  if (server_socket == -1) {
    perror("Socket creation failed");
    exit(1);
  }

  // Reuse of a local address immediately after the socket is closed
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  }

  // Configure server address
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,            // IPv4
    .sin_port = htons(PORT),          // Convert the port number from host byte order to network byte order (big-endian)
    .sin_addr.s_addr = INADDR_ANY     // Listen on all available network interfaces (IPv4 addresses)
  };

  // Bind socket to address and port
  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
      perror("Socket binding failed");
      exit(1);
  }

  // Listen for incoming connections
  if (listen(server_socket, MAX_CONNECTIONS) == -1) {
      perror("Listen failed");
      exit(1);
  }

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int channel = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    if (channel == -1) {
      perror("Accept failed");
      exit(1);
    }
    
    char request[1024];
    recv(channel, request, sizeof(request), 0);

    char method[10];
    char url[256];
    sscanf(request, "%s %s", method, url);

    route_request(channel, method, url);

    close(channel);
  }

  close(server_socket);
  return 0;
}