/*
*/

#if !defined(__SU_H) 
#define __SU_H

#define MSU_VERSION "4.0.2p0"

#ifdef __MTEST
#define SUEMLog fprintf
#endif /* __MTEST */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
                                                                                
#include <unistd.h>
#include <strings.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum {
  sumkNONE = 0,
  sumkStatusCode,
  sumkArgs,
  sumkAuth,
  sumkCommand,
  sumkData,
  sumkClient,
  sumkTimeStamp,
  sumkCheckSum };

#define MAX_SUNAME        64
#define MAX_SUPATH        256
#define MAX_SULINE        1024
#define MAX_SUBUFFER      65536
#define MAX_SURANGE       256
#define MAX_SUTIME        2140000000    

#ifndef NULL
#  define NULL (void *)0
#endif /* NULL */
 
#ifndef MIN
#  define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif /* MIN */
 
#ifndef MAX
#  define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif /* MAX */
 
#ifndef TRUE
#  define TRUE 1
#endif /* TRUE */
 
#ifndef FALSE
#  define FALSE 0
#endif /* FALSE */

#ifndef NONE
#  define NONE "[NONE]"
#endif /* NONE */

#ifndef ALL
#  define ALL "[ALL]"
#endif /* ALL */

#ifndef DEFAULT
#  define DEFAULT "[DEFAULT]"
#endif /* DEFAULT */

#ifndef SUCCESS
# define SUCCESS 1
#endif /* SUCCESS */

#ifndef FAILURE
# define FAILURE 0
#endif /* FAILURE */

#ifndef suscNOERROR
enum { suscNOERROR = 0, suscNOMEMORY, suscNOENT };
#endif /* suscNOERROR */

#ifndef DBG
# define DBG(S,X,F) if ((S != NULL) && (S->EMT != NULL) && (*S->EMT >= X) && (*S->EMF & F))
#endif /* DBG */

#ifndef sfNONE
enum sulogfacility {
  sfNONE     = 0,
  sfEM       = (1 << 0),
  sfMeta     = (1 << 1),
  sfQueue    = (1 << 2),
  sfJob      = (1 << 3),
  sfResource = (1 << 4),
  sfObj      = (1 << 5),
  sfFile     = (1 << 6),
  sfConfig   = (1 << 7),
  sfUtil     = (1 << 8),
  sfSocket   = (1 << 9),
  sfUI       = (1 << 10),
  sfOS       = (1 << 11),
  sfSec      = (1 << 12),
  sfMI       = (1 << 13),
  sfSC       = (1 << 14),
  sfRange    = (1 << 15),
  sfStat     = (1 << 16),
  sfCP       = (1 << 17),
  sfPolicy   = (1 << 18),
  sfXML      = (1 << 19),
  sfRM       = (1 << 20),
  sfAM       = (1 << 21),
  sfStruct   = (1 << 22) };

#define sfALL (sfEM | sfMeta | sfQueue | sfJob | sfResource | sfObj | sfFile |    \
               sfConfig | sfUtil | sfSocket | sfUI | sfOS | sfSec | sfMI | sfSC | \
               sfRange | sfStat | sfCP | sfPolicy | sfXML | sfRM | sfAM)
#endif /* sfNONE */

enum SUEMPEnum {
  suempNONE = 0,
  suempState,
  suempThreshold,
  suempFacilityList,
  suempLogFileMaxSize,
  suempLogFileRollDepth,
  suempLogDir,
  suempLogFileName };

enum SUCompList {
  sucmpNONE = 0,
  sucmpLT,
  sucmpLE,
  sucmpEQ,
  sucmpGT,
  sucmpGE,
  sucmpNE,
  sucmpEQ2,
  sucmpNE2,
  sucmpSSUB,
  sucmpSNE,
  sucmpSEQ };


/* sync w/SUIFlagType */

enum SUDFEnum {
  sudfNONE = 0,
  sudfXML,
  sudfFT,
  sudfVerbose };

#ifndef SUDEFRCLIST
#define SUDEFRCLIST

enum SURCListEnum {
  surclNONE = 0,
  surclYear,
  surclDecade,
  surclCentury,
  surclMillenium };
#endif /* SUDEFRCLIST */

/* sync with SObjList[] in SConst.c */

enum SUObjectList {
  suoNONE = 0,
  suoLocalJob,
  suoLocalRsv,
  suoUser,      /* grid user */
  suoJob,       /* grid job */
  suoQueue,     /* grid queue */
  suoRsv,       /* grid rsv */
  suoRes,       /* grid resource/cluster */
  suoSystem };  /* grid scheduler/server */

enum SUKeyList {
  sukNONE = 0,
  sukName,
  sukUser,
  sukRsvID };

#define MAX_SULIST     32
#define MAX_SUWORD     32
#define MAX_SUATTR     64

#define MAX_SUCONSTRAINT  4

/* sync w/MSUDataMode */

enum SUDataModeEnum { 
  sudmNONE = 0, 
  sudmHuman, 
  sudmXML, 
  sudmFlatText };

enum SUOAttrMode { 
  suoattrNONE = 0, 
  suoattrAdd, 
  suoattrCheck, 
  suoattrAppend, 
  suoattrReplace };

