/* HEADER */

        
/* Contains:                                           *
 *   int MSUListen(S)                                  *
 *   int MSUInitialize(S,Host,Port,TimeOut,SocketType) *
 *   int MSUInitializeResponse(S)                      *
 *   int MSUConnect(S,Force,EMsg)                      *
 *   int MSUCreate(SP)                                 *
 *   int MSUAcceptClient(ServerSD,ClientSD,HostName,SocketType) *
 *   int MSUSendData(S,TimeLimit,DoSocketLayerAuth,IsResponse) *
 *   int MSUSendPacket(sd,Message,MessageSize,TimeOut,SC) *
 *   int MSURecvData(S,TimeLimit,Flag,SC,EMsg)         *
 *   int MSURecvPacket(sd,Buffer,BufSize,Pattern,TimeOut) *
 *   int MSUSelectWrite(sd,TimeLimit)                  *
 *   int MSUSelectRead(sd,TimeLimit)                   *
 *   int MSUSetAttr(S,AIndex,Value)                    *
 *   int MUSystemF(Command,TimeLimit,PID)              *
 *   int MUClearChild(PID)                             *
 *   char *MOSGetHostName(HostName,FullHostName,Address) *
 *   int MSUDisconnect(S)                              *
 *                                                     */


#include "moab.h"
#include "moab-proto.h"  
#include "msu.h"

extern char *MSON[];
extern const char *MBool[];


#ifdef __AIX42
# define _NO_PROTO
# include <arpa/inet.h>
# undef _NO_PROTO
#else /* __AIX42 */
# ifndef __NT
#  include <arpa/inet.h>
# endif /* __NT */
#endif /* AIX42 */

#ifdef __NT
#include <winsock2.h>
#endif  /* __NT */

#define SOCKETQUEUESIZE      64
#define SOCKETFLAGS      (int)0

extern mlog_t    mlog;

extern msched_t  MSched;
extern mpid_t    MPID[];

extern mx_t      X;

extern const char *MCKeyword[];
extern const char *MSockAttr[];

char tmpSBuf[MMSG_BUFFER];  /* NOTE: global to avoid compiler-specific stack failures */
                            /* (not threadsafe) */


msocket_t MClS[MMAX_CLIENT];  /* no longer used in live code */

int MSUClientCount = 0;       /* keeps a count of number of connected clients */

/* local prototypes */

int MAIX_ISSET(int,int *);
int MAIX_SET(int,int *);
int MAIX_CLR(int,int *);

/* END local prototypes */




