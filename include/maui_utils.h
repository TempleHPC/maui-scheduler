/*
 * Collection of utility functions and data structures for maui
 */

#ifndef MAUI_UTILS_H
#define MAUI_UTILS_H

#include "moab-local.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>

#define WRITE 1
#define READ 0
#define TIMELIMIT 3000000
#define MAXLINE 1024
#define MAXBUFFER 65536
#define MAX_MDESCKSUM_ITERATION 4
#define MAXNAME 64
#define TRUE 1
#define FALSE 0

#define CONFIGFILE "maui.cfg"
#define MSCHED_ENVSMPVAR "MAUISMP"
#define MSCHED_ENVPARVAR "MAUIPARTITION"
#define GLOBAL_MPARNAME "ALL"
#define NONE "[NONE]"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/** Struct to collect generic command line flag properties */
typedef struct _client_info {
    char  *configfile;          /**< Name of config file to use */
    char  *host;                /**< Host name of Maui server to contact */
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
extern int connectToServer(int *, int, char *);

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
extern int generateBuffer(char *, char *, char *);

/* an algorithm to build checksum */
extern int secPSDES(unsigned int *, unsigned int *);

/* get attribute value contained in config file */
extern char *getConfigVal(FILE *, char *);

/* convert time to string but ignore the year value */
extern char *getDateString(time_t *);

/* convert time number to string with format: [DD:]HH:MM:SS */
extern char *timeToString(long );

#endif
