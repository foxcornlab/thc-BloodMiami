#include "../include/bloodmiami.h"

static const char *sql_errors[] = {
    "mysql_fetch_array",
    "ORA-01756",
    "Microsoft OLE DB Provider for ODBC Drivers",
    "PostgreSQL query failed",
    "Warning: mysql_",
    "MySQLSyntaxErrorException",
    "valid MySQL result",
    "Unclosed quotation mark",
    "Microsoft JET Database",
    NULL
};

static const char *nosql_errors[] = {
    "MongoError",
    "CouchDB",
    "Redis",
    "Cassandra",
    "MongoDB",
    "WriteResult",
    "BSONObj",
    NULL
};

static const char *xss_indicators[] = {
    "<script>",
    "javascript:",
    "onerror=",
    "onload=",
    "alert(",
    "document.cookie",
    "window.location",
    NULL
};

static const char *js_errors[] = {
    "SyntaxError",
    "ReferenceError",
    "TypeError",
    "eval(",
    "Function(",
    "document.write",
    NULL
};

static int check_vulnerability(const char *response, const char **patterns, const char *vuln_type) {
    if (!response || !patterns) {
        return 0;
    }

    (void)vuln_type; /* used by callers for the report line, kept for API symmetry */
    
    for (int i = 0; patterns[i] != NULL; i++) {
        if (string_contains(response, patterns[i])) {
            return 1;
        }
    }
    
    return 0;
}

int inject_language_test(config_t *config, payload_t *payload) {
    http_response_t *response;
    int vulnerability_found = 0;
    
    if (config->verbose) {
        log_message(LOG_INFO, "Testing payload: %s", payload->payload);
    }
    
    response = http_request(config->target_url, payload->payload, config);
    
    if (!response) {
        if (config->verbose) {
            log_message(LOG_ERROR, "Failed to send request for payload: %s", payload->payload);
        }
        return 0;
    }
    
    if (response->response_code == 0) {
        if (config->verbose) {
            log_message(LOG_WARNING, "No response received for payload: %s", payload->payload);
        }
        http_response_free(response);
        return 0;
    }
    
    if (check_vulnerability(response->data, sql_errors, "SQL Injection")) {
        log_vulnerability(payload->payload, response->data, "SQL Injection");
        vulnerability_found = 1;
    }
    else if (check_vulnerability(response->data, nosql_errors, "NoSQL Injection")) {
        log_vulnerability(payload->payload, response->data, "NoSQL Injection");
        vulnerability_found = 1;
    }
    else if (check_vulnerability(response->data, xss_indicators, "XSS")) {
        log_vulnerability(payload->payload, response->data, "Cross-Site Scripting (XSS)");
        vulnerability_found = 1;
    }
    else if (check_vulnerability(response->data, js_errors, "JavaScript Injection")) {
        log_vulnerability(payload->payload, response->data, "JavaScript Injection");
        vulnerability_found = 1;
    }
    else if (response->response_code >= 500) {
        log_vulnerability(payload->payload, "Server Error 500", "Potential Injection (Server Error)");
        vulnerability_found = 1;
    }
    else if (strlen(response->data) == 0 && response->response_code == 200) {
        if (config->verbose) {
            log_message(LOG_WARNING, "Empty response for payload: %s", payload->payload);
        }
    }
    
    http_response_free(response);
    return vulnerability_found;
}