int MSUIPCInitialize()

  {
  int index;

#ifndef __MPROD
  const char *FName = "MSUIPCInitialize";

  MDB(2,fSOCK) MLog("%s()\n",
    FName);
#endif /* !__MPROD */

#ifdef __NT
  /* initialize winsock */
#endif /* __NT */

#ifdef __MGSS
  /* initialize grid credential handler */

  MSched.CredHandler = MGSSPostCred;
#endif /* __MGSS */

  memset(MClS,0,sizeof(MClS));

  for (index = 0;index < MMAX_CLIENT;index++)
    {
    MClS[index].sd = -1;
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MSUIPCInitialize() */




int MSUInitialize(

  msocket_t *S,          /* I (modified) */
  char      *RHostName,  /* I (optional) */
  int        RPort,      /* I */
  long       CTimeout,   /* I */
  long       SFlags)     /* I */

  {
  if (S == NULL)
    {
    return(FAILURE);
    }

  memset(S,0,sizeof(msocket_t));

  S->sd         = -1;

  /* version is unknown, known versions are > 0 */

  S->Version    = -1;

  MUStrCpy(S->RemoteHost,RHostName,sizeof(S->RemoteHost));

  S->RemotePort = RPort;

  S->Timeout    = CTimeout;

  S->Flags      = SFlags;

  return(SUCCESS);
  }  /* END MSUInitialize() */




int MUISExtractData(

  msocket_t *S)  /* I */

  {
  if (S == NULL)
    {
    return(FAILURE);
    }

  /* NYI */

  return(SUCCESS);
  }  /* END MUISExtractData() */




int MUISCreateFrame(

  msocket_t *S,          /* I (modified) */
  mbool_t    DoCompress, /* I */
  mbool_t    IsResponse) /* I */

  {
  const char *FName = "MUISCreateFrame";

  if (S == NULL)
    {
    return(FAILURE);
    }

  switch (S->WireProtocol)
    {
    case mwpS32:

      {
      mxml_t *E = NULL;
      mxml_t *tE;
      mxml_t *BE;
      mxml_t *RE;
      mxml_t *SE = NULL;

      char Signature[MMAX_LINE];
      char Digest[MMAX_LINE];

      int   BufSize;

      char *tmpBuf = NULL;

      /* create message framing */

      if (S->SE != NULL)
        {
        /* frame previously created */

        return(SUCCESS);
        }

      MXMLCreateE(&E,MSON[msonEnvelope]);

      MXMLSetAttr(E,"count",(void *)"1",mdfString);
      MXMLSetAttr(E,"name",(void *)MSCHED_SNAME,mdfString);

      if ((S->IsNonBlocking == TRUE) && (IsResponse == FALSE))
        {
        MXMLSetAttr(E,"type",(void *)"nonblocking",mdfString);
        }

      MXMLSetAttr(E,"version",(void *)MOAB_VERSION,mdfString);
      MXMLSetAttr(E,"component",(void *)"ClusterScheduler",mdfString);

      if (S->CSAlgo != mcsaNONE)
        {
        SE = NULL;

        MXMLCreateE(&SE,"Signature");
        MXMLAddE(E,SE);

        /* NOTE:  signature currently not populated */
        }

      BE = NULL;

      MXMLCreateE(&BE,MSON[msonBody]);

      /* NOTE:  actor not needed in body for S3 3.0 */

      if (S->ClientName != NULL)
        MXMLSetAttr(BE,"actor",(void *)S->ClientName,mdfString);
      else
        MXMLSetAttr(BE,"actor",(void *)MUUIDToName(MOSGetEUID()),mdfString);

      MXMLAddE(E,BE);

      S->SE = (void *)E;

      if (IsResponse == TRUE)
        {
        tE = NULL;

        MXMLCreateE(
          &tE,
          MSON[msonResponse]);

        MXMLAddE(BE,tE);

        /* add status information */

        /* get first 'Body' child (ignore signature) */

        if ((MXMLGetChild((mxml_t *)S->SE,MSON[msonBody],NULL,&BE) == FAILURE) ||
            (MXMLGetChild(BE,NULL,NULL,&RE) == FAILURE))
          {
          return(FAILURE);
          }

        if (MS3SetStatus(
             RE,
             (S->StatusCode == 0) ? (char *)"Success" : (char *)"Failure",
             (enum MSFC)(S->StatusCode % 1000),
             S->SMsg) == FAILURE)
          {
          return(FAILURE);
          }
 
        /* add data */

        if (S->SDE == NULL)
          {
          MXMLCreateE((mxml_t **)&S->SDE,MSON[msonData]);

          if ((DoCompress == TRUE) && (S->SPtr != NULL))
            {
            if (MSecCompress(
                  (unsigned char *)S->SPtr,  /* I/O */
                  strlen(S->SPtr),
                  NULL,
                  MCONST_CKEY) == FAILURE)
              {
              MDB(0,fSOCK) MLog("WARNING:  cannot compress data in %s\n",
                FName);

              return(FAILURE);
              }
            }
          else
            {
            /* NOTE:  handle proper freeing of send data */

            ((mxml_t *)S->SDE)->Val = S->SPtr;

            /* should S->SPtr be set to null?  should S->SBuffer be freed? */
            }  /* END if ((DoCompress == TRUE) && ...) */
          }

        MXMLAddE(RE,(mxml_t *)S->SDE);

        S->SDE = NULL;
        }  /* END if (IsResponse == TRUE) */
      else
        {
        /* message is request */

        /* add data */

        if (S->SDE != NULL)
          {
          RE = (mxml_t *)S->SDE;

          S->SDE = NULL;
          }
        else
          {
          RE = NULL;

          MXMLFromString(&RE,S->SBuffer,NULL,NULL);
          }

        if (S->ClientName != NULL)
          MXMLSetAttr(RE,"actor",(void *)S->ClientName,mdfString);
        else
          MXMLSetAttr(RE,"actor",(void *)MUUIDToName(MOSGetEUID()),mdfString);

        MXMLAddE(BE,RE);
        }  /* END else (IsResponse == TRUE) */

      /* NOTE:  do not check exit status, only checksum first 64K */

      MXMLToXString(BE,&tmpBuf,&BufSize,MMAX_BUFFER << 5,NULL,TRUE);

      if (S->DoEncrypt == TRUE)
        {
        /* encrypt body element */

        /* NYI */ 
        }

      /* generate checksum on first 64k body element */

      if (SE != NULL)
        {
        MSecGetChecksum(
          tmpBuf,
          strlen(tmpBuf),
          Signature,
          Digest,
          mcsaHMAC64,
          S->CSKey);

        tE = NULL;
        MXMLCreateE(&tE,"DigestValue");

        MXMLSetVal(
          tE,
          (void *)Digest,
          mdfString);

        MXMLAddE(SE,tE);

        tE = NULL;
        MXMLCreateE(&tE,"SignatureValue");

        MXMLSetVal(
          tE,
          (void *)Signature,
          mdfString);

        MXMLAddE(SE,tE);
        }  /* END if (SE != NULL) */

      MUFree(&tmpBuf);
      }    /* END BLOCK */

      break;

    case mwpXML:

      /* NO-OP */

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END MUISCreateFrame() */




int MSUListen(

  msocket_t *S)  /* I */

  {
  int                sd;
  int                flags;
  int                on;

  struct sockaddr_in sin;

#ifndef __MPROD
  const char *FName = "MSUListen";

  MDB(2,fSOCK) MLog("%s(%s)\n",
    FName,
    (S != NULL) ? "S" : "NULL");
#endif /* !__MPROD */

  if (S == NULL)
    {
    return(FAILURE);
    }

  /* create stream socket, listen on S->RemotePort */

  if ((sd = socket(
              AF_INET,
              MISSET(S->Flags,msftTCP) ? SOCK_STREAM : SOCK_DGRAM,
              0)) == -1)
    {
    MDB(0,fSOCK) MLog("WARNING:  cannot open service socket, errno: %d (%s)\n",
      errno,
      strerror(errno));

    return(FAILURE);
    }

  flags = fcntl(sd,F_GETFD,0);

  if (flags >= 0)
    {
    flags |= FD_CLOEXEC;

    fcntl(sd,F_SETFD,flags);
    }

  /* allow port re-use */

  on = 1;

  if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) == -1)
    {
    MDB(1,fSOCK) MLog("WARNING:  cannot set socket REUSEADDR option, errno: %d (%s)\n",
      errno,
      strerror(errno));
    }

  memset((char *)&sin,0,sizeof(sin));

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);

  /* assign port/bind socket to IP address and port */

  if (S->RemotePort == -1)
    {
    for (S->RemotePort = 10000;S->RemotePort < 20000;S->RemotePort++)
      {
      sin.sin_port = htons((unsigned short)S->RemotePort);

      if (bind(sd,(struct sockaddr *)&sin,sizeof(sin)) == -1)
        {
        if (errno == EADDRINUSE)
          continue;

        MDB(0,fSOCK) MLog("WARNING:  cannot bind to port %d, errno: %d (%s)\n",
          S->RemotePort,
          errno,
          strerror(errno));

        close(sd);

        return(FAILURE);
        }
      else
        {
        break;
        }
      }

    if (S->RemotePort == 20000)
      {
      MDB(0,fSOCK) MLog("ERROR:    cannot locate available port\n");

      close(sd);

      return(FAILURE);
      }
    }
  else
    {
    sin.sin_port = htons((unsigned short)S->RemotePort);

    if (bind(sd,(struct sockaddr *)&sin,sizeof(sin)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot bind to port %d, errno: %d (%s)\n",
        S->RemotePort,
        errno,
        strerror(errno));

      close(sd);

      return(FAILURE);
      }
    }

  MDB(6,fSOCK) MLog("INFO:     socket bound to port %d\n",
    S->RemotePort);

  if (MISSET(S->Flags,msftTCP))
    {
    /* enable non-blocking mode */

    if ((flags = fcntl(sd,F_GETFL,0)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot get socket attribute values, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(sd);

      return(FAILURE);
      }

    if (fcntl(sd,F_SETFL,(flags | O_NDELAY)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot set socket NDELAY attribute, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(sd);

      return(FAILURE);
      }

    /* establish queue size */

    listen(sd,SOCKETQUEUESIZE);
    }

  MDB(2,fSOCK) MLog("INFO:     opened service socket on port %d\n",
    S->RemotePort);

  MDB(6,fSOCK) MLog("INFO:     fd: %d  family: %d  addr: %p\n",
    sd,
    sin.sin_family,
    &sin.sin_addr);

  S->sd = sd;

  return(SUCCESS);
  }  /* END MSUListen() */





int MSUConnect(

  msocket_t *S,     /* I (modified) */
  mbool_t    Force, /* I */
  char      *EMsg)  /* O (optional,minsize=MMAX_LINE) */

  {
  struct sockaddr_in  s_sockaddr;
  struct hostent     *s_hostent;
  struct in_addr      in;

  int                 flags;

  char               *hptr = "localhost";

#ifndef __MPROD
  const char *FName = "MSUConnect";

  MDB(4,fSOCK) MLog("%s(%s,%s,EMsg)\n",
    FName,
    (S != NULL) ? "S" : "NULL",
    MBool[Force]);
#endif /* !__MPROD */

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (S == NULL)
    {
    if (EMsg != NULL)
      strcpy(EMsg,"invalid parameter");
  
    return(FAILURE);
    }

  if (S->sd >= 0)
    {
    if (Force == FALSE)
      {
      /* connection already established */

      return(SUCCESS);
      }

    MSUDisconnect(S);
    }

  S->sd = -1;

  memset(&s_sockaddr,0,sizeof(s_sockaddr));

  if ((S->RemoteHost[0] != '\0'))
    {
    hptr = S->RemoteHost;
    }

  if (inet_aton(hptr,&in) == 0)
    {
    if ((s_hostent = gethostbyname(hptr)) == (struct hostent *)NULL)
      {
      MDB(1,fSOCK) MLog("ERROR:    cannot resolve IP address from hostname '%s', errno: %d (%s)\n",
        hptr,
        errno,
        strerror(errno));

      if (EMsg != NULL)
        strcpy(EMsg,"cannot resolve address");

      return(FAILURE);
      }

    memcpy(&s_sockaddr.sin_addr,s_hostent->h_addr,s_hostent->h_length);
    }
  else
    {
    memcpy(&s_sockaddr.sin_addr,&in.s_addr,sizeof(s_sockaddr.sin_addr));
    }

  MDB(5,fSOCK) MLog("INFO:     trying to connect to %s (Port: %d)\n",
    inet_ntoa(s_sockaddr.sin_addr),
    S->RemotePort);

  s_sockaddr.sin_family = AF_INET;

  s_sockaddr.sin_port = htons(S->RemotePort);

  if ((S->sd = socket(
        PF_INET,
        MISSET(S->Flags,msftTCP) ? SOCK_STREAM : SOCK_DGRAM,
        0)) < 0)
    {
    MDB(1,fSOCK) MLog("ERROR:    cannot create socket, errno: %d (%s)\n",
      errno,
      strerror(errno));

    if (EMsg != NULL)
      strcpy(EMsg,"cannot create socket");

    return(FAILURE);
    }

  fcntl(S->sd,F_SETFD,1);

  if (MISSET(S->Flags,msftTCP) && 
     (S->Timeout > 0) &&
     (S->SocketProtocol != mspHalfSocket) &&
     (S->SocketProtocol != mspS3Challenge))
    {
    /* enable non-blocking mode on client */

    if ((flags = fcntl(S->sd,F_GETFL,0)) == -1)
      {
      MDB(1,fSOCK) MLog("WARNING:  cannot get socket attribute values, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(S->sd);

      if (EMsg != NULL)
        strcpy(EMsg,"cannot enable non-blocking mode");

      return(FAILURE);
      }

    if (fcntl(S->sd,F_SETFL,(flags | O_NDELAY)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot set socket NDELAY attribute, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(S->sd);

      if (EMsg != NULL)
        strcpy(EMsg,"cannot enable non-blocking mode");

      return(FAILURE);
      }

    MDB(5,fSOCK) MLog("INFO:     non-blocking mode established\n");
    }

  if (connect(S->sd,(struct sockaddr*)&s_sockaddr,sizeof(s_sockaddr)) == -1)
    {
    if ((errno == EINPROGRESS) && (S->Timeout > 0))
      {
      /* wait if non-blocking */

      if (MSUSelectWrite(S->sd,S->Timeout) == FAILURE)
        {
        /* connect() has taken too long */

        MDB(4,fSOCK) MLog("ERROR:    cannot connect to server '%s' on port %d, errno: %d (%s)\n",
          hptr,
          S->RemotePort,
          errno,
          strerror(errno));

        close(S->sd);

        if (EMsg != NULL)
          strcpy(EMsg,"cannot establish connection");

        return(FAILURE);
        }
      }
    else
      {
      MDB(4,fSOCK) MLog("ERROR:    cannot connect to server '%s' on port %d, errno: %d (%s)\n",
        (hptr != NULL) ? hptr : "NULL",
        S->RemotePort,
        errno,
        strerror(errno));

      close(S->sd);

      if (EMsg != NULL)
        {
        sprintf(EMsg,"cannot establish connection - %s",
          strerror(errno));
        }

      return(FAILURE);
      }   /* END if (errno == EINPROGRESS) && ...) */
    }     /* END if (connect() == -1) */

  MDB(5,fSOCK) MLog("INFO:     successful connect to %s server (sd: %d)\n",
    MISSET(S->Flags,msftTCP) ? "TCP" : "UDP",
    S->sd);

  return(SUCCESS);
  }  /* END MSUConnect() */




/* NOTE:  MSUAcceptClient() should be combined with MSUReadData() */

int MSUAcceptClient(

  msocket_t  *S,           /* I */
  msocket_t  *C,           /* O (modified) */
  char       *HostName,    /* O (optional) */
  int         SocketType)  /* I (bitmap of enum MSockFTypeEnum) */

  {
  int sd;

#if defined(__LINUX) || defined(_SOCKLEN_T)
  socklen_t          addrlen;     
#elif defined(__AIX51)
  unsigned long      addrlen;
#else
  int                addrlen;
#endif /* __LINUX || _SOCKLEN_T */

  struct sockaddr_in c_sockaddr;
  int                flags;
  char              *NetAddr;
  struct hostent    *hoststruct;

  struct in_addr    *NA;

#ifndef __MPROD
  const char *FName = "MSUAcceptClient";

  MDB(9,fSOCK) MLog("%s(%d,ClientSD,HostName,%s)\n",
    FName,
    S->sd,
    MISSET(SocketType,msftTCP) ? "TCP" : "UDP");
#endif /* !__MPROD */

  if ((S == NULL) && (C == NULL))
    {
    return(FAILURE);
    }

  addrlen = sizeof(struct sockaddr_in);

  if ((sd = accept(S->sd,(struct sockaddr *)&c_sockaddr,&addrlen)) == -1)
    { 
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
      return(FAILURE);
      }

    MDB(9,fSOCK) MLog("INFO:     accept call failed, errno: %d (%s)\n",
      errno,
      strerror(errno));

    return(FAILURE);
    }

  NA = (struct in_addr *)&c_sockaddr.sin_addr.s_addr;

  NetAddr = inet_ntoa(*NA);

  MDB(3,fSOCK) MLog("INFO:     connect request from %s\n",
    NetAddr);

  if (MISSET(SocketType,msftTCP))
    {
    /* enable non-blocking mode on client */

    if ((flags = fcntl(sd,F_GETFL,0)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot get client socket attribute values, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(sd);

      return(FAILURE);
      }

    if (fcntl(sd,F_SETFL,(flags | O_NDELAY)) == -1)
      {
      MDB(0,fSOCK) MLog("WARNING:  cannot set client socket NDELAY attribute, errno: %d (%s)\n",
        errno,
        strerror(errno));

      close(sd);

      return(FAILURE);
      }

    MDB(8,fSOCK) MLog("INFO:     non-blocking mode established\n");

#   if !defined(__AIX41) && !defined(__AIX42) && !defined(__AIX43) && !defined(__AIX51)

    {
    struct linger Linger;

    /* enable linger mode */

    Linger.l_onoff  = 1;
    Linger.l_linger = 60;

    if (setsockopt(sd,SOL_SOCKET,SO_LINGER,&Linger,sizeof(struct linger)) == -1)
      {
      MDB(1,fSOCK) MLog("WARNING:  cannot set client socket LINGER attribute, errno: %d (%s)\n",
        errno,
        strerror(errno));
      }

    MDB(6,fSOCK) MLog("INFO:     socket linger enabled\n");
    }  /* END BLOCK */

#   endif  /* END !__AIX** */
    }  /* END if (MISSET(SocketType,msftTCP)) */

  if ((hoststruct = gethostbyaddr(
                      (char *)&c_sockaddr.sin_addr.s_addr,
                      sizeof(c_sockaddr.sin_addr.s_addr),
                      AF_INET)) == (struct hostent *)NULL)
    {
    MDB(2,fSOCK)
      {
      MDB(2,fSOCK) MLog("WARNING:  cannot get hostname of client, errno %d (%s)\n",
        errno,
        strerror(errno));
      }
/*
    close(sd);

    return(FAILURE);
*/
    }
  else
    {
    MDB(2,fSOCK) MLog("INFO:     received service request from host '%s'\n",
      hoststruct->h_name);
    }

  if (HostName != NULL)
    {
    if (hoststruct == NULL)
      strcpy(HostName,"[UNKNOWN]");
    else
      strcpy(HostName,hoststruct->h_name);
    }

  C->sd = sd;

  MDB(5,fSOCK) MLog("INFO:     %s client connected at sd %d\n",
    MISSET(SocketType,msftTCP) ? "TCP" : "UDP",
    sd);
 
  return(SUCCESS);
  }  /* END MSUAcceptClient() */





#define MCONST_SMALLPACKETSIZE 8192

int MSUSendData(

  msocket_t *S,          /* I */
  long       TimeLimit,  /* I */
  mbool_t    DoSocketLayerAuth, /* I */
  mbool_t    IsResponse) /* I */

  {
  char TSLine[MMAX_LINE];
  char CKLine[MMAX_LINE];
  char SHeader[MMAX_LINE + MCONST_SMALLPACKETSIZE];

  char CKSum[MMAX_LINE];

  /* NOTE:  RH7.x compiler fails on full local MMAX_SBUFFER tmpSBuf */

  time_t Now;

  long   PacketSize;

  char  *sptr = NULL;

  enum MStatusCodeEnum SC;

  const char *FName = "MSUSendData";

#ifndef __MPROD
  MDB(2,fSOCK) MLog("%s(%s,%ld,%s,%s)\n",
    FName,
    (S != NULL) ? "S" : "NULL",
    TimeLimit,
    (DoSocketLayerAuth == TRUE) ? "TRUE" : "FALSE",
    (IsResponse == TRUE) ? "TRUE" : "FALSE");
#endif /* !__MPROD */

  /* initialize */
  TSLine[0]='\0';
  CKLine[0]='\0';
  SHeader[0]='\0';

  tmpSBuf[0] = '\0';  /* tmpSBuf is global (not threadsafe) */

  switch (S->WireProtocol)
    {
    case mwpS32:

      /* create message framing */

      MUISCreateFrame(S,TRUE,IsResponse);

      MXMLToString(
        (mxml_t *)S->SE,
        tmpSBuf,
        sizeof(tmpSBuf),
        NULL,
        !MSched.EnableEncryption);

      sptr = S->SBuffer;

      S->SBuffer  = tmpSBuf;
      S->SBufSize = strlen(tmpSBuf);

      DoSocketLayerAuth = FALSE;

      break;

    case mwpXML:

      if (S->SE != NULL)
        {
        char tmpStatus[MMAX_LINE];

        int  HeadSize;
        int  Align;

        /* package string */

        switch (S->SocketProtocol)
          {
          case mspHTTP:
          case mspHTTPClient:

            {
            int   len;

            char *BPtr;
            int   BSpace;

            /* mxml_t *tE = NULL; */

            BPtr   = tmpSBuf;
            BSpace = sizeof(tmpSBuf);

            /* build XML header */

            /* NOTE:  S->E appended to end of header (tE ignored) */

            len = snprintf(BPtr,BSpace,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

            if (len >= 0) { BPtr += len; BSpace -= len; }

            /*
            MXMLCreateE(&tE,"Message");

            MXMLSetAttr(
              tE,
              "xmlns",
              "http://www.scidac.org/ScalableSystems/AllocationManager",
              mdfString);

            MXMLSetAttr(
              tE,
              "xmlns:xsi",
              "http://www.w3.org/2001/XMLSchema-instance",
              mdfString);

            MXMLSetAttr(
              tE,
              "xsi:schemaLocation",
              "http://www.scidac.org/ScalableSystems/AllocationManager am.xsd",
              mdfString);

            MXMLAddE(tE,S->E);
            */

            MXMLToString(
              (mxml_t *)S->SE,
              BPtr,
              BSpace,
              NULL,
              TRUE);
         
            /* 
            MXMLDestroyE(&tE); 
            S->SE = NULL;
            */

            MXMLDestroyE((mxml_t **)&S->SE);
            }  /* END BLOCK */
 
            break;

          default:

            {
            char *BPtr;
            int   BSpace;

            sptr = S->SBuffer;

            S->SBuffer = tmpSBuf;

            MUSNInit(&BPtr,&BSpace,tmpSBuf,sizeof(tmpSBuf));

            MUSNPrintF(&BPtr,&BSpace,"%s%d ",
              MCKeyword[mckStatusCode],
              scSUCCESS);

            Align = (int)strlen(tmpSBuf) + (int)strlen(MCKeyword[mckArgs]);

            MUSNPrintF(&BPtr,&BSpace,"%*s%s",
              16 - (Align % 16),
              " ",
              MCKeyword[mckData]);

            HeadSize = (int)strlen(S->SBuffer);

            MXMLToString(
              (mxml_t *)S->SE,
              S->SBuffer + HeadSize,
              sizeof(tmpSBuf),
              NULL,
              TRUE);
 
            if (MXMLGetAttr((mxml_t *)S->SE,"status",NULL,tmpStatus,sizeof(tmpStatus)) == SUCCESS) 
              {
              char *ptr;
 
              ptr = S->SBuffer + strlen(MCKeyword[mckStatusCode]);

              *ptr = tmpStatus[0];
              }
            }

            break;
          }  /* END switch(S->SocketProtocol) */

        S->SBufSize = strlen(S->SBuffer) + 1;
        }  /* END if (S->SE != NULL) */ 

      break;

    default:

      /* no where else is this set until here */

      if (S->SBuffer != NULL)
        {
        S->SBufSize = strlen(S->SBuffer);
        }
      else
        {
        S->SBufSize = 0;

        /* FAIL and exit below */
        }

      break;
    }  /* END switch (S->WireProtocol) */

  /* initialize connection/build header */

  if (S->SBuffer == NULL)
    {
    MDB(2,fSOCK) MLog("ALERT:    empty message in %s\n",
      FName);

    return(FAILURE);
    }

  SHeader[0] = '\0';

  switch (S->SocketProtocol)
    {
    case mspHalfSocket:

      /* NO-OP */

      break;

    case mspS3Challenge:

      {
      char *ptr;
  
      char  tmpChallenge[MMAX_LINE];
      char  tmpResponse[MMAX_LINE];

      char  tmpLine[MMAX_LINE];

      /* NOTE:  assume previously connected */

      if (S->CSKey[0] != '\0')
        {
        /* must receive challenge to generate header */

        /* NOTE:  read to first '\n' */

        ptr = tmpChallenge;

        if (MSURecvPacket(
             S->sd,
             &ptr,
             MMAX_LINE,
             "\n",
             MAX(1000000,TimeLimit),
             NULL) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot read half socket data\n");

          return(FAILURE);
          }

        /* remove '\n' */

        ptr[strlen(ptr) - 1] = '\0';

        if (MSecGetChecksum(
             tmpChallenge,
             strlen(tmpChallenge),
             tmpResponse,
             NULL,
             mcsaMD5,
             S->CSKey) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot determine digest\n");

          return(FAILURE);
          }

        /* FORMAT:  <DIGEST>\n */

        sprintf(tmpLine,"%s\n",
          tmpResponse);

        if (MSUSendPacket(
              S->sd,
              tmpLine,
              strlen(tmpLine),
              MAX(1000000,TimeLimit),
              &SC) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot send packet data\n");

          return(FAILURE);
          }

        /* receive response */

        if (MSURecvPacket(
             S->sd,
             &ptr,
             1,
             NULL,
             MAX(1000000,TimeLimit),
             NULL) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot read half socket data\n");

          return(FAILURE);
          }

        if (ptr[0] != '1')
          {
          MDB(1,fSOCK) MLog("ALERT:    invalid challenge response '%c'\n",
            ptr[0]);

          return(FAILURE);
          }
        }    /* END if (S->CSKey[0] != '\0') */

      /* FORMAT:  <BYTECOUNT> <MESSAGE> */

      sprintf(SHeader,"%d ",
        (int)strlen(S->SBuffer));
      }    /* END BLOCK */

      break;

    case mspHTTPClient:
    case mspHTTP:

      {
      if (S->URI != NULL)
        {
        /* FORMAT:  'GET <URI> %s\r\n\r\n' */

        sprintf(SHeader,"GET %s %s\r\n\r\n",
          S->URI,
          "HTTP/1.0");
        }
      else
        {
        char tmpBuf[MMAX_NAME];

        /* FORMAT:  'POST /SSSRMAP3 HTTP/1.1\r\nContent-Type: text/xml; '  */
        /*          'charset="UTF-8"\r\nContent-Length:_<LENGTH>\r\n\r\n' */

        sprintf(tmpBuf,"%x",
          (unsigned int)strlen(S->SBuffer));

        MUStrToUpper(tmpBuf,NULL,0);

        /* remove 'Connection: close' */

        sprintf(SHeader,"POST /%s %s\r\nContent-Type: %s; charset=\"utf-8\"\r\nTransfer-Encoding: %s;\r\n\r\n%s\r\n",
          "SSSRMAP3",
          "HTTP/1.1",
          "text/xml",
          "chunked",
          tmpBuf);
        }

      /* append chunk terminator */

      /* perform bounds checking (NYI) */

      S->SBuffer[S->SBufSize] = '0';
      S->SBuffer[S->SBufSize++] = '\r';
      S->SBuffer[S->SBufSize++] = '\n';
      S->SBuffer[S->SBufSize++] = '\0';

      }  /* END BLOCK */

      break;

    default:

      /* FORMAT:  <SIZE><CHAR>CK=<CKSUM><WS>TS=<TS><WS>ID=<ID><WS>[CLIENT=<CLIENT><WS>]DT=<MESSAGE> */

      CKLine[0] = '\0';

      SHeader[sizeof(SHeader) - 1] = '\0';

      if (DoSocketLayerAuth == TRUE)
        {
        char tmpStr[MMAX_BUFFER];

        time(&Now);

        sprintf(tmpStr,"%s%ld %s%s",
          MCKeyword[mckTimeStamp],
          (long)Now,
          MCKeyword[mckAuth],
          MUUIDToName(MOSGetEUID()));

        if (S->Name[0] != '\0')
          {
          sprintf(temp_str," %s%s",
            MCKeyword[mckClient],
            S->Name);
          strcat(TSLine,temp_str);
          }

        sprintf(TSLine,"%s %s",
          tmpStr,
          MCKeyword[mckData]);
        
        MSecGetChecksum2(
          TSLine,
          strlen(TSLine),
          S->SBuffer,
          strlen(S->SBuffer),  /* NOTE:  was S->SBufSize */
          CKSum,
          NULL,
          S->CSAlgo,
          S->CSKey);

        sprintf(CKLine,"%s%s %s",
          MCKeyword[mckCheckSum], 
          CKSum,
          TSLine);
        }  /* END if (DoSocketLayerAuth == TRUE) */
        
      PacketSize = S->SBufSize;

      if (isprint(S->SBuffer[0]))
        {
        /* check for binary data - FIXME */

        PacketSize = strlen(S->SBuffer);
        }

      sprintf(SHeader,"%08ld\n%s",
        PacketSize + (long)strlen(CKLine),
        CKLine);

      break;
    }  /* END switch (S->SocketProtocol) */

  MDB(7,fSOCK) MLog("INFO:     header created '%s'\n",
    SHeader);

  /* send data */

  switch (S->SocketProtocol)
    {
    case mspS3Challenge:
    case mspHalfSocket:

      if (SHeader[0] != '\0')
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot send packet header '%s'\n",
          SHeader);

        if (MSUSendPacket(
              S->sd,
              SHeader,
              strlen(SHeader),
              MAX(1000000,TimeLimit),
              &SC) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot send packet header '%s'\n",
            SHeader);

          return(FAILURE);
          }
        }    /* END if (SHeader[0] != '\0') */

      if (MSUSendPacket(
            S->sd,
            S->SBuffer,
            S->SBufSize,
            MAX(1000000,TimeLimit),
            &SC) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot send packet data\n");

        return(FAILURE);
        }

      if (shutdown(S->sd,SHUT_WR) == -1)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot close send connections (%d : %s)\n",
          errno,
          strerror(errno));

        if (errno != ENOTCONN)
          {
          /* cannot properly close connection */

          /* NOTE:  cannot be certain data was successfully transferred */

          return(FAILURE);
          }
        }

      break;

    case mspHTTP:
    default:

      PacketSize = (long)strlen(SHeader);

      if (S->SBufSize <= MCONST_SMALLPACKETSIZE)
        {
        memcpy(
          SHeader + PacketSize,
          S->SBuffer,
          S->SBufSize);
  
        PacketSize += S->SBufSize; 

        MDB(6,fSOCK) MLog("INFO:     sending short packet '%.512s'\n",
          MUPrintBuffer(SHeader,PacketSize));
        }

      if (MSUSendPacket(
            S->sd,
            SHeader,
            PacketSize,
            TimeLimit,
            &SC) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot send packet header info, '%.128s'\n",
          SHeader);

        if (SC == mscNoEnt)
          S->StatusCode = msfConnRejected;
        else
          S->StatusCode = msfEGWireProtocol;

        return(FAILURE);
        }

      if (S->SBufSize > MCONST_SMALLPACKETSIZE)
        {
        if (MSUSendPacket(
              S->sd,
              S->SBuffer,
              S->SBufSize,
              TimeLimit,
              &SC) == FAILURE)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot send packet data\n");

          return(FAILURE);
          }
        }
      
      break;
    }  /* END switch (S->SocketProtocol) */

  /* clean up data */

  switch (S->WireProtocol)
    {
    case mwpS32:

      S->SBuffer = sptr;

      break;

    case mwpXML:

      if (S->SE != NULL)
        {
        MXMLDestroyE((mxml_t **)&S->SE);
        }

      S->SBuffer = sptr;
   
      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (S->WireProtocol) */

  return(SUCCESS);
  }  /* END MSUSendData() */





