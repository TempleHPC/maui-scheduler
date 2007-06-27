/* HEADER */
        
/* Contains:                                  *
 *   int MJobGetStartPriority(J,PIndex,Priority,Buffer) *
 *   int __MJobStartPrioComp(a,b)             *
 *                                            */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t    mlog;

extern mjob_t   *MJob[];
extern mstat_t   MStat;
extern mqos_t    MQOS[];
extern mpar_t    MPar[];
extern mattrlist_t MAList;
extern msched_t  MSched;

extern mx_t      X;

#ifndef ABS
#define ABS(a) ((a >= 0) ? (a) : 0-(a))
#endif /* ABS */




int MJobGetStartPriority(

  mjob_t *J,        /* I */
  int     PIndex,   /* I */
  double *Priority, /* O */
  int     Mode,     /* I */
  char  **BPtr,     /* O (optional,minsize=MMAX_BUFFER) */
  int    *BSpace)   /* O (optional) */

  {
  double        Prio;
  double        APrio;

  double        XFactor;

  double        FSTargetUsage;
  int           FSMode;

  char          CHeader[MAX_MPRIOCOMPONENT][MAX_MNAME];    
  char          CWLine[MAX_MPRIOCOMPONENT][MAX_MNAME];    
  char          CFooter[MAX_MPRIOCOMPONENT][MAX_MNAME];  
  char          CLine[MAX_MPRIOCOMPONENT][MAX_MNAME];

  char          tmpHeader[MAX_MNAME];
  char          tmpCWLine[MAX_MNAME];
  char          tmpLine[MAX_MNAME];

  double        CFactor[MAX_MPRIOCOMPONENT];
  double        SFactor[MAX_MPRIOSUBCOMPONENT];           

  long          CWeight[MAX_MPRIOCOMPONENT];
  long          SWeight[MAX_MPRIOSUBCOMPONENT];

  static double TotalCFactor[MAX_MPRIOCOMPONENT];
  static double TotalSFactor[MAX_MPRIOSUBCOMPONENT];        

  double        CP[MAX_MPRIOCOMPONENT];
  double        SP[MAX_MPRIOSUBCOMPONENT];

  static double TotalPriority;

  static int    SDisplay[MAX_MPRIOSUBCOMPONENT];

  mfsc_t       *F;
  mfsc_t       *GF;

  mpar_t       *GP;

  mreq_t       *RQ;

  int           rqindex;
  int           qindex;
  int           index;

  int           cindex;
  int           sindex;
  int           sindex1;
  int           sindex2;

  long          CCap[MAX_MPRIOCOMPONENT];
  long          SCap[MAX_MPRIOSUBCOMPONENT];

  long          EffQTime;

  long          tmpL;
  double        tmpD;
  char          tmpS[MAX_MNAME];

  unsigned long MinWCLimit;

  const char *FName = "MJobGetStartPriority";

  DBG(6,fSCHED) DPrint("%s(%s,%d,%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "[NONE]",
    PIndex,
    (Priority != NULL) ? "Priority" : "NULL",
    ((BPtr != NULL) && (*BPtr != NULL)) ? "Buffer" : "NULL");

  /* NOTE:  NULL 'J' allowed */

  /* NOTE: routine used to diagnose priority */

  F  = &MPar[PIndex % MAX_MPAR].FSC;
  GF = &MPar[0].FSC;

  GP = &MPar[0];

  if (X.XJobGetStartPriority != (int (*)())0)
    {
    return (*X.XJobGetStartPriority)(
             X.xd,
             J,
             Priority,
             (BPtr != NULL) ? *BPtr : NULL);
    }

  for (index = 1;index < MAX_MPRIOCOMPONENT;index++)
    {
    CWeight[index] = (F->PCW[index] != -1) ? F->PCW[index] : GF->PCW[index]; 
    CCap[index]    = (F->PCC[index] > 0) ? F->PCC[index] : GF->PCC[index];      
    CFactor[index] = 0.0;
    CP[index]      = 0.0;
    }  /* END for (index) */
 
  for (index = 0;index < MAX_MPRIOSUBCOMPONENT;index++)
    {
    SWeight[index] = (F->PSW[index] != -1) ? F->PSW[index] : GF->PSW[index];     
    SCap[index]    = (F->PSC[index] > 0) ? F->PSC[index] : GF->PSC[index];      
    SFactor[index] = 0.0;
    SP[index]      = 0.0;
    }  /* END for (index) */

  if ((J != NULL) && (J->Cred.Q != NULL))
    {
    SWeight[mpsSQT] += J->Cred.Q->QTWeight;
    SWeight[mpsSXF] += J->Cred.Q->XFWeight;

    MinWCLimit = (F->XFMinWCLimit != -1) ? F->XFMinWCLimit : GF->XFMinWCLimit;
    }
  else
    {
    MinWCLimit = 0;
    }

  if ((J != NULL) && (J->Cred.G != NULL))
    {
    if (J->Cred.G->ClassWeight > 0)
      SWeight[mpsCC] = J->Cred.G->ClassWeight;
    }

  if (Mode == 1)
    {
    /* initialize summary values/create header */

    memset(TotalCFactor,0,sizeof(TotalCFactor));
    memset(TotalSFactor,0,sizeof(TotalSFactor));      
    memset(SDisplay,FALSE,sizeof(SDisplay));        

    memset(CHeader,0,sizeof(CHeader));      
    memset(CWLine,0,sizeof(CWLine));  

    TotalPriority = 0.0;

    if (CWeight[mpcCred] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';     

      for (index = mpsCU;index <= mpsCC;index++)
        {
        if (SWeight[index] == 0)
          continue;

        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }

        switch(index)
          {
          case mpsCU: strcat(tmpHeader," User"); break;
          case mpsCG: strcat(tmpHeader,"Group"); break;        
          case mpsCA: strcat(tmpHeader,"Accnt"); break;        
          case mpsCC: strcat(tmpHeader,"Class"); break;        
          case mpsCQ: strcat(tmpHeader,"  QOS"); break;        
          }

        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          (long)SWeight[index]);
        }  /* END for (index) */

      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcCred],"  Cred(%s)",
          tmpHeader);

        sprintf(CWLine[mpcCred]," %5ld(%s)",
          CWeight[mpcCred],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcCred] != 0) */

    if (CWeight[mpcFS] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';
 
      for (index = mpsFU;index <= mpsFC;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }
 
        switch(index)
          {
          case mpsFU: strcat(tmpHeader," User"); break;
          case mpsFG: strcat(tmpHeader,"Group"); break;
          case mpsFA: strcat(tmpHeader,"Accnt"); break;
          case mpsFC: strcat(tmpHeader,"Class"); break;
          case mpsFQ: strcat(tmpHeader,"  QOS"); break;
          }  /* END switch(index) */
 
        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          SWeight[index]);
        }  /* END for (index) */
 
      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcFS],"    FS(%s)",
          tmpHeader);
 
        sprintf(CWLine[mpcFS]," %5ld(%s)",
          CWeight[mpcFS],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcFS] != 0) */

    if (CWeight[mpcAttr] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';

      for (index = mpsAAttr;index <= mpsAState;index++)
        {
        /* handle job attr based priority weights */

        switch(index)
          {
          case mpsAAttr:

            /* NYI */

            break;

          case mpsAState:

            /* NYI */

            break;

          default:

            /* NO-OP */

            break;
          }  /* END switch(index) */
        }    /* END for (index) */
      }      /* END if (CWeight[mpcAttr] != 0) */

    if (CWeight[mpcServ] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';
 
      for (index = mpsSQT;index <= mpsSBP;index++)
        {
        /* handle QOS based service priority weights */

        switch(index)
          {
          case mpsSQT:

            if (SWeight[mpsSQT] != 0)
              {
              SDisplay[mpsSQT] = TRUE;
              }
            else
              {
              for (qindex = 0;qindex < MAX_MQOS;qindex++)
                {
                if (MQOS[qindex].QTWeight > 0)
                  {
                  SDisplay[mpsSQT] = TRUE;     
 
                  break;
                  }
                }
              }

            break;

          case mpsSXF:
 
            if (SWeight[mpsSXF] != 0)
              {
              SDisplay[mpsSXF] = TRUE;
              }
            else
              {
              for (qindex = 0;qindex < MAX_MQOS;qindex++)
                {
                if (MQOS[qindex].XFWeight > 0)
                  {
                  SDisplay[mpsSXF] = TRUE;
 
                  break;
                  }
                }
              }
 
            break;
          }  /* END switch(index) */

        if ((SWeight[index] == 0) && (SDisplay[index] == FALSE))
          continue;
 
        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }
 
        switch(index)
          {
          case mpsSQT:  strcat(tmpHeader,"QTime"); break;
          case mpsSXF:  strcat(tmpHeader,"XFctr"); break;
          case mpsSSPV: strcat(tmpHeader,"SPVio"); break;
          case mpsSBP:  strcat(tmpHeader,"Bypas"); break;
          }
 
        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          SWeight[index]);
        }  /* END for (index) */
 
      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcServ],"  Serv(%s)",
          tmpHeader);
 
        sprintf(CWLine[mpcServ]," %5ld(%s)",
          CWeight[mpcServ],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcServ] != 0) */

    if (CWeight[mpcTarg] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';
 
      for (index = mpsTQT;index <= mpsTXF;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }
 
        switch(index)
          {
          case mpsTQT: strcat(tmpHeader,"QTime"); break;
          case mpsTXF: strcat(tmpHeader,"XFctr"); break;
          }
 
        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          SWeight[index]);
        }  /* END for (index) */
 
      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcTarg],"  Targ(%s)",
          tmpHeader);
 
        sprintf(CWLine[mpcTarg]," %5ld(%s)",
          CWeight[mpcTarg],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcTarg] != 0) */

    if (CWeight[mpcRes] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';
 
      for (index = mpsRNode;index <= mpsRWallTime;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }
 
        switch(index)
          {
          case mpsRNode:     strcat(tmpHeader," Node"); break;
          case mpsRProc:     strcat(tmpHeader," Proc"); break;
          case mpsRMem:      strcat(tmpHeader,"  Mem"); break;
          case mpsRSwap:     strcat(tmpHeader," Swap"); break;
          case mpsRDisk:     strcat(tmpHeader," Disk"); break;
          case mpsRPS:       strcat(tmpHeader,"   PS"); break;  
          case mpsRPE:       strcat(tmpHeader,"   PE"); break;  
          case mpsRUProc:    strcat(tmpHeader,"UProc"); break;  
          case mpsRUJob:     strcat(tmpHeader," UJob"); break;  
          case mpsRWallTime: strcat(tmpHeader,"WTime"); break;  
          }
 
        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          SWeight[index]);
        }  /* END for (index) */
 
      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcRes],"   Res(%s)",
          tmpHeader);
 
        sprintf(CWLine[mpcRes]," %5ld(%s)",
          CWeight[mpcRes],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcRes] != 0) */

    if (CWeight[mpcUsage] != 0)
      {
      tmpHeader[0] = '\0';
      tmpCWLine[0] = '\0';
 
      for (index = mpsUCons;index <= mpsUPerC;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpHeader[0] != '\0')
          {
          strcat(tmpHeader,":");
          strcat(tmpCWLine,":");
          }
 
        switch(index)
          {
          case mpsUCons:     strcat(tmpHeader,"Cons "); break;
          case mpsURem:      strcat(tmpHeader,"Rem  "); break;
          case mpsUPerC:     strcat(tmpHeader,"PerC "); break;
          }
 
        sprintf(tmpCWLine,"%s%5ld",
          tmpCWLine,
          SWeight[index]);
        }  /* END for (index) */
 
      if (tmpHeader[0] != '\0')
        {
        sprintf(CHeader[mpcUsage],"   Res(%s)",
          tmpHeader);
 
        sprintf(CWLine[mpcUsage]," %5ld(%s)",
          CWeight[mpcUsage],
          tmpCWLine);
        }
      }    /* END if (CWeight[mpcUsage] != 0) */

    MUSNPrintF(BPtr,BSpace,"%-20s %10s%c %*s%*s%*s%*s%*s%*s%*s\n",
      "Job",
      "PRIORITY",
      '*',
      (int)strlen(CHeader[mpcCred]),
      CHeader[mpcCred],
      (int)strlen(CHeader[mpcFS]),
      CHeader[mpcFS],
      (int)strlen(CHeader[mpcAttr]),
      CHeader[mpcAttr],
      (int)strlen(CHeader[mpcServ]),
      CHeader[mpcServ],
      (int)strlen(CHeader[mpcTarg]),
      CHeader[mpcTarg],
      (int)strlen(CHeader[mpcRes]),
      CHeader[mpcRes],
      (int)strlen(CHeader[mpcUsage]),
      CHeader[mpcUsage]);

    MUSNPrintF(BPtr,BSpace,"%20s %10s%c %*s%*s%*s%*s%*s%*s%*s\n",
      "Weights",
      "--------",
      ' ',
      (int)strlen(CWLine[mpcCred]),
      CWLine[mpcCred],
      (int)strlen(CWLine[mpcFS]),
      CWLine[mpcFS],
      (int)strlen(CWLine[mpcAttr]),
      CWLine[mpcAttr],
      (int)strlen(CWLine[mpcServ]),
      CWLine[mpcServ],
      (int)strlen(CWLine[mpcTarg]),
      CWLine[mpcTarg],
      (int)strlen(CWLine[mpcRes]),
      CWLine[mpcRes],
      (int)strlen(CWLine[mpcUsage]),
      CWLine[mpcUsage]);

    MUSNPrintF(BPtr,BSpace,"\n");

    DBG(5,fUI) DPrint("INFO:     %s header created\n",
      FName);

    return(SUCCESS);
    }  /* END if (Mode == 1) */

  if (Mode == 2)
    {
    /* display priority footer */

    memset(CFooter,'\0',sizeof(CFooter));  

    for (cindex = mpcServ;cindex <= mpcUsage;cindex++)
      {
      switch (cindex)
        {
        case mpcServ:

          sindex1 = mpsSQT;
          sindex2 = mpsSBP;
 
          break;

        case mpcTarg:

          sindex1 = mpsTQT;
          sindex2 = mpsTXF;
 
          break;

        case mpcCred:

          sindex1 = mpsCU;
          sindex2 = mpsCC;
 
          break;

        case mpcFS:

          sindex1 = mpsFU;
          sindex2 = mpsFC;
 
          break;

        case mpcAttr:

          sindex1 = mpsAAttr;
          sindex2 = mpsAState;

          break;

        case mpcRes:

          sindex1 = mpsRNode;
          sindex2 = mpsRWallTime;
 
          break;

        case mpcUsage:

          sindex1 = mpsUCons;
          sindex2 = mpsUPerC;

          break;

        default:

          return(FAILURE);

          /*NOTREACHED*/

          break;
        }  /* END switch(cindex) */

      if (CWeight[cindex] != 0)
        {
        tmpLine[0] = '\0';
 
        for (sindex = sindex1;sindex <= sindex2;sindex++)
          {
          if (SWeight[sindex] == 0)
            continue;
 
          if (tmpLine[0] != '\0')
            {
            strcat(tmpLine,":");
            }

          sprintf(tmpS,"%3.1lf",
            (TotalPriority != 0.0) ?
              ABS(((double)TotalSFactor[sindex] * SWeight[sindex] * CWeight[cindex] * 100.0 / TotalPriority)) :
              0.0);

          sprintf(tmpLine,"%s%5.5s",
            tmpLine,
            tmpS);
          }  /* END for (sindex) */
 
        if (tmpLine[0] != '\0')
          {
          sprintf(tmpS,"%3.1lf",
            (TotalPriority != 0.0) ?
              ABS(((double)TotalCFactor[cindex] * CWeight[cindex] * 100.0 / TotalPriority)) :
              0.0);
 
          sprintf(CFooter[cindex]," %5.5s(%s)",
            tmpS,
            tmpLine);
          }
        }
      }    /* END for (cindex) */

    MUSNPrintF(BPtr,BSpace,"\n");

    MUSNPrintF(BPtr,BSpace,"%-20s %10s%c %*s%*s%*s%*s%*s%*s%*s\n",
      "Percent Contribution",
      "--------",
      ' ',
      (int)strlen(CFooter[mpcCred]),
      CFooter[mpcCred],
      (int)strlen(CFooter[mpcFS]),
      CFooter[mpcFS],
      (int)strlen(CFooter[mpcAttr]),
      CFooter[mpcAttr],
      (int)strlen(CFooter[mpcServ]),
      CFooter[mpcServ],
      (int)strlen(CFooter[mpcTarg]),
      CFooter[mpcTarg],
      (int)strlen(CFooter[mpcRes]),
      CFooter[mpcRes],
      (int)strlen(CFooter[mpcUsage]),
      CFooter[mpcUsage]);

    MUSNPrintF(BPtr,BSpace,"\n");

    MUSNPrintF(BPtr,BSpace,"* indicates system prio set on job\n");

    return(SUCCESS);
    }  /* END if (Mode == 2) */

  if (J->SpecWCLimit[0] == 0)
    {
    DBG(0,fSCHED) DPrint("ERROR:    job '%s' has NULL WCLimit field\n",
      J->Name);
    }

  if (J->EffQueueDuration > 0)
    {
    EffQTime = MSched.Time - J->EffQueueDuration;
    }
  else if (MPar[0].UseSystemQueueTime == TRUE)
    {
    EffQTime = J->SystemQueueTime;
    }
  else
    {
    EffQTime = J->SubmitTime;
    }

  XFactor = (double)(((unsigned long)(MSched.Time - EffQTime) + J->SpecWCLimit[0])) / 
    MAX(MinWCLimit,J->SpecWCLimit[0]);

  /* calculate cred factor */

  if ((J->Cred.U != NULL) && (J->Cred.U->F.Priority != 0))       
    SFactor[mpsCU] = J->Cred.U->F.Priority;
  else if (MSched.DefaultU != NULL)
    SFactor[mpsCU] = MSched.DefaultU->F.Priority;

  if ((J->Cred.G != NULL) && (J->Cred.G->F.Priority != 0))          
    SFactor[mpsCG] = J->Cred.G->F.Priority;
  else if (MSched.DefaultG != NULL)
    SFactor[mpsCG] = MSched.DefaultG->F.Priority;

  if ((J->Cred.A != NULL) && (J->Cred.A->F.Priority != 0))     
    SFactor[mpsCA] = J->Cred.A->F.Priority; 
  else if (MSched.DefaultA != NULL)
    SFactor[mpsCA] = MSched.DefaultA->F.Priority;         
    
  if (J->Cred.Q != NULL)
    SFactor[mpsCQ] = J->Cred.Q->F.Priority;

  if ((J->Cred.C != NULL) && (J->Cred.C->F.Priority != 0))
    SFactor[mpsCC] = J->Cred.C->F.Priority;
  else if (MClassGetPrio(J,&tmpL) == SUCCESS)
    SFactor[mpsCC] = (double)tmpL;
  else if (MSched.DefaultC != NULL)
    SFactor[mpsCC] = MSched.DefaultC->F.Priority;
 
  CFactor[mpcCred] = 0;

  for (index = mpsCU;index <= mpsCC;index++)
    {
    SFactor[index] = (SCap[index] > 0) ? 
      MIN(SFactor[index],SCap[index]) : 
      SFactor[index];

    CFactor[mpcCred] += SWeight[index] * SFactor[index];
    }  /* END for (index) */
 
  /* calculate FS factor (target utilization delta) */

  if ((GF->FSPolicy != fspNONE) && 
     ((GP->F.FSUsage[0] + GP->F.FSFactor) > 0.0))
    {
    /* user */

    if (J->Cred.U != NULL)
      {
      if (J->Cred.U->F.FSTarget > 0.0)
        {
        FSTargetUsage = J->Cred.U->F.FSTarget;
        FSMode        = J->Cred.U->F.FSMode;
        }
      else if ((MSched.DefaultU != NULL) && (MSched.DefaultU->F.FSTarget > 0.0))
        {
        FSTargetUsage = MSched.DefaultU->F.FSTarget;
        FSMode        = MSched.DefaultU->F.FSMode;
        }
      else
        {
        FSTargetUsage = 0.0;
        FSMode        = mfstNONE;
        }

      if (FSTargetUsage > 0.0)
        {
        if (MSched.PercentBasedFS == TRUE)
          {
          SFactor[mpsFU] =  1.0 - 
            (((J->Cred.U->F.FSUsage[0] + J->Cred.U->F.FSFactor) / 
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0)/FSTargetUsage);
          }
        else
          {
          SFactor[mpsFU] = FSTargetUsage - 
            (J->Cred.U->F.FSUsage[0] + J->Cred.U->F.FSFactor) / 
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }

        switch(FSMode)
          {
          case mfstCeiling: SFactor[mpsFU] = MIN(SFactor[mpsFU],0.0); break;
          case mfstFloor:   SFactor[mpsFU] = MAX(SFactor[mpsFU],0.0); break;      
          case mfstCapAbs:  SFactor[mpsFU] = 0.0; break;      
          case mfstCapRel:  SFactor[mpsFU] = 0.0; break;
          }
        }
      }

    /* group */

    if (J->Cred.G != NULL)
      { 
      if (J->Cred.G->F.FSTarget > 0.0)
        {
        FSTargetUsage = J->Cred.G->F.FSTarget;
        FSMode        = J->Cred.G->F.FSMode;
        }
      else if ((MSched.DefaultG != NULL) && (MSched.DefaultG->F.FSTarget > 0.0))
        {
        FSTargetUsage = MSched.DefaultG->F.FSTarget;
        FSMode        = MSched.DefaultG->F.FSMode;
        }
      else
        {
        FSTargetUsage = 0.0;
        FSMode        = mfstNONE;
        }
 
      if (FSTargetUsage > 0.0)
        {
        if (MSched.PercentBasedFS == TRUE)
          {
          SFactor[mpsFG] =  1.0 - 
            (((J->Cred.G->F.FSUsage[0] + J->Cred.G->F.FSFactor) / 
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0)/FSTargetUsage);
          }
        else
          {
          SFactor[mpsFG] = FSTargetUsage -
            (J->Cred.G->F.FSUsage[0] + J->Cred.G->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }
 
        switch(FSMode)
          {
          case mfstCeiling: SFactor[mpsFG] = MIN(SFactor[mpsFG],0.0); break;
          case mfstFloor:   SFactor[mpsFG] = MAX(SFactor[mpsFG],0.0); break;
          case mfstCapAbs:  SFactor[mpsFG] = 0.0; break;
          case mfstCapRel:  SFactor[mpsFG] = 0.0; break;
          }
        }
      }

    /* account */
 
    if ((J->Cred.A != NULL) && (J->Cred.A->F.FSTarget > 0.0))
      {
      FSTargetUsage = J->Cred.A->F.FSTarget;
      FSMode        = J->Cred.A->F.FSMode;
      }
    else if ((MSched.DefaultA != NULL) && (MSched.DefaultA->F.FSTarget > 0.0))
      {
      FSTargetUsage = MSched.DefaultA->F.FSTarget;
      FSMode        = MSched.DefaultA->F.FSMode;
      }
    else
      {
      FSTargetUsage = 0.0;
      FSMode        = mfstNONE;
      }
 
    if (FSTargetUsage > 0.0)
      {
      if (J->Cred.A != NULL)
        {
        if (MSched.PercentBasedFS == TRUE)
          {
          SFactor[mpsFA] = 1.0 -
            (((J->Cred.A->F.FSUsage[0] + J->Cred.A->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0)/FSTargetUsage);
          }
        else
          {
          SFactor[mpsFA] = FSTargetUsage -
            (J->Cred.A->F.FSUsage[0] + J->Cred.A->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }
        }
      else
        {
        SFactor[mpsFA] = 0;
        }
 
      switch(FSMode)
        {
        case mfstCeiling: SFactor[mpsFA] = MIN(SFactor[mpsFA],0.0); break;
        case mfstFloor:   SFactor[mpsFA] = MAX(SFactor[mpsFA],0.0); break;
        case mfstCapAbs:  SFactor[mpsFA] = 0.0; break;
        case mfstCapRel:  SFactor[mpsFA] = 0.0; break;
        }
      }

    /* class */

    if ((J->Cred.C != NULL) && (J->Cred.C->F.FSTarget > 0.0))
      {
      FSTargetUsage = J->Cred.C->F.FSTarget;
      FSMode        = J->Cred.C->F.FSMode;
      }
    else if ((MSched.DefaultC != NULL) && (MSched.DefaultC->F.FSTarget > 0.0))
      {
      FSTargetUsage = MSched.DefaultC->F.FSTarget;
      FSMode        = MSched.DefaultC->F.FSMode;
      }
    else
      {
      FSTargetUsage = 0.0;
      FSMode        = mfstNONE;
      }

    if (FSTargetUsage > 0.0)
      {
      if (J->Cred.C != NULL)
        {
        if (MSched.PercentBasedFS == TRUE)
          {
          SFactor[mpsFC] = 1.0 -
            (((J->Cred.C->F.FSUsage[0] + J->Cred.C->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0)/FSTargetUsage); 
          }
        else
          {
          SFactor[mpsFC] = FSTargetUsage -
            (J->Cred.C->F.FSUsage[0] + J->Cred.C->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }
        }
      else
        {
        SFactor[mpsFC] = 0;
        }

      switch(FSMode)
        {
        case mfstCeiling: SFactor[mpsFC] = MIN(SFactor[mpsFC],0.0); break;
        case mfstFloor:   SFactor[mpsFC] = MAX(SFactor[mpsFC],0.0); break;
        case mfstCapAbs:  SFactor[mpsFC] = 0.0; break;
        case mfstCapRel:  SFactor[mpsFC] = 0.0; break;
        }
      }

    /* QOS */

    if ((J->Cred.Q != NULL) && (J->Cred.Q->F.FSTarget > 0.0))
      {
      FSTargetUsage = J->Cred.Q->F.FSTarget;
      FSMode        = J->Cred.Q->F.FSMode;
      }
    else if ((MSched.DefaultQ != NULL) && (MSched.DefaultQ->F.FSTarget > 0.0))
      {
      FSTargetUsage = MSched.DefaultQ->F.FSTarget;
      FSMode        = MSched.DefaultQ->F.FSMode;
      }
    else
      {
      FSTargetUsage = 0.0;
      FSMode        = mfstNONE;
      }

    if (FSTargetUsage > 0.0)
      {
      if (J->Cred.Q != NULL)
        {
        if (MSched.PercentBasedFS == TRUE)
          {
          SFactor[mpsFQ] = 1.0 - 
            (((J->Cred.Q->F.FSUsage[0] + J->Cred.Q->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0)/FSTargetUsage);
          }
        else
          {
          SFactor[mpsFQ] = FSTargetUsage -
            (J->Cred.Q->F.FSUsage[0] + J->Cred.Q->F.FSFactor) /
            (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }
        }
      else
        {
        SFactor[mpsFQ] = 0;
        }

      switch(FSMode)
        {
        case mfstCeiling: SFactor[mpsFQ] = MIN(SFactor[mpsFQ],0.0); break;
        case mfstFloor:   SFactor[mpsFQ] = MAX(SFactor[mpsFQ],0.0); break;
        case mfstCapAbs:  SFactor[mpsFQ] = 0.0; break;
        case mfstCapRel:  SFactor[mpsFQ] = 0.0; break;
        }
      }

    CFactor[mpcFS] = 0;

    for (index = mpsFU;index <= mpsFC;index++)
      {
      SFactor[index] = (SCap[index] > 0) ?
        MIN(SFactor[index],SCap[index]) :
        SFactor[index];

      CFactor[mpcFS] += SWeight[index] * SFactor[index];
      }  /* END for (index) */
    }    /* END if FS[0].FSPolicy */

  /* calculate attr component */

  CFactor[mpcAttr] = 0;

  /* NYI */

  for (index = mpsAAttr;index <= mpsAState;index++)
    {
    SFactor[index] = (SCap[index] > 0) ?
      MIN(SFactor[index],SCap[index]) :
      SFactor[index];

    CFactor[mpcAttr] += SWeight[index] * SFactor[index];
    }  /* END for (index) */

  /* calculate service component */

  /* queue time factor (in minutes) */
 
  SFactor[mpsSQT] = MAX(0.0,(double)((long)MSched.Time - EffQTime) / 60.0);

  SFactor[mpsSXF] = (double)XFactor;

  SFactor[mpsSSPV] = (J->Flags & (1 << mjfSPViolation)) ? 1.0 : 0.0;

  SFactor[mpsSBP] = (double)J->Bypass;       

  CFactor[mpcServ] = 0;

  for (index = mpsSQT;index <= mpsSBP;index++)
    {
    SFactor[index] = (SCap[index] > 0) ?
      MIN(SFactor[index],SCap[index]) :
      SFactor[index];

    CFactor[mpcServ] += SWeight[index] * SFactor[index];
    }  /* END for (index) */

  /* calculate target component */

  /* target QT subcomponent */

  if (J->Cred.Q != NULL)
    { 
    if (J->Cred.Q->QTTarget > 0)
      {
      /* give exponentially increasing priority as we approach target qtime */
      /* Equation:  (QTTarget - QTCurrent)^(-2)                             */
 
      SFactor[mpsTQT] = (double)pow((MAX(.0001,J->Cred.Q->QTTarget - EffQTime)),-2.0);
      }

    /* target XF subcomponent */

    if (J->Cred.Q->XFTarget > 0.0)
      {
      /* give exponentially increasing priority as we approach target xfactor */
      /* Equation:  (XFTarget - XFCurrent)^(-2)                               */

      SFactor[mpsTXF] = (double)pow((MAX(.0001,J->Cred.Q->XFTarget - XFactor)),-2.0);
      }
    }

  SFactor[mpsTQT] = (SCap[mpsTQT] > 0) ? MIN(SFactor[mpsTQT],SCap[mpsTQT]) : SFactor[mpsTQT];
  SFactor[mpsTXF] = (SCap[mpsTXF] > 0) ? MIN(SFactor[mpsTXF],SCap[mpsTXF]) : SFactor[mpsTXF];

  CFactor[mpcTarg] =
    SWeight[mpsTQT] * SFactor[mpsTQT] +
    SWeight[mpsTXF] * SFactor[mpsTXF];

  /* determine resource factor */

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    SFactor[mpsRNode]    += RQ->NodeCount;
    SFactor[mpsRProc]    += RQ->TaskCount * RQ->DRes.Procs; 
    SFactor[mpsRMem ]    += RQ->TaskCount * RQ->DRes.Mem ;    
    SFactor[mpsRSwap]    += RQ->TaskCount * RQ->DRes.Swap;    
    SFactor[mpsRDisk]    += RQ->TaskCount * RQ->DRes.Disk;  
    SFactor[mpsRPS  ]    += RQ->TaskCount * RQ->DRes.Procs * J->WCLimit; 
    }  /* END for (rqindex) */

  if ((J->Cred.U != NULL) && (&J->Cred.U->L.AP != NULL))
    SFactor[mpsRUProc] = J->Cred.U->L.AP.Usage[mptMaxProc][0];

  if ((J->Cred.U != NULL) && (&J->Cred.U->L.AP != NULL))
    SFactor[mpsRUJob] = J->Cred.U->L.AP.Usage[mptMaxJob][0];

  SFactor[mpsRWallTime] = J->WCLimit; 
          
  MJobGetPE(J,GP,&SFactor[mpsRPE]);
 
  sindex1 = mpsRNode;
  sindex2 = mpsRWallTime;

  CFactor[mpcRes] = 0;

  for (sindex = sindex1;sindex <= sindex2;sindex++)
    { 
    SFactor[sindex] = (SCap[sindex] > 0) ? 
      MIN(SFactor[sindex],SCap[sindex]) : 
      SFactor[sindex];

    CFactor[mpcRes] += SWeight[sindex] * SFactor[sindex];
    }  /* END for (sindex) */

  /* calculate usage factor */

/*
  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    MJobGetRunPriority(J,0,&UsageFactor,NULL);
    }
  else
    {
    UsageFactor = 0.0;
    }
*/

  Prio = 0.0;
  APrio = 0.0;

  for (index = mpcServ;index <= mpcUsage;index++)
    {
    CFactor[index] = (CCap[index] > 0) ? 
      MIN(CFactor[index],CCap[index]) : 
      CFactor[index];

    Prio  += (double)CWeight[index] * CFactor[index];
    APrio += ABS((double)CWeight[index] * CFactor[index]);
    }  /* END for (index) */

  if ((BPtr != NULL) && (*BPtr != NULL))
    {
    TotalPriority += APrio;

    for (index = 1;index < MAX_MPRIOCOMPONENT;index++)
      {
      if (APrio != 0.0)
        CP[index] = ABS((double)CWeight[index] * CFactor[index]) / APrio * 100.0;
      else
        CP[index] = 0.0;

      TotalCFactor[index] += ABS(CFactor[index]);
      }  /* END for (index) */

    memset(CLine,0,sizeof(CLine));
 
    for (index = 1;index < MAX_MPRIOSUBCOMPONENT;index++)
      {
      SP[index] = (APrio != 0.0) ? 
        ABS((double)SWeight[index] * SFactor[index]) / APrio * 100.0:
        0.0;        

      if (index >= mpsUCons)
        SP[index] *= CWeight[mpcUsage];
      else if (index >= mpsRNode)
        SP[index] *= CWeight[mpcRes];     
      else if (index >= mpsFU)
        SP[index] *= CWeight[mpcFS];
      else if (index >= mpsCU)
        SP[index] *= CWeight[mpcCred];
      else if (index >= mpsTQT)
        SP[index] *= CWeight[mpcTarg];
      else if (index >= mpsSQT)
        SP[index] *= CWeight[mpcServ];

      TotalSFactor[index] += ABS(SFactor[index]);
      }  /* END for (index) */

    if (CWeight[mpcCred] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsCU;index <= mpsCC;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }

        sprintf(tmpS,"%3.1lf",ABS(SFactor[index]));
         
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS); 
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcCred]);

        sprintf(CLine[mpcCred]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcCred] != 0) */

    if (CWeight[mpcFS] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsFU;index <= mpsFC;index++)
        {
        if (SWeight[index] == 0)
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }
 
        tmpD = (double)CWeight[mpcFS] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf",
          tmpD);
 
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS); 
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcFS]);
 
        sprintf(CLine[mpcFS]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcFS] != 0) */

    if (CWeight[mpcAttr] != 0)
      {
      tmpLine[0] = '\0';

      for (index = mpsAAttr;index <= mpsAState;index++)
        {
        if (SWeight[index] == 0)
          continue;

        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }

        tmpD = (double)CWeight[mpcAttr] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf",
          tmpD);

        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS);
        }  /* END for (index) */

      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcAttr]);

        sprintf(CLine[mpcAttr]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcCred] != 0) */

    if (CWeight[mpcServ] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsSQT;index <= mpsSBP;index++)
        {
        if ((SWeight[index] == 0) && (SDisplay[index] == FALSE))
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }
 
        tmpD = (double)CWeight[mpcServ] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf",
          tmpD);
 
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS);
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcServ]);
 
        sprintf(CLine[mpcServ]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcServ] != 0) */

    if (CWeight[mpcTarg] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsTQT;index <= mpsTXF;index++)
        {
        if ((SWeight[index] == 0) && (SDisplay[index] == FALSE))
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }
 
        tmpD = (double)CWeight[mpcTarg] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf",
          CP[mpcTarg]);
 
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS);
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcTarg]);
 
        sprintf(CLine[mpcTarg]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcTarg] != 0) */

    if (CWeight[mpcRes] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsRNode;index <= mpsRWallTime;index++)
        {
        if ((SWeight[index] == 0) && (SDisplay[index] == FALSE))
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }
 
        tmpD = (double)CWeight[mpcRes] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf",
          tmpD);
 
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS);
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcRes]);
 
        sprintf(CLine[mpcRes]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcRes] != 0) */

    if (CWeight[mpcUsage] != 0)
      {
      tmpLine[0] = '\0';
 
      for (index = mpsUCons;index <= mpsUPerC;index++)
        {
        if ((SWeight[index] == 0) && (SDisplay[index] == FALSE))
          continue;
 
        if (tmpLine[0] != '\0')
          {
          strcat(tmpLine,":");
          }
 
        tmpD = (double)CWeight[mpcUsage] * SWeight[index] * SFactor[index];

        sprintf(tmpS,"%3.1lf", 
          tmpD);
 
        sprintf(tmpLine,"%s%5.5s",
          tmpLine,
          tmpS);
        }  /* END for (index) */
 
      if (tmpLine[0] != '\0')
        {
        sprintf(tmpS,"%3.1lf",
          CP[mpcUsage]);
 
        sprintf(CLine[mpcUsage]," %5.5s(%s)",
          tmpS,
          tmpLine);
        }
      }    /* END if (CWeight[mpcUsage] != 0) */

    MUSNPrintF(BPtr,BSpace,"%-20s %10.0lf%c %*s%*s%*s%*s%*s%*s%*s\n",
      J->Name,
      Prio,
      (J->SystemPrio > 0) ? '*' : ' ',
      (int)strlen(CLine[mpcCred]),
      CLine[mpcCred],
      (int)strlen(CLine[mpcFS]),
      CLine[mpcFS],
      (int)strlen(CLine[mpcAttr]),
      CLine[mpcAttr],
      (int)strlen(CLine[mpcServ]),
      CLine[mpcServ],
      (int)strlen(CLine[mpcTarg]),
      CLine[mpcTarg],
      (int)strlen(CLine[mpcRes]),
      CLine[mpcRes],
      (int)strlen(CLine[mpcUsage]),
      CLine[mpcUsage]);
    } /* END if (BPtr != NULL) */

  /* clip prio at min value */

  if ((GP->RejectNegPrioJobs == FALSE) && (GP->EnableNegJobPriority == FALSE))
    {
    /* establish min start prio value of '1.0' */

    if (Prio < 1.0)
      Prio = 1.0;
    }

  DBG(3,fSCHED) DPrint("INFO:     job '%s' Priority: %8.0lf\n",
    J->Name,
    Prio);

  DBG(3,fSCHED) DPrint("INFO:     Cred: %6.0lf(%04.1lf)  FS: %6.0lf(%04.1lf)  Attr: %6.0lf(%04.1lf)  Serv: %6.0lf(%04.1lf)  Targ: %6.0lf(%04.1lf)  Res: %6.0lf(%04.1lf)  Us: %6.0lf(%04.1lf)\n",
    (double)CWeight[mpcCred] * CFactor[mpcCred],
    CP[mpcCred],
    (double)CWeight[mpcFS] * CFactor[mpcFS],
    CP[mpcFS],
    (double)CWeight[mpcAttr] * CFactor[mpcAttr],
    CP[mpcAttr],
    (double)CWeight[mpcServ] * CFactor[mpcServ],
    CP[mpcServ],
    (double)CWeight[mpcTarg] * CFactor[mpcTarg],
    CP[mpcTarg],
    (double)CWeight[mpcRes] * CFactor[mpcRes],
    CP[mpcRes],
    (double)CWeight[mpcUsage] * CFactor[mpcUsage],
    CP[mpcUsage]);

  /* cap job start priority */

  if (Prio > (double)MAX_PRIO_VAL)
    Prio = (double)MAX_PRIO_VAL;

  /* incorporate system priority value */

  if (J->SystemPrio > 0)
    {
    if (J->SystemPrio > MAX_PRIO_VAL)
      Prio += (double)(J->SystemPrio - (MAX_PRIO_VAL << 1));
    else
      Prio = (double)(MAX_PRIO_VAL + J->SystemPrio);
    }

  DBG(5,fSCHED) DPrint("INFO:     job '%s'  priority: %8.2lf\n",
    J->Name,
    Prio);

  if (Priority != NULL)
    *Priority = Prio;

  return(SUCCESS);
  }  /* END MJobGetStartPriority() */




