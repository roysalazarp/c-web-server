#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void log_error (const char *message) {
    perror(message);
    fprintf(stderr, "Error code: %d\n", errno);
}

// reviewed ✅
char* retrieve_header (const char* request_headers, const char* key) {
    char* key_start = strstr(request_headers, key);
    if (key_start == NULL) return NULL;

    // Move the pointer to the start of the value and skip spaces and colon
    while (*key_start != '\0' && (*key_start == ' ' || *key_start == ':')) {
        key_start++;
    }

    char* value_end = strchr(key_start, '\n'); // Find the end of the value
    if (value_end == NULL) return NULL;

    size_t value_length = value_end - key_start;
    
    char* value = (char*)malloc(value_length + 1);
    if (value == NULL) {
        log_error("Failed to allocate memory\n");
        return NULL;
    }

    strncpy(value, key_start, value_length);
    value[value_length] = '\0'; // Null-terminate the string

    return value;
};