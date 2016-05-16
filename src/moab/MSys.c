/* HEADER */

#include "moab.h"
#include "moab-proto.h"



extern mlog_t mlog;

/* NOTE:  must sync with declaration */

mjob_t   *MJob[MAX_MJOB];
mnode_t  *MNode[MAX_MNODE];
mgcred_t *MUser[MAX_MUSER + MAX_MHBUF];
mgcred_t  MGroup[MAX_MGROUP + MAX_MHBUF];
int       FSGroupKeys[MAX_MGROUP];    /* HvB holds the FairShare Group Keys */
mgcred_t  MAcct[MAX_MACCT + MAX_MHBUF];
mres_t   *MRes[MAX_MRES];    
srsv_t    SRes[MAX_MSRES];
srsv_t    OSRes[MAX_MSRES];
mjobl_t   MJobName[MAX_MJOB + MAX_MHBUF];
mam_t     MAM[MAX_MAM];
mrange_t  MRange[MAX_MRANGE];
mrclass_t MRClass[MAX_MRCLASS];   /* available resource classes */
int       MAQ[MAX_MJOB];          /* terminated by '-1' value   */
mre_t     MRE[MAX_MRES << 2];
mclass_t  MClass[MAX_MCLASS];
msched_t  MSched;
mckpt_t   MCP;
mpar_t    MPar[MAX_MPAR];
mrm_t     MRM[MAX_MRM];
mqos_t    MQOS[MAX_MQOS];
mframe_t  MFrame[MAX_MFRAME];
mattrlist_t MAList;               /* dynamic scheduling attributes */
mstat_t   MStat;
mclient_t MClient[MAX_MCLIENT];
mjob_t   *MJobTraceBuffer;
mrmfunc_t MRMFunc[MAX_MRMTYPE];
msim_t    MSim;
msys_t    MSys;                   /* cluster layout */
m64_t     M64;

mx_t      X;
int       MFQ[MAX_MJOB];          /* terminated by '-1' value      */
int       MUIQ[MAX_MJOB];

char     *MParam[MAX_MCFG];

extern mjob_t       *MJobTraceBuffer;
extern const mcfg_t  MCfg[];

extern const char *MAMOType[];
extern const char *MJobFlags[];
extern const char *MSysNodeAttr[];
extern const char *MNodeState[];
extern const char *MRMType[];
extern const char *MS3CName[];
extern const char *MSchedAttr[];
extern const char *MCredCfgParm[];
extern const char *MSchedMode[];

mrclass_t MRClass[MAX_MRCLASS];   /* resource classes */

/* local prototpyes */

int __MSysTestRLMerge();

/* END local prototypes */

#include "__MGridStub.c"




int MSysInitialize(mbool_t DoInit)

  {
  int       index;
  int       srindex;

  msched_t *S;

  time_t    tmpTime;

  const char *FName = "MSysInitialize";

  DBG(5,fALL) DPrint("%s()\n",
    FName);

  /* initialize all data structures */

  S = &MSched;

  memset(S,0,sizeof(MSched));

  time(&tmpTime);

  S->Time = (long)tmpTime;

  S->X    = (void *)&X;

  M64Init(&M64);

  MOSSyslogInit(S);

  MUBuildPList((mcfg_t *)MCfg,MParam);

  S->T[mxoAcct]  = &MAcct[0];
  S->S[mxoAcct]  = sizeof(mgcred_t);
  S->M[mxoAcct]  = MAX_MACCT + MAX_MHBUF;
  S->E[mxoAcct]  = &MAcct[MAX_MACCT + MAX_MHBUF - 1];

  S->T[mxoAM]    = &MAM[0];
  S->S[mxoAM]    = sizeof(mam_t);
  S->M[mxoAM]    = MAX_MAM;
  S->E[mxoAM]    = &MAM[MAX_MAM - 1];

  S->T[mxoClass] = &MClass[0];
  S->S[mxoClass] = sizeof(mclass_t);
  S->M[mxoClass] = MAX_MCLASS;
  S->E[mxoClass] = &MClass[MAX_MCLASS - 1];

  S->T[mxoCP]    = &MCP;
  S->S[mxoCP]    = sizeof(mckpt_t);
  S->M[mxoCP]    = 1;
  S->E[mxoCP]    = &MCP;

  S->T[mxoGroup] = &MGroup[0];
  S->S[mxoGroup] = sizeof(mgcred_t);
  S->M[mxoGroup] = MAX_MGROUP + MAX_MHBUF;
  S->E[mxoGroup] = &MGroup[MAX_MGROUP + MAX_MHBUF - 1];

  S->T[mxoJob]   = &MJob[0];
  S->S[mxoJob]   = sizeof(mjob_t);
  S->M[mxoJob]   = MAX_MJOB;
  S->E[mxoJob]   = &MJob[MAX_MJOB - 1];

  S->T[mxoNode]  = &MNode[0];
  S->S[mxoNode]  = sizeof(mnode_t);
  S->M[mxoNode]  = MAX_MNODE;
  S->E[mxoNode]  = &MNode[MAX_MNODE - 1];

  S->T[mxoPar]   = &MPar[0];
  S->S[mxoPar]   = sizeof(mpar_t);
  S->M[mxoPar]   = MAX_MPAR;
  S->E[mxoPar]   = &MPar[MAX_MPAR - 1];

  S->T[mxoQOS]   = &MQOS[0];
  S->S[mxoQOS]   = sizeof(mqos_t);
  S->M[mxoQOS]   = MAX_MQOS;
  S->E[mxoQOS]   = &MQOS[MAX_MQOS - 1];

  S->T[mxoRsv]   = &MRes[0];
  S->S[mxoRsv]   = sizeof(mres_t);
  S->M[mxoRsv]   = MAX_MRES;
  S->E[mxoRsv]   = &MRes[MAX_MRES - 1];

  S->T[mxoRM]    = &MRM[0];
  S->S[mxoRM]    = sizeof(mrm_t);
  S->M[mxoRM]    = MAX_MRM;
  S->E[mxoRM]    = &MRM[MAX_MRM - 1];

  S->T[mxoSched] = &MSched;
  S->S[mxoSched] = sizeof(msched_t);
  S->M[mxoSched] = 1;
  S->E[mxoSched] = &MSched;

  S->T[mxoSRes]  = &SRes[0];
  S->S[mxoSRes]  = sizeof(srsv_t);
  S->M[mxoSRes]  = MAX_MSRES;
  S->E[mxoSRes]  = &SRes[MAX_MSRES - 1];

  S->T[mxoUser]  = &MUser[0];
  S->S[mxoUser]  = sizeof(mgcred_t *);
  S->M[mxoUser]  = MAX_MUSER + MAX_MHBUF;
  S->E[mxoUser]  = &MUser[MAX_MUSER + MAX_MHBUF - 1];

  memset(MJob,0,sizeof(MJob));
  memset(MNode,0,sizeof(MNode));

  memset(MJobName,0,sizeof(MJobName));

  memset(MRes,0,sizeof(MRes));
  memset(SRes,0,sizeof(SRes));
  memset(MRE,0,sizeof(MRE));

  memset(MAList,0,sizeof(MAList));
  memset(MPar,0,sizeof(MPar));
  memset(MRClass,0,sizeof(MRClass));
  memset(&MCP,0,sizeof(MCP));

  strcpy(MCP.SVersionList,MCKPT_SVERSIONLIST);
  strcpy(MCP.WVersion,MCKPT_VERSION);

  memset(MRM,0,sizeof(MRM));

  MRMLoadModules();

  memset(MAQ,0,sizeof(MAQ));

  MAQ[0] = -1;

  memset(MRange,0,sizeof(MRange));

  MFUCacheInitialize(&S->Time);

  MQueueInitialize(&MJob[0],DEFAULT);

  MSchedSetDefaults(S);

  MSimSetDefaults();

  MStatSetDefaults();

  MCredSetDefaults();

  /* initialize standing reservations */

  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    OSRes[srindex].TaskCount = 0;

    OSRes[srindex].StartTime = 0;
    OSRes[srindex].EndTime   = 0;

    OSRes[srindex].A         = NULL;
    }  /* END for (srindex) */

  MNodeInitialize(S->GN,MDEF_GNNAME);

  /* load state attribute values */

  strcpy(MAList[eNodeState][0],NONE);

  for (index = 1;MNodeState[index] != NULL;index++)
    {
    MUMAGetIndex(eNodeState,(char *)MNodeState[index],mAdd);
    }

  strcpy(MAList[eJobState][0],NONE);

  for (index = 1;MJobState[index] != NULL;index++)
    {
    MUMAGetIndex(eJobState,(char *)MJobState[index],mAdd);
    }

  strcpy(MAList[eSysAttr][0],NONE);

  for (index = 1;MSysNodeAttr[index] != NULL;index++)
    {
    MUMAGetBM(eSysAttr,(char *)MSysNodeAttr[index],mAdd);
    }  /* END for (index) */

  strcpy(MAList[eJFeature][0],NONE);

  for (index = 1;MJobFlags[index] != NULL;index++)
    {
    MUMAGetBM(eJFeature,(char *)MJobFlags[index],mAdd);
    }

  strcpy(MAList[eFeature][0],NONE);
  strcpy(MAList[eNetwork][0],NONE);
  strcpy(MAList[eOpsys][0],NONE);
  strcpy(MAList[eArch][0],NONE);
  strcpy(MAList[eSysAttr][0],NONE);

  MParAdd(GLOBAL_MPARNAME,NULL);
  MParAdd(DEFAULT_MPARNAME,NULL);

  MClassAdd(ALL,NULL);

  /* initialize frame array */

  for (index = 0;index < MAX_MFRAME;index++)
    {
    MFrame[index].Index = index;
    }  /* END for (index) */

  MSUIPCInitialize();

  MLocalInitialize();

  /*
   * HvB
  */
  memset(FSGroupKeys,-1,sizeof(FSGroupKeys));

  return(SUCCESS);
  }  /* END MSysInitialize() */





