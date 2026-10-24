#include "../include/bloodmiami.h"

static struct option long_options[] = {
    {"target",      required_argument, 0, 't'},
    {"injection",   required_argument, 0, 'i'},
    {"dictionary",  required_argument, 0, 'd'},
    {"verbose",     no_argument,       0, 'v'},
    {"timeout",     required_argument, 0, 'T'},
    {"user-agent",  required_argument, 0, 'U'},
    {"header",      required_argument, 0, 'H'},
    {"concurrency", required_argument, 0, 'c'},
    {"output",      required_argument, 0, 'o'},
    {"help",        no_argument,       0, 'h'},
    {"version",     no_argument,       0, 'V'},
    {0, 0, 0, 0}
};

int parse_arguments(int argc, char *argv[], config_t *config) {
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "t:i:d:vT:U:H:c:o:hV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                config->target_url = strdup(optarg);
                break;
            case 'i':
                if (strlen(optarg) != 1 || (optarg[0] != 'L' && optarg[0] != 'P')) {
                    log_message(LOG_ERROR, "Invalid injection type. Use 'L' for language or 'P' for prompt");
                    return -1;
                }
                config->injection_type = (injection_type_t)optarg[0];
                break;
            case 'd':
                config->dictionary_file = strdup(optarg);
                break;
            case 'v':
                config->verbose = 1;
                break;
            case 'T':
                config->timeout = atoi(optarg);
                if (config->timeout <= 0) {
                    log_message(LOG_ERROR, "Invalid timeout value");
                    return -1;
                }
                break;
            case 'U':
                config->user_agent = strdup(optarg);
                break;
            case 'H':
                if (!config->custom_headers) {
                    config->custom_headers = malloc(sizeof(char*));
                    config->header_count = 0;
                } else {
                    config->custom_headers = realloc(config->custom_headers, 
                                                   (config->header_count + 1) * sizeof(char*));
                }
                config->custom_headers[config->header_count] = strdup(optarg);
                config->header_count++;
                break;
            case 'c':
                config->concurrency = atoi(optarg);
                if (config->concurrency <= 0 || config->concurrency > MAX_THREADS) {
                    log_message(LOG_ERROR, "Invalid concurrency value (1-%d)", MAX_THREADS);
                    return -1;
                }
                break;
            case 'o':
                config->output_file = strdup(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'V':
                print_version();
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    if (!config->target_url) {
        log_message(LOG_ERROR, "Target URL is required (-t)");
        print_usage(argv[0]);
        return -1;
    }
    
    if (!config->dictionary_file) {
        log_message(LOG_ERROR, "Dictionary file is required (-d)");
        print_usage(argv[0]);
        return -1;
    }
    
    if (config->injection_type != INJECT_LANGUAGE && config->injection_type != INJECT_PROMPT) {
        log_message(LOG_ERROR, "Injection type is required (-i L or -i P)");
        print_usage(argv[0]);
        return -1;
    }
    
    return 0;
}

void print_usage(const char *program_name) {
    printf("\n%sBloodMiami v%s - AI Chat-Box Penetration Testing Tool%s\n", 
           COLOR_BOLD COLOR_RED, VERSION, COLOR_RESET);
    printf("\nUsage: %s [OPTIONS]\n", program_name);
    printf("\n%sRequired:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  -t, --target URL      Target chat interface URL\n");
    printf("  -i, --injection TYPE  Injection type: L (language) or P (prompt)\n");
    printf("  -d, --dictionary FILE Dictionary file with payloads\n");
    printf("\n%sOptional:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  -T, --timeout SEC     Request timeout in seconds (default: %d)\n", DEFAULT_TIMEOUT);
    printf("  -U, --user-agent STR  Custom User-Agent string\n");
    printf("  -H, --header STR      Add custom HTTP header\n");
    printf("  -c, --concurrency NUM Concurrent requests (default: %d)\n", DEFAULT_CONCURRENCY);
    printf("  -o, --output FILE     Save results to file\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -V, --version        Show version information\n");
    printf("\n%sExamples:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  %s -t https://example.com/chat -i L -d wordlist/language.txt\n", program_name);
    printf("  %s -t https://chatgpt.com/ -i P -d wordlist/prompt.txt -v\n", program_name);
    printf("  %s -t https://api.chat.com -i L -d custom.txt -c 20 -T 10\n", program_name);
    printf("\n");
}

void print_version(void) {
    printf("BloodMiami v%s\n", VERSION);
    printf("AI Chat-Box Penetration Testing Tool\n");
    printf("Built with libcurl %s\n", LIBCURL_VERSION);
}

void print_banner(void) {
    printf("\n");
    printf("%s", COLOR_RED);
    printf("  ██████╗ ██╗      ██████╗  ██████╗ ██████╗     ███╗   ███╗██╗ █████╗ ███╗   ███╗██╗\n");
    printf("  ██╔══██╗██║     ██╔═══██╗██╔═══██╗██╔══██╗    ████╗ ████║██║██╔══██╗████╗ ████║██║\n");
    printf("  ██████╔╝██║     ██║   ██║██║   ██║██║  ██║    ██╔████╔██║██║███████║██╔████╔██║██║\n");
    printf("  ██╔══██╗██║     ██║   ██║██║   ██║██║  ██║    ██║╚██╔╝██║██║██╔══██║██║╚██╔╝██║██║\n");
    printf("  ██████╔╝███████╗╚██████╔╝╚██████╔╝██████╔╝    ██║ ╚═╝ ██║██║██║  ██║██║ ╚═╝ ██║██║\n");
    printf("  ╚═════╝ ╚══════╝ ╚═════╝  ╚═════╝ ╚═════╝     ╚═╝     ╚═╝╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝\n");
    printf("%s", COLOR_RESET);
    printf("\n%s                    AI Chat-Box Penetration Testing Tool v%s%s\n", 
           COLOR_CYAN, VERSION, COLOR_RESET);
    printf("%s     Foxcorn Lab - Vineeth Kumar (foxcornlab)   %s\n",
           COLOR_WHITE, COLOR_RESET);
    printf("\n");
}