int MSUSendPacket(

  int                   sd,      /* I */
  char                 *Buf,     /* I */
  long                  BufSize, /* I */
  long                  TimeOut, /* I */
  enum MStatusCodeEnum *SC)      /* O (optional) */

  {
  int rc;
  int count;

/*  extern int errno; */

#ifndef __MPROD
  const char *FName = "MSUSendPacket";

  MDB(5,fSOCK) MLog("%s(%d,Buf,%ld,%ld,SC)\n",
    FName,
    sd,
    BufSize,
    TimeOut);
#endif /* !__MPROD */

  if (SC != NULL)
    *SC = mscNoError;

  MDB(9,fSOCK) MLog("INFO:     sending packet '%s'\n",
    MUPrintBuffer(Buf,BufSize));

  count = 0;

  TimeOut = MAX(50000,TimeOut);

  while (count < BufSize)
    {
    if (MSUSelectWrite(sd,TimeOut) == FAILURE)
      {
      MDB(2,fSOCK) MLog("WARNING:  cannot send message within %1.6f second timeout (aborting)\n",
        (double)TimeOut / 1000000);

      return(FAILURE);
      }

    if ((rc = send(sd,(Buf + count),(BufSize - count),SOCKETFLAGS)) < 0)
      {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
        /* should never occur with select */

        MDB(0,fSOCK) MLog("ERROR:    socket blocked (select() indicated socket was available)\n");

        continue;
        }

      MDB(1,fSOCK) MLog("WARNING:  cannot send packet, errno: %d (%s)\n",
        errno,
        strerror(errno));

      rc = errno;

      if (SC != NULL)
        {
        if (errno == ECONNREFUSED)
          {
          /* no service available at remote port */

          *SC = mscNoEnt;
          }
        else
          {
          /* default connection failure */

          *SC = mscNoEnt;
          }
        }

      return(FAILURE);
      }
    else if (rc == 0)
      {
      /* should never occur with select */

      MDB(1,fSOCK) MLog("ERROR:    no data sent (select() indicated socket was available)\n");

      continue;
      }
    else
      {
      MDB(2,fSOCK)
        {
        MDB(4,fSOCK)
          {
          MDB(4,fSOCK) MLog("INFO:     packet sent (%d bytes of %ld)\n",
            rc,
            BufSize);
          }
        else
          {
          if (rc != 0)
            {
            MDB(0,fSOCK) MLog("INFO:     packet sent (%d bytes of %ld)\n",
              rc,
              BufSize);
            }
          }
        }
      }    /* END else ... */

    if (rc > 0)
      count += rc;
    }    /* END while (count < BufSize) */

  return(SUCCESS);
  }  /* END MSUSendPacket() */





