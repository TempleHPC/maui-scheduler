/*
 * showres standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define RELATIVE 1
#define SUMMARY 2
#define GREP 3
#define VERBOSE 4
#define JOB 9
#define NODE 11
#define XML 14
#define MDEF_XMLICCOUNT 16
#define MMAX_XMLATTR 64

#define mbool_t unsigned char

#define CRYPTHEAD "KGV"

enum MDataFormatEnum {
	mdfNONE = 0,
	mdfString,
	mdfInt,
	mdfLong,
	mdfDouble,
	mdfStringArray,
	mdfIntArray,
	mdfLongArray,
	mdfDoubleArray,
	mdfOther,
	mdfLAST
};

enum MNResAttrType {
    mnraNONE = 0,
    mnraDRes,
    mnraEnd,
    mnraName,
    mnraState,
    mnraStart,
    mnraTC,
    mnraType
};

enum MSysAttrType {
    msysaNONE = 0,
    msysaPresentTime,
    msysaStatInitTime,
    msysaSyncTime,
    msysaVersion
};

enum MXMLOType {
    mxoNONE = 0,
    mxoAcct,
    mxoAM,
    mxoClass,
    mxoCluster,
    mxoCP,
    mxoFrame,
    mxoFS,
    mxoGroup,
    mxoJob,
    mxoLimits,
    mxoNode,
    mxoPar,
    mxoPriority,
    mxoQOS,
    mxoQueue,
    mxoRange,
    mxoReq,
    mxoRsv,
    mxoRM,
    mxoSched,
    mxoSim,
    mxoSRes,
    mxoStats,
    mxoSys,
    mxoUser,
    mxoLAST
};

enum MResAttrEnum {
    mraNONE = 0,
    mraAAccount, /* accountable account */
    mraACL,
    mraAGroup, /* accountable group */
    mraAUser,  /* accountable user */
    mraCreds,
    mraDuration,
    mraEndTime,
    mraFlags,
    mraHostExp,
    mraJState,
    mraMaxTasks,
    mraMessages,
    mraName,
    mraNodeCount,
    mraNodeList,
    mraOwner,
    mraResources,
    mraStartTime,
    mraStatCAPS,
    mraStatCIPS,
    mraStatTAPS,
    mraStatTIPS,
    mraTaskCount,
    mraType,
    mraXAttr
};

typedef struct mxml_s {
    char *Name;
    char *Val;

    int ACount;
    int ASize;

    int CCount;
    int CSize;

    char **AName;
    char **AVal;

    struct mxml_s **C;
} mxml_t;

/** Struct for showres options */
typedef struct _showres_info {
    int   type;                /**< Type*/
    char *jobid;               /**< Job ID */
    int   flags;			   /**< Flags */
    char *pName;			   /**< Partition ID */
    int   mode;				   /**< Display mode */
} showres_info_t;

