/* HEADER */
        
/* Contains:                                 *
 *                                           */

/* this file contains routines to allow emulation of a network infrastructure resource *
 * Each network possesses a bandwidth, latency issues ae not considered                *
 * Each network attempts to equally service all requests simultaneously                *
 * All requests require a minimum of one scheduling cycle?                             */

/* MASNet structures */

enum { 
  masnt1NONE = 0,
  masnt1Create,
  masnt1Suspend,
  masnt1Resume,
  masnt1Destroy };

typedef struct {
  int     State;           /* 0: transfer is suspended/completed  1: transfer is active */

  char   *FileName;
  int     FileSize;        /* in KB */
  int     DataStaged;      /* in KB */
  long    StartTime;       /* non-zero when transfer is started */
  long   *CompletionTime;  /* non-zero when transfer is completed */
  } masnet1trans_t;

typedef struct {
  char *Name;
  int   State;

  int   Bandwidth;       /* in MB/s */

  masnet1trans_t *ActiveList[MAX_MJOB];

  int   Duration;        /* in seconds */
  int   MBTransferred;
  int   TransfersCompleted;
  double AvgTransferBW;
  int   AvgTransferDelay;
  } masnet1_t;



const char *MASNet1AttributeType[] = {
  NONE,
  "BANDWIDTH",
  "FILE",
  "NAME",
  "STATE",
  NULL };

enum MASNet1AttrEnum {
  masnet1aNONE = 0,
  masnet1aBandwidth,
  masnet1aFile,
  masnet1aName,
  masnet1aState };

/* prototypes */
 
int  MASNet1(
 
  masnet1_t **N,
  int         CmdIndex,
  void       *IData,
  void      **OData);


int MASNet1Config(
 
  masnet1_t  *N,
  char       *ConfigString,
  void      **OData);


int MASNet1Destroy(
 
  masnet1_t **N);


int MASNet1Create(
 
  masnet1_t **N,
  char       *ConfigString);


int MASNet1Update(
 
  masnet1_t *N);





int MASNet1Query(

  masnet1_t  *N,
  char       *AttrName,
  void      **AttrVal)

  {
  if ((N == NULL) || (AttrName == NULL) || (AttrVal == NULL))
    {
    return(FAILURE);
    }

  /* NYI */

  return(FAILURE);
  }  /* END MASNet1Query() */





