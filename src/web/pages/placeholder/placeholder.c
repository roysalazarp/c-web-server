#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/limits.h>

#include "http_server/http_server.h"
#include "utils/utils.h"

#define HEADERS_BUFFER_SIZE 4096

char* readTemplateFile(const char* filePath) {
  FILE* file = fopen(filePath, "r");
  if (file == NULL) {
    utils_logError("Error opening template file");
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    utils_logError("Failed to seek to a certain position on STREAM");
    fclose(file);
    return NULL;
  }

  long fileSize = ftell(file);
  if (fileSize == -1) {
    utils_logError("Failed to return the current position of STREAM");
    fclose(file);
    return NULL;
  }
  
  rewind(file);

  char* content = (char*)malloc((size_t)(fileSize + 1));

  size_t bytesRead = fread(content, sizeof(char), fileSize, file);

  if (ferror(file) != 0) {
    utils_logError("Error reading from file");
    fclose(file);
    return NULL;
  }

  content[bytesRead] = '\0'; // Null-terminate the string
  fclose(file);

  return content;
}

char* buildTemplatePath(const char* path) {
  char resolvedPath[PATH_MAX];

  if (realpath(".", resolvedPath) == NULL) {
    utils_logError("Failed to get the absolute path of the current working directory");
    return NULL;
  }

  char* templatePath = (char*)malloc(PATH_MAX);
  if (templatePath == NULL) {
    utils_logError("Memory allocation failed");
    return NULL;
  }

  if (snprintf(templatePath, PATH_MAX, "%s/%s", resolvedPath, path) >= PATH_MAX) {
    utils_logError("Formatted string truncated");
    return NULL;
  }

  return templatePath;
}

char* buildHTTPResponse(char* content, const char* headers) {
  size_t responseLength = strlen(content) + strlen(headers);

  char* httpResponse = (char *)malloc(responseLength);
  if (httpResponse == NULL) {
    utils_logError("Memory allocation failed");
    free(content);
    content = NULL;
    exit(EXIT_FAILURE);
  }

  // Generate the HTTP response
  snprintf(httpResponse, responseLength, "%s%s", headers, content);

  return httpResponse;
}

void endpoints_placeholder(int channel, const char* request_headers) {  
  char method[10];
  char url[256];
  char protocol[10];
  
  sscanf(request_headers, "%9s %255s %9s\n", method, url, protocol);
  char* host = httpserver_retrieveHeader(request_headers, "Host");
  printf("%s\n", host);

  free(host);
  
  char* templatePath = buildTemplatePath("src/web/pages/placeholder/placeholder.html");
  if (templatePath == NULL) {
    utils_logError("Failed to build template path");
    free(templatePath);
    exit(EXIT_FAILURE);
  }

  char* templateContent = readTemplateFile(templatePath);
  free(templatePath);
  if (templateContent == NULL) {
    utils_logError("Failed to read template file");
    exit(EXIT_FAILURE);
  }

  char headers[100] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

  char* httpResponse = buildHTTPResponse(templateContent, headers);
  free(templateContent);
  size_t httpResponseLength = strlen(httpResponse);
  
  // Send the HTTP response

  if (send(channel, httpResponse, httpResponseLength, 0) == -1) {
    utils_logError("Failed send buffer");
    free(httpResponse);
    exit(EXIT_FAILURE);
  }

  free(httpResponse);
}