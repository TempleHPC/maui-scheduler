/*
 * Collection of utility functions and data structures for maui
 */

#ifndef MAUI_UTILS_H
#define MAUI_UTILS_H

/** Struct to collect generic command line flag properties */
typedef struct _client_info {
    char  *configfile;          /**< Name of config file to use */
    int    loglevel;            /**< Log level  */
    char  *logfacility;         /**< Log facility  */
    char  *host;                /**< Host name of Maui server to contact */
    char  *keyfile;             /**< Keyfile location for authentication */
    int    port;                /**< Port number of Maui server to contact */
} client_info_t;


/** Return copy of string in newly allocated memory. */
extern char *string_dup(const char *);

/** Safely convert a string to an integer */
extern int string2int(const char *);
#define INVALID_STRING ~0+1

/** Print client communication flag usage message */
extern void print_client_usage();


#endif
