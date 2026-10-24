#include "../include/bloodmiami.h"

static int http_initialized = 0;

int http_init(void) {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return -1;
    }
    http_initialized = 1;
    return 0;
}

void http_cleanup(void) {
    if (http_initialized) {
        curl_global_cleanup();
        http_initialized = 0;
    }
}

size_t write_callback(void *contents, size_t size, size_t nmemb, http_response_t *response) {
    size_t total_size = size * nmemb;
    size_t new_size = response->size + total_size;
    
    // Reallocate buffer
    char *new_data = realloc(response->data, new_size + 1);
    if (!new_data) {
        return 0; // Out of memory
    }
    
    response->data = new_data;
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size = new_size;
    response->data[response->size] = '\0';
    
    return total_size;
}

/* Escape a payload so it can be embedded safely in a JSON string value.
 * Handles quotes, backslashes, control chars and multi-byte UTF-8. */
static char *json_escape(const char *str) {
    if (!str) {
        return strdup("");
    }
    size_t len = strlen(str);
    /* worst case: every byte becomes \uXXXX (6 chars) */
    char *out = malloc(len * 6 + 1);
    if (!out) {
        return NULL;
    }
    char *p = out;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        switch (c) {
            case '"':  *p++ = '\\'; *p++ = '"';  break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
                if (c < 0x20) {
                    sprintf(p, "\\u%04x", c);
                    p += 6;
                } else {
                    *p++ = (char)c;
                }
        }
    }
    *p = '\0';
    return out;
}

http_response_t *http_request(const char *url, const char *payload, config_t *config) {
    CURL *curl;
    CURLcode res;
    http_response_t *response;
    struct curl_slist *headers = NULL;
    char *post_data = NULL;
    
    // Allocate response structure
    response = malloc(sizeof(http_response_t));
    if (!response) {
        return NULL;
    }
    
    response->data = malloc(1);
    response->size = 0;
    response->response_code = 0;
    response->data[0] = '\0';
    
    curl = curl_easy_init();
    if (!curl) {
        free(response->data);
        free(response);
        return NULL;
    }
    
    // Basic options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config->user_agent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Add custom headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    if (config->custom_headers) {
        for (int i = 0; i < config->header_count; i++) {
            headers = curl_slist_append(headers, config->custom_headers[i]);
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Prepare POST data with payload
    if (payload) {
        // Create JSON payload for chat APIs; escape the payload first so
        // quotes/backslashes in it (e.g. NoSQL {"$ne": null}) keep the
        // JSON body valid.
        char *escaped = json_escape(payload);
        if (escaped) {
            size_t json_size = strlen(escaped) + 200;
            post_data = malloc(json_size);
            if (post_data) {
                snprintf(post_data, json_size,
                        "{\"message\":\"%s\",\"model\":\"gpt-3.5-turbo\",\"temperature\":0.7}",
                        escaped);
            }
            free(escaped);
        }

        if (post_data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
        }
    }
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->response_code);
    } else {
        response->response_code = 0;
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (post_data) {
        free(post_data);
    }
    
    return response;
}

void http_response_free(http_response_t *response) {
    if (response) {
        if (response->data) {
            free(response->data);
        }
        free(response);
    }
}