#include "../include/bloodmiami.h"

static int vulnerabilities_found = 0;
static pthread_mutex_t vuln_mutex = PTHREAD_MUTEX_INITIALIZER;

int bloodmiami_init(config_t *config) {
    log_message(LOG_INFO, "Initializing BloodMiami v%s", VERSION);
    
    if (http_init() != 0) {
        log_message(LOG_ERROR, "Failed to initialize HTTP library");
        return -1;
    }
    
    if (!config->target_url) {
        log_message(LOG_ERROR, "Target URL is required");
        return -1;
    }
    
    if (!config->dictionary_file) {
        log_message(LOG_ERROR, "Dictionary file is required");
        return -1;
    }
    
    FILE *dict_file = fopen(config->dictionary_file, "r");
    if (!dict_file) {
        log_message(LOG_ERROR, "Cannot open dictionary file: %s", config->dictionary_file);
        return -1;
    }
    fclose(dict_file);

    if (config->output_file) {
        if (log_open_file(config->output_file) != 0) {
            return -1;
        }
        log_message(LOG_INFO, "Results will be saved to: %s", config->output_file);
    }

    log_message(LOG_SUCCESS, "BloodMiami initialized successfully");
    return 0;
}

int bloodmiami_run(config_t *config) {
    payload_t *payloads = NULL;
    int payload_count = 0;
    struct timespec start_time, end_time;
    
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    log_message(LOG_INFO, "Target: %s", config->target_url);
    log_message(LOG_INFO, "Injection Type: %s", 
               config->injection_type == INJECT_LANGUAGE ? "Language Injection" : "Prompt Injection");
    
    if (load_payloads(config->dictionary_file, &payloads, &payload_count) != 0) {
        log_message(LOG_ERROR, "Failed to load payloads from dictionary");
        return -1;
    }
    
    log_message(LOG_INFO, "Dictionary: %s (%d payloads)", config->dictionary_file, payload_count);
    log_message(LOG_INFO, "Starting injection tests with %d concurrent threads", config->concurrency);
    
    pthread_t *threads = malloc(config->concurrency * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(payload_count * sizeof(thread_data_t));
    
    if (!threads || !thread_data) {
        log_message(LOG_ERROR, "Failed to allocate memory for threads");
        free_payloads(payloads, payload_count);
        return -1;
    }
    
    vulnerabilities_found = 0;
    
    int active_threads = 0;
    for (int i = 0; i < payload_count; i++) {
        if (active_threads >= config->concurrency) {
            pthread_join(threads[active_threads % config->concurrency], NULL);
            active_threads--;
        }
        
        thread_data[i].config = config;
        thread_data[i].payload = &payloads[i];
        thread_data[i].thread_id = i;
        thread_data[i].total_vulns = &vulnerabilities_found;
        thread_data[i].mutex = &vuln_mutex;
        
        if (pthread_create(&threads[active_threads % config->concurrency], NULL, 
                          test_worker, &thread_data[i]) != 0) {
            log_message(LOG_ERROR, "Failed to create thread %d", i);
            continue;
        }
        
        active_threads++;
        
        if (config->verbose) {
            log_progress(i + 1, payload_count);
        }
    }
    
    for (int i = 0; i < (active_threads < config->concurrency ? active_threads : config->concurrency); i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = get_time_diff(start_time, end_time);
    
    log_results(vulnerabilities_found, elapsed);
    
    free(threads);
    free(thread_data);
    free_payloads(payloads, payload_count);
    
    return 0;
}

void *test_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int result = 0;
    
    switch (data->config->injection_type) {
        case INJECT_LANGUAGE:
            result = inject_language_test(data->config, data->payload);
            break;
        case INJECT_PROMPT:
            result = inject_prompt_test(data->config, data->payload);
            break;
        default:
            log_message(LOG_ERROR, "Unknown injection type");
            return NULL;
    }
    
    if (result > 0) {
        pthread_mutex_lock(data->mutex);
        (*data->total_vulns)++;
        pthread_mutex_unlock(data->mutex);
    }
    
    return NULL;
}

int load_payloads(const char *filename, payload_t **payloads, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    char line[MAX_PAYLOAD_LENGTH];
    int capacity = 100;
    *payloads = malloc(capacity * sizeof(payload_t));
    *count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        if (*count >= capacity) {
            capacity *= 2;
            *payloads = realloc(*payloads, capacity * sizeof(payload_t));
        }
        
        (*payloads)[*count].payload = strdup(line);
        (*payloads)[*count].description = strdup("Generic payload");
        (*payloads)[*count].severity = 1;
        (*count)++;
    }
    
    fclose(file);
    return 0;
}

void free_payloads(payload_t *payloads, int count) {
    for (int i = 0; i < count; i++) {
        free(payloads[i].payload);
        free(payloads[i].description);
    }
    free(payloads);
}

void bloodmiami_cleanup(config_t *config) {
    log_close_file();
    http_cleanup();
    
    if (config->custom_headers) {
        for (int i = 0; i < config->header_count; i++) {
            free(config->custom_headers[i]);
        }
        free(config->custom_headers);
    }
    
    log_message(LOG_INFO, "BloodMiami cleanup completed");
}