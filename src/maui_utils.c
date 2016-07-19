
#include "maui_utils.h"

/** Duplicate a string
 *
 * This function will take the string passed as argument
 * create a copy of it in newly allocated memory. The new
 * memory is allocated with malloc() to a suitable size.
 * The function returns a pointer to that copy which can
 * be passed to free() in order to return the allocated
 * memory to the system.
 *
 * @param input string to be copied
 * @return pointer to the new storage with the copy of
 * the input string or NULL if the call to malloc failed.
 */

char *string_dup(const char *input){

    char *output;
    int len;

    len = strlen(input) + 1;
    output = (char *) malloc(len);
    if (output) strcpy(output,input);
    return output;
}

/** Convert string to integer
 *
 * This function will take the string passed as argument,
 * check if it is a valid number and then convert it to
 * and integer. If it is not a number the value INVALID_STRING
 * is returned.
 *
 * @param input string to be converted
 * @return value of string converted to integer or INVALID_STRING.
 */

int string2int(const char *input){

    int i,n;

    if (input == NULL) return INVALID_STRING;

    n = strlen(input);
    if (n == 0) return INVALID_STRING;

    for (i = 0; i < n; i++) {
        if (isdigit(input[i]) || input[i] == '-' || input[i] == '+')
            continue;
        return INVALID_STRING;
    }

    return atoi(input);
}

/** Print common help message for flags related to client/server
 * communication between the command line tools and the maui server. */

void print_client_usage()
{
    puts("  -C, --configfile=FILENAME      set configfile\n"
         "  -H, --host=SERVERHOSTNAME      set serverhost\n"
         "  -P, --port=SERVERPORT          set serverport\n");
}

/** Generate buffer to be sent to the server
 *
 * This function will take the strings passed as arguments and
 * generate the whole buffer which will be sent to the server.
 *
 * @param1 output string to be saved
 * @param2 input string to be used to build the whole buffer
 * @return 1 if succeed.
 */

int generateBuffer(char *request, char *buffer, char *command) {

	char TSLine[MAXLINE], CKSum[MAXLINE], CKLine[MAXLINE], header[MAXBUFFER];
    char tmpStr[MAXBUFFER];

    time_t Now;

    /* build header */
	sprintf(header, "%s%s %s%s %s%s\n", "CMD=", command, "AUTH=",
			getpwuid(geteuid())->pw_name, "ARG=", buffer);

    /* get time stamp */
    time(&Now);
	sprintf(tmpStr, "%s%ld %s%s", "TS=", (long) Now, "AUTH=",
			getpwuid(geteuid())->pw_name);
    sprintf(TSLine, "%s %s", tmpStr, "DT=");

	/* get checksum */
	getChecksum(TSLine, strlen(TSLine), header, strlen(header), CKSum,
			MBUILD_SKEY);

	/* combine checksum and time stamp */
	sprintf(CKLine, "%s%s %s", "CK=", CKSum, TSLine);

	/* write buffer size to buffer */
	sprintf(request, "%08ld\n%s", strlen(header) + (long) strlen(CKLine),
			CKLine);

	/* combine all the strings */
	strcat(request,header);

	return 1;
}

/** Generate checksum
 *
 * This function will take the strings and integers passed as arguments
 * and generate the checksum.
 *
 * @param1 input string to be used for building checksum
 * @param2 input integer to be used for building checksum
 * @param3 input string to be used for building checksum
 * @param4 input integer to be used for building checksum
 * @param5 output string to save the checksum
 * @param6 input string to be used for building checksum
 * @return 1 if succeed.
 */

int getChecksum(char *timeStamp, int timeStampSize, char *header, int headerSize,
	    char *checksum, char *csKeyString)
{
	unsigned int crc;
	unsigned int lword;
	unsigned int rword;

	unsigned int seed;

	int index;

	seed = (unsigned int) strtoul(csKeyString, NULL, 0);

	crc = 0;

	for (index = 0; index < timeStampSize; index++) {
		crc = (unsigned int) secDoCRC((unsigned short) crc, timeStamp[index]);
	}

	for (index = 0; index < headerSize; index++) {
		crc = (unsigned int) secDoCRC((unsigned short) crc, header[index]);
	}

	lword = crc;
	rword = seed;

	secPSDES(&lword, &rword);

	sprintf(checksum, "%08x%08x", (int) lword, (int) rword);
	return 1;
}