int MSURecvData(

  msocket_t *S,               /* I (modified) */
  long       TimeLimit,       /* I */
  mbool_t    DoAuthenticate,  /* I */
  enum MStatusCodeEnum *SC,   /* O (optional) */
  char      *EMsg)            /* O (optional,minsize=MMAX_LINE) */

  {
  char    tmpLine[MMAX_LINE];

  long    TSVal;

  char    CKLine[MMAX_LINE];
  char    CKSum[MMAX_LINE];

  char   *ptr;
  char   *ptr2;

  char   *dptr;

  time_t  Now;

  char    TMarker[MMAX_NAME];


  /* read data from socket.  socket is NOT closed */

#ifndef __MPROD
  const char *FName = "MSURecvData";

  MDB(2,fSOCK) MLog("%s(%s,%ld,%s,SC,EMsg)\n",
    FName,
    (S != NULL) ? "S" : "NULL",
    TimeLimit,
    MBool[DoAuthenticate]);
#endif /* !__MPROD */

  /* initialize */

  if (SC != NULL)
    *SC = mscNoError;

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (S == NULL)
    {
    MDB(1,fSOCK) MLog("ALERT:    invalid socket pointer received\n");

    if (EMsg != NULL)
      strcpy(EMsg,"invalid socket received");

    if (SC != NULL)
      *SC = mscBadParam;

    return(FAILURE);
    }

  if (S->sd <= 0)
    {
    MDB(1,fSOCK) MLog("ALERT:    socket is closed\n");

    if (EMsg != NULL)
      strcpy(EMsg,"socket is closed");

    if (SC != NULL)
      *SC = mscNoEnt;

    return(FAILURE);
    }

  S->RBuffer = NULL;

  TMarker[0] = '\0';

  switch (S->SocketProtocol)
    {
    case mspS3Challenge:

      {
      char  tmpLine[MMAX_LINE];

      char *ptr;

      /* read bytecount */

      ptr = tmpLine;

      memset(tmpLine,0,MMAX_LINE);

      if (MSURecvPacket(S->sd,&ptr,0," ",TimeLimit,SC) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot read bytecount\n");

        if (EMsg != NULL)
          strcpy(EMsg,"cannot read byte count");

        if (SC != NULL)
          *SC = mscNoEnt;

        return(FAILURE);
        }

      /* terminate buffer */

      /* NYI */

      S->RBufSize = strtol(ptr,NULL,10);

      if ((S->RBufSize > MMSG_BUFFER) || (S->RBufSize <= 0))
        {
        /* reject empty messages and potential denial of service attacks */
        /* allow packets between 1 and 2MB bytes */

        MDB(1,fSOCK) MLog("ALERT:    invalid packet size (%ld)\n",
          S->RBufSize);

        if (EMsg != NULL)
          {
          sprintf(EMsg,"invalid packet size requested - %ld bytes",
            S->RBufSize);
          }

        if (SC != NULL)
          *SC = mscNoMemory;

        return(FAILURE);
        }
      if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL)
        {
        MDB(1,fSOCK) MLog("ERROR:    cannot allocate buffer space (%ld bytes requested)  errno: %d (%s)\n",
          S->RBufSize,
          errno,
          strerror(errno));

        if (EMsg != NULL)
          {
          sprintf(EMsg,"cannot allocate %ld bytes for message",
            S->RBufSize);
          }

        if (SC != NULL)
          *SC = mscNoMemory;

        return(FAILURE);
        }

      if (MSURecvPacket(
            S->sd,
            &S->RBuffer,
            S->RBufSize,
            NULL,
            MAX(TimeLimit,1000000),
            NULL) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot receive packet (%ld bytes requested)\n",
          S->RBufSize);

        MUFree(&S->RBuffer);

        if (EMsg != NULL)
          {
          sprintf(EMsg,"cannot receive %ld bytes for message",
            S->RBufSize);
          }

        return(FAILURE);
        }
      }  /* END BLOCK */

      break;

    case mspHalfSocket:

      if (S->RBuffer == NULL)
        {
        /* allocate large receive space */

        S->RBufSize = MMAX_BUFFER << 4;
        if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL)
          {
          MDB(1,fSOCK) MLog("ERROR:    cannot allocate buffer space (%ld bytes requested)  errno: %d (%s)\n",
            S->RBufSize,
            errno,
            strerror(errno));

          if (EMsg != NULL)
            {
            sprintf(EMsg,"cannot allocate %ld bytes for message",
              S->RBufSize);
            }

          return(FAILURE);
          }

        }

      if (MSURecvPacket(S->sd,&S->RBuffer,0,NULL,TimeLimit,NULL) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot read half socket data\n");

        if (EMsg != NULL)
          {
          sprintf(EMsg,"cannot receive %ld bytes for message",
            S->RBufSize);
          }

        return(FAILURE);
        }

      break;

    case mspHTTP:
    case mspHTTPClient:

      /* FORMAT:  'Content-Length: ' or 'Transfer-Encoding' marker */

      {
      ptr = tmpLine;

      /* load HTTP header */

      if (MSURecvPacket(
           S->sd,
           &ptr,
           sizeof(tmpLine),
           "\r\n\r\n",
           MAX(TimeLimit,1000000),
           NULL) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot load HTTP header\n");

        if (EMsg != NULL)
          strcpy(EMsg,"cannot read message header");

        if (SC != NULL)
          *SC = mscNoEnt;

        return(FAILURE);
        }

      S->RBufSize = 0;

      if ((ptr2 = MUStrStr(ptr,"transfer-encoding:",0,TRUE,FALSE)) != NULL)
        {
        ptr2 += strlen("transfer-encoding:") + 1;

        /* check if chunked */

        if (MUStrStr(ptr,"chunked",0,TRUE,FALSE) != NULL)
          {
          /* read initial chunk size */

          if (MSURecvPacket(
             S->sd,
             &ptr2,
             sizeof(tmpLine),
             "\r\n",
             MAX(TimeLimit,1000000),
             NULL) == FAILURE)
            {
            MDB(1,fSOCK) MLog("ALERT:    cannot load HTTP chunk size\n");

            if (EMsg != NULL)
              strcpy(EMsg,"cannot load initial chunk size");

            if (SC != NULL)
              *SC = mscNoMemory;

            return(FAILURE);
            }

          /* NOTE:  chunks are in hex */

          if ((S->RBufSize = strtol(ptr2,NULL,16)) <= 0)
            {
            /* invalid packet length located */

            MDB(1,fSOCK) MLog("ALERT:    cannot determine packet size\n");

            if (EMsg != NULL)
              {
              sprintf(EMsg,"cannot parse initial chunk size - %.16s",
                ptr2);
              }

            return(FAILURE);
            }
          }
        }     /* if ((ptr2 = MUStrStr(ptr)) != NULL) */

      if (S->RBufSize != 0)
        {
        /* chunk length already located */

        /* NO-OP */
        }
      else if (((ptr2 = MUStrStr(ptr,"content-length:",0,TRUE,FALSE)) != NULL) ||
               ((ptr2 = MUStrStr(ptr,"content-length:",0,TRUE,FALSE)) != NULL)) 
        {
        /* extract packet length */

        ptr2 += strlen("content-length:") + 1;

        if ((S->RBufSize = strtol(ptr2,NULL,10)) <= 0)
          {
          /* invalid packet length located */
  
          MDB(1,fSOCK) MLog("ALERT:    cannot determine packet size\n");

          if (EMsg != NULL)
            {
            sprintf(EMsg,"invalid http packet size received - %.16s",
              ptr2);
            }

          return(FAILURE);
          }
        }
      else
        {
        MDB(6,fSOCK) MLog("NOTE:     packet length not specified (%.32s)\n",
          ptr);

        strcpy(TMarker,"</html>");

        /* create 'adequate' buffer */

        S->RBufSize = MMAX_BUFFER;
        }

      if (S->RBuffer == NULL) 
        {
        /* allocate receive space */
        if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL)
          {
          MDB(1,fSOCK) MLog("ERROR:    cannot allocate buffer space (%ld bytes requested)  errno: %d (%s)\n",
            S->RBufSize,
            errno,
            strerror(errno));

          if (EMsg != NULL)
            {
            sprintf(EMsg,"cannot allocate %d bytes for message",
              (int)S->RBufSize);
            }

          if (SC != NULL)
            *SC = mscNoMemory;

          return(FAILURE);
          }

        }

      /* read data */

      if (MSURecvPacket(
           S->sd,
           &S->RBuffer,
           S->RBufSize,
           (TMarker[0] != '\0') ? TMarker : NULL,
           MAX(TimeLimit,1000000),
           NULL) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot read HTTP data\n");

        if (EMsg != NULL)
          strcpy(EMsg,"cannot load HTTP data");

        if (SC != NULL)
          *SC = mscNoEnt;

        return(FAILURE);
        }

      /* terminate buffer */

      S->RBuffer[S->RBufSize] = '\0';

      /* NOTE:  no timestamp, version, or checksum */
      }  /* END BLOCK */

      break;

    default:

      ptr = tmpLine;

      ptr[0] = '\0';

      if (TimeLimit != 0)
        {
        /* allow polling */
        /* TODO: place this logic in other areas? */

        TimeLimit = MAX(TimeLimit,1000000);
        }

      if (MSURecvPacket(
           S->sd,
           &ptr,
           9 * sizeof(char),
           NULL,
           TimeLimit,
           SC) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot determine packet size\n");

        if (EMsg != NULL)
          strcpy(EMsg,"cannot load packet size");

        if ((SC != NULL) && (*SC != mscNoData))
          *SC = mscNoEnt;

        return(FAILURE);
        }

      if (!strncmp(tmpLine,"GET ",strlen("GET ")))
        {
        if (MSched.HTTPProcessF == NULL)
          {
          /* HTTP processing not supported */

          if (EMsg != NULL)
            strcpy(EMsg,"http processing not supported");

          return(FAILURE);
          }

        (*MSched.HTTPProcessF)(S,tmpLine);

        if (shutdown(S->sd,SHUT_WR) == -1)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot close send connections (%d : %s)\n",
            errno,
            strerror(errno));

          if (errno != ENOTCONN)
            {
            /* cannot properly close connection */
 
            /* NOTE:  cannot be certain data was successfully transferred */

            if (EMsg != NULL)
              strcpy(EMsg,"cannot close connection");
 
            return(FAILURE);
            }
          }

        MSUDisconnect(S);

        /* NOTE:  return failure to prevent additional processing (temp) */ 

        if (EMsg != NULL)
          strcpy(EMsg,"socket is closed");
 
        return(FAILURE);
        }  /* END if (!strncmp(tmpLine,"GET ",strlen("GET "))) */

      tmpLine[8] = '\0';

      /* NOTE:  some strtol() routines fail on zero pad */

      sscanf(tmpLine,"%ld",
        &S->RBufSize);

      if ((S->RBufSize > (MMAX_BUFFER << 5)) || (S->RBufSize <= 0))
        {
        /* reject empty messages and potential denial of service attacks */
        /* allow packets between 1 and 2MB bytes */

        MDB(1,fSOCK) MLog("ALERT:    invalid packet size (%ld)\n",
          S->RBufSize);

        if (EMsg != NULL)
          strcpy(EMsg,"packet size is invalid");

        if (SC != NULL)
          *SC = mscNoMemory;

        return(FAILURE);
        }
      if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL)
        {
        MDB(1,fSOCK) MLog("ERROR:    cannot allocate buffer space (%ld bytes requested)  errno: %d (%s)\n",
          S->RBufSize,
          errno,
          strerror(errno));

        if (EMsg != NULL)
          strcpy(EMsg,"cannot allocate memory for message");

        if (SC != NULL)
          *SC = mscNoMemory;

        return(FAILURE);
        }

      if (MSURecvPacket(
            S->sd,
            &S->RBuffer,
            S->RBufSize,
            NULL,
            MAX(TimeLimit,1000000),
            NULL) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    cannot receive packet (%ld bytes requested)\n",
          S->RBufSize);

        MUFree(&S->RBuffer);

        if (EMsg != NULL)
          strcpy(EMsg,"cannot read message");

        if (SC != NULL)
          *SC = mscNoEnt;

        return(FAILURE);
        }

      S->RBuffer[S->RBufSize] = '\0';

      break;
    }  /* switch (S->SocketProtocol) */

  if (S->WireProtocol == mwpNONE)
    {
    /* determine wire protocol */

    if (strstr(S->RBuffer,"<Envelope") != NULL)
      {
      S->WireProtocol = mwpS32;

      /* set default algorithm */

      S->CSAlgo = MSched.DefaultCSAlgo;
      }
    }    /* END if (S->WireProtocol == mwpNONE) */

  /* adjust state */

  switch (S->WireProtocol)
    {
    case mwpS32:

      /* no socket level authentication required */

      DoAuthenticate = FALSE;

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (S->WireProtocol) */

  /* authenticate message */

  if (DoAuthenticate == TRUE)
    {
    switch (S->SocketProtocol)
      {
      case mspS3Challenge:
      case mspHalfSocket:
      case mspHTTP:
      case mspHTTPClient:

        /* NO-OP */

        break;

      default:

        /* verify packet */

        /* NOTE:  client marker must not be in args */

        /* locate data marker */

        if ((dptr = strstr(S->RBuffer,MCKeyword[mckData])) == NULL)
          {
          MDB(3,fSOCK) MLog("ALERT:    cannot locate command data (%.60s)\n",
            S->RBuffer);

          MUFree(&S->RBuffer);

          if (EMsg != NULL)
            strcpy(EMsg,"cannot locate command data");

          return(FAILURE);
          }

        if ((ptr = strstr(S->RBuffer,MCKeyword[mckArgs])) != NULL)
          {
          /* arg marker located */

          dptr = ptr;
          }

        /* set defaults */

        S->Version = 0;

        strcpy(S->Name,NONE);

        if (S->CSKey[0] == '\0')
          strcpy(S->CSKey,MSched.DefaultCSKey);

        S->CSAlgo = MSched.DefaultCSAlgo;

        /* extract client name */

        if (((ptr = strstr(S->RBuffer,MCKeyword[mckClient])) != NULL) &&
             (ptr < dptr))
          {
          ptr += strlen(MCKeyword[mckClient]);

          if ((X.XGetClientInfo != (int (*)(void *,msocket_t *,char *))0) && 
             ((*X.XGetClientInfo)(X.xd,S,ptr) == SUCCESS))
            {
            /* NOTE:  enable logging only during unit testing */

            /*
            MDB(1,fSOCK) MLog("INFO:     using checksum seed '%s' for client '%s'\n",
              S->SKey,
              S->Name);
            */
            }
          else
            {
            /* use default client detection */

            for (ptr2 = ptr;(ptr2 - ptr) < MMAX_NAME;ptr2++)
              {
              if ((*ptr2 == '\0') || (*ptr2 == ':') || isspace(*ptr2))
                {
#ifdef __M32COMPAT
                extern mclient_t MClient[]; 

                mclient_t *C;
                  
                int        index;   

                MUStrCpy(S->Name,ptr,MIN((long)sizeof(S->Name),ptr2 - ptr + 1));
                                                                             
                for (index = 0;index < MMAX_CLIENT;index++)
                  {
                  C = &MClient[index];
                                                                            
                  if (C->Name[0] == '\0')
                    break;
                                                                           
                  if (C->Name[0] == '\1')
                    continue;
                                                                          
                  if (!strcmp(S->Name,C->Name))
                    {
                    strcpy(S->CSKey,C->CSKey);
                                                                         
                    break;
                    }
                  }    /* END for (index) */
#else /* __M32COMPAT */
                mpsi_t *P;

                MUStrCpy(S->Name,ptr,MIN((int)sizeof(S->Name),ptr2 - ptr + 1));

                if (MPeerFind(S->Name,&P,FALSE) == SUCCESS)
                  {
                  if (P->CSKey != NULL)
                    strcpy(S->CSKey,P->CSKey);
                  }
#endif /* __M32COMPAT */

                if (*ptr2 == ':')
                  {
                  ptr2++;

                  S->Version = strtol(ptr2,NULL,10);
                  }
     
                break;
                }
              }  /* END for (ptr2) */
            }    /* END else (X.XGetClientInfo != NULL) */
          }      /* END if (((ptr = strstr(S->RBuffer,MCKeyword[mckClient])) != NULL)... */
 
        /* get checksum */

        if ((ptr = strstr(S->RBuffer,MCKeyword[mckCheckSum])) == NULL)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot locate checksum '%s'\n",
            MUPrintBuffer(S->RBuffer,S->RBufSize));

          free(S->RBuffer);

          S->RBuffer = NULL;
     
          if (EMsg != NULL)
            strcpy(EMsg,"cannot locate checksum");

          if (SC != NULL)
            *SC = mscNoAuth;
 
          return(FAILURE);
          }

        ptr += strlen(MCKeyword[mckCheckSum]);

        MUStrCpy(CKLine,ptr,sizeof(CKLine));

        for (ptr2 = &CKLine[0];*ptr2 != '\0';ptr2++)
          {
          if (isspace(*ptr2))
            {
            *ptr2 = '\0';

            break;
            }
          }    /* END for (ptr2) */

        ptr += strlen(CKLine);

        if ((ptr = strstr(ptr,MCKeyword[mckTimeStamp])) == NULL)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot locate timestamp\n");

          MUFree(&S->RBuffer);

          if (EMsg != NULL)
            strcpy(EMsg,"cannot locate timestamp");

          if (SC != NULL)
            *SC = mscNoAuth;

          return(FAILURE);
          }

        /* verify checksum */

        if (S->CSAlgo != mcsaNONE)
          {
          MSecGetChecksum(
            ptr,
            strlen(ptr),
            CKSum,
            NULL,
            S->CSAlgo,
            S->CSKey);

          if (strcmp(CKSum,CKLine) != 0)
            {
            MDB(1,fSOCK) MLog("ALERT:    checksum does not match (%s:%s)  request '%.120s'\n",
              CKSum,
              CKLine,
	      ptr);

#ifdef __M32COMPAT
            if (strcmp(MSched.Admin4User[0],"ALL"))
#else /* __M32COMPAT */
            if (strcmp(MSched.Admin[4].UName[0],"ALL"))
#endif /* __M32COMPAT */
              {
              MUFree(&S->RBuffer);

              if (EMsg != NULL)
                strcpy(EMsg,"invalid message authentication");

              if (SC != NULL)
                *SC = mscNoAuth;

              return(FAILURE);
              }
            }    /* END if (strcmp(CKSum,CKLine) != 0) */
          }      /* END if (S->CSKey != NULL) */

        /* get timestamp */

        ptr += strlen(MCKeyword[mckTimeStamp]);

        TSVal = strtol(ptr,NULL,10);

        /* locate data */

        if ((ptr = strstr(ptr,MCKeyword[mckData])) == NULL)
          {
          MDB(1,fSOCK) MLog("ALERT:    cannot locate data\n");

          MUFree(&S->RBuffer);

          if (EMsg != NULL)
            strcpy(EMsg,"cannot locate message data");

          return(FAILURE);
          }

        /* verify timestamp */

        time(&Now);

        if ((((long)Now - TSVal) > 3600) || 
            (((long)Now - TSVal) < -3600))
          {
          MDB(1,fSOCK) MLog("ALERT:    timestamp does not match (%lu:%lu)\n",
            (unsigned long)Now,
            (unsigned long)TSVal);

          MUFree(&S->RBuffer);

          if (EMsg != NULL)
            strcpy(EMsg,"invalid timestamp detected");

          if (SC != NULL)
            *SC = mscNoAuth;

          return(FAILURE);
          }

        break;
      }  /* END switch(S->SocketProtocol) */
    }    /* END if (DoAuthenticate == TRUE) */

  /* validate message */

  switch (S->WireProtocol)
    {
    case mwpS32:

      {
      mxml_t *EE = NULL;
      mxml_t *SE;
      mxml_t *BE;
      mxml_t *RE;
      mxml_t *DE;

      char    AName[MMAX_LINE];   /* actor name */
      char    tmpLine[MMAX_LINE];
      char    tEMsg[MMAX_LINE];

      /* validate envelope */

      if (MXMLFromString(&EE,S->RBuffer,NULL,tEMsg) == FAILURE)
        {
        MDB(1,fSOCK) MLog("ALERT:    invalid socket request received (cannot process XML - %s)\n",
          tEMsg);

        MUFree(&S->RBuffer);
        
        if (EMsg != NULL)
          strcpy(EMsg,"cannot parse XML data");

        return(FAILURE);
        }

      S->RE = (void *)EE;

      if (MXMLGetChild(EE,"Body",NULL,&BE) == FAILURE)
        {
        MXMLDestroyE((mxml_t **)&S->RE);

        MDB(1,fSOCK) MLog("ALERT:    invalid socket request received (cannot locate body)\n");

        MUFree(&S->RBuffer);

        if (EMsg != NULL)
          strcpy(EMsg,"cannot locate message body");

        return(FAILURE);
        }

      /* NOTE: 'should' be deprecated - actor located in Request element (S3 3.0) */

      if (MXMLGetAttr(BE,"actor",NULL,AName,sizeof(AName)) == SUCCESS)
        {
        MUStrDup(&S->RID,AName);
        }

      if (MXMLGetAttr(EE,"type",NULL,tmpLine,sizeof(tmpLine)) == SUCCESS)
        {
        if (!strcasecmp(tmpLine,"nonblocking"))
          {
          S->IsNonBlocking = TRUE;
          }
        }

      if (S->CSAlgo != mcsaNONE)
        { 
        if (MXMLGetChild(EE,"Signature",NULL,&SE) == SUCCESS)
          {
          mxml_t *DVE;
          mxml_t *SVE;

          char *BString;
          char  TChar;
          char *tail;

          char tmpLine[MMAX_LINE];

          mpsi_t *P = NULL;

          /* process signature */
     
          MXMLGetChild(SE,"DigestValue",NULL,&DVE);
          MXMLGetChild(SE,"SignatureValue",NULL,&SVE);

          tmpLine[0] = '\0';

          /* extract body string */

          if ((BString = strstr(S->RBuffer,"<Body")) != NULL)
            {
            if ((tail = strstr(BString,"</Body>")) != NULL)
              {
              tail += strlen("</Body>");

              TChar = *tail;
              *tail = '\0';

              if (S->CSKey[0] == '\0')
                {
                /* determine key from actor */

#ifdef __M32COMPAT
                strcpy(S->CSKey,MSched.DefaultCSKey);
#else /* __M32COMPAT */
                if ((MPeerFind(AName,&P,FALSE) == SUCCESS) && (P->CSKey != NULL))
                  {
                  strcpy(S->CSKey,P->CSKey);
                  }
                else if ((MPeerFind(S->RemoteHost,&P,TRUE) == SUCCESS) && (P->CSKey != NULL))
                  {
                  /* try to look-up using hostname */

                  strcpy(S->CSKey,P->CSKey);

                  /* add peer to auth */

                  MSchedAddAdmin(AName,P->RIndex);
                  }
                else
                  {
                  /* use default */

                  strcpy(S->CSKey,MSched.DefaultCSKey);
                  }
#endif /* __M32COMPAT */
                }

              MSecGetChecksum(
                BString,
                strlen(BString),
                tmpLine,
                NULL,
                mcsaHMAC64,
                S->CSKey);
           
              *tail = TChar;
              }
            }
          else
            {
            /* FAILURE: message contains no body */

            MXMLDestroyE((mxml_t **)&S->RE);
 
            MDB(1,fSOCK) MLog("ALERT:    invalid socket request received (cannot locate body marker)\n");

            MUFree(&S->RBuffer);

            if (EMsg != NULL)
              strcpy(EMsg,"cannot locate message body");

            if (SC != NULL)
              *SC = mscBadRequest;

            return(FAILURE);
            } 
            
          if (strcmp(SVE->Val,tmpLine))
            { 
            /* signatures do not match */

            /* NOTE:  if checksum does not match, attempt to locate error message from server */

            if (MXMLGetChild(BE,MSON[msonResponse],NULL,&RE) == SUCCESS)
              {
              char tmpLine[MMAX_LINE];

              enum MSFC tSC;

              if (MS3CheckStatus(RE,&tSC,tmpLine) == FAILURE)
                {
                if (tmpLine[0] != '\0')
                  MUStrDup(&S->SMsg,tmpLine);

                S->StatusCode = (long)tSC;

                if (EMsg != NULL)
                  {
                  snprintf(EMsg,MMAX_LINE,"remote server rejected request, message '%s'",
                    tmpLine);
                  }
                }
              }
           
            MXMLDestroyE((mxml_t **)&S->RE);

            MDB(1,fSOCK) MLog("ALERT:    signatures do not match\n");

            MUFree(&S->RBuffer);
 
            if ((EMsg != NULL) && (EMsg[0] == '\0'))
              {
              /* attempt to locate client */

              if (P == NULL)
                {
                char *ptr;

                if (!strncasecmp(AName,"peer:",strlen("peer:")))
                  {
                  ptr = AName + strlen("peer:");

                  sprintf(EMsg,"unknown client '%.32s'",
                    ptr);
                  }
                else
                  {
                  sprintf(EMsg,"invalid key for client '%.32s'",
                    AName);
                  }
                }
              else
                {
                strcpy(EMsg,"invalid key for client");
                }
              }    /* END if ((EMsg != NULL) && ...) */

            if (SC != NULL)
              *SC = mscNoAuth;

            return(FAILURE);
            }  /* if (strcmp(SVE->Val,tmpLine)) */
          }    /* END if (MXMLGetChild(EE,"Signature",NULL,&SE) == SUCCESS) */
        else
          {
          /* cannot locate signature element */

          /* reject message? */

          /* NYI */
          }
        }
      else
        {
        /* no authentication algorithm specified */

        /* NYI */
        }

      /* NOTE:  get next object of either request or response type */

      if (MXMLGetChild(BE,MSON[msonRequest],NULL,&RE) == SUCCESS)
        {
        MXMLExtractE(
          (mxml_t *)BE,
          (mxml_t *)RE,
          (mxml_t **)&S->RDE);

        S->RPtr = NULL;

        if (S->RID == NULL)
          {
          /* NOTE:  S->RID should be populated before checksum call is made */

          if (MXMLGetAttr(RE,"actor",NULL,AName,sizeof(AName)) == SUCCESS)
            {
            MUStrDup(&S->RID,AName);
            }
          }
        }
      else if (MXMLGetChild(BE,MSON[msonResponse],NULL,&RE) == SUCCESS)
        {
        char tmpLine[MMAX_LINE];

        enum MSFC tSC;

        /* load status information if provided */

        if (MS3CheckStatus(RE,&tSC,tmpLine) == FAILURE)
          {
          if (tmpLine[0] != '\0')
            MUStrDup(&S->SMsg,tmpLine);
          }

        S->StatusCode = (int)tSC;

        if (MXMLGetChild(RE,MSON[msonData],NULL,&DE) == FAILURE)
          {
          /* response message received */
 
          if (S->StatusCode != 0)
            {
            MDB(1,fSOCK) MLog("ALERT:    request failed with status code %03ld (%s)\n",
              S->StatusCode,
              (S->SMsg != NULL) ? S->SMsg : "");

            if (SC != NULL)
              *SC = (enum MStatusCodeEnum)S->StatusCode;

            MDB(7,fSOCK) MLog("INFO:     failed request message '%.256s'\n",
              (S->RBuffer != NULL) ? S->RBuffer : "NULL");

            MXMLDestroyE((mxml_t **)&S->RE);

            MUFree(&S->RBuffer);

            if (EMsg != NULL)
              {
              if (S->SMsg == NULL)
                {
                sprintf(EMsg,"server rejected request with status code %ld",
                  S->StatusCode);
                }
              else
                {
                snprintf(EMsg,MMAX_LINE,"server rejected request with status code %ld - %s",
                  S->StatusCode,
                  S->SMsg);
                }
              }    /* END if (EMsg != NULL) */

            return(FAILURE);
            }

          S->RDE  = NULL;
          S->RPtr = NULL;
 
          MDB(1,fSOCK) MLog("INFO:     successfully received socket response\n");
          }
        else
          {
          MXMLExtractE(
            (mxml_t *)RE,
            (mxml_t *)DE,
            (mxml_t **)&S->RDE);

          S->RPtr = DE->Val;
          }
        }    /* END else if (MXMLGetChild(BE) == SUCCESS) */
      else
        {
        MXMLDestroyE((mxml_t **)&S->RE);

        MDB(1,fSOCK) MLog("ALERT:    invalid socket message received\n");

        MUFree(&S->RBuffer);

        if (EMsg != NULL)
          strcpy(EMsg,"invalid message type received");

        return(FAILURE);
        }    /* END else */
      }      /* END BLOCK */

      break;

    default:

      /* NYI */

      break;
    }  /* END switch (S->WireProtocol) */

  S->IsLoaded = TRUE;

  return(SUCCESS);
  }  /* END MSURecvData() */





