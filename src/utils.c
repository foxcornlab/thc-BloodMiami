#include "../include/bloodmiami.h"

char *string_replace(const char *original, const char *pattern, const char *replacement) {
    size_t pattern_len = strlen(pattern);
    size_t replacement_len = strlen(replacement);
    size_t original_len = strlen(original);
    
    int count = 0;
    const char *temp = original;
    while ((temp = strstr(temp, pattern)) != NULL) {
        count++;
        temp += pattern_len;
    }
    
    if (count == 0) {
        return strdup(original);
    }
    
    size_t new_len = original_len + count * (replacement_len - pattern_len);
    char *result = malloc(new_len + 1);
    
    if (!result) {
        return NULL;
    }
    
    char *dest = result;
    const char *src = original;
    
    while (*src) {
        if (strncmp(src, pattern, pattern_len) == 0) {
            strcpy(dest, replacement);
            dest += replacement_len;
            src += pattern_len;
        } else {
            *dest++ = *src++;
        }
    }
    
    *dest = '\0';
    return result;
}

int string_contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) {
        return 0;
    }
    
    char *haystack_lower = malloc(strlen(haystack) + 1);
    char *needle_lower = malloc(strlen(needle) + 1);
    
    if (!haystack_lower || !needle_lower) {
        free(haystack_lower);
        free(needle_lower);
        return 0;
    }
    
    for (size_t i = 0; i <= strlen(haystack); i++) {
        haystack_lower[i] = tolower(haystack[i]);
    }
    
    for (size_t i = 0; i <= strlen(needle); i++) {
        needle_lower[i] = tolower(needle[i]);
    }
    
    int result = strstr(haystack_lower, needle_lower) != NULL;
    
    free(haystack_lower);
    free(needle_lower);
    
    return result;
}

char *url_encode(const char *str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1); 
    
    if (!encoded) {
        return NULL;
    }
    
    char *output = encoded;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = str[i];
        
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            *output++ = c;
        } else {
            sprintf(output, "%%%02X", c);
            output += 3;
        }
    }
    
    *output = '\0';
    return encoded;
}

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}