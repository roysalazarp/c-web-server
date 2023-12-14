#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* find_needle_address(const char* haystack, const char* needle, int start_position, int direction) {
    if (haystack == NULL || needle == NULL) {
        // Check for NULL pointers
        return NULL;
    }

    size_t haystack_length = strlen(haystack);
    size_t needle_length = strlen(needle);

    if (needle_length > haystack_length) {
        // If the needle is longer than the haystack, it cannot be present
        return NULL;
    }

    size_t end, step;

    if (direction > 0) {
        // Forward search
        end = haystack_length - needle_length;
        step = 1;
    } else {
        // Backward search
        end = 0;
        step = -1;
    }

    for (size_t i = start_position; i != end + step; i += step) {
        if (strncmp(haystack + i, needle, needle_length) == 0) {
            // Found the needle, return its address
            return (char*)(haystack + i);
        }
    }

    // Needle not found
    return NULL;
}

void replace_string(char* haystack, const char* begin_address, const char* end_address, const char* value) {
    if (haystack == NULL || begin_address == NULL || end_address == NULL || value == NULL) {
        // Check for NULL pointers
        return;
    }

    size_t beginIndex = begin_address - haystack;
    size_t endIndex = end_address - haystack;

    if (beginIndex >= endIndex || endIndex > strlen(haystack)) {
        // Invalid address range
        return;
    }

    size_t valueLength = strlen(value);

    // Calculate the length of the remaining string after the replacement
    size_t remainingLength = strlen(haystack + endIndex);

    // Shift the remaining part of the string to accommodate the new value
    memmove(haystack + beginIndex + valueLength, haystack + endIndex, remainingLength + 1);

    // Copy the new value into the specified range
    strncpy(haystack + beginIndex, value, valueLength);
}

int get_position(const char* str, const char* address) {
    if (str == NULL || address == NULL) {
        // Check for NULL pointers
        return -1; // Invalid position
    }

    // Calculate the position by subtracting the base address
    return address - str;
}


char* render_val(char* identifier, char* value, char* html) {
    char* identifier_address = find_needle_address(html, identifier, 0, 1);
    
    int s_a_pos = get_position(html, identifier_address);
    char* identifier_start_address = find_needle_address(html, "{{", s_a_pos, -1);

    int e_a_pos = get_position(html, identifier_start_address);
    char* identifier_end_address = find_needle_address(html, "}}", e_a_pos, 1);
    
    replace_string(html, identifier_start_address, identifier_end_address + 2, value);

    return html;
}
