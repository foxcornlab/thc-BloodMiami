#ifndef BLOODMIAMI_H
#define BLOODMIAMI_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <curl/curl.h>
#include <pthread.h>

#define VERSION "1.0.0"
#define MAX_URL_LENGTH 2048
#define MAX_PAYLOAD_LENGTH 4096
#define MAX_RESPONSE_LENGTH 65536
#define MAX_HEADER_LENGTH 1024
#define DEFAULT_TIMEOUT 5
#define DEFAULT_CONCURRENCY 10
#define MAX_THREADS 50

typedef enum {
    INJECT_LANGUAGE = 'L',
    INJECT_PROMPT = 'P'
} injection_type_t;

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

typedef enum {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_SUCCESS,
    LOG_VULN
} log_level_t;

typedef struct {
    char *data;
    size_t size;
    long response_code;
} http_response_t;

typedef struct {
    char *target_url;
    char *dictionary_file;
    char *output_file;
    char *user_agent;
    char **custom_headers;
    int header_count;
    injection_type_t injection_type;
    int timeout;
    int concurrency;
    int verbose;
} config_t;

typedef struct {
    char *payload;
    char *description;
    int severity;
} payload_t;

typedef struct {
    config_t *config;
    payload_t *payload;
    int thread_id;
    int *total_vulns;
    pthread_mutex_t *mutex;
} thread_data_t;


int bloodmiami_init(config_t *config);
int bloodmiami_run(config_t *config);
void bloodmiami_cleanup(config_t *config);
int load_payloads(const char *filename, payload_t **payloads, int *count);
void free_payloads(payload_t *payloads, int count);

int parse_arguments(int argc, char *argv[], config_t *config);
void print_usage(const char *program_name);
void print_version(void);
void print_banner(void);

void log_message(log_level_t level, const char *format, ...);
void log_vulnerability(const char *payload, const char *response, const char *vuln_type);
void log_progress(int current, int total);
void log_results(int vulnerabilities, double elapsed_time);
int log_open_file(const char *path);
void log_close_file(void);

int http_init(void);
void http_cleanup(void);
http_response_t *http_request(const char *url, const char *payload, config_t *config);
void http_response_free(http_response_t *response);
size_t write_callback(void *contents, size_t size, size_t nmemb, http_response_t *response);

int inject_language_test(config_t *config, payload_t *payload);
int inject_prompt_test(config_t *config, payload_t *payload);

void *test_worker(void *arg);

char *string_replace(const char *original, const char *pattern, const char *replacement);
int string_contains(const char *haystack, const char *needle);
char *url_encode(const char *str);
double get_time_diff(struct timespec start, struct timespec end);

#endif 