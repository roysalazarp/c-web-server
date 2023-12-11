#ifndef UTILS_H
#define UTILS_H

void log_error(const char* message);
char* retrieve_header(const char* request_headers, const char* key);

#endif