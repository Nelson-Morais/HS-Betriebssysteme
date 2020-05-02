#pragma once
#ifndef WEBREQ_H
#define WEBREQ_H

/**
 * WebReq - mini library for downloading websites to HTML files
 *
 * Based on libcurL (https://curl.haxx.se/libcurl, https://curl.haxx.se/docs/copyright.html)
 *
 * Author:  Oliver Kästner
 * Version: 20191012
 */

#ifndef WEBREQ_DEBUG
#define WEBREQ_DEBUG 0
#endif

/**
 * Possible return values (HTTP status codes) of webreq_download()
 */
#define WEBREQ_HTTP_OK 200
#define WEBREQ_HTTP_FORBIDDEN 403
#define WEBREQ_HTTP_NOT_FOUND 404

// the address to the Java ProxyServer
#define WEBREQ_PROXY_URL "http://localhost:8000"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init all internal structures, needs to be called ONCE before use of the lib.
 * Takes the programs start arguments (argc, argv) to search for own parameters:
 * --webreq-delay <delay_ms>  - simulated average delay before requests
 * --webreq-delay-seed <int>  - RNG seed for the delay
 * --webreq-path <path>       - output path for downloaded documents (TODO)
 */
void webreq_init(int argc, char* argv[]);

/**
 * Frees memory used by this library (and libcurl used by it)
 * Only call that at the end of your program
 */
void webreq_cleanup();

/**
 * Sets a simulated delay for downloading pages.
 * The delay will have a gaussian distribution around the given value.
 * Only requests that DON'T use the "ProxyServer" will be delayed.
 *
 * @param delay_ms  the average delay for the web request in milliseconds.
 * @param seed  the seed for the RNG. Set to constant value for reproducible results.
 */
void webreq_set_delay(unsigned int delay_ms, int seed);

/**
 * Sets the base directory where all documents are downloaded to.
 * If the directory does not exists, it will be created.
 *
 * @param path  the base path
 */
int webreq_set_output_path(const char* path);

/**
 * Downloads a webpage to a file
 *
 * @param url  the full URL to the webpage
 * @param filename  the name of the output file (*.html)
 *
 * @return  HTTP status code or a negativ error code if any other error occurs.
 *          If no error occurs, 200 (WEBREQ_HTTP_OK) is returned
 *
 *          A human readable error message can be retrieved using webreq_error(int)
 */
int webreq_download(const char* url, const char* filename);

/**
 * Download the webpage at the given URL to a file of given name
 * using the Java ProxyServer listening on port 8000
 *
 * @param url  the full URL to the webpage
 * @param filename  the name of the output file (*.html)
 *
 * @return  HTTP status code or a negativ error code if any other error occurs.
 *          If no error occurs, 200 (WEBREQ_HTTP_OK) is returned
 *
 *          A human readable error message can be retrieved using webreq_error(int)
 */
int webreq_download_via_proxy(const char* url, const char* filename);

/**
 * Returns a human readable error message for a
 */
const char* webreq_error(int errnum);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBREQ_H