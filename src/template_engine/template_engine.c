#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"

char* find_needle_address (const char* haystack, const char* needle, int start_from, int direction) {
    if (haystack == NULL || needle == NULL) {
        return NULL;
    }

    size_t haystack_length = strlen(haystack);
    size_t needle_length = strlen(needle);

    if (needle_length > haystack_length) {
        log_error("Needle can't be greater than haystack");
        return NULL;
    }

    int end = 0;
    int step = 0;

    if (direction > 0) {
        // Forward search
        end = haystack_length;
        step = 1;
    } else {
        // Backward search
        end = 0;
        step = -1;
    }

    for (int i = start_from; i != end; i += step) {
        if (strncmp(haystack + i, needle, needle_length) == 0) {
            // Found the needle, return its address
            return (char*)(haystack + i);
        }
    }

    log_error("Couldn't find the needle");
    return NULL;
}


int replace_string (char* haystack, const char* begin_address, const char* end_address, const char* value) {
    if (haystack == NULL || begin_address == NULL || end_address == NULL || value == NULL) {
        return -1;
    }

    size_t begin_index = begin_address - haystack;
    size_t end_index = end_address - haystack;

    // int space = end_address - begin_address;

    if (begin_index >= end_index || end_index > strlen(haystack)) {
        log_error("Something is wrong with the indexes");
        return -1;
    }

    size_t value_length = strlen(value);
    
    size_t remaining_haystack_length = strlen(haystack + end_index) + 1; // strncpy will add null terminator

    // TODO: Check for errors
    // Shift the remaining part of the string to accommodate the new value
    memmove(haystack + begin_index + value_length, haystack + end_index, remaining_haystack_length);

    // TODO: Check for errors
    // Copy the new value into the specified range
    strncpy(haystack + begin_index, value, value_length);

    return 0;
}

// reviewed ✅
char* make_end_keyword (char* kw) {
    const char* prefix = "end ";
    
    size_t end_keyword_length = strlen(prefix) + strlen(kw) + 1;
    char* end_keyword = (char*)malloc(end_keyword_length);

    if (end_keyword == NULL) {
        log_error("Memory allocation failed\n");
        return NULL;
    }

    sprintf(end_keyword, "%s%s", prefix, kw);

    return end_keyword;
}

// reviewed ✅
char* copy_substring (const char* source, const char* start, const char* end) {
    size_t length = end - start;

    char* result = (char*)malloc(length + 1);

    if (result == NULL) {
        log_error("Memory allocation failed\n");
        return NULL;
    }

    strncpy(result, start, length);
    result[length] = '\0'; // <-- combining multiple string that end in \0 may cause a problem

    return result;
}

// reviewed ✅
char** make_multiple_copies (const char* source, size_t num_copies) {
    // Allocate memory for an array of strings
    char** copies = (char**)malloc(num_copies * sizeof(char*));

    // Check if memory allocation is successful
    if (copies == NULL) {
        log_error("Memory allocation failed\n");
        return NULL;
    }

    // Allocate memory and copy the source string for each copy
    for (size_t i = 0; i < num_copies; ++i) {
        size_t length = strlen(source) + 1;  // Include space for the null terminator
        copies[i] = (char*)malloc(length);

        // Check if memory allocation for the copy is successful
        if (copies[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for copy %zu\n", i);

            // Clean up previously allocated memory
            for (size_t j = 0; j < i; ++j) {
                free(copies[j]);
            }

            free(copies);
            return NULL;
        }

        // Copy the source string to the new memory
        strcpy(copies[i], source);
    }

    return copies;
}

// reviewed ✅
void free_multiple_copies(char** copies, size_t num_copies) {
    for (size_t i = 0; i < num_copies; ++i) {
        free(copies[i]);
    }
    free(copies);
}

// reviewed ✅
char* concatenate_strings (char** string_array, size_t length) {
    if (string_array == NULL) return NULL;

    // Calculate the total length of the concatenated string
    size_t total_length = 0;
    for (int i = 0; i < length; ++i) {
        total_length += strlen(string_array[i]);
    }

    // Allocate memory for the concatenated string (including space for null terminators)
    char* result = (char*)malloc(total_length + 1);

    // Check if memory allocation is successful
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Concatenate the strings into the result buffer
    result[0] = '\0';  // Start with an empty string
    for (int i = 0; i < length; ++i) {
        strcat(result, string_array[i]);
    }

    return result;
}


int render_val (char* val_keyword, char* value, char* template) {
    char* keyword_address = find_needle_address(template, val_keyword, 0, 1);
    if (keyword_address == NULL) {
        return -1;
    }
    
    int keyword_position_within_template = keyword_address - template;
    
    char* start_braces_address = find_needle_address(template, "{{", keyword_position_within_template, -1);
    if (start_braces_address == NULL) {
        return -1;
    }

    char* end_braces_address = find_needle_address(template, "}}", keyword_position_within_template, 1);
    if (end_braces_address == NULL) {
        return -1;
    }
    
    if (replace_string(template, start_braces_address, end_braces_address + 2, value) == -1) {
        return -1;
    }

    return 0;
}

// reviewed ✅
char* render_for (char* list[], char* template, int list_length) {
    char* location_keyword = list[0];
    char* val_keyword = list[1];
    
    // find the start of the for loop
    char* keyword_address = find_needle_address(template, location_keyword, 0, 1);

    int keyword_position_within_template = keyword_address - template;

    char* start_braces_address = find_needle_address(template, "{{", keyword_position_within_template, -1);
    char* end_braces_address = find_needle_address(template, "}}", keyword_position_within_template, 1);

    // find the start of the for loop
    char* end_keyword = make_end_keyword(location_keyword);
    if (end_keyword == NULL) {
        return NULL;
    }

    char* end_keyword_address = find_needle_address(template, end_keyword, 0, 1);
    free(end_keyword);

    int end_keyword_position_within_template = end_keyword_address - template;

    char* e_start_braces_address = find_needle_address(template, "{{", end_keyword_position_within_template, -1);
    char* e_end_braces_address = find_needle_address(template, "}}", end_keyword_position_within_template, 1);

    char* for_section = copy_substring(template, end_braces_address + 2, e_start_braces_address);
    if (end_keyword == NULL) {
        return NULL;
    }

    size_t values_amount = 0;
    size_t start = 2;

    while (start < list_length) {
        values_amount++;
        start++;
    }

    char** for_list = make_multiple_copies(for_section, values_amount);
    free(for_section);
    if (for_list == NULL) {
        return NULL;
    }

    for (int i = 0; i < values_amount; ++i) {
        render_val(val_keyword, list[i], for_list[i]);
    }

    char* rendered_for = concatenate_strings(for_list, values_amount);
    free_multiple_copies(for_list, values_amount);
    if (rendered_for == NULL) {
        return NULL;
    }

    replace_string(template, start_braces_address, e_end_braces_address + 2, rendered_for);
    free(rendered_for);

    return template;
}