typedef char suattrtype_t[MAX_SULIST][MAX_SUATTR][MAX_SUNAME];

typedef struct {
  int    Size;
  int    Count;
  char **List;
  } suet_t;

enum SUAttrEnum {
  suaNONE = 0,
  suaCKey };


/* NOTE: sync w/mlog_t */

typedef struct
  {
  unsigned long  State;          /* External */

  int            Threshold;      /* External */
  unsigned long  FacilityList;   /* External */

  FILE          *logfp;

  unsigned long  LogFileMaxSize; /* External */
  int            LogFileRollDepth;

  char          *LogDir;         /* External */
  char          *LogFileName;    /* External */

  char          *LogPathName;

  char          *FileMode;

  int            ControlMode;

  char           Buffer[MAX_SUBUFFER];       
  } suem_t;

typedef struct
  {
  int   Type;
  char *Value;
  } suacl_t;

typedef struct
  {
  char *Name;
  char *Value;
  int   CIndex;
  } suattr_t;


/* value of type DType is set to DVal if condition of type CType is set to CVal */

typedef struct
  {
  char *DVal;  /* data value        */
  char *CVal;  /* conditional value */
  int   DType; /* data type         */
  int   CType; /* conditional type  */

  char  Source;
  } sucval_t;

typedef struct
  {
  char          *PathName;    /* (alloc) */
  long           ModifyTime;
  long           RefreshTime;
  unsigned long  BufferSize;
  char          *Buffer;      /* (alloc) */
  char          *Ptr;         /* read ptr */
  } sufilebuf_t;

/* CP class */
 
#define SUMAX_CPINDEX               2048
#define SUCP_OVERHEAD               64
#define SUDEF_CPFILENAME            ".server.ck"
#define SUDEF_CPBUFFERSTEPSIZE      4096
#define SUDEF_CPPAD                 32
#define SUDEF_CPMAXINTERVAL         300
 
typedef struct
  {
  long  UpdateTime;  /* time of last update */
  long  MaxInterval; /* max time between checkpoints */
 
  char *PathName;
 
  char *Buffer;
  int   BufSize;
 
  struct {
     char  State;   /* I=ignore  A=active  0=end marker */
     char *OType;
     char *OName;
     int   Size;    /* total size of record */
     int   Offset;
     long  ETime;
     } Index[SUMAX_CPINDEX];
  }  sucp_t;

typedef struct
  {
  char            *CommandName;
  char            *HomeDir;
  char            *HostName;
  unsigned long    HostAddress;
  char            *UserName;

  char            *SpoolDir;

  int              UID;
  int              PID;
  char           **ArgV;
  int              ProgType;

  int              TimeMode;
  int              TimeRatio;

  int              TimeServerPort;
  char            *TimeServerHost;

  long             RealTime;        /* actual 'real world' time   */
  struct timeval   SimTime;         /* accurate time within simulation */
  long             ETime;   
  } suenv_t;

typedef struct
  {
  char *Name;

  unsigned long  EncryptionMode;
  union
    {
    char CSKey[MAX_SUNAME];
    } EncryptionData;
  } suclcred_t;

enum { suspNONE = 0, suspLocalHost, suspRemoteHost, suspLocalPort, suspRemotePort };

typedef struct
  {
  unsigned long  SocketType;

  unsigned long  TimeOut;

  long           LastActivity;

  int            sd;

  int            RemotePort;
  int            LocalPort;

  char          *RemoteHost;
  char          *LocalHost;

  char          *ClientName;

  unsigned long  Flags;
  unsigned long  State;

  unsigned long  Version;

  unsigned long  AuthenticationMode;
  void          *AuthenticationData;

  int            WireProtocol;
  
  suclcred_t    *Cred;
  } susocket_t;

typedef struct
  {
  char buffer[MAX_SUBUFFER];
  int length;
  int state;
  size_t msgSize;
  } sucomm_parser_t;

/* socket states */

enum { sussNONE = 0, sussClosed, sussOpen, sussBusy };

#define MAX_SUCLCRED 256

enum {
  sustNONE = 0,
  sustTCP  = (1 << 0),
  sustUDP  = (1 << 1) };
 
#define SUDEF_RSVOVERLAP        300
 
 

#ifndef __SUINTERNAL_H

typedef void sumb_t;

typedef struct
  {
  /* order must NOT change */

  int           *EMT;  /* must be first in array (HACK) */
  unsigned long *EMF;

  suem_t        *EM;
  sucp_t        *CP;
  suenv_t       *Env;

  suclcred_t    *Cred[MAX_SUCLCRED];

  char          *Name;

  long           State;
  time_t         PresentTime;

  /* END order must NOT change */
  } su_t;

enum SUDataTypeEnum {
  sudtNONE = 0,
  sudtString,
  sudtInt,
  sudtLong,
  sudtDouble,
  sudtStringArray,
  sudtIntArray,
  sudtLongArray,
  sudtDoubleArray,
  sudtOther };
 
#endif /* __SUINTERNAL_H */

#endif /* !defined(__SU_H) */

/* END msu.h */