/* an algorithm to build checksum */

unsigned short secDoCRC(unsigned short crc, unsigned char val)
{
    int index;
    unsigned int ans;

    ans = (crc ^ val << 8);

    for (index = 0; index < 8; index++) {
        if (ans & 0x8000)
            ans = (ans << 1) ^ 4129;
        else
            ans <<= 1;
    }

    return ((unsigned short)ans);
}

/* an algorithm to build checksum */

int secPSDES(unsigned int *lword, unsigned int *irword)
{
    int index;

    unsigned int ia;
    unsigned int ib;
    unsigned int iswap;
    unsigned int itmph;
    unsigned int itmpl;

    static unsigned int c1[MAX_MDESCKSUM_ITERATION] = {
        (unsigned int)0xcba4e531, (unsigned int)0x537158eb,
        (unsigned int)0x145cdc3c, (unsigned int)0x0d3fdeb2};

    static unsigned int c2[MAX_MDESCKSUM_ITERATION] = {
        (unsigned int)0x12be4590, (unsigned int)0xab54ce58,
        (unsigned int)0x6954c7a6, (unsigned int)0x15a2ca46};

    itmph = 0;
    itmpl = 0;

    for (index = 0; index < MAX_MDESCKSUM_ITERATION; index++) {
        iswap = *irword;

        ia = iswap ^ c1[index];

        itmpl = ia & 0xffff;
        itmph = ia >> 16;

        ib = (itmpl * itmpl) + ~(itmph * itmph);
        ia = (ib >> 16) | ((ib & 0xffff) << 16);

        *irword = (*lword) ^ ((ia ^ c2[index]) + (itmpl * itmph));

        *lword = iswap;
    }

    return 1;
}

/** Connect to server
 *
 * This function will take the objects passed as arguments
 * and connect to the server.
 *
 * @param1 output file descriptor associated with the socket
 * @param2 input integer to declare the server port
 * @param3 input string to declare the server host
 * @return 1 if succeed.
 */

int connectToServer(int *sd, int port, char *host){
    struct sockaddr_in s_sockaddr;
    struct hostent *s_hostent;
    struct in_addr in;

	int flags;

    memset(&s_sockaddr, 0, sizeof(s_sockaddr));

    /* get IP address */
	if (inet_aton(host, &in) == 0) {

		if ((s_hostent = gethostbyname(host)) == (struct hostent *) NULL) {
			printf("ERROR: cannot resolve IP address from hostname '%s'\n", host);
			return 0;
		}

		memcpy(&s_sockaddr.sin_addr, s_hostent->h_addr, s_hostent->h_length);

	} else {
		memcpy(&s_sockaddr.sin_addr, &in.s_addr, sizeof(s_sockaddr.sin_addr));
	}

    s_sockaddr.sin_family = AF_INET;
    s_sockaddr.sin_port = htons(port);

	/* create socket */
	if ((*sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR: cannot create socket, errno: %d (%s)\n", errno,
				strerror(errno));
		return 0;
	}

    fcntl(*sd, F_SETFD, 1);

	/* enable non-blocking mode on client */
	if ((flags = fcntl(*sd, F_GETFL, 0)) == -1) {
		printf("WARNING: cannot get socket attribute values, errno: %d "
				"(%s)\n", errno, strerror(errno));
		return 0;
	}

	/* set socket NDELAY attribute */
	if ((fcntl(*sd, F_SETFL, (flags | O_NDELAY))) == -1) {
		printf("WARNING: cannot set socket NDELAY attribute, errno: %d "
				"(%s)\n", errno, strerror(errno));
		return 0;
	}

    /* connect to server */
	if (connect(*sd, (struct sockaddr *) &s_sockaddr, sizeof(s_sockaddr))
			== -1) {
		if ((errno == EINPROGRESS) && (TIMELIMIT > 0)) {
			/* wait if non-blocking */
			if (!selectWriteOrRead(*sd, TIMELIMIT, WRITE)) {
				printf("ERROR: cannot connect to server '%s' on port %d, "
						"errno: %d (%s)\n", host, port, errno, strerror(errno));
				return 0;
			}
		} else {
			printf("ERROR: cannot connect to server '%s' on port %d, "
					"errno: %d (%s)\n", host, port, errno, strerror(errno));
		}
	}

	return 1;
}

