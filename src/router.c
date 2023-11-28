#include <stdio.h>
#include <string.h>

#include "router.h"

typedef struct {
  char path[256];
  char method[10];
  void (*handler)(int channel);
} route;

route routes[] = {
  {"/", "GET", home}
};

void route_request(int channel, const char* method, const char* url) {
  for (int i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {  
    if (strcmp(routes[i].path, url) == 0 && strcmp(routes[i].method, method) == 0) {
      // Found a matching route, call the handler function
      routes[i].handler(channel);
      return;
    }
  }

  // Handle a 404 Not Found error if no matching route is found.
  not_found(channel);
}