int MASNet1Show(

  masnet1_t *N,
  char      *Buffer)

  {
  int tindex;

  masnet1trans_t *T;

  char tmpString[MAX_MNAME];

  /* display state, configuration, statistics, and activity */

  if ((N == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  Buffer[0] = '\0';

  sprintf(Buffer,"%sResource %s  Type:  %s\n\n",
    Buffer,
    N->Name,
    "Network");

  sprintf(Buffer,"%sState:  %s\n\n",
    Buffer,
    (N->State == 1) ? "Active" : "Down");

  sprintf(Buffer,"%sBandwidth:  %d MB/s\n",
    Buffer,
    N->Bandwidth);

  sprintf(Buffer,"%sUpTime:  %s    Data Transferred:  %d MB   Transfers Completed: %d\n",
    Buffer,
    MULToTString(N->Duration),
    N->MBTransferred,
    N->TransfersCompleted);
  
  sprintf(Buffer,"%sAvg Effective BW/Job: %6.2lf   Avg Delay/Job: %6.2lf\n",
    Buffer,
    (N->TransfersCompleted > 0) ? N->AvgTransferBW / N->TransfersCompleted : 0.0,
    (N->TransfersCompleted > 0) ? (double)N->AvgTransferDelay / N->TransfersCompleted : 0.0);

  strcat(Buffer,"\n");

  for (tindex = 0;tindex < MAX_MNODE;tindex++)
    {
    /* display transaction information */

    T = N->ActiveList[tindex];

    if (T == NULL)
      break;

    if (T == (masnet1trans_t *)1)
      continue;

    if ((T->CompletionTime != NULL) && (*T->CompletionTime > 0))
      {
      strcpy(tmpString,MULToTString(*T->CompletionTime - MSched.Time));
      }
    else
      {
      strcpy(tmpString,"N/A");
      }

    sprintf(Buffer,"%s  Transaction[%d]  %s  %d of %d KB   %s -> %s\n",
      Buffer,
      tindex,
      T->FileName,
      T->DataStaged,     
      T->FileSize,
      MULToTString((T->StartTime > 0) ? T->StartTime - MSched.Time : 0),
      tmpString);
    }  /* END for tindex) */

  strcat(Buffer,"\n");            

  return(SUCCESS);
  }  /* MASNet1Show() */




int MASNet1TransCreate(

  masnet1_t *N,
  char      *FileName,
  int        FileSize,      /* in MB */
  long      *StageTimePtr)

  {
  int tindex;

  masnet1trans_t **T;

  if (N == NULL)
    return(FAILURE);

  for (tindex = 0;tindex < MAX_MNODE;tindex++)
    {
    T = &N->ActiveList[tindex];

    if (*T == (masnet1trans_t *)NULL)
      break;

    if (*T == (masnet1trans_t *)1)
      break;
    }  /* END for (tindex) */

  if (tindex == MAX_MNODE)
    {
    /* transaction buffer is full */

    return(FAILURE);
    }

  (*T) = (masnet1trans_t *)calloc(1,sizeof(masnet1trans_t));

  MUStrDup(&(*T)->FileName,FileName);

  (*T)->FileSize = FileSize << 10;

  (*T)->CompletionTime = (long *)StageTimePtr;

  /* enable transfer */

  (*T)->State = TRUE;

  if (StageTimePtr != NULL)
    *(long *)StageTimePtr = 0;
  
  return(SUCCESS);
  }  /* END MASNet1TransCreate() */




int MASNet1TransDestroy(
 
  masnet1trans_t **T)
 
  {
  if (T == NULL)
    return(FAILURE);
 
  MUFree(&(*T)->FileName);
 
  MUFree((char **)T);

  *T = (masnet1trans_t *)1; 

  return(SUCCESS);
  }  /* END MASNet1TransDestroy() */




int MASNet1(

  masnet1_t **N,
  int         CmdIndex,
  void       *IData,
  void      **OData)

  {
  int rc;

  if (N == NULL)
    return(FAILURE);

  switch(CmdIndex)
    {
    case mascConfig:

      /* add job to transfer list */

      rc = MASNet1Config(*N,(char *)IData,OData);

      break;

    case mascCreate:
 
      rc = MASNet1Create(N,(char *)IData);
 
      break;

    case mascDestroy:

      rc = MASNet1Destroy(N);

      break;

    case mascQuery:

      rc = MASNet1Query(*N,(char *)IData,OData);

      break;

    case mascShow:

      rc = MASNet1Show(*N,(char *)OData);

      break;

    case mascUpdate:
 
      rc = MASNet1Update(*N);
 
      break;

    default:

      rc = FAILURE;

      break;
    }  /* END switch(CmdIndex) */

  return(rc);
  }  /* END MASNet1() */





int MASNet1Config(

  masnet1_t  *N,
  char       *ConfigString,
  void      **OData)

  {
  int   aindex;

  char *ptr;
  char *TokPtr;
  char *TokPtr2;

  char *FName;

  char *tmpP;
  int   Size;

  char  ValLine[MAX_MNAME];
  int   CmpMode;

  int   tindex;

  masnet1trans_t **T;

  if ((N == NULL) || (ConfigString == NULL))
    {
    return(FAILURE);
    }
 
  /* process config data */
 
  /* FORMAT:  [BANDWIDTH[+=|-=]<VAL>][;FILE=<NAME>:<SIZE>]... */
 
  ptr = MUStrTok(ConfigString,"; \t\n",&TokPtr);
 
  while (ptr != NULL)
    {
    if (MUGetPair(
          ptr,
          (const char **)MASNet1AttributeType,
          &aindex,
	  NULL,
          TRUE,
          &CmpMode,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */
 
      ptr = MUStrTok(NULL,"; \t\n",&TokPtr);
 
      continue;
      }
 
    switch(aindex)
      {
      case masnet1aFile:

        /* FORMAT:  <FILENAME>:<FILESIZE> */

        FName = MUStrTok(ValLine,":",&TokPtr2);

        if ((tmpP = MUStrTok(NULL,":",&TokPtr2)) != NULL)
          {
          Size = (int)strtol(tmpP,NULL,0);
          }
        else
          {
          Size = -1;
          }

        if (CmpMode == -1)
          {
          /* locate and remove transaction */
       
          for (tindex = 0;tindex < MAX_MNODE;tindex++)
            {
            T = &N->ActiveList[tindex];
 
            if (*T == (masnet1trans_t *)NULL)
              break;
 
            if (*T == (masnet1trans_t *)1)
              continue;

            if (!strcmp((*T)->FileName,FName))
              {
              MASNet1TransDestroy(T);    

              break;
              }
            }  /* END for (tindex) */
          }
        else
          {
          /* create new transaction */

          MASNet1TransCreate(
            N,
            FName,
            Size,
            (long *)OData);
          }

        break;

      default:

        break;
      }  /* END switch (aindex) */

    ptr = MUStrTok(NULL,"; \t\n",&TokPtr);              
    }  /* END while (ptr != NULL) */

  return(FAILURE);
  }  /* MASNet1Config() */





int MASNet1Create(

  masnet1_t **N,
  char       *ConfigString)

  {
  char *ptr;
  char *TokPtr;

  char  ValLine[MAX_MLINE];
  
  int   aindex;

  mreq_t *RQ;

  if (N == NULL)
    return(FAILURE);

  /* create AS data structure */

  if (*N == NULL)
    {
    *N = (masnet1_t *)calloc(1,sizeof(masnet1_t));
    }

  if (ConfigString != NULL)
    {
    /* process config data */

    /* FORMAT:  BANDWIDTH=<BANDWIDTH>;NAME=<NAME>;STATE=<ACTIVE>... */

    ptr = MUStrTok(ConfigString,"; \t\n",&TokPtr);

    while (ptr != NULL)
      {
      if (MUGetPair(
            ptr,
            (const char **)MASNet1AttributeType,
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
        case masnet1aBandwidth:
       
          /* FORMAT: <BANDWIDTH> */

          (*N)->Bandwidth = (int)strtol(ValLine,NULL,0);

          break;

        case masnet1aName:

          MUStrDup(&(*N)->Name,ValLine);

          break;

        case masnet1aState:

          /* enable network by default */

          (*N)->State = 1;

          break;

        default:
 
          break;
        }  /* END switch(aindex) */

      ptr = MUStrTok(NULL,"; \t\n",&TokPtr);          
      }    /* END while (ptr != NULL) */
    }    /* END if (ConfigString != NULL) */

  /* clear network state */

  /* NYI */

  return(SUCCESS);
  }  /* END MASNet1Create() */




int MASNet1Destroy(
 
  masnet1_t **N)
 
  {
  if (N == NULL)
    return(SUCCESS);

  /* clear AS Data */

  if (*N != NULL)
    {
    MUFree(&(*N)->Name);

    /* NOTE:  must destroy all transactions */

    /* NYI */

    MUFree((char **)N);
    }

  return(SUCCESS);
  }  /* END MASNet1Destroy() */




int MASNet1Update(

  masnet1_t *N)

  {
  long Now;

  int tindex;
 
  int MinTransfer;
  int ActiveTransferCount;

  int TKBytes;

  long Duration;
  long StartTime;

  maslsdata_t *D;

  masnet1trans_t *T;

  /* schedule transfers to interval (Now - MSched.Interval) -> Now */
 
  /* loop through all transfers               *
   * determine minimum outstanding transfer   *
   * determine total data transfer requested  *
   * determine total data transfer possible   *
   * transfer MIN(possible,requested)         *
   * update completed transfers               *
   * continue until all transfers complete or *
   * all network bandwidth consumed           */

  if (N == NULL)
    return(FAILURE);
 
  /* all files/bandwidth in MB, all operations in KB */

  Duration = (long)MSched.Interval / 100;

  TKBytes = (Duration * N->Bandwidth) << 10;

  Now = MSched.Time - Duration;

  while (TKBytes > 0)
    {
    MinTransfer         = MAX_INT;
    ActiveTransferCount = 0;

    for (tindex = 0;tindex < MAX_MJOB;tindex++)
      {
      T = N->ActiveList[tindex];        

      if (T == NULL)
        break;

      if (T == (masnet1trans_t *)1)
        continue;

      if (T->State == 0) 
        continue;

      ActiveTransferCount++;

      if ((T->FileSize - T->DataStaged) < MinTransfer)
        {
        MinTransfer = T->FileSize - T->DataStaged;
        }
      }    /* END for (tindex) */

    if (ActiveTransferCount == 0)
      {
      /* no active transfers required */

      return(SUCCESS);
      }

    MinTransfer = MIN(MinTransfer,TKBytes / ActiveTransferCount);    

    StartTime = Now;

    Now += (MinTransfer >> 10) / N->Bandwidth;

    /* transfer <MinTransfer> KB for all active transfers */

    for (tindex = 0;tindex < MAX_MJOB;tindex++)
      {
      T = N->ActiveList[tindex];
 
      if (T == NULL)
        break;
 
      if (T == (masnet1trans_t *)1)
        continue;
 
      if (T->State == 0)
        continue;

      if (T->DataStaged == 0)
        T->StartTime = StartTime;

      T->DataStaged += MinTransfer;
      TKBytes       -= MinTransfer;
 
      if (T->DataStaged == T->FileSize)
        {
        /* transfer is complete */

        *T->CompletionTime = Now;

        N->TransfersCompleted++;

        N->AvgTransferBW += ((double)T->FileSize / (Now - T->StartTime));

        N->AvgTransferDelay += (Now - T->StartTime);

        T->State = 0;
        }
      }    /* END for (tindex) */
    }      /* END while (TKBytes > 0) */

  N->Duration      += Duration;
  N->MBTransferred += ((Duration * N->Bandwidth) - TKBytes) >> 10;

  return(SUCCESS);
  }  /* END MASNet1Update() */



/* END Net1.c */


