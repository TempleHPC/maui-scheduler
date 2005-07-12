/* HEADER */

#include "moab.h"

extern mlog_t mlog;
extern msched_t MSched;

extern const char *MDSProtocol[];

#ifdef __MGLOBUS

#define DEFAULT_MCREDVAR        "X509_USER_PROXY"

#include <gssapi.h>
#include <globus_common.h>
#include <globus_gss_assist.h>
#include <globus_gsi_credential.h>
#include <globus_gram_client.h>
#include <globus_gram_protocol.h>
#include <globus_gass_transfer.h>
#include <globus_gass_copy.h>

/* Globus 2.2.x hack. The only way to use an arbitrary credential with gram
 * routines is to use globus private variables. */
                                                                                                                                                             
extern globus_io_attr_t globus_i_gram_protocol_default_attr;
int globus_gram_protocol_setup_attr(globus_io_attr_t *attr);

/* Won't compile with Globus 3.2.x if the next line is uncommented: */
/* extern void *globus_i_gram_protocol_credential; */

/* a credential that is unique per UNIX process */
gss_cred_id_t MLocalCredential = GSS_C_NO_CREDENTIAL;
void *CredentialBackup;

/* local globus prototypes */
int __MGlobusConnect(void *,char *);
int __MGlobusGRAMError(int,int,char *);
static void __MGlobusCallBackHandler(void *,char *,int,int);
int MGlobusStageData(char **,char **,char *,char *,char *,int *);
int MGlobusCheckStageStatus(int,mbool_t *,char *);


/* Globus data structures */

/* GRAM synchronization structure */
typedef struct
  {
  globus_mutex_t   mutex;
  globus_cond_t    cond;
  int              status;
  int              done;
  } mgss_gram_monitor_t;
                                                                                                                                                             
/* GASS transfer synchronization structure */
typedef struct
  {
  globus_mutex_t mutex;
  globus_cond_t cond;
  char *output;
  int length;
  int status;
  int done;
  } mgss_transfer_monitor_t;

#endif  /* __MGLOBUS */

/* local prototypes */
int __MGSSGetError(long,long,int);

int MGSSPostCred(
                                                                                
  msocket_t *S)  /* I (modified) */
                                                                                
  {
#ifndef __MPROD
  const char *FName = "MGSSPostCred";

  MDB(7,fGRID) MLog("%s(S)\n",
    FName);
#endif /* !__MPROD */

#ifdef __MGSS
  OM_uint32    MajorSC;
  OM_uint32    MinorSC;
  gss_ctx_id_t context = GSS_C_NO_CONTEXT;
  OM_uint32    flags = 0;
  int          ts;

  char *RemoteSubject = NULL;

  /* TODO: don't use ENV var, use moab.cfg parameter for
     destination subject name */
  if (getenv("DESTSUBJECTNAME") != NULL)
    {
    RemoteSubject = s->GlobusRemoteSubjectName;
    }

  /* push credential to host */
                                                                                
  MajorSC = globus_gss_assist_init_sec_context(
    &MinorSC,
    MLocalCredential,
    &context,
    RemoteSubject,
    GSS_C_DELEG_FLAG,
    &flags,
    &ts,
    __MGSSGetToken,
    &S->sd,
    __MGSSSendToken,
    &S->sd);
                                                                                
  if (MajorSC == GSS_S_COMPLETE)
    MajorSC = gss_delete_sec_context(&MinorSC,&context,GSS_C_NO_BUFFER);
                                                                                
  return(__MGSSGetError(MajorSC,MinorSC,0));
#else
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGSS */
  }  /* END MGSSPostCred() */




/* accept a client delegated credential on socket S.
 * save the delegated credential in C. */
                                                                                
int MGSSAcceptCred(
                                                                                
  void      **C,   /* O (GSS:  credential handler) */
  msocket_t  *S)   /* I (utilized) */
                                                                                
  {
#ifdef __MGSS
  OM_uint32      MajorSC;
  OM_uint32      MinorSC = 0;
  OM_uint32      flags = 0;
  int            ts;
                                                                                
  gss_ctx_id_t   Context = GSS_C_NO_CONTEXT;
#endif /* __MGSS */

#ifndef __MPROD
  const char *FName = "MGSSAcceptCred";
                                                                                                                                                             
  MDB(7,fGRID) MLog("%s(C,S)\n",
    FName);
#endif /* !__MPROD */

#ifdef __MGSS
  if ((C == NULL) || (S == NULL))
    {
    return(FAILURE);
    }
                                                                                
  *C = GSS_C_NO_CREDENTIAL;
                                                                                
  MajorSC = globus_gss_assist_accept_sec_context(
    &MinorSC,
    &Context,
    MLocalCredential,
    NULL,
    &flags,
    NULL,
    &ts,
    (gss_cred_id_t *) C,
    __MGSSGetToken,
   (void *) &S->sd,
    __MGSSSendToken,
    (void *) &S->sd);
                                                                                
  if (MajorSC != GSS_S_COMPLETE)
    {
    return(__MGSSGetError(MajorSC,MinorSC,0));
    }
                                                                                
  MajorSC = gss_delete_sec_context(&MinorSC,&Context,GSS_C_NO_BUFFER);
                                                                                
  return(__MGSSGetError(MajorSC,MinorSC,3));
#else
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGSS */
  }  /* END MGSSAcceptCred() */