int MSURecvPacket(

  int    sd,      /* I */
  char **BufP,    /* O (alloc if NULL ptr passed) */
  long   BufSize, /* I (optional) */
  char  *Pattern, /* I (optional) */
  long   TimeOut, /* I (in us) */
  enum MStatusCodeEnum *SC)  /* O (optional) */

  {
  long count;

  int  rc;
  int  len;

  int  MaxBuf;

  char *ptr;

#ifndef __MPROD
  const char *FName = "MSURecvPacket";

  MDB(3,fSOCK) MLog("%s(%d,BufP,%ld,%s,%ld,SC)\n",
    FName,
    sd,
    BufSize,
    (Pattern != NULL) ? Pattern : "NULL",
    TimeOut);

#endif /* !__MPROD */

  if (BufP == NULL)
    {
    return(FAILURE);
    }

  count = 0;

  MaxBuf = (BufSize > 0) ? BufSize : (MMAX_BUFFER << 4);

  if ((BufSize == 0) || (*BufP == NULL) || (Pattern != NULL))
    {
    char *ptr;
    int   ReadSize;

    if (*BufP == NULL)
      {
      if ((*(char **)BufP = (char *)calloc(
          1,
          MaxBuf)) == NULL)
        {
        /* cannot allocate memory */

        return(FAILURE);
        }
      }

    if (Pattern != NULL)
      {
      ReadSize = 1;

      len = strlen(Pattern);
      }
    else
      {
      ReadSize = MaxBuf;

      len = 0;
      }

    ptr = *(char **)BufP;

    while (TRUE)
      {
      if (MSUSelectRead(sd,TimeOut) == FAILURE)
        {
        MDB(2,fSOCK) MLog("WARNING:  cannot receive message within %1.6lf second timeout (aborting)\n",
          (double)TimeOut / 1000000);

        if (SC != NULL)
          {
          *SC = mscNoData; 
          }

        return(FAILURE);
        }

      rc = recv(sd,&ptr[count],ReadSize,SOCKETFLAGS);

      if (rc == 0)
        {
        /* select indicated data was available, but recv() returned nothing */

        /* sleep and try again */

#ifdef __M32COMPAT
        MUSleep(TimeOut);
#else /* __M32COMPAT */
        MUSleep(TimeOut,FALSE);
#endif /* __M32COMPAT */

        rc = recv(sd,&ptr[count],ReadSize,SOCKETFLAGS);

        if (rc == 0)
          {
          MDB(2,fSOCK) MLog("WARNING:  cannot receive message within %1.6lf second timeout (no data/aborting)\n",
            (double)TimeOut / 1000000);

          return(FAILURE);
          }
        }

      if (rc == -1)
        {
        if (errno == EAGAIN)
          continue;

        /* socket recv call failed */

        break;
        }

      if (count >= MaxBuf)
        {
        /* buffer size reached */

        break;
        }

      /* NOTE:  precludes binary data */

      if ((count > 0) && (ptr[count - 1] == '\0'))
        {
        /* end of string located */

        break;
        }

      count += rc;

      if ((Pattern != NULL) &&
          (count >= len) &&
          (MUStrNCmpCI(Pattern,ptr + count - len,len) == SUCCESS))
        {
        /* termination pattern located */

        break;
        }
      }  /* END while (TRUE) */

    ptr[count] = '\0';
    }  /* END if ((BufSize == 0) || ... ) */
  else
    {
    time_t Start;
    time_t Now;

    ptr = *BufP;

    time(&Start);
    Now = Start;

    while (count < BufSize)
      {
      if (MSUSelectRead(sd,TimeOut) == FAILURE)
        {
        MDB(2,fSOCK) MLog("WARNING:  cannot receive message within %1.6lf second timeout (aborting)\n",
          (double)TimeOut / 1000000);

        if (SC != NULL)
          {
          *SC = mscNoData; 
          }

        return(FAILURE);
        }

      rc = recv(sd,(ptr + count),(BufSize - count),SOCKETFLAGS);

      if (rc > 0)
        {
        count += rc;
        continue;
        }

      time(&Now);

      if (((long)Now - (long)Start) >= (TimeOut / 1000000))
        {
        MDB(2,fSOCK) MLog("WARNING:  cannot receive message within %1.6lf second timeout (aborting)\n",
          (double)TimeOut / 1000000);

        return(FAILURE);
        }

      if (rc < 0)
        {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
          {
          /* continue if packet partially read */

          MDB(0,fSOCK) MLog("ERROR:    socket blocked (select() indicated socket was available)\n");

          continue;
          }

        if ((errno == ECONNRESET) || (errno == EINTR))
          {
          MDB(0,fSOCK) MLog("INFO:     client has disconnected, errno: %d (%s)\n",
            errno,
            strerror(errno));

          return(FAILURE);
          }

        MDB(6,fSOCK) MLog("WARNING:  cannot read client socket, errno: %d (%s)\n",
          errno,
          strerror(errno));

        return(FAILURE);
        }

      if ((rc == 0) && (count == 0))
        {
        /* fail if no packet detected */

        return(FAILURE);
        }

      }     /* END while (count < BufSize) */

    MDB(6,fSOCK) MLog("INFO:     %ld of %ld bytes read from sd %d\n",
      count,
      BufSize,
      sd);
    }  /* END else (BufSize == 0) */

  MDB(8,fSOCK) MLog("INFO:     message '%s' read\n",
    MUPrintBuffer(*BufP,count));

  /* NOTE:  return SUCCESS if buffer full even if entire message not loaded */

  return(SUCCESS);
  }  /* END MSURecvPacket() */