int MSysLoadConfig(
 
  char *Directory,  /* I */
  char *ConfigFile, /* I */
  int   Mode)       /* I */
 
  {
  char  FileName[MAX_MLINE + 1];
  int   count;
 
  int  SC;

  mnode_t *N;
  mnode_t *GN;

  const char *FName = "MSysLoadConfig";
 
  DBG(3,fCONFIG) DPrint("%s(%s,%s,%d)\n",
    FName,
    Directory,
    ConfigFile,
    Mode);
 
  if ((MSched.ConfigBuffer == NULL) || (Mode & (1 << mcmForce)))
    {
    if (!strstr(ConfigFile,Directory))
      {
      if (Directory[strlen(Directory) - 1] == '/')
	{       
        sprintf(FileName,"%s%s",
          Directory,
          ConfigFile);
        }
      else
        {
        sprintf(FileName,"%s/%s",
          Directory, 
          ConfigFile);
        }
      }
    else
      {
      MUStrCpy(FileName,ConfigFile,sizeof(FileName));
      }
 
    if ((MSched.ConfigBuffer = MFULoad(FileName,1,macmWrite,&count,&SC)) == NULL)
      {
      DBG(2,fCONFIG) DPrint("WARNING:  cannot load configuration file '%s' (using internal defaults)\n",
        FileName);
 
      return(FAILURE);
      }
 
    MCfgAdjustBuffer(&MSched.ConfigBuffer,TRUE);
    }  /* END if ((MSched.ConfigBuffer == NULL) || ...) */

  strcpy(FileName + strlen(FileName) - strlen(".cfg"),"-private.cfg");

  /* load general client config */

  MOLoadPvtConfig(NULL,-1,NULL,NULL,FileName);

  /* load peer service config */

  MOLoadPvtConfig(NULL,mxoNONE,"EM",&MSched.EM,NULL);
  MSched.EM.Type = mpstEM;
  MSched.EM.S    = (msocket_t *)calloc(1,sizeof(msocket_t));

  MOLoadPvtConfig(NULL,mxoNONE,"DS",&MSched.DS,NULL);
  MSched.DS.Type = mpstSD;
  MSched.DS.S    = (msocket_t *)calloc(1,sizeof(msocket_t));

  GN = NULL;

  if (MNodeCreate(&GN) == SUCCESS)
    {
    strcpy(GN->Name,MDEF_GNNAME);
    }

  N = &MSched.DefaultN;

  /* allocate sub-structures/initialize default node */

  if (MNodeCreate(&N) == SUCCESS)
    {
    strcpy(MSched.DefaultN.Name,"DEFAULT");
    }

  MSchedLoadConfig(NULL);

  MCfgProcessBuffer(MSched.ConfigBuffer);

  MCredLoadConfig(mxoSys,NULL,NULL,NULL);

  MCredLoadConfig(mxoQOS,"DEFAULT",NULL,NULL);
  MCredLoadConfig(mxoQOS,NULL,NULL,NULL);      

  MCredLoadConfig(mxoUser,"DEFAULT",NULL,NULL);
  MCredLoadConfig(mxoUser,NULL,NULL,NULL);      

  MCredLoadConfig(mxoGroup,"DEFAULT",NULL,NULL);
  MCredLoadConfig(mxoAcct,"DEFAULT",NULL,NULL);
  MCredLoadConfig(mxoClass,"DEFAULT",NULL,NULL);
 
  MCredLoadConfig(mxoGroup,NULL,NULL,NULL);
  MCredLoadConfig(mxoAcct,NULL,NULL,NULL);
  MCredLoadConfig(mxoClass,NULL,NULL,NULL);
  
  MSRLoadConfig(NULL);

  MRMLoadConfig(NULL);
  MAMLoadConfig(NULL,NULL);

  /* NOTE:  two copies of global node */

  MNodeLoadConfig(GN,NULL);

  if (MUMemCCmp(
        (char *)&GN->CRes,
        '\0',
        sizeof(GN->CRes)) == FAILURE)
    {
    /* global resources are configured */

    MNodeAdd(GN->Name,&N);

    MNodeSetState(N,mnsIdle,0);

    memcpy(&N->CRes,&GN->CRes,sizeof(N->CRes));

    MSched.GN = N;
    }  /* END if (MUMemCCmp() == FAILURE) */

  MNodeDestroy(&GN);

  /* NOTE:  must free core node structure */

  MUFree((char **)&GN);

  MNodeLoadConfig(&MSched.DefaultN,NULL);

  sprintf(MSched.KeyFile,"%s/%s",
    MSched.HomeDir,
    MSCHED_KEYFILE);

  MSched.UID = MOSGetEUID();

  MUCheckAuthFile(&MSched,MSched.DefaultCSKey,NULL,TRUE);

  /*
   * HvB: Find all hashkeys of Fairshare groups and save them 
   *      in an array.
  */
  MGroupGetFSGroups();

  return(SUCCESS);
  }  /* END MSysLoadConfig() */




int MSysMemCheck()
 
  {
  const char *FName = "MSysMemCheck";

  DBG(3,fCORE) DPrint("%s()\n",
    FName);
 
  DBG(2,fCORE) DPrint("MNode[%d]             %6.2f\n",
    MAX_MNODE,
    (double)(sizeof(MNode)) / 1048576);
 
  DBG(2,fCORE) DPrint("MJob[%d]              %6.2f\n",
    MAX_MJOB,
    (double)(sizeof(MJob)) / 1048576);
 
  DBG(2,fCORE) DPrint("MJobTraceBuffer[%d]   %6.2f\n",
    MAX_MJOB_TRACE,
    (double)(sizeof(MJobTraceBuffer)) / 1048576);
 
  DBG(2,fCORE) DPrint("MUser[%d]             %6.2f\n",
    MAX_MUSER + MAX_MHBUF,
    (double)(sizeof(MUser)) / 1048576);
 
  DBG(2,fCORE) DPrint("MGroup[%d]            %6.2f\n",
    MAX_MGROUP + MAX_MHBUF,
    (double)(sizeof(MGroup)) / 1048576);
 
  DBG(2,fCORE) DPrint("MAcct[%d]             %6.2f\n",
    MAX_MACCT + MAX_MHBUF,
    (double)(sizeof(MAcct)) / 1048576);
 
  DBG(2,fCORE) DPrint("MRes[%d]              %6.2f\n",
    MAX_MRES,
    (double)(sizeof(MRes)) / 1048576);
 
  DBG(2,fCORE) DPrint("SRes[%4d]              %6.2f\n",
    MAX_MSRES,
    (double)(sizeof(SRes)) / 1048576);
 
  return(SUCCESS);
  }  /* END MSysMemCheck() */