/** waiting for input or output
 *
 * This function will take the numbers passed as arguments
 * and wait for input or output.
 *
 * @param1 input file descriptor associated with the socket
 * @param2 input number to specify the time limit
 * @param3 input integer to specify mode(WRITE or READ)
 * @return 1 if succeed.
 */

int selectWriteOrRead(int sd, unsigned long timeLimit, int flag){
    struct timeval timeOut;
    int numfds;

    fd_set set;

    FD_ZERO(&set);

    FD_SET(sd, &set);

    timeOut.tv_sec = timeLimit / 1000000;
    timeOut.tv_usec = timeLimit % 1000000;

    numfds = sd;
	if (flag == WRITE){
		if (select(numfds + 1, NULL, &set, NULL, &timeOut) > 0) {
			if (FD_ISSET(sd, &set)) {
				return 1;
			}
		}
	}else{
	    if (select(numfds + 1, &set, NULL, NULL, &timeOut) > 0) {
	        if (FD_ISSET(sd, &set)) {
	            return 1;
	        }
	    }
	}

    return 0;

}

/** get message size
 *
 * This function will take the integer passed as argument
 * and return the message size it extracts from the message.
 *
 * @param input file descriptor associated with the socket
 * @return 1 if succeed.
 */

int getMessageSize(int sd){
	char tmpLine[MAXLINE];
	long bufSize;
	char *ptr;

	ptr = tmpLine;
	ptr[0] = '\0';

	if(!recvPacket(sd, &ptr, 9 * sizeof(char))){
		return 0;
	}

	tmpLine[8] = '\0';
	sscanf(tmpLine, "%ld", &bufSize);

	return bufSize;
}

/** send message
 *
 * This function will take the string and integer passed as arguments
 * and send message to server
 *
 * @param1 input file descriptor associated with the socket
 * @param2 input string to be sent to the server
 * @return 1 if succeed.
 */

int sendPacket(int sd, char *request) {
	int rc;
	int count = 0;
	while (count < strlen(request)) {
		if (!selectWriteOrRead(sd, TIMELIMIT, WRITE)) {
			printf("WARNING: cannot send message within %1.6f second timeout "
					"(aborting)\n", (double) TIMELIMIT / 1000000);
			return 0;
		}
		if ((rc = send(sd, request, strlen(request), 0)) < 0) {
			printf("WARNING:  cannot send packet, errno: %d (%s)\n", errno,
					strerror(errno));
			return 0;
		}
		if (rc > 0) count += rc;
	}
	return 1;
}

/** receive message
 *
 * This function will take the string pointer and numbers passed
 * as arguments, receive and save message from server
 *
 * @param1 input string to be sent to the server
 * @param2 output string pointer to save the message
 * @param3 input number to declare the message size
 * @return 1 if succeed.
 */

int recvPacket(int sd, char **bufP, long bufSize){
	int count = 0;
	int rc;
	char *ptr;

	time_t start;
	time_t now;

	ptr = *bufP;

	time(&start);

	while (count < bufSize) {
		if (!selectWriteOrRead(sd, TIMELIMIT, READ)) {
			printf("WARNING: cannot send message within %1.6f second timeout "
					"(aborting)\n", (double) TIMELIMIT / 1000000);
			return 0;
		}

		 rc = recv(sd, (ptr + count), (bufSize - count), 0);

         if (rc > 0) {
             count += rc;
             continue;
         }
		time(&now);
		if (((long) now - (long) start) >= (TIMELIMIT / 1000000)) {
			printf("WARNING: cannot send message within %1.6f second timeout "
					"(aborting)\n", (double) TIMELIMIT / 1000000);
			return 0;
		}

		if (rc < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				/* continue if packet partially read */
				continue;
			}

			if ((errno == ECONNRESET) || (errno == EINTR)) {
				printf("INFO: client has disconnected, errno: %d (%s)\n", errno,
						strerror(errno));
				return 0;
			}

			printf("WARNING:  cannot read client socket, errno: %d (%s)\n",
					errno, strerror(errno));
			return 0;
		}

		if ((rc == 0) && (count == 0)) {
			/* fail if no packet detected */
			puts("no packet detected");
			return 0;
		}
	}
	return 1;
}

