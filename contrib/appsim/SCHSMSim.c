/* CONTRIB:  SCHSMSim.c */
 
/* code to be included in LocalInitialize() in Local.c */

/* contains both HSM simulator and job app sim modules */

#define HSM_MAXTRANS            128

#define HSM_TOTALSIZE      20000000
#define HSM_CACHESIZE       1000000
#define HSM_TAPEBANDWIDTH     40000  
#define HSM_NETBANDWIDTH     100000
#define HSM_NODEBANDWIDTH     20000
#define HSM_TAPECOUNT             4

struct {
  mjob_t *J;
  long    StartTime;
  int     DataTotal;
  int     DataRemaining;
  int     State;
  int     Priority;
  } hsmtrans_t;

struct {
  int TotalSize;       /* size in KB   */
  int CacheSize;       /* size in KB   */
  int TapeBandwidth;   /* BW in KB/sec */
  int NetBandwidth;    /* BW in KB/sec */
  int NodeBandwidth;   /* BW in KB/sec */
  int TapeCount;       /* number of tape drives */
  hsmtrans_t T[HSM_MAXTRANS];

  int ActiveTapeCount;
  int ActiveTransCount;
  } HSM;


typedef struct {
  int   DataState;

  char *InputDataName;
  int   InputDataSize;

  char *OutputDataName;
  int   OutputDataSize;
  } hsmdata_t;


/* allow up to <TAPECOUNT> tape drive to operate simultaneously  */
/* only initiate stage IN/OUT if adequate cache space exists     */
/* tape bandwidth is dedicated                                   */
/* network bandwidth is evenly shared by all active transactions */
/* allow each job to perform only one stage in/out operation at any given time */
/* data only staged when complete file is available */
/* (ie, tape to cache, then cache to compute node) */
/* <MAXTRANS> stage requests can be queued up */




int ContribMASHSM(
 
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
    case mascInitialize:
 
      rc = MASHSMInitialize(J,(char *)IData);
 
      break;
 
    case mascUpdate:
 
      rc = MASHSMUpdate(J,IData);
 
      break;
 
    case mascFinalize:
 
      rc = MASHSMFinalize(J);
 
      break;
 
    default:
 
      rc = FAILURE; 
 
      break;
    }  /* END switch(CmdIndex) */
 
  return(rc);
  }  /* END ContribMASHSM() */



int ContribSCHSMSimInitialize()

  {
  memset(&HSM,0,sizeof(HSM));

  HSM.TotalSize = HSM_TOTALSIZE;
  HSM.CacheSize = HSM_CACHESIZE;

  HSM.TapeBandwidth = HSM_TAPEBANDWIDTH;
  HSM.NetBandwidth  = HSM_NETBANDWIDTH;

  return(SUCCESS);
  }  /* END ContribSCHSMSimInitialize() */





int ContribSCHSMSimInitiateStageOut(

  mjob_t *J,
  hsmdata_t)
 
  {
  int tindex;

  int AvailT;

  AvailT = -1;

  if (HSM.ActiveTrans >= HSM_MAXTRANS)
    {
    return(FAILURE);
    }

  for (tindex = 0;tindex < MAX_HSMTRANS;tindex++)
    {
    /* verify job has no other existing transactions */

    if (HSM.T[tindex].J == J)
      {
      /* job already has active transaction */

      return(FAILURE);
      }
    
    if ((HSM.T[tindex].J == NULL) && (AvailT == -1))
      AvailT == tindex;
    }  /* END for (tindex) */

  HSM.T[tindex].J = J;
  HSM.T[tindex].StartTime = MSched.Time;
  HSM.T[tindex].DataTotal = X;
 
  long    StartTime;
  int     DataTotal;
  int     DataRemaining;
  int     State;
  int     Priority;

  return(SUCCESS);
  }  /* END ContribSCHSMSimStageOut() */



int ContribSCHSMUpdate()

  {
  /* determine number of active transactions */

  /* determine per transaction network bandwidth */

  /* stage data out from cache */

  /* stage to tape */

  /* stage to compute nodes */

  /* stage in data to cache */

  /* adjust job data state */

  return(SUCCESS);
  }  /* END ContribSCHSMUpdate() */
  
/* END SCHSMSim.c */


