/**************************************************************
* 
* Funktionen zum Lesen von URLs mit libcurl.
*
* Oliver Kaestner HS OS
* 
* 30.10.2019: Ersetzt Socket-Implementierung.
*
* To Do:
* - ...
**************************************************************/

#include "web_request.h"

#include <assert.h>
#include "curl/curl.h"
#include <getopt.h>
#include <openssl/opensslv.h> // OPENSSL_VERSION_NUMBER
#include <stdio.h>
#include <string.h> // strncmp

/**
 * locking functions for OpenSSL <= 1.0.2 (<1.1.0)
 * needed for SSL requests to be thread safe.
 *
 * https://curl.haxx.se/libcurl/c/threadsafe.html
 * https://curl.haxx.se/libcurl/c/opensslthreadlock.html
 */
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
#include <openssl/err.h>
#include <pthread.h>

/* This array will store all of the mutexes available to OpenSSL. */
static pthread_mutex_t* mutex_buf = NULL;

void handle_error(const char* file, int lineno, const char* msg);
static void locking_function(int mode, int n, const char* file, int line);
static unsigned long id_function(void);
int thread_setup(void);
int thread_cleanup(void);
#endif

// debug messages
// clang-format off
#if WEBREQ_DEBUG > 0
#define DEBUG(str) printf("[DEBUG] %s\n", str);
#define DEBUG_F(fmt, ...) printf(fmt, __VA_ARGS__);
#else
#define DEBUG(str) ;
#define DEBUG_F(fmt, ...) ;
#endif
// clang-format on

// error codes
#define WEBREQ_ERR_CURL -1
#define WEBREQ_ERR_INIT_NOT_RUN -2
#define WEBREQ_ERR_CANT_OPEN_FILE -3
#define WEBREQ_ERR_PROXY_SSL_NOT_SUPPORTED -4

// parse command line options
void webreq_parse_args(int argc, char* argv[]);

/**
 * Functions and variables for simulated delays
 */
#include <math.h>   // sqrt, log
#include <stdlib.h> // srand, rand, RAND_MAX
#define DELAY_STDDEV_PERCENT 0.05f
static unsigned int webreq_delay_ms_avg = 0;
static unsigned int nextDelay();

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h> // for nanosleep
#else
#include <sys/time.h> // struct timeval, select
#include <unistd.h>   // for usleep
#endif
static void sleep_ms(unsigned int milliseconds);

// for file output
#include <linux/limits.h> // PATH_MAX
#include <sys/stat.h>     // S_IRWXU
#include <unistd.h>       // mkdir()
#include <errno.h>
static int mkdir_p(const char* path);
static char webreq_output_basepath[PATH_MAX] = {0};

// used by cURL to write received data to file
size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

// cURL handle template, will be cloned for each download and set up in webreq_init()
CURL* curl_template = NULL;

void webreq_init(int argc, char* argv[]) {
    DEBUG("webreq_init()")

    // search cmd args for delay, delay-seed etc
    webreq_parse_args(argc, argv);

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    thread_setup();
#endif

    curl_global_init(CURL_GLOBAL_ALL);
    curl_template = curl_easy_init();

    // follow redirects
    curl_easy_setopt(curl_template, CURLOPT_FOLLOWLOCATION, 1);

    // set callback for received data
    curl_easy_setopt(curl_template, CURLOPT_WRITEFUNCTION, write_callback);

    // timeout
    curl_easy_setopt(curl_template, CURLOPT_TIMEOUT, 30);
}

void webreq_cleanup() {
    DEBUG("webreq_cleanup()")

    curl_easy_cleanup(curl_template);
    curl_global_cleanup();

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    thread_cleanup();
#endif
}

void webreq_parse_args(int argc, char* argv[]) {
    int delay = 0;
    int seed = 0;
    int longopt_index;
    // clang-format off
    static struct option longopts[] = {
        {"webreq-delay", required_argument, 0, 'd'},
        {"webreq-delay-seed", required_argument, 0, 's'},
        {"webreq-path", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };
    // clang-format on

    int c;
    opterr = 0; // disable printing errors for unknown parameters

    while (1) {
        c = getopt_long_only(argc, argv, "", longopts, &longopt_index);

        if (c == -1)
            break;

        int isErr = 0;

        // for strtol
        int val;
        char* endptr;
        errno = 0;

        switch (c) {
            case 'd': // fall through
            case 's':
                val = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol");
                    isErr = 1;
                } else if (endptr == optarg) {
                    // no digits found
                    isErr = 1;
                } else if (c == 'd') {
                    (val > 0) ? (delay = val) : (isErr = 1);
                } else {
                    seed = val;
                }
                break;
            case 'p':
                isErr = webreq_set_output_path(optarg);
                break;
            default:
                break; // do nothing, skip
        }

        if (isErr) {
            fprintf(stderr, "[WARN ] invalid value '%s' for argument '%s', ignoring!\n", optarg, longopts[optopt].name);
        }
    }

    if (delay) {
        webreq_set_delay(delay, seed);
    }

    // reset getopt, that other methods can use getopt as well
    optind = 1;
    opterr = 1;
}

void webreq_set_delay(unsigned int _avg_delay_ms, int seed) {
    DEBUG_F("webreq: setting up delay, ms: %d, seed: %d\n", _avg_delay_ms, seed);
    webreq_delay_ms_avg = _avg_delay_ms;
    srand(seed);
}

int webreq_set_output_path(const char* path) {
    strncpy(webreq_output_basepath, path, sizeof (webreq_output_basepath));

    struct stat sb;
    if (stat(webreq_output_basepath, &sb) == 0)
        return 0;

    if (errno != ENOENT) {
        perror("Invalid output path or other error");
        return -1;
    }

    // try to create the directory
    return mkdir_p(webreq_output_basepath);
}