/** get attribute value
 *
 * This function will take the FILE and string pointer passed
 * as arguments and return the attribute value contained in the
 * config file
 *
 * @param1 input config FILE pointer
 * @param2 input string to declare the attribute name
 * @return a string pointer which contains the attribute value.
 */

char *getConfigVal(FILE *f, char *attr){
	char *configBuffer, *ptr, *pch, *val;

	/* read config file and save its content into a buffer*/
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    configBuffer = (char *)malloc(fsize + 1);
    fread(configBuffer, fsize, 1, f);
    /* locate and get attribute value */
    configBuffer[fsize] = '\0';

	if ((ptr = strstr(configBuffer, attr)) == NULL)
		return NULL;
	pch = strtok(ptr, " \n");
	pch = strtok(NULL, " \n");

    val = (char *)malloc(strlen(pch) + 1);
    strcpy(val,pch);

	free(configBuffer);

	return val;
}

/** convert time to string but ignore the year value
 *
 * This function will take the time_t pointer passed
 * as argument and return the formatted string converted
 * from the input
 *
 * @param input time_t pointer
 * @return a string pointer which contains the formatted string.
 */

char *getDateString(time_t *time) {
	static char string[MAXNAME];

	strncpy(string, ctime(time), 19);

	string[19] = '\n';
	string[20] = '\0';

	return (string);
}

/** convert time number to string with format: [DD:]HH:MM:SS
 *
 * This function will take the number of time passed
 * as argument and return the formatted string converted
 * from the input
 *
 * @param input long which represents time value
 * @return a string pointer which contains the formatted string.
 */

char *timeToString(long iTime) {
	static char string[MAXNAME];
	long time;
	int negative = FALSE;

	int index;

	/* FORMAT:  [DD:]HH:MM:SS */

	if (iTime >= 8640000) {
		strcpy(string, "  INFINITY");

		return (string);
	} else if (iTime <= -864000) {
		strcpy(string, " -INFINITY");

		return (string);
	}

	/* determine if number is negative */

	if (iTime < 0) {
		negative = TRUE;

		time = -iTime;
	} else {
		time = iTime;
	}

	string[11] = '\0';

	/* setup seconds */

	string[10] = (time) % 10 + '0';
	string[9] = (time / 10) % 6 + '0';
	string[8] = ':';

	time /= 60;

	/* setup minutes */

	string[7] = (time) % 10 + '0';
	string[6] = (time / 10) % 6 + '0';
	string[5] = ':';

	/* setup hours */

	time /= 60;

	string[4] = (time % 24) % 10 + '0';
	string[3] = (time / 10) ? (((time % 24) / 10) % 10 + '0') : ' ';

	if ((string[4] == '0') && (string[3] == ' '))
		string[3] = '0';

	/* setup days */

	time /= 24;

	if (time > 0) {
		string[2] = ':';
		string[1] = (time) % 10 + '0';
		string[0] = (time / 10) ? ((time / 10) % 10 + '0') : ' ';
	} else {
		string[2] = ' ';
		string[1] = ' ';
		string[0] = ' ';
	}

	if (negative == TRUE) {
		if (string[3] == ' ') {
			string[3] = '-';
		} else if (string[2] == ' ') {
			string[2] = '-';
		} else if (string[1] == ' ') {
			string[1] = '-';
		} else if (string[0] == ' ') {
			string[0] = '-';
		} else {
			string[1] = '9';
			string[0] = '-';
		}
	}

	for (index = 3; index >= 0; index--) {
		if (string[index] == ' ')
			return (&string[index + 1]);
	}

	return (string);
}