/* free GSS credential */
                                                                                
int MGSSFreeCred(
                                                                                
  void *C)  /* I (modified) */
                                                                                
  {
#ifdef __MGSS
  OM_uint32      MajorSC;
  OM_uint32      MinorSC = 0;
                                                                                
  MajorSC = gss_release_cred(&MinorSC,(gss_cred_id_t *)C);
                                                                                
  return(__MGSSGetError(MajorSC,MinorSC,3));
#else /* __MGSS */
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGSS */
  } /* END MGSSFreeCred() */



#define MCONST_TOKENTIMEOUT 100000000
int __MGSSSendToken(
                                                                                
  void   *pfd,  /* I (utilized) */
  void   *Tok,  /* I */
  size_t  Len)  /* I */
                                                                                
  {
#ifdef __MGSS
  int    fd = *(int*)pfd;
  unsigned char buffer[4];
                                                                                
  /*printf("Sending %d bytes\n", Len);*/
                                                                                
  buffer[0] = (Len >> 24) & 0xff;
  buffer[1] = (Len >> 16) & 0xff;
  buffer[2] = (Len >>  8) & 0xff;
  buffer[3] = (Len) & 0xff;
  
  /* FORMAT:  <S1><S2><S3><S4><TOKEN> */                                                                              
  if (MSUSendPacket(fd,(char *)buffer,4,MCONST_TOKENTIMEOUT,NULL) == FAILURE ||
      MSUSendPacket(fd,(char *)Tok,Len,MCONST_TOKENTIMEOUT,NULL) == FAILURE)
    {
    MDB(0,fSOCK) MLog("ALERT:    cannot send gss packet\n");
                                                                                
    return(-1);
    }
                                                                                
  /* SUCCESS */                                                                                
  return(0);
#else /* __MGSS */
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGSS */
  }  /* END __MGSSSendToken() */




int __MGSSGetToken(

  void    *pfd,   /* I */
  void   **ptok,  /* O (alloc) */
  size_t  *plen)  /* O */
                                                                                
  {
#ifdef __MGSS
  int     fd = *((int*)pfd);
  unsigned char buffer[4];
  void   *tok;
  size_t  len;
  char *ptr;
#endif /* __MGSS */

#ifndef __MPROD
  const char *FName = "__MGSSGetToken";
                                                                                                                                                             
  MDB(7,fGRID) MLog("%s()\n",
    FName);
#endif /* !__MPROD */

#ifdef __MGSS                                                                                
  ptr = buffer;

  if (MSURecvPacket(fd,&ptr,4,NULL,MCONST_TOKENTIMEOUT,NULL) == FAILURE)
    {
    MDB(0,fSOCK) MLog("ALERT:    cannot get gss packet\n");

    return(-1);
    }

  len  = ((size_t) buffer[0]) << 24;
  len |= ((size_t) buffer[1]) << 16;
  len |= ((size_t) buffer[2]) <<  8;
  len |= ((size_t) buffer[3]);
                                                                                
  tok = malloc(len);
                                                                                
  if (!tok)
    {
    return(-1);
    }
                                                                                
  if (MSURecvPacket(fd,(char **)&tok,len,NULL,MCONST_TOKENTIMEOUT,NULL) == FAILURE)
    {
    MDB(0,fSOCK) MLog("ALERT:    cannot get gss packet\n");
                                                                                
    return(-1);
    }
                                                                                
  *ptok = tok;
  *plen = len;

  /* SUCCESS */
  return(0);
#else /* __MGSS */
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE); 
#endif /* __MGSS */
  }  /* END __MGSSGetToken() */




