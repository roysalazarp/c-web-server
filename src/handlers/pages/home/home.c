#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "router.h"

void home(int channel) {
  long max_len = pathconf(".", _PC_PATH_MAX); // Current environment maximum path length

  char* cwd = (char *)malloc((size_t)max_len);

  if (cwd == NULL) {
    perror("failed to allocate memory for cwd");
  }

  if (getcwd(cwd, (size_t)max_len) == NULL) {
    perror("getcwd() error");
  }

  char* templatePath = (char *)malloc((size_t)max_len);
  snprintf(templatePath, (size_t)max_len, "%s/templates/home.html", cwd);
  free(cwd);
  cwd = NULL;

  FILE* templateFile = fopen(templatePath, "r");
  free(templatePath);
  templatePath = NULL;

  if (templateFile == NULL) {
    perror("Error opening template file");
    exit(1);
  }

  // Determine the file size to allocate memory
  fseek(templateFile, 0, SEEK_END);
  long fileSize = ftell(templateFile);
  rewind(templateFile);

  // Allocate memory for the HTML template content
  char* templateContent = (char *)malloc((size_t)(fileSize + 1)); // +1 for null terminator

  if (templateContent == NULL) {
    perror("Memory allocation failed");
    fclose(templateFile);
    exit(1);
  }

  // Read the HTML template content into the allocated memory
  size_t bytesRead = fread(templateContent, sizeof(char), fileSize, templateFile);
  templateContent[bytesRead] = '\0'; // Null-terminate the string
  fclose(templateFile);

  size_t contentLength = strlen(templateContent) + 1;

  // Use the allocated memory for html
  char* html = (char *)malloc(contentLength);

  if (html == NULL) {
    perror("Memory allocation failed");
    free(templateContent);
    templateContent = NULL;
    exit(1);
  }

  snprintf(html, contentLength, "%s", templateContent);

  char headers[100] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  size_t httpResponseLength = strlen(templateContent) + strlen(headers);
  free(templateContent);
  templateContent = NULL;

  // Allocate memory for httpResponse
  char* httpResponse = (char *)malloc(httpResponseLength);

  if (httpResponse == NULL) {
      perror("Memory allocation failed");
      free(html);
      html = NULL;
      exit(1);
  }

  // Generate the HTTP response
  snprintf(httpResponse, httpResponseLength, "%s%s", headers, html);  
  free(html);
  html = NULL;

  // Send the HTTP response
  send(channel, httpResponse, httpResponseLength, 0);
  free(httpResponse);
  httpResponse = NULL;
}