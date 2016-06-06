/* HEADER */

#include "moab.h"
#include "msched-proto.h"

extern const char *MClientAttr[];
extern const char *MService[];




int MClientLoadConfig()

  {
  char *ptr;

  char  tmpLine[MAX_MLINE];

  while (MCfgGetSVal(
          MSched.PvtConfigBuffer,
          &ptr,
          MParam[pClientCfg],
          NULL,
          NULL,
          tmpLine,
          sizeof(tmpLine),
          0,
          NULL) == SUCCESS)
    {
    MClientProcessConfig(tmpLine);
    }  /* END while MCfgGetSVal() == SUCCESS) */

  return(SUCCESS);
  }  /* END MClientLoadConfig() */




int MClientProcessConfig(

  char *Value)  /* I */

  {
  char *ptr;
  char *TokPtr;

  int   aindex;

  char  ValLine[MAX_MLINE];

  char  AttrArray[MAX_MNAME];

  if (Value == NULL)
    {
    return(FAILURE);
    }

  ptr = MUStrTokE(Value," \t\n",&TokPtr);

  while(ptr != NULL)
    {
    /* parse name-value pairs */

    /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */

    if (MUGetPair(
          ptr,
          (const char **)MClientAttr,
          &aindex,
          AttrArray,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */

      ptr = MUStrTokE(NULL," \t\n",&TokPtr);

      continue;
      }

    switch (aindex)
      {
      case mcltaTimeout:

        /* NYI */

        break;

      case mcltaCSAlgo:

        /* NYI */

        break;

      default:

        /* attribute not supported */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch (aindex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }    /* END while (ptr != NULL) */

  return(FAILURE);
  }  /* END MClientProcessConfig() */





int MCSendRequest(

  msocket_t *S)  /* I */

  {
  const char *FName = "MCSendRequest";

  DBG(2,fUI) DPrint("%s(%s)\n",
    FName,
    (S != NULL) ? "S" : "NULL");
  if (S == NULL)
    {
    return(FAILURE);
    }
  if (S->SBufSize == 0)
    S->SBufSize = (long)strlen(S->SBuffer);
  if (MSUSendData(S,S->Timeout,TRUE,FALSE) == FAILURE)
    {
    DBG(0,fSOCK) DPrint("ERROR:    cannot send request to server %s:%d (server may not be running)\n",
      S->RemoteHost,
      S->RemotePort);

    MSUDisconnect(S);

    return(FAILURE);
    }
  else
    {
    DBG(1,fUI) DPrint("INFO:     message sent to server\n");

    DBG(3,fUI) DPrint("INFO:     message sent: '%s'\n",
      (S->SBuffer != NULL) ? S->SBuffer : "NULL");
    }
  if (MSURecvData(S,S->Timeout,TRUE,NULL,NULL) == FAILURE)
    {
    fprintf(stderr,"ERROR:    lost connection to server\n");

    return(FAILURE);
    }
  DBG(3,fUI) DPrint("INFO:     message received\n");

  DBG(4,fUI) DPrint("INFO:     received message '%s' from server\n",
    S->RBuffer);

  return(SUCCESS);
  }  /* END MCSendRequest() */




int MCDoCommand(

  char *HostName,   /* I */
  int   Port,       /* I */
  int   CIndex,     /* I */
  char *CmdString,  /* I */
  char *Response)   /* O */

  {
  static msocket_t S;

  char   tmpLine[MAX_MLINE];

  /* initialize socket */

  memset(&S,0,sizeof(S));

  MUStrCpy(S.RemoteHost,HostName,sizeof(S.RemoteHost));
  S.RemotePort = Port;

  strcpy(S.CSKey,MSched.DefaultCSKey);

  S.CSAlgo         = MSched.DefaultCSAlgo;

  S.SocketProtocol = 0;
  S.SBuffer        = tmpLine;

  S.Timeout        = 5;

  if (MSUConnect(&S,FALSE,NULL) == FAILURE)
    {
    DBG(0,fSOCK) DPrint("ERROR:    cannot connect to '%s' port %d\n",
      S.RemoteHost,
      S.RemotePort);

    return(FAILURE);
    }

  sprintf(S.SBuffer,"%s%s %s%s %s%s\n",
    MCKeyword[mckCommand],
    MService[CIndex],
    MCKeyword[mckAuth],
    MUUIDToName(MOSGetEUID()),
    MCKeyword[mckArgs],
    CmdString);

  S.SBufSize = (long)strlen(S.SBuffer);

  if (MCSendRequest(&S) == FAILURE)
    {
    S.RBuffer[0] = '\0';

    return(FAILURE);
    }

  MUStrCpy(Response,S.RBuffer,MAX_MLINE);

  MSUDisconnect(&S);

  return(SUCCESS);
  }  /* END MCDoCommand() */


/* END MClient.c */
