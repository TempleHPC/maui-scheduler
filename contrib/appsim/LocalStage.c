/* HEADER */
        
/* Contains:                                 *
 *                                           */

#define COMPUTE_NODE_STAGE_LOAD  1  /* CPU load of a compute node staging data */

/* prototypes */

int  MASLocalStage(mjob_t *,int,void *,void **); 
int MASLSJobUpdate(mjob_t *);
int MASLSJobDestroy(mjob_t *);
int MASLSJobShow(mjob_t *,char *);
int MASLSJobCreate(mjob_t *,char *);
int MASLSJobConfig(mjob_t *,char *);



/* structures */

typedef struct {
  char *Name;
  int   ASState;

  xres_t *INetRes;
  char   *IFileName;
  int     IFileSize;     /* in KB */
  int     IFileStaged;
  long    IStageTime;

  xres_t *ONetRes;
  char   *OFileName;
  int     OFileSize;     /* in KB */
  int     OFileStaged;
  long    OStageTime;
  } maslsdata_t;



const char *MASLSAttributeType[] = {
  NONE,
  "INPUT",
  "OUTPUT",
  "STATE",
  NULL };

enum MASLSAttrEnum {
  maslsaNONE = 0,
  maslsaInput,
  maslsaOutput,
  maslsaState };






int MASLocalStage(

  mjob_t  *J,
  int      CmdIndex,
  void    *IData,
  void   **OData)

  {
  int rc;

  if (J == NULL)
    return(FAILURE);

  switch(CmdIndex)
    {
    case mascConfig:

      rc = MASLSJobConfig(J,(char *)IData);      

      break;

    case mascCreate:

      rc = MASLSJobCreate(J,(char *)IData);

      break;

    case mascDestroy:
 
      rc = MASLSJobDestroy(J);
 
      break;

    case mascShow:

      rc = MASLSJobShow(J,(char *)OData);

      break;

    case mascUpdate:

      rc = MASLSJobUpdate(J);

      break;

    default:

      rc = FAILURE;

      break;
    }  /* END switch(CmdIndex) */

  return(rc);
  }  /* END MASLocalStage() */





int MASLSJobCreate(

  mjob_t *J,
  char   *ConfigString)

  {
  char *ptr;
  char *TokPtr;

  char *ptr2;
  char *TokPtr2;

  char *ptr3;

  char  ValLine[MAX_MLINE];
  
  int   aindex;

  mreq_t *RQ;

  maslsdata_t *D;

  if (J == NULL)
    return(FAILURE);

  /* NOTE:  by default, each job uses 100% of all dedicated resources */          

  /* set 'RQ->URes.Procs' */
  /* set 'RQ->URes.Mem'   */

  /* create AS data structure */

  if (J->ASData == NULL)
    {
    J->ASData = calloc(1,sizeof(maslsdata_t));
    }

  D = (maslsdata_t *)J->ASData;

  if (ConfigString != NULL)
    {
    /* process config data */

    /* FORMAT:  INPUT=<INPUTDATANAME>:<INPUTDATASIZE>;OUTPUT=<OUTPUTDATANAME>:<OUTPUTDATASIZE> */

    ptr = MUStrTok(ConfigString,"; \t\n",&TokPtr);

    while (ptr != NULL)
      {
      if (MUGetPair(
            ptr,
            (const char **)MASLSAttributeType,
            &aindex,
	    NULL,
            TRUE,
            NULL,
            ValLine,
            MAX_MNAME) == FAILURE)
        {
        /* cannot parse value pair */
 
        ptr = MUStrTok(NULL,"; \t\n",&TokPtr);
 
        continue;
        }

      switch(aindex)
        {
        case maslsaInput:
      
          /* FORMAT: <FILENAME>[<@<SOURCE>]:<FILESIZE> */
 
          if (strchr(ValLine,'@') != NULL)
            {
            if ((ptr2 = MUStrTok(ValLine,":@",&TokPtr2)) != NULL)
              {
              MUStrDup(&D->IFileName,ptr2);
 
              if ((ptr2 = MUStrTok(NULL,":@",&TokPtr2)) != NULL)
                {
                MGResFind(ptr2,-1,&D->INetRes);
                }
              }
            }
          else
            {
            if ((ptr2 = MUStrTok(ValLine,":",&TokPtr2)) != NULL)
              {
              MUStrDup(&D->IFileName,ptr2);
              }
            }
 
          if (D->INetRes == NULL)
            {
            MGResFind(NULL,-1,&D->INetRes);
            }
 
          if ((ptr2 = MUStrTok(NULL,":@",&TokPtr2)) != NULL)
            {
            D->IFileSize = (int)strtol(ptr2,NULL,0);
            }
 
          D->IFileStaged = 0;
 
          break;

        case maslsaOutput:

          /* FORMAT: <FILENAME>[<@<SOURCE>]:<FILESIZE> */

          if (strchr(ValLine,'@') != NULL)
            {
            if ((ptr2 = MUStrTok(ValLine,":@",&TokPtr2)) != NULL)
              {
              MUStrDup(&D->OFileName,ptr2);    

              if ((ptr2 = MUStrTok(NULL,":@",&TokPtr2)) != NULL)
                {
                MGResFind(ptr2,-1,&D->ONetRes);
                }
              }
            }
          else
            {
            if ((ptr2 = MUStrTok(ValLine,":",&TokPtr2)) != NULL)
              {
              MUStrDup(&D->OFileName,ptr2);
              }
            }

          if (D->ONetRes == NULL)
            {
            MGResFind(NULL,-1,&D->ONetRes);        
            }

          if ((ptr2 = MUStrTok(NULL,":@",&TokPtr2)) != NULL)
            {
            D->OFileSize = (int)strtol(ptr2,NULL,0);
            }
                 
          D->OFileStaged = 0;

          break;

        default:
 
          break;
        }  /* END switch(aindex) */

      ptr = MUStrTok(NULL,"; \t\n",&TokPtr);         
      }  /* END while (ptr != NULL) */
    }    /* END if (ConfigString != NULL) */

  /* clear resource usage */

  RQ = J->Req[0];

  memset(&RQ->URes,0,sizeof(RQ->URes));

  return(SUCCESS);
  }  /* END MASLSJobCreate() */