int MSUSelectWrite(

  int           sd,        /* I */
  unsigned long TimeLimit) /* I */

  {
  struct timeval TimeOut;
  int            numfds;

#if defined(__AIX41)

  struct sellist
    {
    int fdsmask[64];
    } wset;

#else /* __AIX41 */

  fd_set wset;

#endif /* __AIX41 */

#ifndef __MPROD
  const char *FName = "MSUSelectWrite";

  MDB(7,fSOCK) MLog("%s(%d,%lu)\n",
    FName,
    sd,
    TimeLimit);
#endif /* !__MPROD */

#if defined(__AIX41)

  memset(&wset,0,sizeof(wset));

  AIX_SET(sd,wset.fdsmask);

#else

  FD_ZERO(&wset);

  FD_SET(sd,&wset);

#endif

  TimeOut.tv_sec  = TimeLimit / 1000000;
  TimeOut.tv_usec = TimeLimit % 1000000;

  numfds = sd;

  if (select(numfds + 1,NULL,&wset,NULL,&TimeOut) > 0)
    {
#if defined(__AIX41)

    if (AIX_ISSET(sd,wset.fdsmask))
      {
      return(SUCCESS);
      }

#else

    if (FD_ISSET(sd,&wset))
      {
      return(SUCCESS);
      }

#endif
    }  /* END if (select() > 0) */

  return(FAILURE);
  }  /* END MSUSelectWrite() */





