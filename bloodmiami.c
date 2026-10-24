#include "include/bloodmiami.h"

int main(int argc, char *argv[]) {
    config_t config = {0};
    int result = 0;
    
    config.timeout = DEFAULT_TIMEOUT;
    config.concurrency = DEFAULT_CONCURRENCY;
    config.user_agent = "BloodMiami/1.0.0";
    
    print_banner();

    if (parse_arguments(argc, argv, &config) != 0) {
        return EXIT_FAILURE;
    }
    
    if (bloodmiami_init(&config) != 0) {
        log_message(LOG_ERROR, "Failed to initialize BloodMiami");
        return EXIT_FAILURE;
    }
    
    result = bloodmiami_run(&config);
    
    bloodmiami_cleanup(&config);
    
    return result;
}