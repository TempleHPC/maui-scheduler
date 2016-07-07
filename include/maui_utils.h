/*
 * Collection of utility functions and data structures for maui
 */

#ifndef MAUI_UTILS_H
#define MAUI_UTILS_H

#include "moab-local.h"

#define WRITE 1
#define READ 0
#define TIMELIMIT 3000000
#define MAXLINE 1024
#define MAXBUFFER 65536
#define MAX_MDESCKSUM_ITERATION 4

#define CONFIGFILE "maui.cfg"

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

/* send message to server */
extern int sendPacket(int , char *);

/* connect to server */
extern int connectToServer(int *, int);

/* waiting for input or output */
extern int selectWriteOrRead(int , unsigned long , int);

/* receive message from server */
extern int recvPacket(int , char **, long );

/* extract message size */
extern int getMessageSize(int);

/* generate checksum */
extern int getChecksum(char *, int , char *, int , char *, char *);

/* an algorithm to build checksum */
extern unsigned short secDoCRC(unsigned short , unsigned char );

/* Generate buffer to be sent to the server */
extern int generateBuffer(char *, char *);

/* an algorithm to build checksum */
extern int secPSDES(unsigned int *, unsigned int *);

#endif