int MSUSelectRead(

  int           sd,        /* I */
  unsigned long TimeLimit) /* I (in us) */

  {
  struct timeval TimeOut;
  int            numfds;

#if defined(__AIX41)

  struct sellist
    {
    int fdsmask[64];
    } rset;

#else

  fd_set rset;

#endif

#ifndef __MPROD
  const char *FName = "MSUSelectRead";

  MDB(7,fSOCK) MLog("%s(%d,%lu)\n",
    FName,
    sd,
    TimeLimit);
#endif /* !__MPROD */

#if defined(__AIX41)

  memset(&rset,0,sizeof(rset));

  MAIX_SET(sd,rset.fdsmask);

#else

  FD_ZERO(&rset);

  FD_SET(sd,&rset);

#endif

  TimeOut.tv_sec  = TimeLimit / 1000000;
  TimeOut.tv_usec = TimeLimit % 1000000;

  numfds = sd;

  if (select(numfds + 1,&rset,NULL,NULL,&TimeOut) > 0)
    {
#if defined(__AIX41)

    if (MAIX_ISSET(sd,rset.fdsmask))
      {
      return(SUCCESS);
      }

#else

    if (FD_ISSET(sd,&rset))
      {
      return(SUCCESS);
      }

    MDB(2,fSOCK) MLog("MSUSelectRead-FD is not set\n");
#endif
    }  /* END if (select() > 0) */

  MDB(2,fSOCK) MLog("MSUSelectRead-select failed\n");

  return(FAILURE);
  }  /* END MSUSelectRead() */





int MAIX_ISSET(

  int  FD,
  int *List)

  {
  return(MISSET(List[FD / 32],(FD % 32)));
  }  /* MAIX_ISSET() */






int MAIX_SET(

  int  FD,
  int *List)

  {
  MSET(List[FD / 32],(FD % 32));

  return(SUCCESS); 
  }  /* END MAIX_SET() */





int MAIX_CLR(

  int  FD,
  int *List)

  {
  MUNSET(List[FD / 32],(FD % 32));

  return(SUCCESS);
  }  /* END MAIX_CLR() */




int MUSystemF(

  char *Command,   /* I */
  int   TimeLimit, /* I (in usec) */
  int  *PID)       /* O */

  {
  int StatLoc;
  int Flags;
  int pid;

  char Line[MMAX_LINE];
  char *Arg[MMAX_ARG];
  char *Cmd;

  int  aindex;
  int  TimeStep;
  int  step;

  char *TokPtr;

#ifndef __MPROD
  const char *FName = "MUSystemF";

  MDB(3,fSOCK) MLog("%s(%s,%d,PID)\n",
    FName,
    Command,
    TimeLimit);
#endif /* !__MPROD */

  strcpy(Line,Command);

  Cmd = MUStrTok(Line," \t\n",&TokPtr);

  Arg[0] = Cmd;

  aindex = 1;

  while ((Arg[aindex++] = MUStrTok(NULL," \t\n",&TokPtr)) != NULL);

  Arg[aindex] = NULL;

  if ((pid = fork()) == -1)
    {
    MDB(0,fSOCK) MLog("ERROR:    cannot fork, errno: %d (%s)\n",
      errno,
      strerror(errno));

    return(FAILURE);
    }

  if (pid == 0)
    {
    /* child process */

    if (execv(Cmd,Arg) == -1)
      {
      /* child has failed */

      exit(0);
      }

    exit(0);
    }

  MDB(5,fSOCK) MLog("INFO:     child process %d forked\n",
    pid);

  if (PID != NULL)
    *PID = pid;

  if (TimeLimit == -1)
    {
    return(SUCCESS);
    }

  /* wait for child to complete */
 
  Flags = WNOHANG;

  step = 0;

  TimeStep = TimeLimit / 10000 + 1;

  /* wait for child to complete */

  while (step++ < TimeStep)
    {
    if (waitpid(pid,&StatLoc,Flags) == pid)
      {
      MDB(3,fSOCK) MLog("INFO:     command '%s' spawned\n",
        Command);

      return(SUCCESS);
      }

#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(_AIX51)

    usleep(10000);

#else

    sleep(1);

#endif /* __AIX** */
    }

  MDB(3,fSOCK) MLog("ALERT:    spawn of command '%s' did not return within %d us\n",
    Command,
    TimeLimit);

  if (kill(pid,9) == -1)
    {
    MDB(0,fSOCK) MLog("ERROR:    cannot kill process %d\n",
      pid);
    }
  else
    {
    /* clear defunct child processes */

    MUClearChild(NULL);
    }

  return(FAILURE);
  }  /* END MUSystemF() */






int MUClearChild(

  int *PID)  /* O (optional) */

  {
  int pid;
  int StatLoc;
  int Flags;

#ifndef __MPROD
  const char *FName = "MUClearChild";

  MDB(3,fSOCK) MLog("%s(PID)\n",
    FName);
#endif /* !__MPROD */
  
  Flags = WNOHANG;

  while ((pid = waitpid(-1,&StatLoc,Flags)) != -1)
    {  
    /* if no waiting processes */

    if (PID != NULL)
      *PID = pid;
 
    if (pid == 0)
      {
      MDB(4,fSOCK) MLog("INFO:     no child processes found\n");

      return(SUCCESS);
      }
    else
      {
      MDB(3,fSOCK) MLog("INFO:     child PID %d cleared\n",
        pid);
 
      if (PID != NULL)
        {
        return(SUCCESS);
        }

#ifndef __M32COMPAT
      /* must keep track of harvested processes for triggers */

      index = 0;

      while (index < MMAX_PID)
        {
        if (MPID[index].PID == 0)
          break;

        index++;
        }

      if (index == MMAX_PID)
        index = 0;

      MPID[index].PID = pid;
      MPID[index].StatLoc = StatLoc;
#endif /* __M32COMPAT */
      }
    }    /* END while ((pid = waitpid(-1,&StatLoc,Flags)) != -1) */

  return(FAILURE);
  }  /* END MUClearChild() */




#ifndef __M32COMPAT

mbool_t MOSHostIsLocal(

  char *HostName)  /* I */

  {
  int aindex;

  mulong tmpAddr;

  if ((HostName == NULL) || (HostName[0] == '\0'))
    {
    return(FAILURE);
    }

  if (MSched.LocalHost[0] == '\0')
    {
    if (MOSGetHostName(NULL,MSched.LocalHost,&MSched.LocalAddr) == FAILURE)
      {
      MDB(0,fCONFIG) MLog("ERROR:    cannot determine local hostname\n");

      fprintf(stderr,"ERROR:    cannot determine local hostname\n");

      exit(1);
      }
    }

  if (!strcasecmp(HostName,MSched.LocalHost))
    {
    /* hostname match found */

    return(TRUE);
    }

  /* get address of specified host */

  if (MOSGetHostName(HostName,NULL,&tmpAddr) == SUCCESS)
    {
    if (tmpAddr == MSched.LocalAddr)
      {
      /* host address match found */

      return(TRUE);
      }
    }

  /* check local host and aliases */

  for (aindex = 0;aindex < MMAX_HOSTALIAS;aindex++)
    {
    if (MSched.ServerAlias[aindex][0] == '\0')
      break;

    if (!strcasecmp(HostName,MSched.ServerAlias[aindex]))
      {
      /* alias hostname match found */

      return(TRUE);
      }

    if (MSched.ServerAliasAddr[aindex] == 0)
      {
      if (MOSGetHostName(
           NULL,
           MSched.ServerAlias[aindex],
           &MSched.ServerAliasAddr[aindex]) == -1)
        {
        MSched.ServerAliasAddr[aindex] = MBNOTSET;
        }
      }

    if (tmpAddr == MSched.ServerAliasAddr[aindex])
      {
      /* alias host address match found */

      return(TRUE);
      }
    }    /* END for (aindex) */

  /* specified host name is not local */

  return(FALSE);
  }  /* END MOSHostIsLocal() */

