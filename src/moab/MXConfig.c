/* HEADER */

  /* base scheduler functions */

  X->JobSetCreds          = MJobSetCreds;
  X->JobAllocateResources = MJobAllocMNL;
  X->JobGetStartPriority  = MJobGetStartPriority;      
  X->JobCheckPolicies     = MJobCheckPolicies;
  X->JobDistributeTasks   = MJobDistributeTasks;
  X->JobFind              = MJobFind;      
  X->JobGetTasks          = MJobSelectMNL;          
  X->JobGetFeasibleTasks  = MReqGetFNL;          
  X->JobStart             = MJobStart;        
  X->JobBuildCL           = MJobBuildCL;
  X->JobBuildACL          = NULL;
  X->JobSetQOS            = MJobSetQOS;
  X->JobNameAdjust        = MJobGetName;
  X->JobValidate          = MJobValidate;
  X->JobDetermineCreds    = MJobDetermineCreds;

  X->BackFill             = MQueueBackFill;                        
  X->DoWikiCommand        = MWikiDoCommand;

  X->ResFind              = MResFind;
  X->JobGetRange          = MJobGetRange;         
  X->ReservationDestroy   = MResDestroy;     
  X->ReservationCreate    = MResCreate;
  X->ReservationJCreate   = MResJCreate;

  X->AcctFind             = MAcctFind;

  X->RMCancelJob          = MRMJobCancel;      
  X->RMJobStart           = MRMJobStart;       

  X->PBSInitialize        = NULL;

  X->JobGetSNRange        = MJobGetSNRange;
  X->JobSetAttr           = MJobSetAttr;
  X->ResSetAttr           = MResSetAttr;

  X->QOSGetAccess         = MQOSGetAccess;

  X->QueuePrioritizeJobs  = MQueuePrioritizeJobs;     
  X->QueueScheduleJobs    = MQueueScheduleIJobs;      

  X->SimJobSubmit         = MSimJobSubmit;
  X->SRCreate             = MSRSetRes;
  X->WikiLoadJob          = MWikiJobLoad;

  X->QBankDoTransaction   = MAMQBDoCommand;

  /* extension functions */

  X->XUIHandler                 = XUIHandler;

  X->XJobProcessWikiAttr        = XJobProcessWikiAttr;

  X->XJobDestroy                = XJobDestroy;

  X->XPBSInitialize             = XPBSInitialize;
  X->XPBSNMGetData              = XPBSNMGetData;
  X->XRMInitialize              = XRMInitialize;
  X->XRMResetState              = XRMResetState;
  X->XRMVerifyData              = XRMVerifyData;
  X->XRMJobResume               = XRMJobResume;
  X->XRMJobSuspend              = XRMJobSuspend;
  X->XUpdateState               = XUpdateState;
  X->XMetaStoreCompletedJobInfo = NULL;
  X->XAllocMachinePrio          = XAllocMachinePrio;
  X->XAllocLoadBased            = XAllocLoadBased;

  X->XGetClientInfo             = NULL;

  X->XJobAllocateResources      = NULL;
 
  /* base scheduler data */

  X->Acct         = MAcct;
  X->AttrList     = &MAList;
  X->Sched        = &MSched;
  X->Stat         = &MStat;
  X->CREndTime    = &CREndTime;
  X->CurrentHostName = CurrentHostName;
  X->dlog         = &mlog;
  X->Group        = MGroup;
  X->Job          = MJob;
  X->MNode        = MNode;
  X->MRange       = MRange;  
  X->MPar         = MPar;
  X->PresentTime  = &MSched.Time; 
  X->MQOS         = MQOS;
  X->Res          = MRes;
  X->RM           = MRM;
  X->AM           = MAM;
  X->User         = MUser;

  /* extension data */

  X->xd           = NULL;

/* END MXConfig.c */

