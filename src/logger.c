#include "../include/bloodmiami.h"
#include <stdarg.h>

static FILE *log_file = NULL;

int log_open_file(const char *path) {
    if (log_file) {
        return 0;
    }
    log_file = fopen(path, "w");
    if (!log_file) {
        fprintf(stderr, "Cannot open output file: %s\n", path);
        return -1;
    }
    return 0;
}

void log_close_file(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_message(log_level_t level, const char *format, ...) {
    va_list args;
    time_t now;
    struct tm *tm_info;
    char time_buffer[20];
    const char *color = "";
    const char *prefix = "";
    
    time(&now);
    tm_info = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm_info);
    
    switch (level) {
        case LOG_ERROR:
            color = COLOR_RED;
            prefix = "[-]";
            break;
        case LOG_WARNING:
            color = COLOR_YELLOW;
            prefix = "[!]";
            break;
        case LOG_INFO:
            color = COLOR_BLUE;
            prefix = "[*]";
            break;
        case LOG_SUCCESS:
            color = COLOR_GREEN;
            prefix = "[+]";
            break;
        case LOG_VULN:
            color = COLOR_BOLD COLOR_RED;
            prefix = "[!]";
            break;
    }
    
    printf("%s%s%s %s[%s]%s ", color, prefix, COLOR_RESET, 
           COLOR_WHITE, time_buffer, COLOR_RESET);
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    
    if (log_file) {
        fprintf(log_file, "%s [%s] ", prefix, time_buffer);
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

void log_vulnerability(const char *payload, const char *response, const char *vuln_type) {
    log_message(LOG_VULN, "VULNERABLE: %s detected", vuln_type);
    printf("    %sPayload:%s %s\n", COLOR_YELLOW, COLOR_RESET, payload);
    
    char truncated_response[256];
    if (strlen(response) > 200) {
        strncpy(truncated_response, response, 200);
        truncated_response[200] = '\0';
        strcat(truncated_response, "...");
    } else {
        strcpy(truncated_response, response);
    }
    
    printf("    %sResponse:%s %s\n", COLOR_CYAN, COLOR_RESET, truncated_response);
}

void log_progress(int current, int total) {
    int percentage = (current * 100) / total;
    int bar_length = 50;
    int filled = (percentage * bar_length) / 100;
    
    printf("\r%s[*]%s Progress: [", COLOR_BLUE, COLOR_RESET);
    
    for (int i = 0; i < bar_length; i++) {
        if (i < filled) {
            printf("%s█%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf("░");
        }
    }
    
    printf("] %d%% (%d/%d)", percentage, current, total);
    fflush(stdout);
    
    if (current == total) {
        printf("\n");
    }
}

void log_results(int vulnerabilities, double elapsed_time) {
    printf("\n");
    log_message(LOG_SUCCESS, "Scan completed: %d vulnerabilities found in %.1f seconds", 
               vulnerabilities, elapsed_time);
    
    if (vulnerabilities > 0) {
        log_message(LOG_WARNING, "Target appears to be vulnerable to injection attacks");
    } else {
        log_message(LOG_INFO, "No vulnerabilities detected with current payloads");
    }
}