static void free_structs(showres_info_t *, client_info_t *);
static int process_args(int, char **, showres_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(showres_info_t *, client_info_t *);
static int MXMLDestroyE(mxml_t **);
static int MXMLCreateE(mxml_t **, char *);
static int MXMLFromString(mxml_t **, char *, char **, char *);
static int MSecComp64BitToBufDecoding(char *, int , char *, int *);
static int MSecEncryption(char *, char *, int );
static int MSecDecompress(unsigned char *, unsigned int, unsigned char *,
		unsigned int, unsigned char **, char *);
static int MXMLAddE(mxml_t *, mxml_t *);
static int MXMLGetChild(mxml_t *, char *, int *, mxml_t **);
static int showReservedNodes(char *, int, int);
static int extractData(char *, mxml_t **, mxml_t **);
static int MXMLSetAttr(mxml_t *, char *, void *, enum MDataFormatEnum);
static int MXMLGetAttr(mxml_t *, char *, int *, char *, int);

int main(int argc, char **argv) {

    showres_info_t showres_info;
    client_info_t client_info;

	char *response, *msgBuffer, *ptr;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&showres_info, 0, sizeof(showres_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showres_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host)){
		    free_structs(&showres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		msgBuffer = buildMsgBuffer(&showres_info, &client_info);
		generateBuffer(request, msgBuffer, "showres");
		free(msgBuffer);

		if (!sendPacket(sd, request)){
		    free_structs(&showres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((bufSize = getMessageSize(sd)) == 0){
		    free_structs(&showres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("ERROR: cannot allocate memory for message");
		    free_structs(&showres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize)){
		    free_structs(&showres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		ptr = strstr(response, "ARG=") + strlen("ARG=");

		if(showres_info.type == NODE)
			showReservedNodes(ptr, showres_info.flags, showres_info.mode);
		else
			printf("\n%s\n", ptr);

		free(response);

    }

    free_structs(&showres_info, &client_info);

    exit(0);
}

int MXMLDestroyE(

    mxml_t **EP) /* I (modified) */

{
    int index;

    mxml_t *E;

    if (EP == NULL) {
        return (FAILURE);
    }

    E = *EP;

    if (E == NULL) {
        return (SUCCESS);
    }

    if (E->C != NULL) {
        /* destroy children */

        for (index = 0; index < E->CCount; index++) {
            if (E->C[index] == NULL) continue;

            MXMLDestroyE(&E->C[index]);
        } /* END for (index) */

        free(E->C);
    } /* END if (E->C != NULL) */

    /* free attributes */

    if (E->AName != NULL) {
        for (index = 0; index < E->ACount; index++) {
            if (E->AName[index] == NULL) break;

            free(E->AName[index]);

            if ((E->AVal != NULL) && (E->AVal[index] != NULL))
                free(E->AVal[index]);
        } /* END for (index) */

        if (E->AVal != NULL) {
            free(E->AVal);
        }

        if (E->AName != NULL) {
            free(E->AName);
        }
    } /* END if (E->AName != NULL) */

    /* free name */

    if (E->Name != NULL) free(E->Name);

    if (E->Val != NULL) free(E->Val);

    free(E);

    *EP = NULL;

    return (SUCCESS);
} /* END MXMLDestroyE() */

int MXMLCreateE(

    mxml_t **E, /* O */
    char *Name) /* I (optional) */

{
    /* NOTE:  should 'Name' be mandatory? */

    if (E == NULL) {
        return (FAILURE);
    }

    if ((*E = (mxml_t *)calloc(1, sizeof(mxml_t))) == NULL) {
        return (FAILURE);
    }

    if ((Name != NULL) && (Name[0] != '\0')) (*E)->Name = strdup(Name);

    return (SUCCESS);
} /* END MXMLCreateE() */

int MXMLFromString(

    mxml_t **EP,     /* O (populate or create - will be freed on failure) */
    char *XMLString, /* I */
    char **Tail,     /* O (optional) */
    char *EMsg)      /* O (optional) */

{
    mxml_t *E;
    char *ptr;

    char *tail;

    int index;

    mbool_t ElementIsClosed = FALSE;

    mbool_t DoAppend = FALSE;

    char tmpNLine[MAXLINE + 1];
    char tmpVLine[MAXBUFFER + 1];

    if (EP != NULL) *EP = NULL;

    if (EMsg != NULL) EMsg[0] = '\0';

    if ((XMLString == NULL) || (EP == NULL)) {
        if (EMsg != NULL) strcpy(EMsg, "invalid arguments");

        return (FAILURE);
    }

    if ((ptr = strchr(XMLString, '<')) == NULL) {
        if (EMsg != NULL) strcpy(EMsg, "no XML in string");

        return (FAILURE);
    }

    if (ptr[1] == '/') {
        /* located tail marker */

        if (EMsg != NULL) strcpy(EMsg, "premature termination marker");

        return (FAILURE);
    }

    /* NOTE:  should support append/overlay parameter (NYI) */

    /* ignore 'meta' elements */

    while ((ptr[1] == '?') || (ptr[1] == '!')) {
        ptr++;

        /* ignore 'meta' elements */

        if (*ptr == '?') {
            ptr++;

            if ((ptr = strstr(ptr, "?>")) == NULL) {
                /* cannot locate end of meta element */

                return (FAILURE);
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL) strcpy(EMsg, "cannot locate post-meta XML");

                return (FAILURE);
            }
        } /* END if (*ptr == '?') */

        /* ignore 'comment' element */

        if (!strncmp(ptr, "!--", 3)) {
            ptr += 3;

            if ((ptr = strstr(ptr, "-->")) == NULL) {
                /* cannot locate end of comment element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate comment termination marker");

                return (FAILURE);
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }
        } /* END if (!strncmp(ptr,"!--",3)) */
        else if (ptr[1] == '!') {
            char *ptr2;

            ptr++;
            ptr++;

            while (*ptr != '\0') {
                if (strchr("<[>", *ptr) != NULL) {
                    break;
                }

                ptr++;
            }

            if ((ptr2 = strchr("<[>", *ptr)) == NULL) {
                /* cannot locate end element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }

            switch (*ptr2) {
                case '[':

                    ptr = strstr(ptr, "]>");

                    break;

                default:

                    /* NYI */

                    return (FAILURE);

                    /* NOTREACHED */

                    break;
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }
        } /* END if (*ptr == '!') */
    }     /* END while ((ptr[1] == '?') || (ptr[1] == '!')) */

    /* remove whitespace */

    while (isspace(*ptr)) ptr++;

    /* extract root element */

    if (*ptr != '<') {
        /* cannot located start of element */

        if (EMsg != NULL) strcpy(EMsg, "cannot locate start of root element");

        return (FAILURE);
    }

    ptr++; /* ignore '<' */

    index = 0;

    while ((*ptr != ' ') && (*ptr != '>')) {
        if ((ptr[0] == '/') && (ptr[1] == '>')) {
            ElementIsClosed = TRUE;

            break;
        }

        tmpNLine[index++] = *(ptr++);

        if ((index >= MAXLINE) || (ptr[0] == '\0')) {
            if (EMsg != NULL)
                sprintf(EMsg, "element name is too long - %.10s", tmpNLine);

            return (FAILURE);
        }
    }

    tmpNLine[index] = '\0';

    if ((*EP == NULL) && (MXMLCreateE(EP, tmpNLine) == FAILURE)) {
        if (EMsg != NULL)
            sprintf(EMsg, "cannot create XML element '%s'", tmpNLine);

        return (FAILURE);
    }

    E = *EP;

    if ((E->ACount > 0) || (E->CCount > 0)) {
        DoAppend = TRUE;
    }

    if (ElementIsClosed == TRUE) {
        ptr += 2; /* skip '/>' */

        if (Tail != NULL) *Tail = ptr;

        return (SUCCESS);
    }

    while (*ptr == ' ') ptr++;

    while (*ptr != '>') {
        /* extract attributes */

        /* FORMAT:  <ATTR>="<VAL>" */

        index = 0;

        while ((*ptr != '=') && (*ptr != '\0')) {
            tmpNLine[index++] = *(ptr++);

            if (index >= MAXLINE) break;
        }

        tmpNLine[index] = '\0';

        if (*ptr != '\0') ptr++; /* skip '=' */

        if (*ptr != '\0') ptr++; /* skip '"' */

        if (*ptr == '\0') {
            if (EMsg != NULL)
                sprintf(EMsg, "string is corrupt - early termination");

            MXMLDestroyE(EP);

            return (FAILURE);
        }

        index = 0;

        while ((*ptr != '"') || ((ptr > XMLString) && (*(ptr - 1) == '\\'))) {
            if (((*ptr == '&') && (*(ptr + 1) == 'l') && (*(ptr + 2) == 't') &&
                 (*(ptr + 3) == ';'))) {
                tmpVLine[index++] = '<';

                ptr += 4;
            } else if (((*ptr == '&') && (*(ptr + 1) == 'g') &&
                        (*(ptr + 2) == 't') && (*(ptr + 3) == ';'))) {
                tmpVLine[index++] = '>';

                ptr += 4;
            } else {
                tmpVLine[index++] = *(ptr++);
            }

            if ((index >= MAXBUFFER) || (*ptr == '\0')) {
                MXMLDestroyE(EP);

                /* locate tail */

                if (Tail != NULL) *Tail = ptr + strlen(ptr);

                if (EMsg != NULL)
                    sprintf(EMsg, "attribute name is too long - %.10s",
                            tmpVLine);

                return (FAILURE);
            }
        }

        tmpVLine[index] = '\0';

        MXMLSetAttr(E, tmpNLine, (void *)tmpVLine, mdfString);

        ptr++; /* ignore '"' */

        while (*ptr == ' ') ptr++;

        if ((ptr[0] == '/') && (ptr[1] == '>')) {
            /* element terminator reached */

            ptr += 2; /* skip '/>' */

            if (Tail != NULL) *Tail = ptr;

            return (SUCCESS);
        }
    } /* END while (*ptr != '>') */

    ptr++; /* ignore '>' */

    /* skip whitespace */

    while (isspace(*ptr)) ptr++;

    /* NOTE:  value can occur before, after, or spread amongst children */

    /* extract value */

    if (*ptr != '<') {
        char *ptr2;

        index = 0;

        if (!strncmp(ptr, CRYPTHEAD, strlen(CRYPTHEAD))) {
            char *tail;
            int len;

            mxml_t *C;

            char *tmpBuf;

            /* compressed data detected, '<' symbol guaranteed to not be part of
             * string */

            tail = strchr(ptr, '<');

            /* determine size of value */

            len = tail - ptr;

            /* uncompress data */

            tmpBuf = NULL;

            /* NOTE:  CRYPTHEAD header indicates encryption/compression */

            MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (MXMLAddE(E, C) == FAILURE)) {
                MXMLDestroyE(EP);

                if ((EMsg != NULL) && (EMsg[0] == '\0')) {
                    strcpy(EMsg, "cannot add child element");
                }

                return (FAILURE);
            }

            tmpBuf = NULL;

            /* move pointer to end of compressed data */

            ptr = tail;
        } /* END if (!strncmp(ptr,CRYPTHEAD,strlen(CRYPTHEAD))) */

        while (*ptr != '<') {
            tmpVLine[index++] = *(ptr++);

            if (index >= MAXBUFFER) break;
        }

        tmpVLine[index] = '\0';

        E->Val = strdup(tmpVLine);

        /* restore '<' symbols */

        for (ptr2 = strchr(E->Val, (char)14); ptr2 != NULL;
             ptr2 = strchr(ptr2, (char)14))
            *ptr2 = '<';
    } /* END if (*ptr != '<') */

    /* extract children */

    while (ptr[1] != '/') {
        mxml_t *C;

        C = NULL;

        if (DoAppend == TRUE) {
            char *ptr2;
            char tmpCName[MAXNAME];

            int index;

            /* FORMAT:  <NAME>... */

            /* locate name */

            ptr2 = ptr + 1; /* ignore '<' */

            index = 0;

            while ((*ptr2 != ' ') && (*ptr2 != '>')) {
                if ((ptr2[0] == '/') && (ptr2[1] == '>')) {
                    break;
                }

                tmpCName[index++] = *(ptr2++);

                if ((index >= MAXLINE) || (ptr2[0] == '\0')) {
                    if (EMsg != NULL)
                        sprintf(EMsg, "element name is too long - %.10s",
                                tmpCName);

                    MXMLDestroyE(EP);

                    return (FAILURE);
                }
            }

            tmpCName[index] = '\0';

            MXMLGetChild(E, tmpCName, NULL, &C);
        }

        if ((MXMLFromString(&C, ptr, &tail, EMsg) == FAILURE) ||
            (MXMLAddE(E, C) == FAILURE)) {
            break;
        }

        ptr = tail;

        if ((ptr == NULL) || (ptr[0] == '\0')) {
            /* XML is corrupt */

            if (Tail != NULL) *Tail = ptr;

            if ((EMsg != NULL) && (EMsg[0] == '\0'))
                strcpy(EMsg, "cannot extract child");

            MXMLDestroyE(EP);

            return (FAILURE);
        }
    } /* END while (ptr[1] != '/') */

    /* ignore whitespace */

    while (isspace(*ptr)) ptr++;

    /* value may follow children */

    if (E->Val == NULL) {
        if (!strncmp(ptr, CRYPTHEAD, strlen(CRYPTHEAD))) {
            char *tail;
            int len;

            mxml_t *C;

            char *tmpBuf;

            /* compressed data detected, '<' symbol guaranteed to not be part of
             * string */

            tail = strchr(ptr, '<');

            /* determine size of value */

            len = tail - ptr;

            /* uncompress data */

            tmpBuf = NULL;

            /* NOTE:  CRYPTHEAD header indicates encryption/compression */

            MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (MXMLAddE(E, C) == FAILURE)) {
                MXMLDestroyE(EP);

                if ((EMsg != NULL) && (EMsg[0] == '\0')) {
                    strcpy(EMsg, "cannot add child element");
                }

                return (FAILURE);
            }

            tmpBuf = NULL;

            /* move pointer to end of compressed data */

            ptr = tail;
        } /* END if (!strncmp(ptr,CRYPTHEAD,strlen(CRYPTHEAD))) */

        index = 0;

        while (*ptr != '<') {
            tmpVLine[index++] = *(ptr++);

            if ((index >= MAXBUFFER) || (*ptr == '\0')) {
                if (EMsg != NULL)
                    sprintf(EMsg, "cannot load value line - %.10s (%s)",
                            tmpVLine,
                            (index >= MAXBUFFER) ? "too long" : "corrupt");

                MXMLDestroyE(EP);

                return (FAILURE);
            }

            if ((*ptr == '/') && (ptr[1] == '>')) break;
        }

        tmpVLine[index] = '\0';

        if (index > 0) E->Val = strdup(tmpVLine);
    } /* END if (tmpVLine[0] == '\0') */

    /* process tail */

    if (*ptr == '/') {
        /* process '/>' */

        ptr++; /* ignore '/' */
    } else if (*ptr != '\0') {
        /* NOTE: corrupt XML string may move ptr beyond string terminator */

        ptr++; /* ignore '<' */

        ptr++; /* ignore '/' */

        ptr += strlen(E->Name);
    }

    if (*ptr == '\0') {
        if (EMsg != NULL) sprintf(EMsg, "xml tail is corrupt");

        MXMLDestroyE(EP);

        return (FAILURE);
    }

    ptr++; /* ignore '>' */

    if (Tail != NULL) *Tail = ptr;

    return (SUCCESS);
} /* END MXMLFromString() */

int MXMLSetAttr(

    mxml_t *E,                   /* I (modified) */
    char *A,                     /* I */
    void *V,                     /* I */
    enum MDataFormatEnum Format) /* I */

{
    int aindex;
    int iindex;

    int rc;

    char tmpLine[MAXLINE];

    char *ptr;

    /* NOTE:  overwrite existing attr if found */

    if ((E == NULL) || (A == NULL)) {
        return (FAILURE);
    }

    if (V != NULL) {
        switch (Format) {
            case mdfString:
            default:

                ptr = (char *)V;

                break;

            case mdfInt:

                sprintf(tmpLine, "%d", *(int *)V);

                ptr = tmpLine;

                break;

            case mdfLong:

                sprintf(tmpLine, "%ld", *(long *)V);

                ptr = tmpLine;

                break;

            case mdfDouble:

                sprintf(tmpLine, "%f", *(double *)V);

                ptr = tmpLine;

                break;
        } /* END switch(Format) */
    } else {
        tmpLine[0] = '\0';

        ptr = tmpLine;
    }

    /* initialize attribute table */

    if (E->AName == NULL) {
        E->AName = (char **)calloc(1, sizeof(char *) * MMAX_XMLATTR);
        E->AVal = (char **)calloc(1, sizeof(char *) * MMAX_XMLATTR);

        if ((E->AName == NULL) || (E->AVal == NULL)) {
            return (FAILURE);
        }

        E->ASize = MMAX_XMLATTR;
        E->ACount = 0;
    }

    /* insert in alphabetical order */

    /* overwrite existing attribute if found */

    iindex = 0;
    rc = 0;

    for (aindex = 0; aindex < E->ACount; aindex++) {
        rc = strcmp(E->AName[aindex], A);

        if (rc > 0) break;

        if (rc == 0) {
            iindex = aindex;

            break;
        }

        iindex = aindex + 1;
    } /* END for (aindex) */

    if (aindex >= E->ACount) {
        iindex = aindex;

        if (aindex >= E->ASize) {
            /* allocate memory */

            E->AName = (char **)realloc(
                E->AName, sizeof(char *) * MAX(16, E->ASize << 1));
            E->AVal = (char **)realloc(E->AVal,
                                       sizeof(char *) * MAX(16, E->ASize << 1));

            if ((E->AVal == NULL) || (E->AName == NULL)) {
                E->ASize = 0;

                return (FAILURE);
            }

            E->ASize <<= 1;
        }
    } /* END if (aindex >= E->ACount) */

    if ((ptr == NULL) && (aindex >= E->ACount)) {
        /* no action required for empty attribute */

        return (SUCCESS);
    }

    /* prepare insertion point */

    if (rc != 0) {
        for (aindex = E->ACount - 1; aindex >= iindex; aindex--) {
            E->AVal[aindex + 1] = E->AVal[aindex];
            E->AName[aindex + 1] = E->AName[aindex];
        } /* END for (aindex) */

        E->AVal[aindex + 1] = NULL;
        E->AName[aindex + 1] = NULL;
    } /* END if (rc != 0) */

    if ((iindex < E->ACount) && (E->AVal[iindex] != NULL))
        free(E->AVal[iindex]);

    E->AVal[iindex] = strdup((ptr != NULL) ? ptr : "");

    if ((rc != 0) || (E->AName[iindex] == NULL)) {
        E->AName[iindex] = strdup(A);

        E->ACount++;
    }

    return (SUCCESS);
} /* END MXMLSetAttr() */

int MSecComp64BitToBufDecoding(

    char *IBuf, int IBufLen, char *OBuf, int *OBufLen)

{
    int IIndex = 0;
    int OIndex = 0;

    static char CDList[256] = {
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 62, 00, 00, 00, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 00, 00, 00, 00, 00, 00, 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 00, 00, 00, 00,
        00, 00, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
        00, 00, 00, 00, 00, 00, 00, 00, 00,
    };

    if ((OBuf == NULL) || (IBuf == NULL)) {
        return (FAILURE);
    }

    do {
        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex]] << 2) & 0xfc) |
                         ((CDList[(int)IBuf[IIndex + 1]] >> 4) & 0x03);

        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex + 1]] << 4) & 0xf0) |
                         ((CDList[(int)IBuf[IIndex + 2]] >> 2) & 0x0f);

        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex + 2]] << 6) & 0xc0) |
                         ((CDList[(int)IBuf[IIndex + 3]]));

        IIndex += 4;
    } while (IIndex < IBufLen);

    if (OBufLen != NULL) *OBufLen = OIndex;

    return (SUCCESS);
} /* MSecComp64BitToBufDecoding() */

