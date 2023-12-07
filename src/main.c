#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "http_server/http_server.h"
#include "web/endpoints.h"

int main() {
  
  httpserver_HTTP* httpServer = httpserver_configureServer();

  httpServer->handle("/", endpoints_placeholder, httpServer);
  httpServer->handle("/account", endpoints_placeholder, httpServer);
  httpServer->handle("/profile", endpoints_placeholder, httpServer);
  httpServer->handle("/profile/user", endpoints_placeholder, httpServer);
  httpServer->handle("/profile/user/{id}", endpoints_placeholder, httpServer);
  
  httpServer->listen(8080, httpServer);
}