#endif /* !__M32COMPAT */




int MOSGetHostName(

  char          *HostName,      /* I */
  char          *FullHostName,  /* O (optional) */
  unsigned long *Address)       /* O (optional) */
 
  {
  char            tmpHostName[MMAX_NAME];
  struct hostent *hoststruct;

  if ((HostName == NULL) || (HostName[0] == '\0'))
    {
    if (gethostname(tmpHostName,sizeof(tmpHostName)) == -1)
      {
      MDB(0,fCONFIG) MLog("ERROR:    cannot get hostname, errno: %d (%s)\n",
        errno,
        strerror(errno));

      return(FAILURE);
      }
    }
  else
    {
    strcpy(tmpHostName,HostName);
    }

  if ((hoststruct = gethostbyname(tmpHostName)) == NULL)
    {
    MDB(0,fCONFIG) MLog("ERROR:    cannot get full hostname for host '%s', errno: %d (%s)\n",
      tmpHostName,
      errno,
      strerror(errno));

    return(FAILURE);
    }

  if (Address != NULL)
    memmove(Address,hoststruct->h_addr,sizeof(mulong));

  if (FullHostName != NULL)
    strcpy(FullHostName,hoststruct->h_name);

  return(SUCCESS);
  }  /* END MOSGetHostName() */




int MSUCallBack(

  char *Host,        /* I */
  int   Port,        /* I */
  char *ClientName,  /* I */
  char *Message)     /* I */

  {
  msocket_t tmpS;
  msocket_t *S;

  char     SBuffer[MMAX_BUFFER];

#ifndef __MPROD
  const char *FName = "MSUCallBack";

  MDB(4,fSOCK) MLog("%s(%s,%d,%s,%s)\n",
    FName,
    Host,
    Port,
    (ClientName != NULL) ? ClientName : "NULL",
    Message);
#endif /* !__MPROD */

  S = &tmpS;

  MSUInitialize(S,Host,Port,2000000,(1 << msftTCP));

  if (MSUConnect(S,FALSE,NULL) == FAILURE)
    {
    MDB(3,fSOCK) MLog("ALERT:    cannot send callback message '%s' to %s:%d (no connect)\n",
      Message,
      Host,
      Port);

    return(FAILURE);
    }

  if ((ClientName == NULL) || (ClientName[0] == '\0'))
    {
    if (MSUSendPacket(S->sd,Message,strlen(Message),2000000,NULL) == FAILURE)
      {
      MDB(3,fSOCK) MLog("ALERT:    cannot send callback message '%s' to %s:%d\n",
        Message,
        Host,
        Port);

      MSUDisconnect(S);

      return(FAILURE);
      }

    MSUDisconnect(S);
    }
  else
    {
    /* must be secured */

    if (S->CSKey[0] == '\0')
      strcpy(S->CSKey,MSched.DefaultCSKey);

    strcpy(S->Name,MSched.Name);

#ifdef __M32COMPAT
    {
    extern mclient_t MClient[];

    int index;

    for (index = 0;index < MMAX_CLIENT;index++)
      {
      if (!strcmp(MClient[index].Name,ClientName))
        {
        strcpy(S->CSKey,MClient[index].CSKey);

        break;
        }
      }    /* END for (index) */
    }  /* END BLOCK */
#else /* __M32COMPAT */
    {
    mpsi_t *P;

    if (MPeerFind(S->Name,&P,FALSE) == SUCCESS)
      {
      if (P->CSKey != NULL)
        strcpy(S->CSKey,P->CSKey);
      }
    }  /* END BLOCK */
#endif /* __M32COMPAT */

    sprintf(SBuffer,"%s%s %s%s %s%s %s%s",
      MCKeyword[mckCommand],
      "scallback",
      MCKeyword[mckAuth],
      MUUIDToName(MOSGetEUID()),
      MCKeyword[mckClient],
      ClientName,
      MCKeyword[mckArgs],
      (Message != NULL) ? Message : "");

    S->SBuffer  = SBuffer;
    S->SBufSize = strlen(S->SBuffer);

    if (MSUSendData(S,MMAX_SOCKETWAIT,FALSE,FALSE) == FAILURE)
      {
      MDB(0,fSOCK) MLog("ERROR:    cannot send callback to %s grid server '%s':%d\n",
        ClientName,
        Host,
        Port);

      MSUFree(S);

      return(FAILURE);
      }

    MSUFree(S);
    }  /* END ((ClientName == NULL) || (ClientName[0] == '\0')) */

  return(SUCCESS);
  }  /* END MSUCallBack() */




int MSUCreate(

  msocket_t **SP)  /* O (alloc) */

  {
  if (SP == NULL)
    {
    return(FAILURE);
    }

  *SP = (msocket_t *)calloc(1,sizeof(msocket_t));

  if (*SP == NULL)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MSUCreate() */




int MSUDisconnect(

  msocket_t *S)  /* I */

  {
#ifndef __MPROD
  const char *FName = "MSUDisconnect";

  MDB(2,fSOCK) MLog("%s(%s)\n",
    FName,
    (S != NULL) ? "S" : "NULL");
#endif /* !__MPROD */

  if (S == NULL)
    {
    return(SUCCESS);
    }

  if (S->sd <= 0)
    {
    return(SUCCESS);
    }

  if ((S->SocketProtocol == mspHalfSocket) || 
      (S->SocketProtocol == mspS3Challenge))
    {
    /* delay required for half socket connections to allow data to be transmitted */

    /* NOTE:  temporary hack approach */

    /* sleep(1); */
    }

  close(S->sd);

  S->sd = -1;

  MSUClientCount--;

  return(SUCCESS);
  }  /* END MSUDisconnect() */





int MSUClose(

  msocket_t *S)  /* I */

  {
  MSUDisconnect(S);
  
  S->State = sussClosed;
 
  return(SUCCESS);
  }  /* END MSUClose() */





int MSUFree(

  msocket_t *S)  /* I (modified) */

  {
  if (S == NULL)
    {
    return(SUCCESS);
    }

  MUFree(&S->RBuffer);
  MUFree(&S->SMsg);
  MUFree(&S->RID);
  MUFree(&S->ClientName);

  if (S->SBIsDynamic == TRUE)
    {
    /* free allocated memory */

    MUFree(&S->SBuffer);

    S->SBIsDynamic = FALSE;
    }
  else
    {
    /* clear pointer to stack space */

    S->SBuffer = NULL;
    }

  if (S->sd > 0)
    {
    MSUDisconnect(S);
    }

  S->WireProtocol   = mwpNONE;
  S->SocketProtocol = mspNONE;

  if (S->SE != NULL)
    {
    MXMLDestroyE((mxml_t **)&S->SE);
    }

  if (S->RE != NULL)
    {
    MXMLDestroyE((mxml_t **)&S->RE);
    }

  if (S->RDE != NULL)
    {
    /* RDE is 'extracted' from RE (must be independently free'd) */

    MXMLDestroyE((mxml_t **)&S->RDE);
    }

  return(SUCCESS);
  }  /* END MSUFree() */




int MSUAdjustSBuffer(

  msocket_t *S,         /* I (modified) */
  int        BufSize,   /* I */
  mbool_t    IncrSize)  /* I */

  {
  int NewSize;

  char *ptr;

  if (S == NULL)
    {
    return(FAILURE);
    }

  if (IncrSize == TRUE)
    {
    NewSize = S->SBufSize + BufSize;
    }
  else
    {
    /* NOTE:  do not allow buffer reduction */

    NewSize = MAX(S->SBufSize,BufSize);
    }

  if (S->SBIsDynamic == FALSE)
    {
    ptr = (char *)calloc(NewSize, 1);

    memcpy(ptr,S->SBuffer,S->SBufSize);
    }
  else
    {
    ptr = (char *)realloc(S->SBuffer,NewSize);
    } 

  if (ptr == NULL)
    {
    /* cannot allocate memory */

    /* original buffer is maintained */

    return(FAILURE);
    }

  S->SBuffer = ptr;
  S->SBufSize = NewSize;

  S->SBIsDynamic = TRUE;

  return(SUCCESS); 
  }  /* END MSUAdjustSBuffer() */




int MSUSetAttr(

  msocket_t            *S,      /* I (modified) */
  enum MSocketAttrEnum  AIndex, /* I */
  void                 *Value)  /* I */

  {
#ifndef __MPROD
  const char *FName = "MSUSetAttr";

  MDB(3,fSOCK) MLog("%s(%s,%s,%s)\n",
    FName,
    (S != NULL) ? "S" : "NULL",
    MSockAttr[AIndex],
    (Value != NULL) ? "Value" : "NULL");
#endif /* !__MPROD */

  switch (AIndex)
    {
    case msockaLocalHost:

      /* FORMAT:  <STRING> */

      /* NYI */

      break;

    case msockaLocalPort:

      /* FORMAT:  <INT> */

      if ((S->State == sussOpen) || (S->State == sussBusy))
        {
        return(FAILURE);
        }

      /* use RemotePort variable - MSU treats this as LocalPort when needed */

      if (Value != NULL)
        S->RemotePort = *(int *)Value;
      else
        S->RemotePort = -1;

      break;

    case msockaRemoteHost:

      /* FORMAT:  <STRING> */

      if ((S->State == sussOpen) || (S->State == sussBusy))
        {
        return(FAILURE);
        }

      if (Value != NULL)
        MUStrCpy(S->RemoteHost,(char *)Value,sizeof(S->RemoteHost));
      else
        S->RemoteHost[0] = '\0';

      break;

    case msockaRemotePort:

      /* FORMAT:  <INT> */

      if ((S->State == sussOpen) || (S->State == sussBusy))
        {
        return(FAILURE);
        }

      if (Value != NULL)
        S->RemotePort = *(int *)Value;
      else
        S->RemotePort = -1;

      break;

    default:

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MSUSetAttr() */



/* perform deep copy of msocket */
int MSUDup(
    
  msocket_t **Dst,  /* O */
  msocket_t  *Src)  /* I */

  {
  msocket_t *S;

  if ((Dst == NULL) ||
      (Src == NULL))
    {
    return(FAILURE);
    }

  /* do shallow memcpy first */
  S = (msocket_t *)calloc(1,sizeof(msocket_t));

  memcpy(S,Src,sizeof(msocket_t));

  /* strings */
  
  S->URI = NULL;  
  MUStrDup(&S->URI,Src->URI);
  
  S->RID = NULL;
  MUStrDup(&S->RID,Src->RID);
  
  S->SMsg = NULL;
  MUStrDup(&S->SMsg,Src->SMsg);
  
  S->ClientName = NULL;
  MUStrDup(&S->ClientName,Src->ClientName);
  
  S->RBuffer = NULL;
  S->RPtr = NULL;
  MUStrDup(&S->RBuffer,Src->RBuffer);
  
  S->SData = NULL;
  MUStrDup(&S->SData,Src->SData);

  if (Src->SBIsDynamic == TRUE)
    {
    MUStrDup(&S->SBuffer,Src->SBuffer);
    S->SPtr = NULL;
    }
  else
    {
    S->SBuffer = NULL;
    S->SBIsDynamic = FALSE;
    S->SBufSize = 0;
    S->SPtr = NULL;
    }

  /* XML */
  MXMLDupE((mxml_t *)Src->RE,(mxml_t **)&S->RE);
  MXMLDupE((mxml_t *)Src->RDE,(mxml_t **)&S->RDE);
  MXMLDupE((mxml_t *)Src->SE,(mxml_t **)&S->SE);
  MXMLDupE((mxml_t *)Src->SDE,(mxml_t **)&S->SDE);

  /* copy Cred? */
  /* NYI */

  *Dst = S;

  return(SUCCESS);  
  }  /* END MSUDup() */


/* END MSU.c */

