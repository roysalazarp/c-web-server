#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"

char *find_needle_address (const char *haystack, const char *needle, int start_from, int direction) {
    if (haystack == NULL || needle == NULL) {
        return NULL;
    }

    size_t haystack_length = strlen(haystack) + 1;
    size_t needle_length = strlen(needle); // we only want characters

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

// null-terminates haystack after modifying
int replace_string (char *haystack, const char *begin_address, const char *end_address, const char *value) {
    if (haystack == NULL || begin_address == NULL || end_address == NULL || value == NULL) {
        return -1;
    }

    size_t begin_index = begin_address - haystack;
    size_t end_index = end_address - haystack;

    if (begin_index >= end_index || end_index > strlen(haystack)) {
        log_error("Something is wrong with the indexes");
        return -1;
    }

    size_t value_length = strlen(value); // we only want characters
    size_t remaining_haystack_length = strlen(haystack + end_index) + 1;

    // TODO: Check for errors
    // Shift the remaining part of the string to accommodate the new value
    memmove(haystack + begin_index + value_length, haystack + end_index, remaining_haystack_length);

    // TODO: Check for errors
    // Copy the new value into the specified range
    strncpy(haystack + begin_index, value, value_length);

    haystack[begin_index + value_length + remaining_haystack_length] = '\0';

    return 0;
}


// Frees dest memory on failure
int make_multiple_copies (char** dest, size_t num_copies, const char *source) {
    for (size_t i = 0; i < num_copies; ++i) {
        size_t source_lenght = strlen(source);
        dest[i] = (char*)malloc(source_lenght * sizeof(char) + 1);

        if (dest[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for copy %zu\n", i);

            // Clean up previously allocated memory
            for (size_t j = 0; j < i; ++j) {
                free(dest[j]);
                dest[j] = NULL;
            }

            free(dest);
            dest = NULL;
            return -1;
        }

        dest[i][0] = '\0';
        strcpy(dest[i], source);

        dest[i][source_lenght] = '\0';
    }

    return 0;
}

void free_multiple_copies (char** copies, size_t num_copies) {
    for (size_t i = 0; i < num_copies; ++i) {
        free(copies[i]);
        copies[i] = NULL;
    }

    free(copies);
    copies = NULL;
}

int render_val (char *val_keyword, char *value, char *template) {
    char *keyword_address = find_needle_address(template, val_keyword, 0, 1);
    if (keyword_address == NULL) {
        return -1;
    }
    
    int keyword_position_within_template = keyword_address - template;
    
    char *start_braces_address = find_needle_address(template, "{{", keyword_position_within_template, -1);
    if (start_braces_address == NULL) {
        return -1;
    }

    char *end_braces_address = find_needle_address(template, "}}", keyword_position_within_template, 1);
    if (end_braces_address == NULL) {
        return -1;
    }
    
    if (replace_string(template, start_braces_address, end_braces_address + 2, value) == -1) {
        return -1;
    }

    return 0;
}

int render_for (char *list[], char *template, int list_length) {
    char *location_keyword = list[0];
    char *val_keyword = list[1];
    
    char *keyword_address = find_needle_address(template, location_keyword, 0, 1);
    int keyword_position_within_template = keyword_address - template;

    char *start_braces_address = find_needle_address(template, "{{", keyword_position_within_template, -1);
    char *end_braces_address = find_needle_address(template, "}}", keyword_position_within_template, 1);

    const char *prefix = "end ";
    size_t end_keyword_length = strlen(prefix) + strlen(location_keyword);
    char *end_keyword;
    end_keyword = (char*)malloc(end_keyword_length * (sizeof *end_keyword) + 1);
    if (end_keyword == NULL) {
        log_error("Failed to allocate memory for end_keyword\n");
        return -1;
    }

    end_keyword[0] = '\0';

    if (sprintf(end_keyword, "%s%s", prefix, location_keyword) < 0) {
        log_error("Failed to format string\n");
        free(end_keyword);
        end_keyword = NULL;
        return -1;
    }

    char *end_keyword_address = find_needle_address(template, end_keyword, keyword_position_within_template, 1);
    free(end_keyword);
    end_keyword = NULL;

    int end_keyword_position_within_template = end_keyword_address - template;

    char *e_start_braces_address = find_needle_address(template, "{{", end_keyword_position_within_template, -1);
    char *e_end_braces_address = find_needle_address(template, "}}", end_keyword_position_within_template, 1);

    size_t length = e_start_braces_address - (end_braces_address + 2);

    char *for_template;
    for_template = (char*)malloc(length * (sizeof *for_template) + 1);
    if (for_template == NULL) {
        log_error("Failed to allocate memory for for_template\n");
        return -1;
    }

    for_template[0] = '\0';

    if (strncpy(for_template, end_braces_address + 2, length) == NULL) {
        log_error("Failed copy string\n");
        free(for_template);
        for_template = NULL;
        return -1;
    }

    for_template[length] = '\0';

    size_t for_length = 0;
    
    for (int i = 2; list[i] != NULL; i++) {
        for_length++;
    }

    char** for_items = (char**)malloc(for_length * sizeof(char*));
    if (for_items == NULL) {
        log_error("Failed to allocate memory for for_items\n");
        free(for_template);
        for_template = NULL;
        return -1;
    }

    for (size_t i = 0; i < for_length; i++) {
        for_items[i] = NULL;
    }
    
    if (make_multiple_copies(for_items, for_length, for_template) == -1) {
        log_error("Failed to make for_items\n");
        free(for_template);
        for_template = NULL;
        return -1;
    }

    free(for_template);
    for_template = NULL;

    for (int i = 0; i < for_length; ++i) {
        int ri = i + 2;

        if (render_val(val_keyword, list[ri], for_items[i]) == -1) {
            // Clean up previously allocated memory
            for (size_t j = 0; j < i; ++j) {
                free(for_items[j]);
                for_items[j] = NULL;
            }
            free(for_items);
            for_items = NULL;
            return -1;
        }
    }

    size_t total_length = 0;
    for (int i = 0; i < for_length; ++i) {
        total_length += strlen(for_items[i]);
    }

    char *rendered_for;
    rendered_for = (char*)malloc(total_length * (sizeof *rendered_for) + 1);
    if (rendered_for == NULL) {
        log_error("Failed to allocate memory for rendered_for\n");
        return -1;
    }

    rendered_for[0] = '\0'; 

    for (int i = 0; i < for_length; ++i) {
        // TODO: Check for strcat errors
        // strcat will do its work before the null-terminator
        strcat(rendered_for, for_items[i]);
    }

    free_multiple_copies(for_items, for_length);

    if (replace_string(template, start_braces_address, e_end_braces_address + 2, rendered_for) == -1) {
        free(rendered_for);
        rendered_for = NULL;
        return -1;
    }

    free(rendered_for);
    return 0;
}