int MSecEncryption(

    char *SrcString, char *Key, int SrcSize)

{
    int r;
    char *cp_val;
    char *cp_key;
    char result;

    r = 0;

    if ((SrcString != NULL) && (Key != NULL)) {
        cp_val = SrcString;
        cp_key = Key;

        while (r < SrcSize) {
            result = *cp_val ^ *cp_key;
            *cp_val = result;
            cp_val++;
            cp_key++;
            if (*cp_key == '\0') cp_key = Key;
            r++;
        }
    }

    return (SUCCESS);
} /* END MSecEncryption() */

int MSecDecompress(

    unsigned char *SrcString, /* I */
    unsigned int SrcSize,     /* I */
    unsigned char *SDstBuf,   /* I (optional) */
    unsigned int SDstSize,    /* I (required if DstBuf set) */
    unsigned char **DDstBuf,  /* O (optional) */
    char *EKey) /* I (optional - encryption enabled when present) */

{
    static unsigned char *tmpBuf = NULL;
    static unsigned char *OutBuf = NULL;

    static int OutBufSize = 0;

    unsigned int DstSize;

    unsigned int NewLength;

    unsigned int X = 9;
    unsigned int Y = 0;
    unsigned int Pos;
    unsigned int Size;
    unsigned int K;
    int Command;

    unsigned char *DstString;

    unsigned char Bit = 16;

    if ((SrcString == NULL) || (SrcSize <= 0)) {
        return (FAILURE);
    }

    if ((SDstBuf != NULL) && (SDstSize > 0)) {
        DstString = SDstBuf;
        DstSize = SDstSize;
    } else if (DDstBuf != NULL) {
        if ((OutBuf == NULL) &&
            ((OutBuf = (unsigned char *)calloc(MAXBUFFER, 1)) == NULL)) {
            return (FAILURE);
        } else {
            OutBufSize = MAXBUFFER;
        }

        *DDstBuf = NULL;

        DstString = OutBuf;
        DstSize = OutBufSize;
    } else {
        return (FAILURE);
    }

    DstString[0] = '\0';

    NewLength = SrcSize;

    if (tmpBuf == NULL) {
        if ((tmpBuf = (unsigned char *)calloc(NewLength, 1)) == NULL) {
            return (FAILURE);
        }
    } else {
        if (sizeof(tmpBuf) < NewLength) {
            if ((tmpBuf = (unsigned char *)realloc(tmpBuf, NewLength)) ==
                NULL) {
                return (FAILURE);
            }
        }
    }

    tmpBuf[0] = '\0';

    MSecComp64BitToBufDecoding((char *)SrcString, (int)SrcSize, (char *)tmpBuf,
                               (int *)&SrcSize);

    if (EKey != NULL) MSecEncryption((char *)tmpBuf, EKey, SrcSize);

    Command = (tmpBuf[3] << 8) + tmpBuf[4];

    /*
      if (tmpBuf[0] == FLAG_Copied)
        {
        for (Y = 1; Y < SrcSize; DstString[Y-1] = tmpBuf[Y++]);

        return(SrcSize-1);
        }
    */

    for (; X < SrcSize;) {
        if (Bit == 0) {
            Command = (tmpBuf[X++] << 8);

            Command += tmpBuf[X++];
            Bit = 16;
        }

        if (Command & 0x8000) {
            Pos = (tmpBuf[X++] << 4);
            Pos += (tmpBuf[X] >> 4);

            if (Pos) {
                Size = (tmpBuf[X++] & 0x0f) + 3;

                if ((Y + Size) > DstSize) {
                    /* bounds check */

                    if (DDstBuf != NULL) {
                        DstSize <<= 1;

                        if ((OutBuf = (unsigned char *)realloc(
                                 OutBuf, DstSize)) == NULL) {
                            return (FAILURE);
                        }

                        DstString = OutBuf;
                    } else {
                        return (FAILURE);
                    }
                }

                for (K = 0; K < Size; K++) {
                    DstString[Y + K] = DstString[Y - Pos + K];
                }

                Y += Size;
            } else {
                Size = (tmpBuf[X++] << 8);
                Size += tmpBuf[X++] + 16;

                if ((Y + Size) > DstSize) {
                    /* bounds check */

                    if (DDstBuf != NULL) {
                        DstSize <<= 1;

                        if ((OutBuf = (unsigned char *)realloc(
                                 OutBuf, DstSize)) == NULL) {
                            return (FAILURE);
                        }

                        DstString = OutBuf;
                    } else {
                        return (FAILURE);
                    }
                }

                for (K = 0; K < Size; DstString[Y + K++] = tmpBuf[X])
                    ;

                X++;
                Y += Size;
            }
        } else {
            if (Y >= DstSize) {
                /* bounds check */

                if (DDstBuf != NULL) {
                    DstSize <<= 1;

                    if ((OutBuf = (unsigned char *)realloc(OutBuf, DstSize)) ==
                        NULL) {
                        return (FAILURE);
                    }

                    DstString = OutBuf;
                } else {
                    return (FAILURE);
                }
            }

            DstString[Y++] = tmpBuf[X++];
        } /* END else () */

        Command <<= 1;
        Bit--;
    } /* END for() */

    /* terminate buffer */

    if (Y < DstSize) OutBuf[Y] = '\0';

    if (DDstBuf != NULL) {
        *DDstBuf = OutBuf;
    }

    return (SUCCESS);
} /* END MSecDecompress() */