/* order high to low */
 
int MJobStartPrioComp(
 
  int *A,  /* I */
  int *B)  /* I */
 
  {
  static int tmp;
 
  tmp = MJob[*B]->StartPriority - MJob[*A]->StartPriority;
 
  return(tmp);
  }  /* END MJobStartPrioComp() */





int MPrioConfigShow(

  int   Mode,    /* I */
  int   PIndex,  /* I (partition index) */
  char *Buffer)  /* O */

  {
  int     index;
  mfsc_t *F;

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  F = &MPar[PIndex].FSC;   

  strcat(Buffer,"\n# Priority Weights\n\n");
 
  for (index = 1;index < MAX_MPRIOCOMPONENT;index++)
    {
    if (F->PCW[index] != -1)
      strcat(Buffer,MUShowLArray(MParam[pServWeight + index - 1],PIndex,F->PCW[index]));
    }  /* END for (index) */
 
  for (index = 1;index < MAX_MPRIOSUBCOMPONENT;index++)
    {
    if (F->PSW[index] != -1)
      strcat(Buffer,MUShowLArray(MParam[pSQTWeight + index - 1],PIndex,F->PSW[index]));
    }  /* END for (index) */
 
  if (F->XFMinWCLimit != -1)
    strcat(Buffer,MUShowSArray(MParam[pXFMinWCLimit],PIndex,MULToTString(F->XFMinWCLimit)));
 
  for (index = 1;index < MAX_MPRIOCOMPONENT;index++)
    {
    if (F->PCC[index] > 0)
      strcat(Buffer,MUShowLArray(MParam[pServCap + index - 1],PIndex,F->PCC[index]));
    }  /* END for (index) */
 
  for (index = 1;index < MAX_MPRIOSUBCOMPONENT;index++)
    {
    if (F->PSC[index] > 0)
      strcat(Buffer,MUShowLArray(MParam[pSQTCap + index - 1],PIndex,F->PSC[index]));
    }  /* END for (index) */
 
  strcat(Buffer,"\n");

  return(SUCCESS);
  }  /* END MPrioConfigShow() */


/* END MPriority.c */


