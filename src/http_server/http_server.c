#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#include "http_server.h"
#include "web/endpoints.h"
#include "utils/utils.h"

volatile sig_atomic_t running = 1;

unsigned int httpserver_urlToIndex(const char* url) {
  // TODO: handle collisions
  size_t lenght = strnlen(url, httpserver_MAX_URL_LENGTH);
  unsigned int hash_value = 0;
  for (int i=0; i < lenght; i++) {
    hash_value += url[i];
    hash_value = (hash_value * url[i]) % httpserver_MAX_ENDPOINTS;
  }
  return hash_value;
}

httpserver_Handlers httpserver_findHandler(const char* url, httpserver_HTTP* self) {
  unsigned int index = httpserver_urlToIndex(url);
  return self->handlers[index];
}

void httpserver_handle(const char* url, httpserver_Handlers fn, httpserver_HTTP* self) {
  unsigned int index = httpserver_urlToIndex(url);
  self->handlers[index] = fn;
  self->registered_routes++; 
}

void httpserver_cleanup(httpserver_HTTP* self, int server_socket) {
  free(self);
  close(server_socket);
}

void sigint_handler(int signo) {
  if (signo == SIGINT) {
    running = 0;
  }
}

void httpserver_listen(unsigned int port, httpserver_HTTP* self) {
  signal(SIGINT, sigint_handler);
  
  self->port = port;

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    utils_logError("Socket creation failed");
    close(server_socket);
    httpserver_cleanup(self, server_socket);
    exit(EXIT_FAILURE);
  }

  // Reuse of a local address immediately after the socket is closed
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
    utils_logError("setsockopt(SO_REUSEADDR) failed");
  }

  // Configure server address
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,            // IPv4
    .sin_port = htons(self->port),    // Convert the port number from host byte order to network byte order (big-endian)
    .sin_addr.s_addr = INADDR_ANY     // Listen on all available network interfaces (IPv4 addresses)
  };

  // Bind socket to address and port
  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    utils_logError("Socket binding failed");
    close(server_socket);
    httpserver_cleanup(self, server_socket);
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_socket, httpserver_MAX_CONNECTIONS) == -1) {
    utils_logError("Listen failed");
    close(server_socket);
    httpserver_cleanup(self, server_socket);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port: %d...\n", self->port);

  while (running) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int channel = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (channel == -1) {
      utils_logError("Accept failed");
      close(server_socket);
      httpserver_cleanup(self, server_socket);
      exit(EXIT_FAILURE);
    }
    
    char request_headers[1024];

    if (recv(channel, request_headers, sizeof(request_headers), 0) == -1) {
      utils_logError("Failed to read bytes");
      close(server_socket);
      httpserver_cleanup(self, server_socket);
      exit(EXIT_FAILURE);
    }

    char url[256];
    
    sscanf(request_headers, "%*s %255s %*s\n", url);

    httpserver_Handlers requestHandler = self->findHandler(url, self);

    if (requestHandler == NULL) {
      endpoints_notFound(channel, request_headers);
    } else {
      requestHandler(channel, request_headers);
    }

    close(channel);

    if (!running) {
      break;
    }
  }

  close(server_socket);
  httpserver_cleanup(self, server_socket);
}

httpserver_HTTP* httpserver_configureServer() {
  httpserver_HTTP* http = (httpserver_HTTP*)malloc(sizeof(httpserver_HTTP));
  
  if (http == NULL) {
    utils_logError("Failed to allocate memory for http server\n");
    free(http);
    exit(EXIT_FAILURE);
  }

  *http = (httpserver_HTTP){
    .self = http,
    .port = 0,
    .registered_routes = 0,
    .handle = httpserver_handle,
    .findHandler = httpserver_findHandler,
    .listen = httpserver_listen,
  };

  for (int i=0; i < httpserver_MAX_ENDPOINTS; i++) {
    http->handlers[i] = NULL;
  }

  return http;
}

char* httpserver_retrieveHeader(const char* request_headers, const char* key) {
  const char* keyStart = strstr(request_headers, key);
  if (keyStart == NULL) return NULL;

  // Move the pointer to the start of the value and skip spaces and colon
  while (*keyStart != '\0' && (*keyStart == ' ' || *keyStart == ':')) {
    keyStart++;
  }

  const char* valueEnd = strchr(keyStart, '\n'); // Find the end of the value
  if (valueEnd == NULL) return NULL;

  size_t valueLength = valueEnd - keyStart;
  char* value = (char*)malloc(valueLength + 1);
  strncpy(value, keyStart, valueLength);
  value[valueLength] = '\0'; // Null-terminate the string

  return value;
};