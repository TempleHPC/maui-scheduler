/* HEADER */

#ifndef __MCOM_H
#define __MCOM_H

/* core defines */

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif /* MIN */

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif /* MAX */

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef NONE
#define NONE "[NONE]"
#endif /* NONE */

#ifndef ALL
#define ALL "[ALL]"
#endif /* ALL */

#ifndef DEFAULT
#define DEFAULT "[DEFAULT]"
#endif /* DEFAULT */

#ifndef SUCCESS
#define SUCCESS 1
#endif /* SUCCESS */

#ifndef FAILURE
#define FAILURE 0
#endif /* FAILURE */

#ifndef MMAX_NAME
#define MMAX_NAME 64
#endif /* MMAX_NAME */

#ifndef MMAX_LINE
#define MMAX_LINE 1024
#endif /* MMAX_LINE */

#ifndef MMAX_BUFFER
#define MMAX_BUFFER 65536
#endif /* MMAX_BUFFER */

#ifndef MCONST_CKEY
#define MCONST_CKEY "hello"
#endif /* MCONST_CKEY */

#ifndef mbool_t
#define mbool_t unsigned char
#endif /* mbool_t */

#define CRYPTHEAD "KGV"

/* enumerations */

/* sync w/MDFormat */

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

enum MSocketProtocolEnum {
    mspNONE = 0,
    mspSingleUseTCP,
    mspHalfSocket,
    mspHTTPClient,
    mspHTTP,
    mspS3Challenge
};

enum MWireProtocolEnum { mwpNONE = 0, mwpAVP, mwpXML, mwpHTML, mwpS32 };

/* sync w/MS3Action[] */

enum MS3ActionEnum {
    msssaNONE,
    msssaCancel,
    msssaCreate,
    msssaDestroy,
    msssaInitialize,
    msssaList,
    msssaModify,
    msssaNotify,
    msssaQuery,
    msssaStart,
    msssaLAST
};

/* sync w/MCSAlgoType[] */

enum MChecksumAlgoEnum {
    mcsaNONE = 0,
    mcsaDES,
    mcsaHMAC,
    mcsaHMAC64,
    mcsaMD5,
    mcsaPasswd,
    mcsaRemote
};

/* sync w/MS3CName[] */

enum MPeerServiceEnum {
    mpstNONE = 0,
    mpstNM, /* system monitor     */
    mpstQM, /* queue manager      */
    mpstSC, /* scheduler          */
    mpstMS, /* meta scheduler     */
    mpstPM, /* process manager    */
    mpstAM, /* allocation manager */
    mpstEM, /* event manager      */
    mpstSD, /* service directory  */
    mpstWWW
}; /* web                */

/* sync w/MS3VName[] */

enum MS3VEnum { msssV0_2 = 0, msssV3_0, msssV4_0 };

/* const defines */

#define MMAX_SBUFFER 65536

#ifndef MMSG_BUFFER
#define MMSG_BUFFER (MMAX_SBUFFER << 5)
#endif /* MMSG_BUFFER */

#define MCONST_S3XPATH 3
#define MCONST_S3URI "SSSRMAP3"

#define MMAX_S3ATTR 256
#define MMAX_S3VERS 4
#define MMAX_S3JACTION 64

/* default defines */

#define MDEF_CSALGO mcsaDES
#define MMAX_SOCKETWAIT 5000000

#define MMAX_XMLATTR 64
#define MDEF_XMLICCOUNT 16

/* structures */

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

/* failure codes */

/* sync w/MFC[] */

enum MSFC {
    msfENone = 0,            /* success */
    msfGWarning = 100,       /* general warning */
    msfEGWireProtocol = 200, /* general wireprotocol/network failure */
    msfEBind = 218,          /* cannot bind socket */
    msfEGConnect = 220,      /* general connection failure */
    msfCannotConnect = 222,  /* cannot connect */
    msfCannotSend = 224,     /* cannot send data */
    msfCannotRecv = 226,     /* cannot receive data */
    msfConnRejected = 230,   /* connection rejected */
    msfETimedOut = 232,      /* connection timed out */
    msfEFraming = 240,       /* general framing failure */
    msfEEOF = 246,           /* unexpected end of file */
    msfEGMessage = 300,      /* general message format error */
    msfENoObject = 311,      /* no object specified in request */
    msfEGSecurity = 400,     /* general security failure */
    msfESecClientSig = 422, /* security - signature creation failed at client */
    msfESecServerAuth = 424, /* security - server auth failure */
    msfESecClientAuth = 442, /* security - client auth failure */
    msfEGEvent = 500,        /* general event failure */
    msfEGServer = 700,       /* general server error */
    msfEGServerBus = 720,    /* general server business logic failure */
    msfEGClient = 800,       /* general client error */
    msfECInternal = 820,     /* client internal error */
    msfECResUnavail = 830,   /* client resource unavailable */
    msfECPolicy = 840,       /* client policy failure */
    msfEGMisc = 900,         /* general miscellaneous error */
    msfUnknownError = 999
}; /* unknown failure */

/* sync w/MSockAttr[] */

enum MSocketAttrEnum {
    msockaNONE = 0,
    msockaLocalHost,
    msockaLocalPort,
    msockaRemoteHost,
    msockaRemotePort,
    msockaLAST
};

#ifdef __M32COMPAT

#define mlog_t dlog_t

#endif /* __M32COMPAT */

#endif /* __MCOM_H */

/* END mcom.h */
