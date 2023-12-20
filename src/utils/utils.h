#ifndef UTILS_H
#define UTILS_H

long int calculate_file_bytes_length (char* file_path);
int read_file (char* file_content, char* file_path, long file_size);
int build_template_path (char* buffer, const char* path);
size_t calculate_combined_length (int num_strings, ...);

void log_error(const char* message);
char* retrieve_header(const char* request_headers, const char* key);

#endif