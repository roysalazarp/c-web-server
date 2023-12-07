#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#define httpserver_MAX_CONNECTIONS 100
#define httpserver_MAX_URL_LENGTH 256
#define httpserver_MAX_ENDPOINTS 50

typedef struct httpserver_HTTP httpserver_HTTP;
typedef void (*httpserver_Handlers)(int channel, const char* request_headers);
typedef httpserver_Handlers (*httpserver_FindHandler)(const char* url, httpserver_HTTP* self);
typedef void (*httpserver_Handle)(const char* url, httpserver_Handlers fn, httpserver_HTTP* self);
typedef void (*httpserver_Listen)(unsigned int port, httpserver_HTTP* self);

struct httpserver_HTTP {
  httpserver_HTTP* self;
  httpserver_Handlers handlers[httpserver_MAX_ENDPOINTS];
  unsigned int registered_routes;
  unsigned int port;
  httpserver_FindHandler findHandler;
  httpserver_Handle handle;
  httpserver_Listen listen;
};

httpserver_HTTP* httpserver_configureServer();
char* httpserver_retrieveHeader(const char* request_headers, const char* key);
unsigned int httpserver_urlToIndex(const char* url);

#endif