int MGlobusExec(

  void  *C,         /* I */
  char  *CmdString, /* I */
  long   Timeout,   /* I */
  char  *JobString, /* I */
  char  *EMsg)      /* O (optional) */

  {
#ifdef __MGLOBUS
  int mask = GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE |
             GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING |
             GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED;

  mgss_gram_monitor_t monitor = {0, 0, 0, 0};

  char *contact;
  char *job = NULL;

  int   rc;
#endif /* __MGLOBUS */

#ifndef __MPROD
  const char *FName = "MGlobusExec";

  MDB(3,fGRID) MLog("%s(C,%s,%ld,%.32s,%s)\n",
    FName,
    (CmdString != NULL) ? CmdString : "NULL",
    Timeout,
    (JobString != NULL) ? JobString : "",
    (EMsg != NULL) ? "EMsg" : "NULL");
#endif /* !__MPROD */

#ifdef __MGLOBUS
  if (EMsg != NULL)
    EMsg[0] = '\0';

  if ((CmdString == NULL) || (JobString == NULL))
    {
    if (EMsg != NULL)
      strcpy(EMsg,"internal error - uninitialized variables");

    return(FAILURE);
    }

  if (__MGlobusConnect(C,EMsg) == FAILURE)
    {
    return(FAILURE);
    }

  globus_mutex_init(&monitor.mutex,NULL);

  rc = globus_cond_init(&monitor.cond,NULL);

  rc = globus_gram_client_callback_allow(__MGlobusCallBackHandler,&monitor,&contact);

  if (rc != GLOBUS_SUCCESS)
    {
    return(__MGlobusGRAMError(rc,0,EMsg));
    }

  rc = globus_gram_client_job_request(CmdString,JobString,mask,contact,&job);

  if (rc != GLOBUS_SUCCESS)
    {
    free(contact);
    return(FAILURE);
    }

  return(SUCCESS);
#else /* __MGLOBUS */
  MDB(3,fGRID) MLog("WARNING:    Globus services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGLOBUS */
  }  /* END MGlobusExec() */




int MGlobusJobToRSL(

  mjob_t *J,       /* I */
  mrm_t  *R,       /* I */
  char   *Buf,     /* O */
  int     BufSize) /* I */

  {
  char tmpLine[MMAX_LINE];
  char PreString[MMAX_LINE];

  mbool_t  StageFile;

  enum MDSProtoEnum Protocol;

  int  index;
  int  rc;

  mreq_t *RQ;

  mds_t *DS;

  char  *BPtr;
  int    BSpace;

  msdata_t *IData;

  /* NOTE:  stdin,executable handled in post processing */

  mbool_t ExecSpecified = FALSE;

  struct {
    enum MXMLOTypeEnum OType;
    int      AIndex;
    char    *RSLToken;
    char     Format;
    mbool_t  IsRequired;
    } RSLAttr[] = {
  { mxoJob,  mjaReqProcs,      "count",        mdfString, TRUE  },
  { mxoJob,  mjaReqNodes,      "hostCount",    mdfString, FALSE },
  { mxoJob,  mjaArgs,          "arguments",    mdfString, FALSE },
  { mxoJob,  mjaIWD,           "directory",    mdfString, FALSE },
  { mxoJob,  mjaStdOut,        "stdout",       mdfString, FALSE },
  { mxoJob,  mjaStdErr,        "stderr",       mdfString, FALSE },
  { mxoJob,  mjaReqAWDuration, "MaxWallTime",  mdfString, TRUE  },
  { mxoJob,  mjaClass,         "queue",        mdfString, FALSE },
  { mxoJob,  mjaAccount,       "project",      mdfString, FALSE },
  { mxoJob,  mjaEnv,           "environment",  mdfString, FALSE },
/*  { mxoJob,  mjaTaskReq,       "requirements", mdfString, FALSE }, */ /* NOTE:  need usage info */
  { mxoNONE, mrqaNONE,         NULL,           mdfString, FALSE }
  };

  const char *FName = "MGlobusJobToRSL";

  if (Buf != NULL)
    Buf[0] = '\0';
                                                                                
  MDB(3,fGRID) MLog("%s(%s,%s,%s,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (Buf != NULL) ? "Buf" : "NULL",
    BufSize);

  if ((J == NULL) || (R == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  /* initialize RSL string */

  MUSNInit(&BPtr,&BSpace,Buf,BufSize);

  MUSNPrintF(&BPtr,&BSpace,"&");

  /* NOTE:  re-enable per rm attribute conversion */

  /* SResGetCon(R,RQ->ConList); */

  for (index = 0;RSLAttr[index].OType != mxoNONE;index++)
    {
    switch (RSLAttr[index].OType)
      {
      case mxoJob:

        rc = MJobAToString(
               J,
               (enum MJobAttrEnum)RSLAttr[index].AIndex,
               tmpLine,
               (enum MFormatModeEnum)RSLAttr[index].Format);

        break;

      case mxoReq:

        rc = MReqAToString(
               J,
               RQ,
               (enum MReqAttrEnum)RSLAttr[index].AIndex,
               tmpLine,
               (enum MFormatModeEnum)RSLAttr[index].Format);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (RSLAttr[index].OType) */

    if ((rc == FAILURE) || (tmpLine[0] == '\0'))
      {
      if (RSLAttr[index].IsRequired == TRUE)
        {
        MDB(3,fGRID) MLog("ALERT:    job '%s' is missing attribute '%s' for conversion to RSL\n",
          J->Name,
          RSLAttr[index].RSLToken);

        return(FAILURE);
        }

      continue;
      }

    MUSNPrintF(&BPtr,&BSpace,"(%s=\"%s%s\")",
      (IData->IsExec == TRUE) ? "executable" : "stdin",
      PreString,
      tmpLine);
    }  /* END for (index) */
 
  IData = J->SIData;

  while (IData != NULL)
    {
    StageFile = FALSE;
    PreString[0] = '\0';

    if ((J->SRM != NULL) && (J->SRM->DS != NULL))
      {
      if ((IData->Scope == mdlHost) ||
         ((IData->Scope == mdlResource) && (J->SRM != R)))
        {
        /* input data is remote */
                                                                                
        StageFile = TRUE;
        }
      }

    if (StageFile == TRUE)
      {
      /* verify DS is active */
                                                                                
      /* NOTE:  currently implemented in client until credential delegation is enabled */
                                                                                
      /* build stage string */
                                                                                
      /* SDataGetDefProtocol(RQ->IData,SJ->SubmitR,NULL,&Protocol); (re-enable NYI) */
                                                                                
      DS = (J->SRM->DS != NULL) ?
        J->SRM->DS[Protocol] : NULL;
                                                                                
      if (DS != NULL)
        {
        sprintf(PreString,"%s://%s",
          MDSProtocol[DS->Protocol],
          (DS->HostName != NULL) ? DS->HostName : "localhost");
                                                                                
        if (DS->Port > 0)
          {
          sprintf(PreString,"%s:%d",
            PreString,
            DS->Port);
          }
        }
     
      if (IData->IsExec == TRUE)
        ExecSpecified = TRUE;
 
      MUSNPrintF(&BPtr,&BSpace,"(%s=\"%s%s\")",
        RSLAttr[index].RSLToken,
        PreString,
        tmpLine);
      }  /* END if (StageFile == TRUE) */

    IData = IData->Next;
    }  /* END while (IData != NULL) */

  if (ExecSpecified == FALSE)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* MGlobusJobToRSL() */




int MGlobusJobStage(
  
  mjob_t *J,            /* I job */
  mrm_t  *R,            /* I destination resource */
  char   *JobString,    /* I job string */
  enum MRMTypeEnum Format, /* I */
  char   *Destination,  /* I destination system  */
  char   *JobManager,   /* I job manager         */
  char   *JobName,      /* O (optional,minsize=MMAX_NAME) job handle */
  char   *EMsg)         /* O (optional,minsize=MMAX_LINE) submission response */
          
  {
  char  CmdString[MMAX_BUFFER];
  char  tmpBuffer[MMAX_LINE];
         
  char *jptr;
        
  void *C;
       
  int rc;
      
#ifndef __MPROD
  const char *FName = "MGlobusJobStage";
                                     
  MDB(3,fGRID) MLog("%s(J,%s,%s,%ld,%s,%s,JobName,EMsg)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (JobString != NULL) ? JobString : "NULL",
    Format,
    Destination,
    (JobManager != NULL) ? JobManager : "NULL");
#endif /* !__MPROD */
                                            
  if (EMsg != NULL)
    {
    EMsg[0] = '\0';
    }
                                           
  if ((JobString == NULL) ||
      (JobString[0] == '\0') ||
      (Destination == NULL))
    {
    if (EMsg != NULL)
      strcpy(EMsg,"internal error");
                                                       
    return(FAILURE);
    }
 
  if (J == NULL)
    {
    C = NULL;
    }
  else
    {
    if ((J->Cred.U == NULL) ||
        (J->Cred.U->GCred == NULL) ||
        (J->Cred.U->GCred->C == NULL))
       {
      MDB(1,fGRID) MLog("ALERT:    credential not initialized in %s\n",
        FName);
                        
      if (EMsg != NULL)
        strcpy(EMsg,"no credential");

      return(FAILURE);
      }

    if ((J->Cred.U->GCred->ExpireTime > 0) &&
        (J->Cred.U->GCred->ExpireTime < MSched.Time))
      {
      /* cred has expired */

      if (EMsg != NULL)
        strcpy(EMsg,"expired credential");

      return(FAILURE);
      }

    C = J->Cred.U->GCred->C;
    }  /* END else (SJ == NULL) */

  MDB(3,fGRID) MLog("INFO:     launching job on resource '%s' with command '%s'\n",
    Destination,
    JobString);

  if ((JobString == NULL) || (JobString[0] == '\0'))
    {
    tmpBuffer[0] = '\0';

    if ((J->RMSubmitString != NULL) && (J->RMSubmitType != mrmtNONE))
      {
      jptr = J->RMSubmitString;

      Format = (enum MRMTypeEnum)J->RMSubmitType;
      }
    else if (MGlobusJobToRSL(J,R,tmpBuffer,sizeof(tmpBuffer)) == SUCCESS)
      {
      jptr = tmpBuffer;

      Format = mrmtRSL;
      }
    else
      {
      MDB(2,fGRID) MLog("ALERT:    cannot translate job '%s' for staging\n",
        J->Name);

      if (EMsg != NULL)
        strcpy(EMsg,"cannot translate job to RSL");

      return(FAILURE);
      }
    }
  else
    {
    jptr = JobString;
    }

  switch(Format)
    {
    case mrmtPBS:

      sprintf(CmdString,"%s/jobmanager-pbsnative",
        Destination);

      break;

    case mrmtLL:

      sprintf(CmdString,"%s/jobmanager-llnative",
        Destination);

      break;

    case mrmtSGE:

      sprintf(CmdString,"%s/jobmanager-sgenative",
        Destination);

      break;

    case mrmtLSF:

      sprintf(CmdString,"%s/jobmanager-lsfnative",
        Destination);

      break;

    case mrmtRSL:
    case mrmtNONE:
    default:

      sprintf(CmdString,"%s",
        Destination);
 
      break;
    }  /* END switch (Format) */
  
  rc = MGlobusExec(
    C,
    CmdString,
    R->P[0].Timeout,
    JobString,
    EMsg);

  MDB(3,fGRID) MLog("INFO:     MGlobusExec %s\n",
    (rc == SUCCESS) ? "succeeded" : "failed");

  if (JobName != NULL)
    {
    JobName[0] = '\0';

    /* NYI */
    }

  return(rc);
  }  /* END MGlobusJobStage() */




/* Handle a Globus GSS error */

int __MGSSGetError(
  
  long MajorSC,
  long MinorSC,
  int  Level)
 
  {
#ifdef __MGSS
  char *msg = NULL;

  if (MajorSC == GSS_S_COMPLETE)
    {
    return(SUCCESS);
    }

  /* cast because of Globus specific data types */
  globus_gss_assist_display_status_str(&msg,NULL,(OM_uint32)MajorSC,(OM_uint32)MinorSC,0);

  MDB(3,fGRID)
    {
    switch (Level)
      {
      case 0:

        MLog("ALERT:    %s\n", 
          msg);

        break;

      case 1:

        MLog("ERROR:    %s\n", 
          msg);

        break;

      default:

        MLog("WARNING:  %s\n", 
          msg);

        break;
      }
    }

  free(msg);

  return(FAILURE);
#else /* __MGSS */
  MDB(3,fGRID) MLog("WARNING:    grid credential services were invoked, but are not currently enabled");
  return(FAILURE);
#endif /* __MGSS */

  }  /* END __MGSSGetError() */

#ifdef __MGLOBUS

/* Globus helper function */
int __MGlobusConnect(

  void *C,
  char *EMsg)

  {
  int rc;
 
  globus_i_gram_protocol_credential = ((C != NULL) ? C : MLocalCredential);
  
  rc = globus_gram_protocol_setup_attr(&globus_i_gram_protocol_default_attr);                                                                                                                                                           
  if (rc != GLOBUS_SUCCESS)
    {
    MDB(0,sfMeta) MLog("ALERT:    Globus GRAM connection setup failed\n");
    __MGlobusGRAMError(rc,0,EMsg);
    return(FAILURE);
    }

  return(SUCCESS);
  } /* END __MGlobusConnect() */

/* Handle a Globus GRAM error */

int __MGlobusGRAMError(

  int   rc,     /* I */
  int   Level,  /* I */
  char *EMsg)   /* O (optional) */

  {
  const char *s;

  if (EMsg != NULL)
    EMsg[0] = '\0';

  /* ask Globus to give a more verbose error */
  s = globus_gram_protocol_error_string(rc);

  if (rc == GLOBUS_SUCCESS)
    {
    return(SUCCESS);
    }

  if (EMsg != NULL)
    {
    strncpy(EMsg,s,MMAX_LINE);
    }

  MDB(Level,fGRID)
    {
    switch (Level)
      {
      case 0:

        MLog("ALERT:    Globus GRAM error: %s\n",
          s);

        break;

      case 1:

        MLog("ERROR:    Globus GRAM error: %s\n",
          s);

        break;

      default:

        MLog("WARNING:  Globus GRAM error: %s\n",
          s);

        break;
      }  /* END switch(Level) */
    }

  return(FAILURE);
  }  /* END __MGlobusGRAMError() */


/* This function is called by Globus code as a handler.
 * It unlocks a Globus mutex if job is done or failed. */

static void __MGlobusCallBackHandler(

  void *Data,
  char *Job,
  int  Status,
  int  Error)

  {
  mgss_gram_monitor_t *monitor = (mgss_gram_monitor_t *)Data;

  switch(Status) {

      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
          monitor->status = GLOBUS_SUCCESS;
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
          monitor->status = Error;
          break;
      default:
          monitor->status = Error;
          break;
  } /* END switch(Status) */

  globus_mutex_lock(&monitor->mutex);
  monitor->done = 1;
  globus_cond_signal(&monitor->cond);
  globus_mutex_unlock(&monitor->mutex);

  }  /* END __MGlobusCallBackHandler() */

#endif /* __MGLOBUS */

/* initialize Globus data structures and aquire host credential */

#ifdef MNOT

int MGlobusInitialize(

  char *CertFile,  /* I (optional) */
  char *EMsg)      /* O (optional) */

  {
#ifdef __MGLOBUS
  OM_uint32 MajorSC = 0;
  OM_uint32 MinorSC = 0;
#endif /* __MGLOBUS */

#ifndef __MPROD
  const char *FName = "MGlobusInitialize";

  MDB(1,fGRID) MLog("%s(%s)\n",
    FName,
    (CertFile != NULL) ? CertFile : "NULL");
#endif /* __MPROD */

#ifdef __MGLOBUS
  
  if (EMsg != NULL)
    EMsg[0] = '\0';

  /* set system defaults */

  /* NYI */

  /*
  if (s->DefDS != NULL)
    {
    long tmpL = sdspGridFTP;

    SDSSetAttr(
      s->DefDS,
      sdsaProtocol,
      NULL,
      (void *)&tmpL,
      sdmLong,
      mSet);
    } */  /* END if (s->DefDS != NULL) */

  if (CertFile != NULL)
    {
    if (setenv(DEFAULT_MCREDVAR,CertFile,1) == -1)
      {
      MDB(0,fGRID) MLog("ALERT:    setenv(%s,%s) failed, errno: %d (%s)\n",
        DEFAULT_MCREDVAR,
        CertFile,
        errno,
        strerror(errno));

      return(FAILURE);
      }
    }    /* END if (CertFile != NULL) */
 
  if (globus_module_activate(GLOBUS_COMMON_MODULE) != GLOBUS_SUCCESS ||
      globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE) != GLOBUS_SUCCESS ||
      globus_module_activate(GLOBUS_GASS_TRANSFER_MODULE) != GLOBUS_SUCCESS ||
      globus_module_activate(GLOBUS_GASS_COPY_MODULE) != GLOBUS_SUCCESS)
    {
    MDB(1,fGRID) MLog("ALERT:    globus module activation failed\n");

    return(FAILURE);
    }

  CredentialBackup = globus_i_gram_protocol_credential;

  MajorSC = globus_gss_assist_acquire_cred(
    &MinorSC,
    GSS_C_BOTH,
    &MLocalCredential);

  if (MajorSC != GSS_S_COMPLETE)
    {
    MDB(1,fGRID) MLog("ALERT:    cannot acquire globus credential\n");

    return(__MGSSGetError(MajorSC,MinorSC,0));
    }

  /* TODO: set Moab's host credential here */
  /* s->C = (void *)MLocalCredential; */

  if ((SGICredExpirationTime(s->C,&s->CETime) == FAILURE) ||
      (s->CETime < su->Env->ETime))
    {
    /* credential is expired */

    MDB(1,fGRID) MLog(su,"ALERT:    globus credential is expired\n");

    if (EMsg != NULL)
      strcpy(EMsg,"globus credential is expired");

    return(FAILURE);
    }

  if (SGICredIdentity(MLocalCredential,s->CName) == FAILURE)
    {
    return(FAILURE);
    }

  MDB(3,fGRID) MLog(su,"INFO:     session credential: %s\n",
    s->CName);

  return(__MGSSGetError(MajorSC,MinorSC,0));

#else /* __MGLOBUS */
  
  MDB(3,fGRID) MLog("WARNING:    Globus credential services were invoked, but are not currently enabled");
  return(FAILURE);

#endif /* __MGLOBUS */

  }  /* END MGlobusInitialize() */

#endif /* MNOT */



#ifdef __MGLOBUS

/* return the credential subject name */ 
/* (subject name) */

int MGlobusGetCredName(

  void *C,          /* I (Globus credential handle) */
  char *CredName)   /* O (subject name,minsize=MMAX_LINE) */

  {
  OM_uint32       MajorSC;
  OM_uint32       MinorSC = 0;
  gss_name_t      name = NULL;
  gss_buffer_desc buffer;

  if ((C == NULL) || (CredName == NULL))
    {
    return(FAILURE);
    }

  CredName[0] = '\0';

  MajorSC = gss_inquire_cred(&MinorSC,C,&name,NULL,NULL,NULL);

  if (MajorSC != GSS_S_COMPLETE)
    {
    return(__MGSSGetError(MajorSC,MinorSC,0));
    }

  MajorSC = gss_display_name(&MinorSC,name,&buffer,NULL);

  if (MajorSC != GSS_S_COMPLETE)
    {
    free(name);

    return(__MGSSGetError(MajorSC,MinorSC,0));
    }

  MUStrCpy(CredName,buffer.value,MMAX_LINE);

  free(name);

  gss_release_buffer(&MinorSC,&buffer);

  return(SUCCESS);
  } /* END MGlobusGetCredName() */

#endif /* __MGLOBUS */


#ifdef MNOT

#ifdef __MGLOBUS

/* Stages data via the globus-url-copy command. */
int MGlobusStageData(

  char **SrcURLs,     /* I */
  char **DstURLs,     /* I */
  char *Username,     /* I */
  char *StatusFile,   /* I */
  char *URLFile,      /* I */
  int  *copyPID)      /* O (negative if error) */

  {
  
  /* TODO: don't use hard-coded value for globus-url-copy,
   * but instead support dynamic changing of this location
   * via a config file. */  
  char *globus_url_copy = "/usr/local/bin/globus-url-copy";
  char *copyArgs[5];

  int argIndex;
  int copyUID;
  
  FILE *urlFileDesc;

  int urlIndex;
  
  struct passwd * copyUserInfo;
  
#ifndef __MPROD

  const char *FName = "MGlobusStageData";

  MDB(1,fGRID) MLog("%s(SrcURLs,DstURLs,%s,%s,%s)\n",
    FName,
    (Username != NULL) ? Username : "NULL",
    (StatusFile != NULL) ? StatusFile : "NULL",
    (URLFile != NULL) ? URLFile : "NULL");    
#endif /* !__MPROD */

  if ((SrcURLs == NULL) ||
      (DstURLs == NULL) ||
      (Username == NULL) ||
      (StatusFile == NULL) ||
      (URLFile == NULL) ||
      (copyPID == NULL))
    {
    return(FAILURE);
    }

  if ((SrcURLs[0] == NULL) ||
      (DstURLs[0] == NULL))
    {
    return(FAILURE);
    }

  *copyPID = -1;

  argIndex = 0;

  copyArgs[argIndex++] = globus_url_copy;

  if ((urlFileDesc = fopen(URLFile,"w")) == NULL)
    {
    MDB(1,fGRID) MLog("ERROR:    cannot open globus-url file '%s' (errno %d: '%')\n",
      URLFile,
      errno,
      strerror(errno));

    return(FAILURE);
    }

  fprintf(urlFileDesc,"# Moab globus-url-copy multi-file data staging list\n# Auto generated.\n\n");
      
  /* TODO: Change MMAX_NAME to a const more meaningful to data staging */
  for (urlIndex = 0; urlIndex < MMAX_NAME; urlIndex++)
    {
    if (SrcURLs[urlIndex] == NULL)
      break;

    if (DstURLs[urlIndex] == NULL)
      {
      MDB(1,fGRID) MLog("ERROR:    src globus-url '%s' without corresponding dst url\n",
        SrcURLs[urlIndex]);
         
      return(FAILURE);
      }

    fprintf(urlFileDesc,"\"%s\" \"%s\"\n",
      SrcURLs[urlIndex],
      DstURLs[urlIndex]);

    }

  fclose(urlFileDesc);

  /* pass proper arguments to globus-url-copy */
  /* add parameters to control parallelism and TCP buffer size
   * to increase performance - NYI */
  copyArgs[argindex++] = "-f";
  copyArgs[argindex++] = URLFile;
  copyArgs[argindex]   = NULL;

  /* prepare to launch globus-url-copy! */
  
  *copyPID = fork();

  if (*copyPID > 0)
    {
    /* parent success */

    return(SUCCESS);    
    }

  if (*copyPID < 0)
    {
    /* parent error */

    return(FAILURE);
    }

  /* child code */

  copyUserInfo = getpwnam(Username);

  if (copyUserInfo == NULL)
    {
    MDB(1,fGRID) MLog("ERROR:    globus-url-copy failed - could not locate UID for user '%s'\n",
      Username);

    exit(1);
    }

  copyUID = copyUserInfo->pw_uid;

  /* become user that is copying */
  if (setuid(uid) < 0)
    {
    MDB(1,fGRID) MLog("ERROR:    globus-url-copy failed - could not become user '%s'! Ensure Moab is running as root.\n",
      Username);

    exit(1);
    }

  /* start the globus-url-copy */
  if (execv(globus-url-copy,copyArgs) < 0)
    {
    /* failure (child did not spawn globus-url-copy) */

    MDB(1,fGRID) MLog("ERROR:    globus-url-copy failed - could not launch staging process (errno: %d: '%s')\n",
      errno,
      strerror(errno));

    exit(1);
    }

  exit(1);

  /* NOTREACHED */

  return(FAILURE);
    
  }  /* END MGlobusStageData() */

#endif /* MNOT */

int MGlobusCheckStageStatus(

  int copyPID,     /* I */
  mbool_t *IsDone, /* I (whether or not stage is completed) */
  char    *EMsg)   /* O (optional) */

  {
  int childStatus;
  int rc;
  
  if (copyPID <= 0)
    {
    return(FAILURE);
    }

  if (IsDone == NULL)
    {
    return(FAILURE);
    }

  if (EMsg != NULL)
    {
    EMsg[0] = '\0';
    }

  *IsDone = FALSE;

  rc = waitpid(copyPID,&childStatus,WNOHANG);

  if (rc == 0)
    {
    *IsDone = FALSE;

    return(SUCCESS);
    }
  else if (rc == -1)
    {
    /* error! */

    MDB(1,fGRID) MLog("WARNING:  cannot check copy PID %d - doesn't exist!\n",
      copyPID);

    if (EMsg != NULL)
      {
      sprintf(EMsg,"cannot check copy PID %d - doesn't exist!\n",
        copyPID);
      }
    
    return(FAILURE);
    }

  /* child has terminated - determine if success or not */
  *IsDone = TRUE;

  if (WIFEXITED(childStatus))
    {
    if (WEXITSTATUS(childStatus) == 0)
      {      
      MDB(1,fGRID) MLog("WARNING:  globus-url-copy process %d returned with code %d\n",
        copyPID,
        WEXITSTATUS(childStatus));

      if (EMsg != NULL)
        {
        sprintf(EMsg,"globus-url-copy process %d returned with code %d\n",
          copyPID,
          WEXITSTATUS(childStatus));
        }

      return(SUCCESS);
      }
    else
      {
      /* abnormal termination */
      
      MDB(1,fGRID) MLog("WARNING:  globus-url-copy process %d did not exit normally\n",
        copyPID,
        WEXITSTATUS(childStatus));

      if (EMsg != NULL)
        {
        sprintf(EMsg,"globus-url-copy process %d did not exit normally\n",
          copyPID,
          WEXITSTATUS(childStatus));
        }

      return(FAILURE);
      }
    }  /* END if (WIFEXITED(childStatus)) */

  /* signal termination of child */

  MDB(1,fGRID) MLog("WARNING:  globus-url-copy process %d was terminated by signal %d\n",
    copyPID,
    WTERMSIG(childStatus));

  if (EMsg != NULL)
    {
    sprintf(EMsg,"globus-url-copy process %d was terminated by signal %d\n",
      copyPID,
      WTERMSIG(childStatus));
    }

  return(FAILURE);     
  }  /* END MGlobusCheckStageStatus() */

#endif /* __MGLOBUS */



/* END MGrid.c */