/**
 * Performs the actual request
 *
 * @return the HTTP status code or any custom error code (WEBREQ_ERR_*)
 */
long webreq_perform_download(const char* url, const char* filename, int useProxy) {
    DEBUG_F("[DEBUG] webreq_perform_download(): URL: %s, File: %s\n", url, filename);

    if (!curl_template)
        return WEBREQ_ERR_INIT_NOT_RUN;

    if (useProxy && strncmp(url, "https", 5) == 0)
        return WEBREQ_ERR_PROXY_SSL_NOT_SUPPORTED;

    FILE* outfile;

    if (webreq_output_basepath[0]) {
        // output directory is set
        char outfile_path[PATH_MAX];
        snprintf(outfile_path, PATH_MAX, "%s/%s", webreq_output_basepath, filename);
        outfile = fopen(outfile_path, "w");
    } else {
        // output to current working directory
        outfile = fopen(filename, "w");
    }

    if (!outfile)
        return WEBREQ_ERR_CANT_OPEN_FILE;

    // clone the handle template
    CURL* curl = curl_easy_duphandle(curl_template);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

    if (useProxy)
        curl_easy_setopt(curl, CURLOPT_PROXY, WEBREQ_PROXY_URL);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return WEBREQ_ERR_CURL;
    }

    long httpcode = 0L;
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
    DEBUG_F("[DEBUG] webreq_perform_download(): HTTP-code: %li\n", httpcode)

    curl_easy_cleanup(curl);
    fclose(outfile);

    return httpcode;
}

int webreq_download(const char* url, const char* filename) {
    // delay the request if a delay is set up
    if (webreq_delay_ms_avg != 0) {
        unsigned int ms = nextDelay();
        DEBUG_F("[DEBUG] sleeping for %d ms\n", ms);
        sleep_ms(ms);
    }

    return webreq_perform_download(url, filename, 0);
}

int webreq_download_via_proxy(const char* url, const char* filename) {
    return webreq_perform_download(url, filename, 1);
}

/**
 * Error handling
 */
const char* webreq_error(int errnum) {
    DEBUG("webreq_error()")
    if (errnum > 0)
        return "no error";

    switch (errnum) {
        case WEBREQ_ERR_CURL:
            return "curl returned an error";
        case WEBREQ_ERR_INIT_NOT_RUN:
            return "webreq_init() hasn't been called";
        case WEBREQ_ERR_CANT_OPEN_FILE:
            return "failed to open file";
        case WEBREQ_ERR_PROXY_SSL_NOT_SUPPORTED:
            return "The Java ProxyServer doesn't support HTTPS requests";
        default:
            return "unknown error";
    }
}

/**
 * Callback function that handles the received data
 *
 * https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 */
size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    FILE* file = (FILE*) userdata;
    size_t nBytes = fwrite(ptr, size, nmemb, file);
    return nBytes;
}

#if OPENSSL_VERSION_NUMBER < 0x1010000fL

void handle_error(const char* file, int lineno, const char* msg) {
    fprintf(stderr, "** %s:%d %s\n", file, lineno, msg);
    ERR_print_errors_fp(stderr);
    /* exit(-1); */
}

static void locking_function(int mode, int n, const char* file, int line) {
    if (mode & CRYPTO_LOCK)
        pthread_mutex_lock(&mutex_buf[n]);
    else
        pthread_mutex_unlock(&mutex_buf[n]);
}

static unsigned long id_function(void) {
    return ((unsigned long) pthread_self());
}

int thread_setup(void) {
    int i;

    mutex_buf = malloc(CRYPTO_num_locks() * sizeof (pthread_mutex_t));
    if (!mutex_buf)
        return 0;
    for (i = 0; i < CRYPTO_num_locks(); i++)
        pthread_mutex_init(&mutex_buf[i], NULL);
    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);
    return 1;
}

int thread_cleanup(void) {
    int i;

    if (!mutex_buf)
        return 0;
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    for (i = 0; i < CRYPTO_num_locks(); i++)
        pthread_mutex_destroy(&mutex_buf[i]);
    free(mutex_buf);
    mutex_buf = NULL;
    return 1;
}
#endif // if OPENSSL_VERSION_NUMBER < 0x1010000fL

/**
 * Calculates a Gaussian distributed delay
 *
 * Code from: https://de.wikipedia.org/wiki/Polar-Methode
 */
static unsigned int nextDelay() {
    double u, v, q, p;

    do {
        u = (rand() / ((double) RAND_MAX)) * 2.0 - 1;
        v = (rand() / ((double) RAND_MAX)) * 2.0 - 1;
        q = u * u + v * v;
    } while (q >= 1.0 || q == 0.0);

    p = sqrt(-2.0 * log(q) / q);

    return (int) (webreq_delay_ms_avg + (webreq_delay_ms_avg * DELAY_STDDEV_PERCENT) * u * p) + 0.5;
}

/**
 * cross-platform sleep function
 * https://stackoverflow.com/a/28827188
 *
 */
static void sleep_ms(unsigned int milliseconds) {
    if (milliseconds == 0)
        return;
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = milliseconds % 1000 * 1000;
    select(0, NULL, NULL, NULL, &tv);
#endif
}

/**
 * By "JonathonReinhard" from
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
 */
static int mkdir_p(const char* path) {
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char* p;

    errno = 0;

    /* Copy string so it's mutable */
    if (len > sizeof (_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1;
    }

    return 0;
}