int MASLSJobShow(

  mjob_t *J,
  char   *Buffer)

  {
  maslsdata_t *D;

  if ((J == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  Buffer[0] = '\0';

  D = (maslsdata_t *)J->ASData;

  /* display staging file name, size, and status */

  if (D->IFileName[0] != '\0')
    {
    sprintf(Buffer,"%sINPUT:   File: %s:%d over %s\n",
      Buffer,
      D->IFileName,
      D->IFileSize,
      (D->INetRes != NULL) ? D->INetRes->Name : "DEFAULT");
    }

  if (D->OFileName[0] != '\0')
    {
    sprintf(Buffer,"%sOUTPUT:  File: %s:%d over %s\n",
      Buffer,
      D->OFileName,
      D->OFileSize,
      (D->ONetRes != NULL) ? D->ONetRes->Name : "DEFAULT");
    }

  return(SUCCESS);
  }  /* END MASLSJobShow() */





int MASLSJobDestroy(
 
  mjob_t *J)
 
  {
  maslsdata_t *D;

  J->ASFunc = NULL;

  /* clear AS Data */

  if (J->ASData != NULL)
    {
    D = (maslsdata_t *)J->ASData;

    MUFree(&D->IFileName);
    MUFree(&D->OFileName);

    MUFree((char **)J->ASData);
    }

  return(SUCCESS);
  }  /* END MASLSJobDestroy() */





int MASLSJobConfig(

  mjob_t *J,
  char   *ConfigString)

  {
  char *ptr;
  char *TokPtr;

  int   aindex;
  char  ValLine[MAX_MLINE];

  char  tmpLine[MAX_MLINE];

  int   index;

  maslsdata_t *D;
 
  if ((J == NULL) || (ConfigString == NULL))
    {
    return(FAILURE);
    }

  D = (maslsdata_t *)J->ASData;      

  if (D == NULL)
    return(FAILURE);

  /* process config data */
 
  /* FORMAT:  INPUT=<INPUTDATANAME>:<INPUTDATASIZE>;OUTPUT=<OUTPUTDATANAME>:<OUTPUTDATASIZE>;STATE=<STATE>; */
 
  ptr = MUStrTok(ConfigString,"; \t\n",&TokPtr);
 
  while (ptr != NULL)
    {
    if (MUGetPair(
          ptr,
          (const char **)MASLSAttributeType,
          &aindex,
	  NULL,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */

      ptr = MUStrTok(NULL,"; \t\n",&TokPtr);
 
      continue;
      }
 
    switch(aindex)
      {
      case maslsaState:
 
        index = MUGetIndex(ValLine,MJobState,0,0);

        switch(index)
          {
          case mjsStarting:
          case mjsRunning:

            if ((D->ASState != mjsStarting) && (D->ASState != mjsRunning))
              {
              if ((D->IFileSize > 0) && (D->INetRes->Func != NULL))
                {
                /* initiate stagein transaction */

                sprintf(tmpLine,"FILE+=%s:%d",
                  D->IFileName,
                  D->IFileSize);

                (*D->INetRes->Func)(
                  &D->INetRes->Data,
                  mascConfig,
                  tmpLine,
                  (void **)&D->IStageTime);
                }

              D->ASState = index;
              }

            break;

          default:

            /* NO-OP */

            break;
          }  /* END switch(index) */
 
        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(aindex) */
  
    ptr = MUStrTok(NULL,"; \t\n",&TokPtr);       
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MASLSJobConfig() */




int MASLSJobUpdate(

  mjob_t      *J)

  {
  long         Now;

  mreq_t      *RQ;

  maslsdata_t *D;

  double       CR;

  char         tmpLine[MAX_MLINE];

  RQ = J->Req[0];        

  /* default app sim jobs utilize 100% of dedicated resources */

  if (J == NULL)
    return(FAILURE);

  D = (maslsdata_t *)J->ASData;

  if (D == NULL)
    return(FAILURE);

  switch (J->State)
    {
    case mjsRunning:
    case mjsStarting:

      /* job is active */          

      if ((D->IFileName == NULL) || (D->IStageTime > 0))
        {
        /* no file to stage or file staging is complete */

        if ((D->IFileName == NULL) && (D->IStageTime == 0))
          {
          D->IStageTime = J->StartTime;
          }

        if (MSched.Time - D->IStageTime < MSched.RMPollInterval)
          {
          /* job utilizes increased compute resources once data is staged */      

          if ((D->IStageTime > 0) && (D->INetRes != NULL))
            {
            /* staging completed this iteration */
  
            CR = (double)(MSched.Time - D->IStageTime) * 100.0 / MSched.Interval;
            }
          else
            {
            CR = 1.0;
            }
  
          RQ->URes.Procs = (int)(RQ->DRes.Procs * 100 * CR);
          RQ->URes.Mem   = (int)(RQ->DRes.Mem * CR);
          RQ->URes.Swap  = (int)(RQ->DRes.Swap * CR);
          RQ->URes.Disk  = (int)(RQ->DRes.Disk * CR);
  
          RQ->LURes.Procs = RQ->DRes.Procs * 100;
          RQ->LURes.Mem   = RQ->DRes.Mem;
          RQ->LURes.Swap  = RQ->DRes.Swap;
          RQ->LURes.Disk  = RQ->DRes.Disk;
  
          if (D->INetRes != NULL)  
            {
            /* transfer complete */
  
            if (D->INetRes->Func != NULL)
              {
              /* remove transaction */
   
              sprintf(tmpLine,"FILE-=%s",
                D->IFileName);
   
              (*D->INetRes->Func)(
                &D->INetRes->Data,
                mascConfig,
                tmpLine,
                (void **)NULL);
  
              D->INetRes = NULL;
              }
            }

          D->ASState = J->State;
          }
        }
      else
        {
        /* input file is staging */

        /* compute nodes are partially loaded when staging data */

        RQ->URes.Procs = RQ->DRes.Procs * COMPUTE_NODE_STAGE_LOAD;
        }

      break;
       
    case mjsIdle:

      /* job is idle */

      /* job data is static */

      if (J->State != D->ASState)      
        {
        memset(&RQ->URes,0,sizeof(RQ->URes));

        D->ASState = J->State;    
        }
 
      break;
 
    case mjsCompleted:
    case mjsRemoved:

      /* job is completed */

      /* job is freed once output data is staged */

      if (J->State != D->ASState)
        {
        memset(&RQ->URes,0,sizeof(RQ->URes));

        D->ASState = J->State;   
        }

      break;
 
    default:

      /* NO-OP */

      break;
    }  /* END switch(J->State) */

  return(SUCCESS);
  }  /* END MASLSJobUpdate() */



/* END LocalStage.c */