int MXMLAddE(

    mxml_t *E, /* I (modified) */
    mxml_t *C) /* I (required) */

{
    if ((E == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    if (E->CCount >= E->CSize) {
        if (E->C == NULL) {
            E->C = (mxml_t **)calloc(1, sizeof(mxml_t *) * MDEF_XMLICCOUNT);

            E->CSize = MDEF_XMLICCOUNT;
        } else {
            E->C = (mxml_t **)realloc(
                E->C, sizeof(mxml_t *) * MAX(16, E->CSize << 1));

            E->CSize <<= 1;
        }

        if (E->C == NULL) {
            /* cannot alloc memory */

            return (FAILURE);
        } /* END if (E->C == NULL) */
    }     /* END if (E->CCount >= E->CSize) */

    E->C[E->CCount] = C;

    E->CCount++;

    return (SUCCESS);
} /* END MXMLAddE() */

int MXMLGetChild(

    mxml_t *E,   /* I */
    char *CName, /* I (optional) */
    int *CTok,   /* I (optional) */
    mxml_t **C)  /* O */

{
    int cindex;
    int cstart;

    if (C != NULL) *C = NULL;

    if ((E == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    if (CTok != NULL)
        cstart = *CTok;
    else
        cstart = -1;

    for (cindex = cstart + 1; cindex < E->CCount; cindex++) {
        if (E->C[cindex] == NULL) continue;

        if ((CName == NULL) || !strcmp(CName, E->C[cindex]->Name)) {
            *C = E->C[cindex];

            if (CTok != NULL) *CTok = cindex;

            return (SUCCESS);
        }
    } /* END for (cindex) */

    return (FAILURE);
} /* END MXMLGetChild() */

int extractData(

    char *Buf,   /* I */
    mxml_t **PE, /* O */
    mxml_t **DE) /* O (optional) */

{
    char *ptr;

    mxml_t *E = NULL;
    mxml_t *C = NULL;

    if ((Buf == NULL) || (PE == NULL)) {
        return (FAILURE);
    }

    if (((ptr = strstr(Buf, "<response")) == NULL) ||
        (MXMLFromString(&E, ptr, NULL, NULL) == FAILURE)) {
        /* cannot parse response */

        fprintf(stderr,
                "ERROR:    cannot parse server response (corrupt data)\n");

        return (FAILURE);
    }

    /* check status code */

    if (MXMLGetChild(E, "status", NULL, &C) == FAILURE) {
        /* cannot locate status code */

        fprintf(stderr, "ERROR:    cannot parse server response (no status)\n");

        MXMLDestroyE(&E);

        return (FAILURE);
    }

    if (strcmp(C->Val, "success")) {
        /* command failed */

        char tmpMsg[MAXLINE];
        char tmpBuf[MAXNAME];

        int tmpSC;

        if (MXMLGetAttr(C, "message", NULL, tmpMsg, sizeof(tmpMsg)) ==
            FAILURE) {
            strcpy(tmpMsg, "N/A");
        }

        if (MXMLGetAttr(C, "code", NULL, tmpBuf, sizeof(tmpBuf)) == SUCCESS) {
            tmpSC = (int)strtol(tmpBuf, NULL, 0);
        } else {
            tmpSC = -1;
        }

        fprintf(stderr, "ERROR:    command failed: '%s' (rc: %d)\n", tmpMsg,
                tmpSC);

        MXMLDestroyE(&E);

        return (FAILURE);
    } /* END if (strcmp(C->Val,"success")) */

    if (MXMLGetChild(E, "data", NULL, &C) == FAILURE) {
        fprintf(stderr, "ERROR:    cannot parse server response (no data)\n");

        MXMLDestroyE(&E);

        return (FAILURE);
    }

    *PE = E;

    if (DE != NULL) {
        *DE = C;
    } /* END if (DE != NULL) */

    return (SUCCESS);
} /* END extractData() */

int showReservedNodes(char *Buffer, int flags, int mode)
{
    long StartTime;
    long EndTime;

    char NodeName[MAXNAME];
    char ResName[MAXNAME];
    char JState[MAXNAME];
    char ResType[MAXNAME];
    char DurationLine[MAXNAME];
    char StartLine[MAXNAME];
    char EndLine[MAXNAME];
    char TaskString[MAXNAME];
    char UserName[MAXNAME];

    char tmpLine[MAXLINE];

    long now;

    int ncount;

    time_t tmpTime;

    mxml_t *E = NULL;
    mxml_t *DE = NULL;
    mxml_t *C = NULL;
    mxml_t *NE = NULL;
    mxml_t *RE = NULL;

    int RTok;
    int NTok;

    const char *MXO[] = {NONE,    "acct",     "am",    "class", "cluster", "cp",
                         "frame", "fs",       "group", "job",   "limits",  "node",
                         "par",   "priority", "qos",   "queue", "range",   "req",
                         "rsv",   "rm",       "sched", "sim",   "sres",    "stats",
                         "sys",   "user",     NULL};

    const char *MSysAttr[] = {NONE,       "time",    "statInitTime",
                              "syncTime", "version", NULL};

    const char *MResAttr[] = {
        NONE,        "AAccount", "ACL",       "AGroup",   "AUser",    "creds",
        "duration",  "endtime",  "flags",     "HostExp",  "JState",   "MaxTasks",
        "Messages",  "Name",     "NodeCount", "NodeList", "Owner",    "Resources",
        "starttime", "StatCAPS", "StatCIPS",  "StatTAPS", "StatTIPS", "TaskCount",
        "Type",      "XAttr",    NULL};

    const char *MNResAttr[] = {NONE,    "DRes", "End",  "Name", "State",
                               "Start", "TC",   "Type", NULL};

    if (flags & (1 << XML)) {
        fprintf(stdout, "%s\n", Buffer);

        return (SUCCESS);
    }

    /* display standard node reservation results */

    if (extractData(Buffer, &E, &DE) == FAILURE) {
        /* cannot extract XML data */

        return (FAILURE);
    }

    /* determine time */

    if ((MXMLGetChild(DE, (char *)MXO[mxoSys], NULL, &C) == SUCCESS) &&
        (MXMLGetAttr(C, (char *)MSysAttr[msysaPresentTime], NULL, tmpLine,
                     sizeof(tmpLine)) == SUCCESS)) {
        now = strtol(tmpLine, NULL, 0);
    } else {
        time(&tmpTime);

        now = (long)tmpTime;
    }

    if (flags & (1 << SUMMARY)) {
        char *ptr;

        fprintf(stdout, "%20s %10s %c %12s %12s %5s\n\n", "JobName", "User",
                'S', "StartTime", "EndTime", "Count");

        RTok = -1;

        while (MXMLGetChild(DE, "res", &RTok, &RE) == SUCCESS) {
            MXMLGetAttr(RE, (char *)MResAttr[mraName], NULL, ResName,
                        sizeof(tmpLine));
            MXMLGetAttr(RE, (char *)MResAttr[mraStartTime], NULL, tmpLine,
                        sizeof(tmpLine));
            StartTime = strtol(tmpLine, NULL, 0);

            MXMLGetAttr(RE, (char *)MResAttr[mraEndTime], NULL, tmpLine,
                        sizeof(tmpLine));
            EndTime = strtol(tmpLine, NULL, 0);

            MXMLGetAttr(RE, (char *)MResAttr[mraJState], NULL, JState,
                        sizeof(JState));

            MXMLGetAttr(RE, (char *)MResAttr[mraTaskCount], NULL, TaskString,
                        sizeof(TaskString));

            MXMLGetAttr(RE, (char *)MResAttr[mraCreds], NULL, tmpLine,
                        sizeof(tmpLine));

            if ((ptr = strstr(tmpLine, "USER=")) != NULL) {
                ptr += strlen("USER=");

                ptr = strtok(ptr, " \t,\n");

                strcpy(UserName, ptr);
            } else {
                strcpy(UserName, "N/A");
            }

            strcpy(StartLine, timeToString(StartTime - now));
            strcpy(EndLine, timeToString(EndTime - now));

            fprintf(stdout, "%20s %10s %1.1s %12s %12s %5s\n", ResName,
                    UserName, JState, StartLine, EndLine, TaskString);
        } /* END while (MXMLGetChild(DE,"res",&RTok,&RE) == SUCCESS) */

        MXMLDestroyE(&E);

        return (SUCCESS);
    } /* END if (flags & (1 << SUMMARY)) */

    fprintf(stdout, "reservations on %s\n", getDateString(&now));

    if (!(mode & (1 << RELATIVE))) {
        fprintf(stdout, "%20s  %9s %18s %10s %4s %11s %11s %s%20s\n\n",
                "NodeName", "Type", "ReservationID", "JobState", "Task",
                "Start", "Duration", (flags & (1 << VERBOSE)) ? "     " : "",
                "StartTime");
    } else {
        fprintf(stdout, "%20s  %9s %18s %10s %4s %8s --> %8s  (%8s)\n\n",
                "NodeName", "Type", "ReservationID", "JobState", "Task",
                "Start", "End", "Duration");
    }

    NTok = -1;

    ncount = 0;

    while (MXMLGetChild(DE, "node", &NTok, &NE) == SUCCESS) {
        int rcount = 0;

        MXMLGetAttr(NE, "name", NULL, NodeName, sizeof(NodeName));

        RTok = -1;

        while (MXMLGetChild(NE, "nres", &RTok, &RE) == SUCCESS) {
            MXMLGetAttr(RE, (char *)MNResAttr[mnraName], NULL, ResName,
                        sizeof(ResName));
            MXMLGetAttr(RE, (char *)MNResAttr[mnraStart], NULL, tmpLine,
                        sizeof(tmpLine));
            StartTime = strtol(tmpLine, NULL, 0);

            MXMLGetAttr(RE, (char *)MNResAttr[mnraEnd], NULL, tmpLine,
                        sizeof(tmpLine));
            EndTime = strtol(tmpLine, NULL, 0);

            MXMLGetAttr(RE, (char *)MNResAttr[mnraType], NULL, ResType,
                        sizeof(ResType));

            MXMLGetAttr(RE, (char *)MNResAttr[mnraState], NULL, JState,
                        sizeof(JState));

            MXMLGetAttr(RE, (char *)MNResAttr[mnraTC], NULL, TaskString,
                        sizeof(TaskString));

            /* display node/res information */

            fprintf(stdout, "%20s  %9s %18s %10s %4s ",
                    ((rcount == 0) || (mode & (1 << GREP))) ? NodeName : " ",
                    ResType, ResName, JState, TaskString);

            /* display res timeframe */

            if (mode & (1 << RELATIVE)) {
                if ((EndTime - StartTime) > 8553600)
                    fprintf(stdout, "%8ld --> %8s  (%8s)\n", (StartTime - now),
                            "INFINITE", "INFINITE");
                else
                    fprintf(stdout, "%8ld --> %8ld  (%08ld)\n",
                            (StartTime - now), (EndTime - now),
                            (EndTime - StartTime));
            } else {
                strcpy(StartLine, timeToString(StartTime - now));

                if ((EndTime - StartTime) > 8553600) {
                    strcpy(DurationLine, "INFINITE");
                } else {
                    strcpy(DurationLine, timeToString(EndTime - StartTime));
                }

                tmpTime = (time_t)StartTime;

                fprintf(stdout, "%11s %11s  %20s", StartLine, DurationLine,
                        (flags & (1 << VERBOSE)) ? ctime(&tmpTime)
                                                    : getDateString(&StartTime));
            } /* END else (mode & (1 << RELATIVE)) */

            rcount++;
        } /* END while (MXMLGetChild(RE) == SUCCESS) */

        ncount++;
    } /* END while (MXMLGetChild(NE) == SUCCESS) */

    fprintf(stdout, "%d nodes reserved\n", ncount);

    MXMLDestroyE(&E);

    return (SUCCESS);
}

int MXMLGetAttr(

    mxml_t *E,   /* I */
    char *AName, /* I/O */
    int *ATok,   /* I (optional) */
    char *AVal,  /* O (optional) */
    int VSize)   /* I */

{
    /* NOTE:  set AName to empty string to get Name */

    int aindex;
    int astart;

    int EVSize;

    if (AVal != NULL) AVal[0] = '\0';

    if (E == NULL) {
        return (FAILURE);
    }

    EVSize = (VSize > 0) ? VSize : MAXLINE;

    if (ATok != NULL)
        astart = *ATok;
    else
        astart = -1;

    for (aindex = astart + 1; aindex < E->ACount; aindex++) {
        if ((AName == NULL) || (AName[0] == '\0') ||
            !strcmp(AName, E->AName[aindex])) {
            if ((AName != NULL) && (AName[0] == '\0')) {
                strncpy(AName, E->AName[aindex], MAXNAME);
                AName[MAXNAME - 1] = '\0';
            }

            if (AVal != NULL) {
                strncpy(AVal, E->AVal[aindex], EVSize);
                AVal[EVSize - 1] = '\0';
            }

            if (ATok != NULL) *ATok = aindex;

            return (SUCCESS);
        }
    } /* END for (aindex) */

    return (FAILURE);
} /* END MXMLGetAttr() */

/* combine and save information into a buffer */
char *buildMsgBuffer(showres_info_t *showres_info, client_info_t *client_info) {
	char *buffer;
	int len = 0;

	if(showres_info->jobid == NULL)
		showres_info->jobid = string_dup(NONE);


	if (showres_info->pName == NULL)
		if ((showres_info->pName = getenv(MSCHED_ENVPARVAR)) == NULL)
			showres_info->pName = string_dup(GLOBAL_MPARNAME);

	/* calculate the length of the whole buffer */

	/* plus one for a white space or a 0 */
	len += strlen(showres_info->jobid) + 1;
	len += strlen(showres_info->pName) + 1;

    if (showres_info->type == 0)
    	showres_info->type = JOB;

	if ((buffer = (char *) malloc(len + 33)) == NULL) {
		puts("ERROR: memory allocation failed");
		free_structs(showres_info, client_info);
		exit(EXIT_FAILURE);
	}

	/* build buffer */
	sprintf(buffer, "%d %s %d %s", showres_info->type, showres_info->pName,
			showres_info->flags, showres_info->jobid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showres_info_t *showres_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"grep",        no_argument,       0, 'g'},
            {"node",  		no_argument,       0, 'n'},
            {"partition",   required_argument, 0, 'p'},
            {"relativemode",no_argument,       0, 'r'},
            {"summary",     no_argument,       0, 's'},
            {"verbose",     no_argument,       0, 'v'},
            {"xml",         no_argument,       0, 'x'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVgnp:rsvxC:H:P:",
                         options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

          case 'h':
              print_usage();
              free_structs(showres_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              free_structs(showres_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'g':
        	  showres_info->mode |= (1 << GREP);
              break;

          case 'n':
        	  showres_info->type = NODE;
              break;

          case 'p':
        	  showres_info->pName = string_dup(optarg);
              break;

          case 'r':
        	  showres_info->mode |= (1 << RELATIVE);
              break;

          case 's':
        	  showres_info->flags |= (1 << SUMMARY);
              break;

          case 'v':
        	  showres_info->flags |= (1 << VERBOSE);
              break;

          case 'x':
        	  showres_info->flags |= (1 << XML);
              break;

          case 'C':
              printf ("set configfile to %s\n", optarg);
              client_info->configfile = string_dup(optarg);
              break;

          case 'H':
              printf ("set host to %s\n", optarg);
              client_info->host = string_dup(optarg);
              break;

          case 'P':
              client_info->port = string2int(optarg);
              if (client_info->port != INVALID_STRING)
                  printf ("set port to %s\n", optarg);
              break;

          case '?':
              /* getopt_long already printed an error message. */
              puts ("Try 'showres --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only accept one or no argument */
    if(optind < argc - 1){
        print_usage();
        free_structs(showres_info, client_info);
        exit(EXIT_FAILURE);
    }else if(optind == argc - 1){
		/* copy and save job id from input */
		showres_info->jobid = string_dup(argv[optind]);
    }

    return 1;
}

/* free memory */
void free_structs(showres_info_t *showres_info, client_info_t *client_info) {
    free(showres_info->jobid);
    free(showres_info->pName);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showres [FLAGS] [<RESERVATIONID>]\n\n"
            "Displays all reservations currently in place within Maui.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -g, --grep                     when used with the '-n' flag, shows 'grep'-able output\n"
    		"                                 	with nodename on every line\n"
    		"  -n, --node                     display information regarding all nodes reserved by\n"
    		"                                   RESERVATIONID\n"
    		"  -p, --partition=PARTITIONID    set partition\n"
    		"  -r, --relativemode             when used with the '-n' flag, display reservation\n"
    		"                                   timeframes in relative time mode(compared to the\n"
    		"                                   current time)\n"
    		"  -s, --summary                  display summary of reservations\n"
    		"  -v, --verbose                  show long reservation start dates including the\n"
    		"                                   reservation year\n"
    		"  -x, --xml                      display nodes in xml format\n");
    print_client_usage();
}