int __MSysTestXML(
 
  char *XMLString)  /* I */

  {
  char *tail;

  mxml_t *E = NULL;

  fprintf(stdout,"XMLString: '%s'\n\n",
    XMLString);

  if (MXMLFromString(&E,XMLString,&tail,NULL) == FAILURE)
    {
    exit(1);
    }

  if (tail != NULL)
    {
    fprintf(stdout,"tail: '%s'\n\n",
      XMLString);
    }

  exit(0);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END __MSysTestXML() */


  

int __MSysTestRLMerge()
 
  {
  mrange_t R1[] = {
    {956601443,956612113,22,22,0},
    {956612113,956631180,30,30,0},
    {956631180,956635459,33,33,0},
    {956712600,956766449,17,17,0},
    {956766449,956770944,119,119,0},
    {956770944,956810762,131,131,0},
    {956810762,956981059,133,133,0},
    {956981059,957198515,183,183,0},
    {957198515,2139740800,187,187,0},
    {0,0,0,0,0}
    };
 
  mrange_t R2[] = {
    {956635459,956635459,1,1,0},
    {956981059,2139740800,1,1,0},
    {0,0,0,0,0}
    };
 
  /*
  mrange_t R1[] = {
    {888709099,888736699,4,4},
    {888736699,888741199,7,7},
    {888741199,888763698,19,19},
    {888763709,888763709,19,19},
    {888763709,888763711,171,171},
    {888763711,2139999999,190,190},
    {0,0,0,0}
    };
 
  mrange_t R2[] = {
    {888736699,888763698,1,1},
    {888763709,888763709,1,1},
    {888763711,2139999999,1,1},
    {0,0,0,0}
    };
  */
 
  MRLMerge(R1,R2,1,NULL);
 
  exit(0);

  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* END __MSysTestRLMerge() */





int __MSysTestJobSelectFRL()

  {
  mjob_t tmpJ;

  int    RCount;

  mrange_t R1[] = {
    { 1022172521, 1022173705, 476, 119, 0 },
    { 1022173705, 1022176944, 496, 124, 0  },
    { 1022176944, 1022179074, 500, 125, 0  },
    { 1022179074, 1022182393, 516, 129, 0  },
    { 1022182393, 1022185130, 520, 130, 0  },
    { 1022185130, 1022188330, 536, 134, 0  },
    { 1022188330, 1022189431, 600, 150, 0  },
    { 1022189431, 1022208469, 664, 166, 0  },
    { 1022208469, 1022211377, 680, 170, 0  },
    { 1022211377, 1022215080, 728, 182, 0  },
    { 1022215080, 1022250175, 744, 186, 0  },
    { 1022250175, 1022252002, 748, 187, 0  },
    { 1022252002, 1022252096, 752, 188, 0  },
    { 1022252096, 1022253053, 756, 189, 0  },
    { 1022253053, 1022255920, 804, 201, 0  },
    { 1022255920, 2139992800, 808, 202, 0 },
    { 0, 0, 0, 0, 0 } };

  memset(&tmpJ,0,sizeof(tmpJ));

  strcpy(tmpJ.Name,"test");

  tmpJ.Request.TC = 512;
  tmpJ.Request.NC = 128;
  
  MJobSelectFRL(&tmpJ,R1,1,&RCount);

  exit(0);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END __MSysTestJobSelectFRL() */




int __MSysTestNPrioF()

  {
  mnode_t tmpN;

  memset(&tmpN,0,sizeof(tmpN));

  MNodeProcessPrioF(&tmpN,"6*LOAD + -.01 * CMEM - JOBCOUNT");

  exit(0);
 
  /*NOTREACHED*/
  
  return(SUCCESS);
  }  /* END __MSysTestNPrioF() */




int __MSysTestRLAND()

  {
  mrange_t R1[] = {
    { 1016704575, 2139856000, 4, 1, 0 },
/*
    {1,3,1,1},
    {5,6,1,1},
    {6,7,2,2},
    {11,13,2,2},
    {13,14,1,1},
    {15,16,1,1},
    {17,18,1,1},
*/
    {0,0,0,0,0}
    };
 
  mrange_t R2[] = {
    {1016704575, 2139856000, 4, 1, 0 },
/*
    {0,2,1,1},
    {4,6,1,1},
    {8,9,1,1},
    {10,11,1,1},
    {13,14,1,1},
    {14,15,1,1},
    {15,16,1,1},
    {16,17,1,1},
*/
    {0,0,0,0,0}
    };

  mrange_t C[MAX_MRANGE];
 
  /*
  mrange_t R1[] = {
    {888709099,888736699,4,4},
    {888736699,888741199,7,7},
    {888741199,888763698,19,19},
    {888763709,888763709,19,19},
    {888763709,888763711,171,171},
    {888763711,2139999999,190,190},
    {0,0,0,0}
    };
 
  mrange_t R2[] = {
    {888736699,888763698,1,1},
    {888763709,888763709,1,1},
    {888763711,2139999999,1,1},
    {0,0,0,0}
    };
  */ 
 
  MRLAND(C,R1,R2);
 
  exit(0);

  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* END __MSysTestRLAND() */




int __MSysTestJobGetSNRange()
 
  {
  mjob_t   tmpJ;
  mnode_t  tmpN;
 
  mre_t   RE[MAX_MRES_DEPTH << 1];
  mres_t *R[MAX_MRES_DEPTH];
  short   RC[MAX_MRES_DEPTH];
 
  mreq_t  tmpRQ;
 
  mrange_t GRange[MAX_MRANGE];
  mrange_t ARange[MAX_MRANGE];
 
  char    Affinity;

  char NAvailPolicy[MAX_MRESOURCETYPE];
 
  memset(&tmpJ,0,sizeof(tmpJ));
  memset(&tmpN,0,sizeof(tmpN));
  memset(&tmpRQ,0,sizeof(tmpRQ));
  memset(NAvailPolicy,0,sizeof(NAvailPolicy));
 
  /* configure general */
 
  MSched.Time     = 1025;
  MSched.ResDepth = 8;

  mlog.logfp     = stderr;
  mlog.Threshold = 8;
 
  /* configure reservation */

#ifdef __MNOT1
  MResInitialize(&MRes[0],"META");

  MRes[0]->Type       = 2;
  MRes[0]->StartTime  = 10;
  MRes[0]->EndTime    = 3600;
  MRes[0]->DRes.Procs = 1;
  MRes[0]->Flags      = (1 << mrfByName);
 
  /* configure node */
 
  tmpN.CRes.Procs = 2;
  tmpN.ARes.Procs = 2; 

  /* link node */
 
  tmpN.R  = R;
  tmpN.RE = RE;
  tmpN.RC = RC;
 
  tmpN.R[0]  = MRes[0];
  tmpN.RC[0] = 1;
 
  tmpN.RE[0].Type = mreStart;
  tmpN.RE[0].DRes.Procs = 1;
 
  tmpN.RE[1].Type = mreEnd;
  tmpN.RE[1].DRes.Procs = 1;
 
  tmpN.RE[2].Type = mreNONE;
 
  tmpN.RE[0].Time = tmpN.R[0]->StartTime;
  tmpN.RE[1].Time = tmpN.R[0]->EndTime;
 
  tmpN.RE[0].Index = 0;
  tmpN.RE[1].Index = 0;
 
  /* configure job */
 
  tmpJ.Req[0] = &tmpRQ;
  tmpJ.WCLimit = 1800;
  tmpJ.SpecWCLimit[0] = 1800;
 
  tmpRQ.DRes.Procs = 1;
 
  /* configure range requirements */
 
  GRange[0].TaskCount = 1;
  GRange[0].StartTime = 0;
  GRange[0].EndTime   = MAX_MTIME;
 
  GRange[1].EndTime   = 0;
#else /* __MNOT1 */
  MResInitialize(&MRes[0],"1041");

  MRes[0]->Type       = mrtJob;
  MRes[0]->StartTime  = 1025;
  MRes[0]->EndTime    = 2825;
  MRes[0]->DRes.Procs = 1;
  MRes[0]->Flags      = 0;

  MResInitialize(&MRes[1],"1040");

  MRes[1]->Type       = mrtJob;
  MRes[1]->StartTime  = 1000;
  MRes[1]->EndTime    = 2800;
  MRes[1]->DRes.Procs = 1;
  MRes[1]->Flags      = 0;

  /* configure node */

  strcpy(tmpN.Name,"rocky4");

  tmpN.CRes.Procs = 2;
  tmpN.ARes.Procs = 2;

  tmpN.NAvailPolicy = NAvailPolicy;

  tmpN.NAvailPolicy[mrProc] = mrapDedicated;

  /* link node */

  tmpN.R  = R;
  tmpN.RE = RE;
  tmpN.RC = RC;

  tmpN.R[0]  = MRes[0];
  tmpN.RC[0] = 1;

  tmpN.R[1]  = MRes[1];
  tmpN.RC[1] = 1;

  tmpN.RE[0].Type = mreStart;
  tmpN.RE[0].DRes.Procs = 1;
  tmpN.RE[0].Time = tmpN.R[1]->StartTime;
  tmpN.RE[0].Index = 1;

  tmpN.RE[1].Type = mreStart;
  tmpN.RE[1].DRes.Procs = 1;
  tmpN.RE[1].Time = tmpN.R[0]->StartTime;
  tmpN.RE[1].Index = 0;

  tmpN.RE[2].Type = mreEnd;
  tmpN.RE[2].DRes.Procs = 1;
  tmpN.RE[2].Time = tmpN.R[1]->EndTime;
  tmpN.RE[2].Index = 1;

  tmpN.RE[3].Type = mreEnd;
  tmpN.RE[3].DRes.Procs = 1;
  tmpN.RE[3].Time = tmpN.R[0]->EndTime;
  tmpN.RE[3].Index = 0;

  tmpN.RE[4].Type = mreNONE;

  MNode[0] = &tmpN;

  /* configure job */

  strcpy(tmpJ.Name,"1042");

  tmpJ.Req[0]         = &tmpRQ;
  tmpJ.WCLimit        = 1800;
  tmpJ.SpecWCLimit[0] = 1800;

  tmpRQ.DRes.Procs = 1;

  /* configure range requirements */

  GRange[0].TaskCount = 1;
  GRange[0].NodeCount = 0;

  GRange[0].StartTime = 2800;
  GRange[0].EndTime   = MAX_MTIME;

  GRange[1].EndTime   = 0;

#endif /* __MNOT1 */

  MJobGetSNRange(
    &tmpJ,
    &tmpRQ, 
    &tmpN,
    GRange,
    MAX_MRANGE,
    &Affinity,
    NULL,
    ARange,
    NULL,
    NULL);

  exit(0);

  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* __MSysTestJobGetSNRange() */




int __MSysTestFeatureSub()

  {
  int NFMap[4] = { 58720258, 0, 0, 0 };
  int RFMap[4] = {301989888, 0, 0, 0};

  if (MAttrSubset(NFMap,RFMap,sizeof(NFMap),0) != SUCCESS)
    {
    /* test succeeded */

    exit(0);
    }

  exit(1);
 
  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* __MSysTestFeatureSub() */




int __MSysTestResParse(

  char *RString)  /* I */

  {
  mjob_t  tmpJ;
  mreq_t  tmpRQ;

  int     RMType = mrmtLSF;

  memset(&tmpJ,0,sizeof(tmpJ));
  memset(&tmpRQ,0,sizeof(tmpRQ));

  if (MReqRResFromString(&tmpJ,&tmpRQ,RString,RMType,FALSE) == SUCCESS)
    {
    /* test succeeded */

    exit(0);
    }

  exit(1);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END __MSysTestResParse() */




int __MSysTestSubmit()

  {
  char *SubmitString = "#PBS -l nodes=2,walltime=100\n/bin/sleep 60;hostname\n";
  
  char  JobName[MAX_MNAME];

  int   SC;
  char  Output[MAX_MLINE];

  mjob_t tmpJ;

  mjob_t *J = NULL;

  /* NOTE:  launch environment passed via job (user, group, iwd, env) */

  int rc;

  memset(&tmpJ,0,sizeof(tmpJ));

  J = &tmpJ;

  MUserAdd("heather",&tmpJ.Cred.U);
  MGroupAdd("heather",&tmpJ.Cred.G);
  MUStrDup(&J->E.IWD,"/tmp");

  if (X.XRMJobSubmit != (int (*)())0)
    {
    rc = (*X.XRMJobSubmit)(
      X.xd,
      SubmitString,
      &MRM[0],
      &J,
      JobName,
      Output,
      &SC);

    if (rc == SUCCESS)
      {
      fprintf(stdout,"NOTE:  job '%s' launched\n",
        JobName);

      exit(0);
      }
    else
      {
      fprintf(stdout,"NOTE:  submission failed '%s'\n",
        Output);

      exit(1);
      }
    }

  fprintf(stdout,"NOTE:  submission disabled\n");

  exit(1);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END __MSysTestSubmit() */





int MSysDoTest()
 
  {
  char *tptr;
  char *aptr;
  char *ptr;

  int   aindex;

  const char *TName[] = {
    NONE,
    "SCHED",
    "XML",
    "RANGEAND",
    "RANGECOLLAPSE",
    "RANGEMERGE",
    "GETSNRANGE",
    "FEATURESUB",
    "NODEPRIO",
    "RESPARSE",
    "SUBMIT",
    "WIKINODE",
    "WIKIJOB",
    "RMX",
    "JOBNAME",
    "JOBDIST",
    NULL };

  enum {
    mirtNONE = 0,
    mirtSched,
    mirtXML,
    mirtRLAND,
    mirtJobSelectFRL,
    mirtRLMerge,
    mirtJobGetSNRange,
    mirtFeatureSubset,
    mirtNodePrio,
    mirtResParse,
    mirtSubmit,
    mirtWikiNode,
    mirtWikiJob,
    mirtRMExtension,
    mirtJobName,
    mirtJobDist };
 
  if ((tptr = getenv(MSCHED_ENVTESTVAR)) == NULL)
    {
    return(SUCCESS);
    }

  aindex = MUGetIndex(tptr,TName,TRUE,0);

  aptr = NULL;

  if ((ptr = strchr(tptr,':')) != NULL)
    {
    aptr = ptr + 1;
    }

  switch(aindex)
    {
    case mirtSched:

      MSchedTest(); 

      break;

    case mirtXML:

      __MSysTestXML(aptr);

      break;

    case mirtRLAND:

      __MSysTestRLAND(); 

      break;

    case mirtJobSelectFRL:

      __MSysTestJobSelectFRL();

      break;

    case mirtRLMerge:
 
      __MSysTestRLMerge(); 

      break;

    case mirtJobGetSNRange:

      __MSysTestJobGetSNRange();

      break;

    case mirtFeatureSubset:

      __MSysTestFeatureSub();

      break;

    case mirtNodePrio:

      __MSysTestNPrioF();

      break;

    case mirtResParse:

      __MSysTestResParse(aptr);

      break;

    case mirtSubmit:

      __MSysTestSubmit();

      break;

    case mirtWikiNode:

      MWikiTestNode(aptr);

      break;

    case mirtWikiJob:

      MWikiTestJob(aptr);

      break;

    case mirtRMExtension:

      MJobTestRMExtension(aptr);

      break;

    case mirtJobName:

      MJobTestName(aptr);

      break;

    case mirtJobDist:

      MJobTestDist();

      break;
 
    default:
   
      /* cannot determine test */

      fprintf(stderr,"ERROR:    invalid test specified (%s)\n",
        tptr);

      exit(1);

      /*NOTREACHED*/

      break;
    }  /* END switch(aindex) */
 
  return(SUCCESS);
  }  /* END MSysDoTest() */




int MSysRegExtEvent(

  char *Message,  /* I */
  int   AType,    /* I:  action type */
  long  EFlags,   /* I:  event flags */
  int   Prio)     /* I:  event priority */

  {
  time_t T;

  char tmpLine[MAX_MLINE];

  const char *XHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\">";
  const char *ReqO    = "event-manager-requests";

  char CName[MAX_MNAME];

  char TString[MAX_MNAME];
  char DString[MAX_MNAME];

  const char *FName = "MSysRegExtEvent";

  DBG(2,fCORE) DPrint("%s(%s,%d,%ld,%d)\n",
    FName,
    (Message != NULL) ? Message : "NULL",
    AType,
    EFlags,
    Prio);

  time(&T);

  strncpy(TString,ctime(&T),24);
  TString[24] = '\0';

  strcpy(CName,MSCHED_SNAME);

  strcpy(DString,NONE);

  sprintf(tmpLine,"%s<%s><event component=\"%s\" time=\"%s msg=\"%s\" data=\"%s\"/></%s>",
    XHeader,
    ReqO,
    CName,
    TString,
    Message,
    DString,
    ReqO);

  /* send message */

  /* NYI */

#ifdef __MSSSLIB

  /* NYI */

#endif /* __MSSSLIB */

  return(SUCCESS);
  }  /* END MSysRegExtEvent() */




int MSysRegEvent(

  char *Message,  /* I */
  int   AType,    /* I:  action type */
  long  EFlags,   /* I:  event flags */
  int   Prio)     /* I:  event priority */

  {
  char *ASList[32];

  const char *FName = "MSysRegEvent";

  DBG(2,fCORE) DPrint("%s(%s,%d,%ld,%d)\n",
    FName,
    (Message != NULL) ? Message : "NULL",
    AType,
    EFlags,
    Prio);

  if (EFlags & (1 << mefExternal))
    {
    MSysRegExtEvent(Message,AType,EFlags,Prio);
    }

  if (Prio > 0)
    {
    ASList[0] = NULL;
    ASList[1] = Message;
    ASList[2] = NULL;

    MSysLaunchAction(
      ASList,
      (AType != 0) ? AType : mactAdminEvent);
    }

  return(SUCCESS);
  }  /* END MSysRegEvent() */




int MSysLaunchAction(

  char **ASList,
  int    AType)

  {
  static char  Exec[MAX_MLINE];
  static char  Line[MAX_MBUFFER];

  int  pid;
  int  rc;

  const char *FName = "MSysLaunchAction";

  DBG(2,fCORE) DPrint("%s(ASList,%d)\n",
    FName,
    AType);

  if (MSched.Action[AType][0] == '\0')
    {
    DBG(5,fCORE) DPrint("INFO:     scheduler action %d disabled\n",
      AType);

    return(SUCCESS);
    }

  if (MSched.Action[AType][0] == '/')
    {
    strcpy(Exec,MSched.Action[AType]);
    }
  else if (MSched.ToolsDir[strlen(MSched.ToolsDir) - 1] == '/')
    {
    sprintf(Exec,"%s%s",
      MSched.ToolsDir,
      MSched.Action[AType]);
    }
  else
    {
    sprintf(Exec,"%s/%s",
      MSched.ToolsDir,
      MSched.Action[AType]);
    }

  if (ASList[1] == NULL)
    {
    sprintf(Line,"\"%s %s\"",
      MLogGetTime(),
      "NODATA");

    DBG(7,fCORE) DPrint("INFO:     launching '%s' (AString: '%s')\n",
      Exec,
      Line);
    }
  else
    {
    DBG(7,fCORE) DPrint("INFO:     launching '%s' (AString: '%s', ...)\n",
      Exec,
      ASList[1]);
    }

  ASList[0] = Exec;

  /* fork process */

  if ((pid = fork()) == -1)
    {
    DBG(0,fCORE) DPrint("ALERT:    cannot fork for action '%s', errno: %d (%s)",
      MSched.Action[AType],
      errno,
      strerror(errno));

    return(FAILURE);
    }

  if (pid == 0)
    {
    /* if child */

    if (ASList[1] == NULL)
      {
      rc = execl(Exec,Exec,Line,NULL);
      }
    else
      {
      rc = execv(Exec,ASList);
      }

    if (rc == -1)
      {
      /* child has failed */

      DBG(1,fCORE) DPrint("ALERT:    cannot exec action '%s', rc: %d, errno: %d (%s)\n",
        Exec,
        rc,
        errno,
        strerror(errno));
      }

    exit(0);
    }  /* END if (pid == 0) */

  DBG(2,fCORE) DPrint("INFO:     action '%s' launched with message '%s'\n",
    MSched.Action[AType],
    ASList[1]);

  return(SUCCESS);
  }  /* END MSysLaunchAction() */




int MSysDSQuery(

  char *SName,     /* I  service name */
  char *CName,     /* I  cluster name */
  char *HostName,  /* O  service hostname (minsize=MMAX_NAME) */
  int  *Port,      /* O  service port */
  char *WProtocol, /* O  service wire protocol (minsize=MMAX_NAME) */
  char *SProtocol) /* O  service socket protocol (minsize=MMAX_NAME) */

  {
  char    *RspPtr = NULL;

  char     CmdString[MMAX_LINE];

  mxml_t *E  = NULL;
  mxml_t *LE = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *tE;

#ifndef __MPROD
  const char *FName = "MSysDSQuery";

  MDB(4,fCORE) MLog("%s(%s,%s,HostName,Port,WProtocol,SProtocol)\n",
    FName,
    (SName != NULL) ? SName : "NULL",
    (CName != NULL) ? CName : "NULL");
#endif /* !__MPROD */

  if (HostName != NULL)
    HostName[0] = '\0';

  if (Port != NULL)
    *Port = -1;

  if (WProtocol != NULL)
    WProtocol[0] = '\0';

  if (SProtocol != NULL)
    SProtocol[0] = '\0';

  if (SName == NULL)
    {
    return(FAILURE);
    }

  /* create request string */

  /* FORMAT:

  <get-location><location><component>$SNAME</component>
  <host match="false"></host><port match="false"></port>
  <protocol match="false"></protocol></location></get-location>

  */

  MXMLCreateE(&E,"add-location");
  MXMLCreateE(&LE,"location");
  MXMLAddE(E,LE);

  CE = NULL;
  MXMLCreateE(&tE,"component");
  MXMLSetVal(tE,(void *)SName,mdfString);
  MXMLAddE(LE,CE);

  tE = NULL;
  MXMLCreateE(&tE,"host");
  MXMLSetAttr(tE,"match",(void *)"false",mdfString);
  MXMLAddE(CE,tE);

  tE = NULL;
  MXMLCreateE(&tE,"port");
  MXMLSetAttr(tE,"match",(void *)"false",mdfString);
  MXMLAddE(CE,tE);

  tE = NULL;
  MXMLCreateE(&tE,"protocol");
  MXMLSetAttr(tE,"match",(void *)"false",mdfString);
  MXMLAddE(CE,tE);

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(&MSched.DS,CmdString,&RspPtr,NULL,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot query service '%s'\n",
      SName);

    return(FAILURE);
    }

  if (MXMLFromString(&E,RspPtr,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot parse DS query response '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  /* process LR3 response */

  if (MXMLGetChild(E,"error",NULL,&tE) == SUCCESS)
    {
    char EType[MMAX_LINE];
    char Msg[MMAX_LINE];

    MXMLGetAttr(tE,"type",NULL,EType,sizeof(EType));
    MXMLGetAttr(tE,"msg",NULL,Msg,sizeof(Msg));

    MDB(2,fCORE) MLog("ALERT:    cannot process DS query response '%s' (FailureType: %s  Msg: '%s'\n",
      RspPtr,
      EType,
      Msg);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(E,"locations",NULL,&RE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS query response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(RE,"location",NULL,&LE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS query response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  /* query succeeded, extract data */

  if (HostName != NULL)
    {
    if ((MXMLGetChild(LE,"host",NULL,&tE) == SUCCESS) && (tE->Val != NULL))
      {
      MUStrCpy(HostName,tE->Val,MMAX_NAME);
      }
    }

  if (Port != NULL)
    {
    if ((MXMLGetChild(LE,"port",NULL,&tE) == SUCCESS) && (tE->Val != NULL))
      {
      *Port = (int)strtol(tE->Val,NULL,10);
      }
    }

  if (SProtocol != NULL)
    {
    if ((MXMLGetChild(LE,"host",NULL,&tE) == SUCCESS) && (tE->Val != NULL))
      {
      /* NOTE:  directory service protocol must be translated between S3 and local protocols */

      /* NYI */

      /* MUStrCpy(SProtocol,tE->Val,MMAX_NAME); */
      }
    }

  /* NOTE:  wire protocol not supported */

  MXMLDestroyE(&E);

  MDB(2,fCORE) MLog("INFO:     information for service '%s' successfully queried\n",
    SName);

  return(SUCCESS);
  }  /* END MSysDSQuery() */




int MSysSynchronize()

  {
  return(SUCCESS);

  if (MSched.Sync.UpdateTime <= 0)
    {
    char  tmpBuf[MAX_MLINE];

    char  tmpLine[MAX_MLINE];
    char  tmpHost[MAX_MLINE];

    char *RspPtr = NULL;

    mpsi_t tmpP;

    memset(&tmpP,0,sizeof(tmpP));

    sprintf(tmpLine,"%s/%s",
      DEFAULT_MHSYNCLOCATION,
      MRMType[MRM[0].Type]);

    tmpP.Type = mpstWWW;

    tmpP.Data = (void *)tmpLine;

    tmpBuf[0] = '\0';

    strcpy(tmpHost,DEFAULT_MHSERVER);

    tmpP.HostName = tmpHost;
    tmpP.Port     = DEFAULT_MHPORT;

    tmpP.Timeout = 3;

    if (MS3DoCommand(&tmpP,tmpBuf,&RspPtr,NULL,NULL,NULL) == FAILURE)
      {
      DBG(2,fCORE) DPrint("ALERT:    cannot sync\n");

      return(FAILURE);
      }

    /* process updates */

    DBG(4,fCORE) DPrint("INFO:     received update '%s'\n",
      RspPtr);
    
    /* NYI */

    MUFree(&RspPtr);

    MSched.Sync.UpdateTime = MSched.Time;
    }  /* END if (MSched.Sync.UpdateTime <= 0) */

  return(SUCCESS);
  }  /* END MSysSynchronize() */




int MSysEMSubmit(

  mpsi_t *EM,      /* I */
  char   *SName,   /* I: service name */
  char   *EName,   /* I: event name */
  char   *Message) /* I: event message */

  {
  char     tmpLine[MAX_MLINE];
  char    *RspPtr = NULL;

  char     CmdString[MAX_MLINE];

  mxml_t *E = NULL;
  mxml_t *RE = NULL;
 
  const char *FName = "MSysEMSubmit";

  DBG(4,fCORE) DPrint("%s(EM,%s,%s,%s)\n",
    FName,
    (SName != NULL) ? SName : "NULL",
    (EName != NULL) ? EName : "NULL",
    (Message != NULL) ? Message : "NULL");

  if ((EM == NULL) || (SName == NULL) || (EName == NULL))
    {
    return(FAILURE);
    }

  if ((EM->HostName == NULL) || (EM->HostName[0] == '\0'))
    {
    /* EM disabled */

    DBG(6,fCORE) DPrint("INFO:     EM disabled\n");

    return(SUCCESS);
    }

  MXMLCreateE(&E,"event-manager-requests");
  MXMLCreateE(&RE,"event");

  MXMLSetAttr(RE,"component",(void *)SName,mdfString);

  MXMLSetAttr(RE,"msg",(void *)EName,mdfString);

  strcpy(tmpLine,ctime((time_t *)&MSched.Time));

  MXMLSetAttr(RE,"time",(void *)tmpLine,mdfString);

  if (Message != NULL)
    MXMLSetAttr(RE,"data",(void *)Message,mdfString);

  MXMLAddE(E,RE);

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(EM,CmdString,&RspPtr,NULL,NULL,NULL) == FAILURE)
    {
    DBG(2,fCORE) DPrint("ALERT:    cannot submit event '%s'\n",
      EName);

    return(FAILURE);
    }

  if (MXMLFromString(&E,RspPtr,NULL,NULL) == FAILURE)
    {
    DBG(2,fCORE) DPrint("ALERT:    cannot process EM response '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  if (MXMLGetChild(E,"event-ok",NULL,&RE) == FAILURE)
    {
    DBG(2,fCORE) DPrint("ALERT:    EM submission failed '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  MXMLDestroyE(&E);

  DBG(4,fCORE) DPrint("INFO:     event '%s' successfully submitted\n",
    EName);

  return(SUCCESS);
  }  /* END MSysEMSubmit() */




int MSysEMRegister(

  mpsi_t *EM,       /* I */
  char   *SName,    /* I: service name */
  char   *EName,    /* I: event name */
  char   *EData,    /* I: specific event data required (optional) */
  char   *Dst)      /* I: event destination */

  {
  char    *RspPtr = NULL;

  char     CmdString[MAX_MLINE];

  char     tmpLine[MAX_MLINE];

  mxml_t *E = NULL;
  mxml_t *RE = NULL;

  const char *FName = "MSysEMRegister";

  DBG(4,fCORE) DPrint("%s(EM,%s,%s,%s,%s)\n",
    FName,
    (SName != NULL) ? SName : "NULL",
    (EName != NULL) ? EName : "NULL",
    (EData != NULL) ? EData : "NULL",
    (Dst != NULL) ? Dst : "NULL");

  if ((EM == NULL) || (SName == NULL) || (EName == NULL))
    {
    return(FAILURE);
    }

  if ((EM->HostName == NULL) || (EM->HostName[0] == '\0'))
    {
    /* EM disabled */

    DBG(6,fCORE) DPrint("INFO:     EM disabled\n");

    return(SUCCESS);
    }

  MXMLCreateE(&E,"event-manager-requests");
  MXMLCreateE(&RE,"notification");

  MXMLSetAttr(RE,"component",(void *)SName,mdfString);

  MXMLSetAttr(RE,"msg",(void *)EName,mdfString);

  if (EData != NULL)
    MXMLSetAttr(RE,"data",(void *)EData,mdfString);
  else
    MXMLSetAttr(RE,"data",(void *)"*",mdfString);

  if (Dst != NULL)
    MXMLSetAttr(RE,"respond_to",(void *)Dst,mdfString);

  MXMLAddE(E,RE);

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(EM,CmdString,&RspPtr,NULL,NULL,NULL) == FAILURE)
    {
    DBG(2,fCORE) DPrint("ALERT:    cannot register for event '%s'\n",
      EName);

    return(FAILURE);
    }

  if (MXMLFromString(&E,RspPtr,NULL,NULL) == FAILURE)
    {
    DBG(2,fCORE) DPrint("ALERT:    cannot process EM response '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  if ((MXMLGetAttr(E,"outcome",NULL,tmpLine,0) == FAILURE) ||
      strcmp(tmpLine,"success"))
    {
    DBG(2,fCORE) DPrint("ALERT:    EM registration failed '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  MXMLDestroyE(&E);

  DBG(4,fCORE) DPrint("INFO:     event '%s' subscription successfully registered\n",
    EName);

  return(SUCCESS);
  }  /* END MSysEMRegister() */




int MSysDSUnregister(

  char *SName,      /* I: service name */
  char *CName,      /* I: cluster name */
  char *HostName,   /* I */
  int   Port,       /* I */
  char *WProtocol,  /* I */
  char *SProtocol)  /* I */

  {
  char    *RspPtr = NULL;

  char     CmdString[MMAX_LINE];

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *tE = NULL;

#ifndef __MPROD
  const char *FName = "MSysDSUnregister";

  MDB(4,fCORE) MLog("%s(%s,%s,%s,Port,WProtocol,SProtocol)\n",
    FName,
    (SName != NULL) ? SName : "NULL",
    (CName != NULL) ? CName : "NULL",
    (HostName != NULL) ? HostName : "NULL");
#endif /* !__MPROD */

  if (SName == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.DS.HostName == NULL) || (MSched.DS.HostName[0] == '\0'))
    {
    /* DS disabled */

    MDB(6,fCORE) MLog("INFO:     DS disabled\n");

    return(SUCCESS);
    }

  /* create request string */

  /* FORMAT */

  /* <del-location><location><component>$SNAME</component></location></del-location> */

  MXMLCreateE(&E,"del-location");
  MXMLCreateE(&RE,"location");
  MXMLAddE(E,RE);

  MXMLCreateE(&tE,"component");
  MXMLAddE(RE,tE);

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(&MSched.DS,CmdString,&RspPtr,NULL,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot un-register service '%s'\n",
      CName);

    return(FAILURE);
    }

  if (MXMLFromString(&E,RspPtr,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS response '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  /* NOTE:  processing LR3 response */

  if (MXMLGetChild(E,"error",NULL,&RE) == SUCCESS)
    {
    char EType[MMAX_LINE];
    char Msg[MMAX_LINE];

    MXMLGetAttr(RE,"type",NULL,EType,sizeof(EType));
    MXMLGetAttr(RE,"msg",NULL,Msg,sizeof(Msg));

    MDB(2,fCORE) MLog("ALERT:    cannot process DS response '%s' (FailureType: %s  Msg: '%s'\n",
      RspPtr,
      EType,
      Msg);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(E,"locations",NULL,&RE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS unregistration response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(RE,"location",NULL,&tE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS unregistration response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  /* NOTE:  must extract and record deregistration failure message (NYI) */

  MXMLDestroyE(&E);

  MDB(2,fCORE) MLog("INFO:     service '%s' successfully registered\n",
    SName);

  return(SUCCESS);
  }  /* END MSysDSUnregister() */





int MSysDSRegister(

  char *SName,      /* I: service name */
  char *CName,      /* I: cluster name */
  char *HostName,   /* I */
  int   Port,       /* I */
  char *WProtocol,  /* I */
  char *SProtocol)  /* I */

  {
  char    *RspPtr = NULL;

  char     CmdString[MMAX_LINE];

  mxml_t  *E = NULL;
  mxml_t  *RE = NULL;
  mxml_t  *tE;

  char    *Version = NULL;

#ifndef __MPROD
  const char *FName = "MSysDSRegister";

  MDB(4,fCORE) MLog("%s(%s,%s,%s,Port,WProtocol,SProtocol)\n",
    FName,
    (SName != NULL) ? SName : "NULL",
    (CName != NULL) ? CName : "NULL",
    (HostName != NULL) ? HostName : "NULL");
#endif /* !__MPROD */

  if (SName == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.DS.HostName == NULL) || (MSched.DS.HostName[0] == '\0'))
    {
    /* DS disabled */

    MDB(6,fCORE) MLog("INFO:     DS disabled\n");

    return(SUCCESS);
    }

  /* FORMAT:
<?xml version="1.0" encoding="UTF-8"?><add-location><location>
<component>$SName</component>
<host>$HostName</host>
<port>$Port</port>
<protocol>challenge</protocol>
<schema_version>1234</schema_version>
<tier>1</tier>
</location></add-location>
  */

  /* create request string */

  MXMLCreateE(&E,"add-location");
  MXMLCreateE(&RE,"location");

  tE = NULL;
  MXMLCreateE(&tE,"component");
  MXMLSetVal(tE,(void *)SName,mdfString);
  MXMLAddE(RE,tE);

  if (CName != NULL)
    {
    /* MXMLSetAttr(RE,"cluster",(void *)CName,mdfString); */
    }

  if (HostName != NULL)
    {
    tE = NULL;
    MXMLCreateE(&tE,"host");
    MXMLSetVal(tE,(void *)HostName,mdfString);
    MXMLAddE(RE,tE);
    }

  if (Port != -1)
    {
    tE = NULL;
    MXMLCreateE(&tE,"port");
    MXMLSetVal(tE,(void *)&Port,mdfInt);
    MXMLAddE(RE,tE);
    }

  if (SProtocol != NULL)
    {
    tE = NULL;
    MXMLCreateE(&tE,"protocol");
    MXMLSetVal(tE,(void *)SProtocol,mdfString);
    MXMLAddE(RE,tE);
    }

  if (Version != NULL)
    {
    tE = NULL;
    MXMLCreateE(&tE,"schema_version");
    MXMLSetVal(tE,(void *)Version,mdfString);
    MXMLAddE(RE,tE);
    }

  MXMLAddE(E,RE);

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(&MSched.DS,CmdString,&RspPtr,NULL,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot register service '%s'\n",
      CName);

    return(FAILURE);
    }

  if (MXMLFromString(&E,RspPtr,NULL,NULL) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS registration response '%s'\n",
      RspPtr);

    return(FAILURE);
    }

  /* NOTE:  processing LR3 response */

  if (MXMLGetChild(E,"error",NULL,&RE) == SUCCESS)
    {
    char EType[MMAX_LINE];
    char Msg[MMAX_LINE];

    MXMLGetAttr(RE,"type",NULL,EType,sizeof(EType));
    MXMLGetAttr(RE,"msg",NULL,Msg,sizeof(Msg));

    MDB(2,fCORE) MLog("ALERT:    cannot process DS registration response '%s' (FailureType: %s  Msg: '%s'\n",
      RspPtr,
      EType,
      Msg);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(E,"locations",NULL,&RE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS registration response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetChild(RE,"location",NULL,&tE) == FAILURE)
    {
    MDB(2,fCORE) MLog("ALERT:    cannot process DS registration response '%s'\n",
      RspPtr);

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  /* NOTE:  do not know how failure messages are encapsulated */

  /* must extract and record actual failure (NYI) */

  MXMLDestroyE(&E);

  MDB(2,fCORE) MLog("INFO:     service '%s' successfully registered\n",
    SName);

  return(SUCCESS);
  }  /* END MSysDSRegister() */





int MSysUpdateTime(

  msched_t *S)  /* I */

  {
  const char *FName = "MSysUpdateTime";

  DBG(3,fALL) DPrint("%s()\n",
    FName);

  /* update time, day, iteration, interval, and runtime */

  /* update time */

  if ((MSched.Mode != msmSim) || (MSched.Iteration != 0))
    {
    MUGetTime(&MSched.Time,mtmRefresh,S);
    }

  /* update day */

  {
  time_t tmpT;

  char   tmpDay[MAX_MNAME];

  tmpT = (time_t)MSched.Time;

  MUStrCpy(tmpDay,ctime(&tmpT),4);

  /* FORMAT:  DDD ... */

  if (strcmp(MSched.Day,tmpDay) != 0)
    {
    /* starting new day */

    DBG(2,fALL) DPrint("INFO:     starting new day: %s",
      MULToDString(&MSched.Time));

    strcpy(MSched.Day,tmpDay);
    }    /* END if (strcmp(MSched.Day,tmpDay) != 0) */
  }      /* END BLOCK */

  /* get exact time (update SchedTime, Interval) */

  {
  struct timeval  tvp;

  long            interval;

  gettimeofday(&tvp,NULL);

  /* determine time interval in 1/100's of a second */

  if ((MSched.Mode == msmSim) && (MSched.TimePolicy != mtpReal))
    {
    interval = MSched.RMPollInterval * 100;
    }
  else
    {
    interval = (tvp.tv_sec  - MSched.SchedTime.tv_sec) * 100 +
               (tvp.tv_usec - MSched.SchedTime.tv_usec) / 10000;
    }

  if (interval < 0)
    {
    DBG(1,fSCHED) fprintf(stderr,"ALERT:    negative interval detected (%ld)\n",
      interval);

    MSched.Interval = 0;
    }
  else if (MSched.SchedTime.tv_sec == 0)
    {
    /* first pass, time not yet initialized */

    MSched.Interval = 0;
    }
  else
    {
    MSched.Interval = interval;

    MStat.SchedRunTime += MSched.Interval;
    }

  memcpy(&MSched.SchedTime,&tvp,sizeof(struct timeval));
  }  /* END BLOCK */

  return(SUCCESS);
  }  /* END MSysUpdateTime() */




int MSysCheck()

  {
  int      jindex;
  int      nindex;

  mjob_t  *J;
  mnode_t *N;

  mrm_t   *RM;

  char Message[MAX_MLINE];

  const char *FName = "MSysCheck";

  DBG(4,fCORE) DPrint("%s()\n",
    FName);

  MLimitEnforceAll(&MPar[0]);

  /* located jobs which have violated wallclock limits */

  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];

    RM = (J->RM != NULL) ? J->RM : &MRM[0];

    /* locate jobs which have allocated 'down' nodes */

    if ((MSched.Time - J->StartTime) > MPar[0].MaxJobStartTime)
      {
      if (RM->Type == mrmtPBS)
        {
        if ((J->State == mjsIdle) &&
           ((J->EState == mjsStarting) || (J->EState == mjsRunning)))
          {
          DBG(2,fCORE) DPrint("ALERT:    PBS job '%s' in state '%s' was started %s ago.  assuming prolog hang and cancelling job\n",
            J->Name,
            MJobState[J->State],
            MULToTString(MSched.Time - J->StartTime));

          MRMJobCancel(J,"MAUI_INFO:  job cannot start\n",NULL);
          }
        }

      for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
        {
        if (J->NodeList[nindex].N == NULL)
          break;

        N = J->NodeList[nindex].N;

        if (((N->State == mnsIdle) ||
             (N->State == mnsDown)) &&
            ((MSched.Time - N->StateMTime) > 300))
          {
          DBG(1,fCORE) DPrint("ALERT:    job '%s' has been in state '%s' for %ld seconds.  node '%s' is in state '%s'  (job '%s' will be cancelled)\n",
            J->Name,
            MJobState[J->State],
            MSched.Time - J->StartTime,
            N->Name,
            MNodeState[N->State],
            J->Name);

          sprintf(Message,"JOBCORRUPTION:  job '%s' (user %s) has been in state '%s' for %ld seconds.  node '%s' is in state '%s'  (job '%s' will be cancelled)\n",
            J->Name,
            J->Cred.U->Name,
            MJobState[J->State],
            MSched.Time - J->StartTime,
            N->Name,
            MNodeState[N->State],
            J->Name);

          MSysRegEvent(Message,0,0,1);

          if (N->State == mnsDown)
            {
            MRMJobCancel(J,"MAUI_ERROR:  job has 'DOWN' node allocated\n",NULL);

            break;
            }
          }  /* END if N->State    */
        }    /* END for nindex     */
      }      /* END if MSched.Time */
    }        /* END for (jindex)   */

  /* clear all defunct child processes */

  MUClearChild(NULL);

  return(SUCCESS);
  }  /* END MSysCheck() */




int MSysDestroyObjects()

  {
  int jindex;
  int nindex;

  mnode_t *N;

  const char *FName = "MSysDestroyObjects";

  DBG(1,fSTRUCT) DPrint("%s()\n",
    FName);

  for (jindex = 1;jindex < MAX_MJOB;jindex++)
    {
    if ((MJob[jindex] == NULL) || (MJob[jindex] == (mjob_t *)1))
      continue;

    MJobRemove(MJob[jindex]);
    }  /* END for (jindex) */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if (N == NULL)
      break;

    MNodeRemove(N);
    }  /* END for (nindex) */

  return(SUCCESS);
  }  /* END MSysDestroyObjects() */





void MSysShutdown(

  int Signo)  /* I */

  {
  const char *FName = "MSysShutdown";

  DBG(2,fALL) DPrint("%s(%d)\n",
    FName,
    Signo);

  DBG(0,fALL) DPrint("INFO:     received signal %d.  shutting down server\n",
    Signo);

  if (MSysDSUnregister(
       (char *)MS3CName[mpstSC],
       MRM[0].Name,
       MSched.ServerHost,
       MSched.ServerPort,
       NULL,
       NULL) == FAILURE)
    {
    DBG(1,fRM) DPrint("ALERT:    cannot unregister with directory service\n");
    }

  MAMShutdown(&MAM[0]);

  MFSShutdown(&MPar[0].FSC);

  if ((MSched.Mode != msmSim) ||
      (getenv(MSCHED_ENVCKTESTVAR) != NULL))
    {
    MCPCreate(MCP.CPFileName);
    }

  MSUDisconnect(&MSched.ServerS);

  MStatShutdown();

  MQOSFreeTable();
  MJobFreeTable();
  MNodeFreeTable();
  MUserFreeTable();
  MResFreeTable();

  MLogShutdown();

  exit(0);
  }  /* END MSysShutdown() */




int MSchedLoadConfig(

  char *Buf)  /* I (optional) */

  {
  char   IndexName[MAX_MNAME];

  char   Value[MAX_MLINE];

  char  *ptr;
  char  *head;

  /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
  /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */

  /* load all/specified AM config info */

  head = (Buf != NULL) ? Buf : MSched.ConfigBuffer;

  if (head == NULL)
    {
    return(FAILURE);
    }

  /* load all sched config info */

  ptr = head;

  IndexName[0] = '\0';

  while (MCfgGetSVal(
           head,
           &ptr,
           MCredCfgParm[mxoSched],
           IndexName,
           NULL,
           Value,
           sizeof(Value),
           0,
           NULL) != FAILURE)
      {
      if (IndexName[0] != '\0')
        {
        /* set scheduler name */

        MSchedSetAttr(&MSched,msaName,(void *)IndexName,mdfString,mSet);
        }

      /* load sys specific attributes */

      MSchedProcessConfig(&MSched,Value);

      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */
 
  return(SUCCESS);
  }  /* END MSchedLoadConfig() */




int MSchedProcessConfig(

  msched_t *S,     /* I (modified) */
  char     *Value) /* I */

  {
  int   aindex;

  char *ptr;
  char *TokPtr;

  char  ValLine[MAX_MLINE];

  if ((S == NULL) ||
      (Value == NULL) ||
      (Value[0] == '\0'))
    {
    return(FAILURE);
    }

  /* process value line */

  ptr = MUStrTok(Value," \t\n",&TokPtr);

  while(ptr != NULL)
    {
    /* parse name-value pairs */

    /* FOAMAT:  <VALUE>[,<VALUE>] */

    if (MUGetPair(
          ptr,
          (const char **)MSchedAttr,
          &aindex,
          NULL,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */

      ptr = MUStrTok(NULL," \t\n",&TokPtr);

      continue;
      }

    switch(aindex)
      {
      case msaFBServer:

        MUURLParse(ValLine,NULL,S->FBServerHost,NULL,0,&S->FBServerPort,TRUE);

        break;

      case msaServer:

        MUURLParse(ValLine,NULL,S->ServerHost,NULL,0,&S->ServerPort,TRUE);

        break;

      case msaMode:

        S->Mode = MUGetIndex(ValLine,MSchedMode,FALSE,S->Mode);
        S->SpecMode = S->Mode;

        break;

      default:

        DBG(4,fAM) DPrint("WARNING:  sys attribute '%s' not handled\n",
          MSchedAttr[aindex]);

        break;
      }  /* END switch(aindex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MSchedProcessConfig() */




int MSysToPrimary()

  {
  const char *FName = "MSysToPrimary";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* load checkpointed state */

  /* update service directory */

  /* change mode */

  MSched.Mode = MSched.SpecMode;

  /* enable user interface */

  /* NYI */

  return(SUCCESS);
  }  /* END MSysToPrimary() */





int MSysToSecondary()

  {
  const char *FName = "MSysToSecondary";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* disable user interface */

  if (MSched.ServerS.sd > 0)
    {
    MSUDisconnect(&MSched.ServerS);
    }

  /* change mode */

  MSched.Mode = msmTest;

  return(SUCCESS);
  }  /* END MSysToSecondary() */




int MSysCheckPrimary()

  {
  static int FBFailureCount = 0;

  char tmpBuf[MAX_MBUFFER];

  static char *CmdString = "0 [NONE] 0";

  int rc;

  const char *FName = "MSysCheckPrimary";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* contact primary */

  rc = MCDoCommand(
    MSched.FBServerHost,
    MSched.FBServerPort,
    svcShowQ,
    CmdString,
    tmpBuf);

  DBG(2,fALL) DPrint("INFO:     connection to FBServer %s:%d %s\n",
    MSched.FBServerHost,
    MSched.FBServerPort,
    (rc == SUCCESS) ? "succeeded" : "failed");

  if ((MSched.Mode == msmTest) && (MSched.FBActive == TRUE))
    {
    if (rc == FAILURE)
      {
      FBFailureCount++;

      if (FBFailureCount >= MSched.FBFailureCount)
        {
        /* if multiple failed connection attempts, become primary */

        MSysToPrimary();
        }
      }
    else
      {
      FBFailureCount = 0;
      }

    sleep(MSched.FBPollInterval);
    }
  else
    {
    if (rc == FAILURE)
      {
      /* if single successful connection, become secondary */

      MSysToSecondary();
      }
    }

  return(SUCCESS);
  }  /* END MSysCheckPrimary() */




int MSysDiagnose(

  char *SBuf,     /* O */
  int   SBufSize, /* I */
  long  Flags)    /* I */

  {
  if (SBuf == NULL)
    {
    return(FAILURE);
    }

  SBuf[0] = '\0';

  sprintf(temp_str,"Initialized: S:%s/I:%s  CCount: %d  FCount: %d  QCount: %d  JCount: %d  RCount: %d\n",
    (MSched.G.SIsInitialized == TRUE) ? "TRUE" : "FALSE",
    (MSched.G.IIsInitialized == TRUE) ? "TRUE" : "FALSE",
    MSched.G.CCount,
    MSched.G.FailureCount,
    MSched.G.QCount,
    MSched.G.JCount,
    MSched.G.RCount);
  strcat(SBuf,temp_str);

  if (MSim.StopIteration == MSched.Iteration)
    {
    sprintf(temp_str,"\nNOTE:  scheduler is currently stopped\n");
    strcat(SBuf,temp_str);
    }

  if (MSched.G.Messages != NULL)
    {
    sprintf(temp_str,"\nMessages:\n  %s\n",
      MSched.G.Messages);
    strcat(SBuf,temp_str);
    }

  return(SUCCESS);
  }  /* END MSysDiagnose() */




int MSysStartServer(

  int IsFBServer)  /* I (boolean) */

  {
  const char *FName = "MSysStartServer";

  DBG(3,fALL) DPrint("%s()\n",
    FName);

  DBG(0,fALL) DPrint("starting %s version %s (PID: %d) on %s",
    MSCHED_NAME,
    MSCHED_VERSION,
    MSched.PID,
    MULToDString(&MSched.Time));

  MSysMemCheck();

  MStatInitialize(&MStat.P);

  if (IsFBServer == TRUE)
    {
    MSysToSecondary();

    MSysCheckPrimary();
    }

  /* set up user interface socket */

  MSUInitialize(
    &MSched.ServerS,
    NULL,
    MSched.ServerPort,
    MSched.ClientTimeout,
    (1 << TCP));

  if (MSUListen(&MSched.ServerS) == FAILURE)
    {
    DBG(0,fALL) DPrint("ERROR:    cannot open user interface socket on port %d\n",
      MSched.ServerPort);

    fprintf(stderr,"ERROR:    cannot open user interface socket on port %d\n",
      MSched.ServerPort);

    exit(1);
    }

  /* enable extension interface */

  MSUInitialize(
    &MSched.ServerSH,
    NULL,
    MSched.ServerPort + 1,
    MSched.ClientTimeout,
    (1 << TCP));

  if (MSUListen(&MSched.ServerSH) == FAILURE)
    {
    DBG(7,fALL) DPrint("ERROR:    cannot open extension interface socket on port %d\n",
      MSched.ServerPort);
    }

  MFSInitialize(&MPar[0].FSC);

  if (MAM[0].Type != mamtNONE)
    MAMActivate(&MAM[0]);

  MCPLoad(MCP.CPFileName,mckptResOnly);

  if (MSched.Mode == msmSim)
    {
    /* initialize simulation */

    MSimInitialize();
    }  /* END if (MSched.Mode == msmSim) */
  else
    {
    /* initialize resource manager */

    MRMInitialize();

    /* initialize allocation manager */

    MAMInitialize(NULL);
    }  /* END else (MSched.Mode == msmSim) */

  return(SUCCESS);
  }  /* END MSysStartServer() */


/* END MSys.c */



